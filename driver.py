#!/usr/bin/env python
##########################################################################
#
# Copyright 2011 Jose Fonseca
# All Rights Reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
##########################################################################/

'''Main test driver.'''


import optparse
import os.path
import platform
import re
import subprocess
import sys
import time


def popen(command, *args, **kwargs):
    if kwargs.get('cwd', None) is not None:
        sys.stdout.write('cd %s && ' % kwargs['cwd'])
    if 'env' in kwargs:
        for name, value in kwargs['env'].iteritems():
            if value != os.environ.get(name, None):
                sys.stdout.write('%s=%s ' % (name, value))
    sys.stdout.write(' '.join(command) + '\n')
    sys.stdout.flush()
    return subprocess.Popen(command, *args, **kwargs)


def _get_build_path(path):
    if options.build is not None:
        path = os.path.abspath(os.path.join(options.build, path))
    if not os.path.exists(path):
        sys.stderr.write('error: %s does not exist\n' % path)
        sys.exit(1)
    return path


class TestCase:

    max_frames = None

    trace_file = None

    def __init__(self, name, args, cwd=None, build=None, results = '.'):
        self.name = name
        self.args = args
        self.cwd = cwd
        self.build = build
        self.results = results

        if not os.path.exists(results):
            os.makedirs(results)

    expected_dump = None

    def standalone(self):
        p = popen(self.args, cwd=self.cwd)
        p.wait()
        if p.returncode:
            self.skip('application returned code %i' % p.returncode)

    def trace(self):
        if self.trace_file is None:
            self.trace_file = os.path.abspath(os.path.join(self.results, self.name + '.trace'))
        if os.path.exists(self.trace_file):
            os.remove(self.trace_file)
        else:
            trace_dir = os.path.dirname(self.trace_file)
            if not os.path.exists(trace_dir):
                os.makedirs(trace_dir)

        env = os.environ.copy()
        
        system = platform.system()
        if system == 'Windows':
            # TODO
            self.skip('tracing not supported on Windows')
            wrapper = _get_build_path('wrappers/opengl32.dll')
        elif system == 'Darwin':
            wrapper = _get_build_path('wrappers/OpenGL')
            env['DYLD_LIBRARY_PATH'] = os.path.dirname(wrapper)
        else:
            wrapper = _get_build_path('glxtrace.so')
            env['LD_PRELOAD'] = wrapper

        env['TRACE_FILE'] = self.trace_file
        if self.max_frames is not None:
            env['TRACE_FRAMES'] = str(self.max_frames)

        p = popen(self.args, env=env, cwd=self.cwd)
        p.wait()

        if not os.path.exists(self.trace_file):
            self.fail('no trace file generated\n')
    
    call_re = re.compile(r'^([0-9]+) (\w+)\(')

    def dump(self):

        cmd = [_get_build_path('tracedump'), '--color=never', self.trace_file]
        p = popen(cmd, stdout=subprocess.PIPE)

        swapbuffers = 0
        flushes = 0

        ref_line = ''
        if self.ref_dump is not None:
            ref = open(self.ref_dump, 'rt')
            ref_line = ref.readline().rstrip()
        for line in p.stdout:
            line = line.rstrip()
            mo = self.call_re.match(line)
            assert mo
            if mo:
                call_no = int(mo.group(1))
                function_name = mo.group(2)
                if function_name == 'glXSwapBuffers':
                    swapbuffers += 1
                if function_name in ('glFlush', 'glFinish'):
                    flushes += 1
                src_line = line[mo.start(2):]
                sys.stdout.write(src_line + '\n')
                if ref_line:
                    if src_line == ref_line:
                        ref_line = ref.readline().rstrip()
        p.wait()
        if p.returncode != 0:
            self.fail('tracedump returned code %i' % p.returncode)
        if ref_line:
            self.fail('missing call %' % ref_line)

    def run(self):
        self.standalone()
        self.trace()
        self.dump()

        self.pass_()
        return

        ref_prefix = os.path.abspath(os.path.join(self.results, self.name + '.ref.'))
        src_prefix = os.path.join(self.results, self.name + '.src.')
        diff_prefix = os.path.join(self.results, self.name + '.diff.')


        if not os.path.isfile(trace):
            sys.stdout.write('SKIP (no trace)\n')
            return
        args = [_get_build_path('glretrace')]
        if swapbuffers:
            args += ['-db']
            frames = swapbuffers
        else:
            args += ['-sb']
            frames = flushes
        args += ['-s', src_prefix]
        args += [trace]
        p = popen(args, stdout=subprocess.PIPE)
        image_re = re.compile(r'^Wrote (.*\.png)$')
        images = []
        for line in p.stdout:
            line = line.rstrip()
            mo = image_re.match(line)
            if mo:
                image = mo.group(1)
                if image.startswith(src_prefix):
                    image = image[len(src_prefix):]
                    images.append(image)
        p.wait()
        if p.returncode != 0:
            sys.stdout.write('FAIL (glretrace)\n')
            return

        for image in images:
            ref_image = ref_prefix + image
            src_image = src_prefix + image
            diff_image = diff_prefix + image
            
            if not os.path.isfile(ref_image):
                continue
            assert os.path.isfile(src_image)

            comparer = Comparer(ref_image, src_image)
            match = comparer.ae()
            sys.stdout.write('%s: %s bits\n' % (image, comparer.precision()))
            if not match:
                comparer.write_diff(diff_image)
                #report.add_snapshot(ref_image, src_image, diff_image)
                sys.stdout.write('FAIL (snapshot)\n')
                return

    def fail(self, reason=None):
        self._exit('FAIL', 1, reason)

    def skip(self, reason=None):
        self._exit('SKIP', 0, reason)

    def pass_(self, reason=None):
        self._exit('PASS', 0, reason)

    def _exit(self, status, code, reason=None):
        if reason is None:
            reason = ''
        else:
            reason = ' (%s)' % reason
        sys.stdout.write('%s%s\n' % (status, reason))
        sys.exit(code)



def main():
    global options

    # Parse command line options
    optparser = optparse.OptionParser(
        usage='\n\t%prog [options] -- program [args] ...',
        version='%%prog')
    optparser.add_option(
        '-B', '--build', metavar='PATH',
        type='string', dest='build', default='..',
        help='path to apitrace build')
    optparser.add_option(
        '-C', '--directory', metavar='PATH',
        type='string', dest='cwd', default=None,
        help='change to directory')
    optparser.add_option(
        '-R', '--results', metavar='PATH',
        type='string', dest='results', default='.',
        help='results directory [default=%default]')
    optparser.add_option(
        '--ref-dump', metavar='PATH',
        type='string', dest='ref_dump', default=None,
        help='reference dump')

    (options, args) = optparser.parse_args(sys.argv[1:])
    if not args:
        optparser.error('program must be specified')

    test = TestCase(
        name = os.path.basename(args[0]), 
        args = args,
        cwd = options.cwd,
        build = options.build,
        results = options.results,
    )
    test.ref_dump = options.ref_dump

    test.run()


if __name__ == '__main__':
    main()
