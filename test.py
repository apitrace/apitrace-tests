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


import math
import optparse
import os.path
import re
import signal
import subprocess
import sys
import time

import Image
import ImageChops
import ImageEnhance


class Comparer:
    '''Image comparer.'''

    def __init__(self, ref_image, src_image, alpha = False):
        self.ref_im = Image.open(ref_image)
        self.src_im = Image.open(src_image)

        # Crop to the minimum size
        ref_w, ref_h = self.ref_im.size
        src_w, src_h = self.src_im.size
        w = min(ref_w, src_w)
        h = min(ref_h, src_h)
        self.ref_im = self.ref_im.crop((0, ref_h - h, w, ref_h))
        self.src_im = self.src_im.crop((0, src_h - h, w, src_h))

        # Ignore alpha
        if not alpha:
            self.ref_im = self.ref_im.convert('RGB')
            self.src_im = self.src_im.convert('RGB')

        self.diff = ImageChops.difference(self.src_im, self.ref_im)

    def write_diff(self, diff_image, fuzz = 0.05):
        # make a difference image similar to ImageMagick's compare utility
        mask = ImageEnhance.Brightness(self.diff).enhance(1.0/fuzz)
        mask = mask.convert('L')

        lowlight = Image.new('RGB', self.src_im.size, (0xff, 0xff, 0xff))
        highlight = Image.new('RGB', self.src_im.size, (0xf1, 0x00, 0x1e))
        diff_im = Image.composite(highlight, lowlight, mask)

        diff_im = Image.blend(self.src_im, diff_im, 0xcc/255.0)
        diff_im.save(diff_image)

    def precision(self):
        # See also http://effbot.org/zone/pil-comparing-images.htm
        h = self.diff.histogram()
        square_error = 0
        for i in range(1, 256):
            square_error += sum(h[i : 3*256: 256])*i*i
        rel_error = float(square_error*2 + 1) / float(self.diff.size[0]*self.diff.size[1]*3*255*255*2)
        bits = -math.log(rel_error)/math.log(2.0)
        return bits

    def ae(self, chantol = 4, pixeltol = 0.03):
        # Compute absolute error
        # chantol = color channel tolerance
        # pixeltol = ratio of pixels we allow to go completely off
        
        # TODO: this is approximate due to the grayscale conversion
        h = self.diff.convert('L').histogram()
        
        ae = sum(h[int(chantol) + 1 : 256])
        
        return ae <= pixeltol*self.diff.size[0]*self.diff.size[1]


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

    standalone = False

    max_frames = 1

    def __init__(self, name, args, cwd=None, build=None, results = '.'):
        self.name = name
        self.args = args
        self.cwd = cwd
        self.build = build
        self.results = results

        if not os.path.exists(results):
            os.makedirs(results)


    def _get_build_path(self, path):
        if self.build is not None:
            path = os.path.abspath(os.path.join(self.build, path))
            if not os.path.exists(path):
                sys.stderr.write('error: %s does not exist\n' % path)
                sys.exit(1)
        return path

    def run(self, report):

        trace = os.path.abspath(os.path.join(self.results, self.name + '.trace'))
        ref_prefix = os.path.abspath(os.path.join(self.results, self.name + '.ref.'))
        src_prefix = os.path.join(self.results, self.name + '.src.')
        diff_prefix = os.path.join(self.results, self.name + '.diff.')

        ld_preload = self._get_build_path('glxtrace.so')

        env = os.environ.copy()
        env['LD_PRELOAD'] = ld_preload
        env['TRACE_FILE'] = trace
        env['TRACE_SNAPSHOT'] = ref_prefix
        env['TRACE_FRAMES'] = str(self.max_frames)

        if self.standalone:
            p = popen(self.args, cwd=self.cwd)
            for i in range(3):
                time.sleep(1)
                if p.poll() is not None:
                    break
            if p.returncode is None:
                p.terminate()
            elif p.returncode:
                sys.stdout.write('SKIP (app)\n')
                return

        p = popen(self.args, env=env, cwd=self.cwd)
        try:
            for i in range(5):
                time.sleep(1)
                if p.poll() is not None:
                    break
        finally:
            if p.returncode is None:
                p.terminate()
                p.wait()
            elif p.returncode:
                if self.standalone:
                    sys.stdout.write('FAIL (trace)\n')
                else:
                    sys.stdout.write('SKIP (app)\n')
                return

        if not os.path.isfile(trace):
            sys.stdout.write('SKIP (no trace)\n')
            return
 
        p = popen([self._get_build_path('apitrace'), 'dump', '--color', trace], stdout=subprocess.PIPE)
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
        p.wait()
        if p.returncode != 0:
            sys.stdout.write('FAIL (apitrace dump)\n')
            return

        args = [self._get_build_path('glretrace')]
        if swapbuffers:
            args += ['-db']
            frames = swapbuffers
        else:
            args += ['-sb']
            frames = flushes
        args += ['-s', src_prefix]
        args += [trace]
        p = popen(args, stdout=subprocess.PIPE)
        image_re = re.compile('^Wrote (.*\.png)$')
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
                report.add_snapshot(ref_image, src_image, diff_image)
                sys.stdout.write('FAIL (snapshot)\n')
                return

        sys.stdout.write('PASS\n')


def main():
    # Parse command line options
    optparser = optparse.OptionParser(
        usage='\n\t%prog [options] -- program [args] ...',
        version='%%prog')
    optparser.add_option(
        '-B', '--build', metavar='PATH',
        type='string', dest='build', default=None,
        help='path to apitrace build')
    optparser.add_option(
        '-C', '--directory', metavar='PATH',
        type='string', dest='cwd', default=None,
        help='change to directory')
    optparser.add_option(
        '-R', '--results', metavar='PATH',
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
