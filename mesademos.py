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


'''Apitrace test suite based on Mesa demos.'''



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


def runtest(demo):

    app = os.path.join(options.mesa_demos, 'src', demo)

    dirname, basename = os.path.split(app)
    
    trace = os.path.abspath(demo.replace('/', '-') + '.trace')

    env = os.environ.copy()
    env['LD_PRELOAD'] = os.path.abspath('glxtrace.so')
    env['TRACE_FILE'] = trace
    
    args = [os.path.join('.', basename)]
    p = popen(args, env=env, cwd=dirname, stdout=subprocess.PIPE)
    time.sleep(1)

    # http://stackoverflow.com/questions/151407/how-to-get-an-x11-window-from-a-process-id
    ref_image = demo.replace('/', '-') + '.ref.png'
    subprocess.call('xwd -name \'%s\' | xwdtopnm | pnmtopng > %s' % (args[0], ref_image), shell=True, stdout=subprocess.PIPE)

    os.kill(p.pid, signal.SIGTERM)

    p = popen(['./tracedump', trace], stdout=subprocess.PIPE)
    stdout, _ = p.communicate()

    call_re = re.compile('^([0-9]+) (\w+)\(')
    double_buffer = False
    for orig_line in stdout.split('\n'):
        line = ansi_strip(orig_line)
        mo = call_re.match(line)
        if mo:
            call_no = int(mo.group(1))
            function_name = mo.group(2)
            if function_name in ignored_function_names:
                continue
            if function_name == 'glXSwapBuffers':
                double_buffer = True
        #print orig_line

    args = ['./glretrace']
    if double_buffer:
        args += ['-db']
    args += ['-s', '/tmp/' + demo.replace('/', '-') + '.']
    args += [trace]
    p = popen(args, stdout=subprocess.PIPE)
    stdout, _ = p.communicate()
    image_re = re.compile('^Wrote (.*\.png)$')
    image = None
    for line in stdout.split('\n'):
        mo = image_re.match(line)
        if mo:
            image = mo.group(1)
    
    if image:
        delta_image = demo.replace('/', '-') + '.diff.png'
        p = popen(["compare", '-alpha', 'opaque', '-metric', 'AE', '-fuzz', '5%', ref_image, image, delta_image])
        _, stderr = p.communicate()


