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

'''Common test suite code.'''


import os.path
import optparse
import sys
import subprocess
import time
import re
import signal


ansi_re = re.compile('\x1b\[[0-9]{1,2}(;[0-9]{1,2}){0,2}m')


def ansi_strip(s):
    # http://www.theeggeadventure.com/wikimedia/index.php/Linux_Tips#Use_sed_to_remove_ANSI_colors
    return ansi_re.sub('', s)


def popen(command, *args, **kwargs):
    if 'cwd' in kwargs:
        sys.stdout.write('cd %s && ' % kwargs['cwd'])
    if 'env' in kwargs:
        for name, value in kwargs['env'].iteritems():
            if value != os.environ.get(name, None):
                sys.stdout.write('%s=%s ' % (name, value))
    sys.stdout.write(' '.join(command) + '\n')
    sys.stdout.flush()
    return subprocess.Popen(command, *args, **kwargs)


ignored_function_names = set([
    'glGetString',
    'glXGetCurrentDisplay',
    'glXGetClientString',
    'glXGetProcAddress',
    'glXGetProcAddressARB',
    'glXQueryVersion',
    'glXGetVisualFromFBConfig',
    'glXChooseFBConfig',
    'glXCreateNewContext',
    'glXMakeContextCurrent',
    'glXQueryExtension',
    'glXIsDirect',
])


class Report:

    def __init__(self, basedir):
        self.basedir = basedir
        if not os.path.exists(basedir):
            os.makedirs(basedir)
        self.html = open(os.path.join(basedir, 'index.html'), 'wt')
        self._header()

    def _header(self):
        self.html.write('<html>\n')
        self.html.write('  <body>\n')
        self.html.write('    <table border="1">\n')
        self.html.write('      <tr><th>Ref</th><th>Src</th><th>&Delta;</th></tr>\n')

    def _image_tag(self, image):
        url = os.path.relpath(image, self.basedir)
        self.html.write('        <td><a href="%s"><img src="%s"/></a></td>\n' % (url, url))

    def add_snapshot(self, ref_image, src_image, diff_image):
        self.html.write('      <tr>\n')
        self._image_tag(ref_image)
        self._image_tag(src_image)
        self._image_tag(diff_image)
        self.html.write('      </tr>\n')
        self.html.flush()

    def _footer(self):
        self.html.write('    </table>\n')
        self.html.write('  </body>\n')
        self.html.write('</html>\n')

    def __del__(self):
        self._footer()
        self.html.close()


class TestCase:

    def __init__(self, name, args, cwd=None, build = '.', results = '.'):
        self.name = name
        self.args = args
        self.cwd = cwd
        self.build = build
        self.results = results

        if not os.path.exists(results):
            os.makedirs(results)

    def run(self, report):

        trace = os.path.abspath(os.path.join(self.results, self.name + '.trace'))

        ld_preload = os.path.abspath(os.path.join(self.build, 'glxtrace.so'))
        if not os.path.exists(ld_preload):
            sys.stderr.write('error: could not find %s\n' % ld_preload)
            sys.exit(1)

        env = os.environ.copy()
        env['LD_PRELOAD'] = ld_preload
        env['TRACE_FILE'] = trace

        window_name = self.args[0]

        p = popen(self.args, cwd=self.cwd)
        for i in range(3):
            time.sleep(1)
            if p.poll() is not None:
                break
            if subprocess.call(['xwininfo', '-name', window_name], stdout=subprocess.PIPE, stderr=subprocess.PIPE) == 0:
                break
        if p.returncode is None:
            p.terminate()
        elif p.returncode:
            sys.stdout.write('SKIP (app)\n')
            return

        ref_image = os.path.join(self.results, self.name + '.ref.png')
        p = popen(self.args, env=env, cwd=self.cwd)
        try:
            for i in range(5):
                time.sleep(1)
                if p.poll() is not None:
                    break
                if subprocess.call(['xwininfo', '-name', window_name], stdout=subprocess.PIPE, stderr=subprocess.PIPE) == 0:
                    break

            if p.returncode is None:
                subprocess.call("xwd -name '%s' | xwdtopnm | pnmtopng > '%s'" % (window_name, ref_image), shell=True)
        finally:
            if p.returncode is None:
                p.terminate()
                p.wait()
            elif p.returncode:
                print p.returncode
                sys.stdout.write('FAIL (trace)\n')
                return

        p = popen([os.path.join(self.build, 'tracedump'), trace], stdout=subprocess.PIPE)
        call_re = re.compile('^([0-9]+) (\w+)\(')
        swapbuffers = 0
        flushes = 0
        for orig_line in p.stdout:
            orig_line = orig_line.rstrip()
            line = ansi_strip(orig_line)
            mo = call_re.match(line)
            if mo:
                call_no = int(mo.group(1))
                function_name = mo.group(2)
                if function_name in ignored_function_names:
                    continue
                if function_name == 'glXSwapBuffers':
                    swapbuffers += 1
                if function_name in ('glFlush', 'glFinish'):
                    flushes += 1
            #print orig_line
        p.wait()
        if p.returncode != 0:
            sys.stdout.write('FAIL (tracedump)\n')
            return

        args = [os.path.join(self.build, 'glretrace')]
        if swapbuffers:
            args += ['-db']
            frames = swapbuffers
        else:
            args += ['-sb']
            frames = flushes
        if os.path.exists(ref_image) and frames < 10:
            snapshot_prefix = os.path.join(self.results, self.name + '.')
            args += ['-s', snapshot_prefix]
        else:
            snapshot_prefix = None
        args += [trace]
        p = popen(args, stdout=subprocess.PIPE)
        image_re = re.compile('^Wrote (.*\.png)$')
        image = None
        for line in p.stdout:
            line = line.rstrip()
            mo = image_re.match(line)
            if mo:
                image = mo.group(1)
        p.wait()
        if p.returncode != 0:
            sys.stdout.write('FAIL (glretrace)\n')
            return

        if image:
            delta_image = os.path.join(self.results, self.name + '.diff.png')
            p = popen([
                'compare',
                '-alpha', 'opaque',
                '-metric', 'AE',
                '-fuzz', '5%',
                '-dissimilarity-threshold', '1',
                ref_image, image, delta_image
            ], stderr = subprocess.PIPE)
            _, stderr = p.communicate()

            try:
                ae = int(stderr)
            except ValueError:
                ae = 9999

            if ae:
                report.add_snapshot(ref_image, image, delta_image)
                sys.stdout.write('FAIL (snapshot)\n')
                return

        sys.stdout.write('PASS\n')


def main():
    # Parse command line options
    optparser = optparse.OptionParser(
        usage='\n\t%prog [options] -- program [args] ...',
        version='%%prog')
    optparser.add_option(
        '--build', metavar='PATH',
        type='string', dest='build', default='.',
        help='path to apitrace build [default=%default]')
    optparser.add_option(
        '--cwd', metavar='PATH',
        type='string', dest='cwd', default='.',
        help='change directory [default=%default]')
    optparser.add_option(
        '--results', metavar='PATH',
        type='string', dest='results', default='results',
        help='results directory [default=%default]')

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

    report = Report(options.results)
    test.run(report)


if __name__ == '__main__':
    main()