tests = [
    'trivial/clear-color',
    'trivial/clear-fbo',
    'trivial/clear-fbo-scissor',
    'trivial/clear-fbo-tex',
    'trivial/clear-random',
    'trivial/clear-repeat',
    'trivial/clear-scissor',
    'trivial/clear-undefined',
    'trivial/createwin',
    'trivial/dlist-begin-call-end',
    'trivial/dlist-dangling',
    'trivial/dlist-degenerate',
    'trivial/dlist-edgeflag',
    'trivial/dlist-edgeflag-dangling',
    'trivial/dlist-flat-tri',
    'trivial/dlist-mat-tri',
    'trivial/dlist-recursive-call',
    'trivial/dlist-tri-flat-tri',
    'trivial/dlist-tri-mat-tri',
    'trivial/draw2arrays',
    'trivial/drawarrays',
    'trivial/drawelements',
    'trivial/drawelements-large',
    'trivial/drawrange',
    'trivial/flat-clip',
    'trivial/fs-tri',
    'trivial/line',
    'trivial/line-clip',
    'trivial/line-cull',
    'trivial/line-flat',
    'trivial/line-smooth',
    'trivial/line-stipple-wide',
    'trivial/line-userclip',
    'trivial/line-userclip-clip',
    'trivial/line-userclip-nop',
    'trivial/line-userclip-nop-clip',
    'trivial/line-wide',
    'trivial/line-xor',
    'trivial/lineloop',
    'trivial/lineloop-clip',
    'trivial/lineloop-elts',
    'trivial/linestrip',
    'trivial/linestrip-clip',
    'trivial/linestrip-flat-stipple',
    'trivial/linestrip-stipple',
    'trivial/linestrip-stipple-wide',
    'trivial/long-fixed-func',
    'trivial/pgon-mode',
    'trivial/point',
    'trivial/point-clip',
    'trivial/point-param',
    'trivial/point-sprite',
    'trivial/point-wide',
    'trivial/point-wide-smooth',
    'trivial/poly',
    'trivial/poly-flat',
    'trivial/poly-flat-clip',
    'trivial/poly-flat-unfilled-clip',
    'trivial/poly-unfilled',
    'trivial/quad',
    'trivial/quad-clip',
    'trivial/quad-clip-all-vertices',
    'trivial/quad-clip-nearplane',
    'trivial/quad-degenerate',
    'trivial/quad-flat',
    'trivial/quad-offset-factor',
    'trivial/quad-offset-unfilled',
    'trivial/quad-offset-units',
    'trivial/quad-tex-2d',
    'trivial/quad-tex-3d',
    'trivial/quad-tex-alpha',
    'trivial/quad-tex-pbo',
    'trivial/quad-tex-sub',
    'trivial/quad-unfilled',
    'trivial/quad-unfilled-clip',
    'trivial/quad-unfilled-stipple',
    'trivial/quads',
    'trivial/quadstrip',
    'trivial/quadstrip-clip',
    'trivial/quadstrip-cont',
    'trivial/quadstrip-flat',
    'trivial/readpixels',
    'trivial/sub-tex',
    'trivial/tex-quads',
    'trivial/tri',
    'trivial/tri-alpha',
    'trivial/tri-alpha-tex',
    'trivial/tri-array-interleaved',
    'trivial/tri-blend',
    'trivial/tri-blend-color',
    'trivial/tri-blend-max',
    'trivial/tri-blend-min',
    'trivial/tri-blend-revsub',
    'trivial/tri-blend-sub',
    'trivial/tri-clear',
    'trivial/tri-clip',
    'trivial/tri-cull',
    'trivial/tri-cull-both',
    'trivial/tri-dlist',
    'trivial/tri-edgeflag',
    'trivial/tri-edgeflag-array',
    'trivial/tri-fbo',
    'trivial/tri-fbo-tex',
    'trivial/tri-fbo-tex-mip',
    'trivial/tri-flat',
    'trivial/tri-flat-clip',
    'trivial/tri-fog',
    'trivial/tri-fp',
    'trivial/tri-fp-const-imm',
    'trivial/tri-lit',
    'trivial/tri-lit-material',
    'trivial/tri-logicop-none',
    'trivial/tri-logicop-xor',
    'trivial/tri-mask-tri',
    'trivial/tri-multitex-vbo',
    'trivial/tri-orig',
    'trivial/tri-point-line-clipped',
    'trivial/tri-query',
    'trivial/tri-repeat',
    'trivial/tri-scissor-tri',
    'trivial/tri-square',
    'trivial/tri-stencil',
    'trivial/tri-stipple',
    'trivial/tri-tex',
    'trivial/tri-tex-1d',
    'trivial/tri-tex-3d',
    'trivial/tri-tri',
    'trivial/tri-unfilled',
    'trivial/tri-unfilled-clip',
    'trivial/tri-unfilled-edgeflag',
    'trivial/tri-unfilled-fog',
    'trivial/tri-unfilled-point',
    'trivial/tri-unfilled-smooth',
    'trivial/tri-unfilled-tri',
    'trivial/tri-unfilled-tri-lit',
    'trivial/tri-unfilled-userclip',
    'trivial/tri-unfilled-userclip-stip',
    'trivial/tri-userclip',
    'trivial/tri-viewport',
    'trivial/tri-z',
    'trivial/tri-z-9',
    'trivial/tri-z-eq',
    'trivial/trifan',
    'trivial/trifan-flat',
    'trivial/trifan-flat-clip',
    'trivial/trifan-flat-unfilled-clip',
    'trivial/trifan-unfilled',
    'trivial/tristrip',
    'trivial/tristrip-clip',
    'trivial/tristrip-flat',
    'trivial/vbo-drawarrays',
    'trivial/vbo-drawelements',
    'trivial/vbo-drawrange',
    'trivial/vbo-noninterleaved',
    'trivial/vbo-tri',
    'trivial/vp-array',
    'trivial/vp-array-hf',
    'trivial/vp-array-int',
    'trivial/vp-clip',
    'trivial/vp-line-clip',
    'trivial/vp-tri',
    'trivial/vp-tri-cb',
    'trivial/vp-tri-cb-pos',
    'trivial/vp-tri-cb-tex',
    'trivial/vp-tri-imm',
    'trivial/vp-tri-invariant',
    'trivial/vp-tri-swap',
    'trivial/vp-tri-tex',
    'trivial/vp-unfilled',

    #'demos/arbfplight',
    #'demos/arbfslight',
    #'demos/arbocclude',
    #'demos/arbocclude2',
    #'demos/bounce',
    #'demos/clearspd',
    #'demos/copypix',
    #'demos/cubemap',
    #'demos/dinoshade',
    #'demos/dissolve',
    #'demos/drawpix',
    #'demos/engine',
    #'demos/fbo_firecube',
    #'demos/fbotexture',
    #'demos/fire',
    #'demos/fogcoord',
    #'demos/fplight',
    #'demos/fslight',
    #'demos/gamma',
    #'demos/gearbox',
    #'demos/gears',
    #'demos/geartrain',
    #'demos/glinfo',
    #'demos/gloss',
    #'demos/gltestperf',
    #'demos/ipers',
    #'demos/isosurf',
    #'demos/lodbias',
    #'demos/morph3d',
    #'demos/multiarb',
    #'demos/paltex',
    #'demos/pointblast',
    #'demos/projtex',
    #'demos/rain',
    #'demos/ray',
    #'demos/readpix',
    #'demos/reflect',
    #'demos/renormal',
    #'demos/shadowtex',
    #'demos/singlebuffer',
    #'demos/spectex',
    #'demos/spriteblast',
    #'demos/stex3d',
    #'demos/teapot',
    #'demos/terrain',
    #'demos/tessdemo',
    #'demos/texcyl',
    #'demos/texenv',
    #'demos/textures',
    #'demos/trispd',
    #'demos/tunnel',
    #'demos/tunnel2',
    #'demos/vao_demo',
    #'demos/winpos',
    #'fp/fp-tri',
    #'fp/point-position',
    #'fp/tri-depth',
    #'fp/tri-depth2',
    #'fp/tri-depthwrite',
    #'fp/tri-depthwrite2',
    #'fp/tri-param',
    #'fp/tri-tex',
    #'fpglsl/fp-tri',
    #'glsl/array',
    #'glsl/bezier',
    #'glsl/bitmap',
    #'glsl/brick',
    #'glsl/bump',
    #'glsl/convolutions',
    #'glsl/deriv',
    #'glsl/fragcoord',
    #'glsl/fsraytrace',
    #'glsl/geom-sprites',
    #'glsl/geom-stipple-lines',
    #'glsl/geom-wide-lines',
    #'glsl/identity',
    #'glsl/linktest',
    #'glsl/mandelbrot',
    #'glsl/multinoise',
    #'glsl/multitex',
    #'glsl/noise',
    #'glsl/noise2',
    #'glsl/pointcoord',
    #'glsl/points',
    #'glsl/samplers',
    #'glsl/shadow_sampler',
    #'glsl/shtest',
    #'glsl/skinning',
    #'glsl/texaaline',
    #'glsl/texdemo1',
    #'glsl/toyball',
    #'glsl/trirast',
    #'glsl/twoside',
    #'glsl/vert-or-frag-only',
    #'glsl/vert-tex',
    #'glsl/vsraytrace',
    #'gs/gs-tri',
    #'perf/copytex',
    #'perf/drawoverhead',
    #'perf/fbobind',
    #'perf/fill',
    #'perf/genmipmap',
    #'perf/readpixels',
    #'perf/swapbuffers',
    #'perf/teximage',
    #'perf/vbo',
    #'perf/vertexrate',
    #'redbook/aaindex',
    #'redbook/aapoly',
    #'redbook/aargb',
    #'redbook/accanti',
    #'redbook/accpersp',
    #'redbook/alpha',
    #'redbook/alpha3D',
    #'redbook/anti',
    #'redbook/bezcurve',
    #'redbook/bezmesh',
    #'redbook/checker',
    #'redbook/clip',
    #'redbook/colormat',
    #'redbook/combiner',
    #'redbook/convolution',
    #'redbook/cube',
    #'redbook/cubemap',
    #'redbook/depthcue',
    #'redbook/dof',
    #'redbook/double',
    #'redbook/drawf',
    #'redbook/feedback',
    #'redbook/fog',
    #'redbook/fogcoord',
    #'redbook/fogindex',
    #'redbook/font',
    #'redbook/hello',
    #'redbook/histogram',
    #'redbook/image',
    #'redbook/light',
    #'redbook/lines',
    #'redbook/list',
    #'redbook/material',
    #'redbook/minmax',
    #'redbook/mipmap',
    #'redbook/model',
    #'redbook/movelight',
    #'redbook/multisamp',
    #'redbook/multitex',
    #'redbook/mvarray',
    #'redbook/nurbs',
    #'redbook/pickdepth',
    #'redbook/picksquare',
    #'redbook/plane',
    #'redbook/planet',
    #'redbook/pointp',
    #'redbook/polyoff',
    #'redbook/polys',
    #'redbook/quadric',
    #'redbook/robot',
    #'redbook/sccolorlight',
    #'redbook/scene',
    #'redbook/scenebamb',
    #'redbook/sceneflat',
    #'redbook/select',
    #'redbook/shadowmap',
    #'redbook/smooth',
    #'redbook/stencil',
    #'redbook/stroke',
    #'redbook/surface',
    #'redbook/surfpoints',
    #'redbook/teaambient',
    #'redbook/teapots',
    #'redbook/tess',
    #'redbook/tesswind',
    #'redbook/texbind',
    #'redbook/texgen',
    #'redbook/texprox',
    #'redbook/texsub',
    #'redbook/texture3d',
    #'redbook/texturesurf',
    #'redbook/torus',
    #'redbook/trim',
    #'redbook/unproject',
    #'redbook/varray',
    #'redbook/wrap',
    #'samples/accum',
    #'samples/bitmap1',
    #'samples/bitmap2',
    #'samples/blendeq',
    #'samples/blendxor',
    #'samples/copy',
    #'samples/cursor',
    #'samples/depth',
    #'samples/eval',
    #'samples/fog',
    #'samples/font',
    #'samples/line',
    #'samples/logo',
    #'samples/nurb',
    #'samples/oglinfo',
    #'samples/olympic',
    #'samples/overlay',
    #'samples/point',
    #'samples/prim',
    #'samples/quad',
    #'samples/rgbtoppm',
    #'samples/select',
    #'samples/shape',
    #'samples/sphere',
    #'samples/star',
    #'samples/stencil',
    #'samples/stretch',
    #'samples/texture',
    #'samples/tri',
    #'samples/wave',
    #'slang/cltest',
    #'slang/sotest',
    #'slang/vstest',
    'tests/afsmultiarb',
    'tests/antialias',
    'tests/arbfpspec',
    'tests/arbfptest1',
    'tests/arbfptexture',
    'tests/arbfptrig',
    'tests/arbgpuprog',
    'tests/arbnpot',
    'tests/arbnpot-mipmap',
    'tests/arbvptest1',
    'tests/arbvptest3',
    'tests/arbvptorus',
    'tests/arbvpwarpmesh',
    'tests/arraytexture',
    'tests/auxbuffer',
    'tests/blendxor',
    'tests/blitfb',
    'tests/bufferobj',
    'tests/bug_3050',
    'tests/bug_3101',
    'tests/bug_3195',
    'tests/bug_texstore_i8',
    'tests/bumpmap',
    'tests/calibrate_rast',
    'tests/condrender',
    'tests/copypixrate',
    'tests/cva',
    'tests/cva_huge',
    'tests/cylwrap',
    'tests/drawbuffers',
    'tests/drawbuffers2',
    'tests/drawstencil',
    'tests/exactrast',
    'tests/ext422square',
    'tests/fbotest1',
    'tests/fbotest2',
    'tests/fbotest3',
    'tests/fillrate',
    'tests/floattex',
    'tests/fogcoord',
    'tests/fptest1',
    'tests/fptexture',
    'tests/getteximage',
    'tests/glutfx',
    'tests/interleave',
    'tests/invert',
    'tests/jkrahntest',
    'tests/lineclip',
    'tests/manytex',
    'tests/mapbufrange',
    'tests/minmag',
    'tests/mipgen',
    'tests/mipmap_comp',
    'tests/mipmap_comp_tests',
    'tests/mipmap_limits',
    'tests/mipmap_tunnel',
    'tests/mipmap_view',
    'tests/multipal',
    'tests/multitexarray',
    'tests/multiwindow',
    'tests/no_s3tc',
    'tests/occlude',
    'tests/packedpixels',
    'tests/pbo',
    'tests/persp_hint',
    'tests/prim',
    'tests/prog_parameter',
    'tests/quads',
    'tests/random',
    'tests/readrate',
    'tests/rubberband',
    'tests/scissor',
    'tests/scissor-viewport',
    'tests/seccolor',
    'tests/shader-interp',
    'tests/shader_api',
    'tests/shadow-sample',
    'tests/sharedtex',
    'tests/stencilreaddraw',
    'tests/stencilwrap',
    'tests/step',
    'tests/streaming_rect',
    'tests/subtex',
    'tests/subtexrate',
    'tests/tex1d',
    'tests/texcmp',
    'tests/texcompress2',
    'tests/texcompsub',
    'tests/texdown',
    'tests/texfilt',
    'tests/texgenmix',
    'tests/texleak',
    'tests/texline',
    'tests/texobj',
    'tests/texobjshare',
    'tests/texrect',
    'tests/unfilledclip',
    'tests/vparray',
    'tests/vpeval',
    'tests/vptest1',
    'tests/vptest2',
    'tests/vptest3',
    'tests/vptorus',
    'tests/vpwarpmesh',
    'tests/yuvrect',
    'tests/yuvsquare',
    'tests/zbitmap',
    'tests/zcomp',
    'tests/zdrawpix',
    'tests/zreaddraw',
    #'vp/vp-tris',
    #'vpglsl/vp-tris',
    #'xdemos/corender',
    #'xdemos/glsync',
    #'xdemos/glthreads',
    #'xdemos/glxcontexts',
    #'xdemos/glxdemo',
    #'xdemos/glxgears',
    #'xdemos/glxgears_fbconfig',
    #'xdemos/glxgears_pixmap',
    #'xdemos/glxheads',
    #'xdemos/glxinfo',
    #'xdemos/glxpbdemo',
    #'xdemos/glxpixmap',
    #'xdemos/glxsnoop',
    #'xdemos/glxswapcontrol',
    #'xdemos/manywin',
    #'xdemos/msctest',
    #'xdemos/multictx',
    #'xdemos/offset',
    #'xdemos/omlsync',
    #'xdemos/opencloseopen',
    #'xdemos/overlay',
    #'xdemos/pbdemo',
    #'xdemos/pbinfo',
    #'xdemos/shape',
    #'xdemos/sharedtex',
    #'xdemos/sharedtex_mt',
    #'xdemos/texture_from_pixmap',
    #'xdemos/wincopy',
    #'xdemos/xfont',
    #'xdemos/xrotfontdemo',
    #'xdemos/yuvrect_client',
]


tests = [
    'trivial/tri',
    'trivial/tri-tex',
]


def main():
    global options

    # Parse command line options
    optparser = optparse.OptionParser(
        usage='\n\t%prog [options] [demo] ...',
        version='%%prog')
    optparser.add_option(
        '--build', metavar='PATH',
        type='string', dest='build', default='.',
        help='path to apitrace build')
    optparser.add_option(
        '--mesa-demos', metavar='PATH',
        type='string', dest='mesa_demos', default=os.environ.get('MESA_DEMOS'),
        help='path to Mesa demos')

    (options, args) = optparser.parse_args(sys.argv[1:])

    if not options.mesa_demos:
        optparser.error('path to Mesa demos not specified')

    if args:
        for test in args:
           runtest(test)
    else:
        for test in tests:
           runtest(test)


if __name__ == '__main__':
    main()
