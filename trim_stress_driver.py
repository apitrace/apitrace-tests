#!/usr/bin/env python

# Copyright 2012 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation files
# (the "Software"), to deal in the Software without restriction,
# including without limitation the rights to use, copy, modify, merge,
# publish, distribute, sublicense, and/or sell copies of the Software,
# and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
# BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
# ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

'''Stress test driver for apitrace trim.'''

import os, errno, re, shutil, subprocess

from base_driver import *

def rm_and_mkdir(dir):

    # Operate only on local directories
    dir = './' + dir

    # Failing to delete a directory that doesn't exist is no failure
    def rmtree_onerror(function, path, excinfo):
        if excinfo[0] == OSError and excinfo[1].errno != errno.ENOENT:
            raise

    shutil.rmtree(dir, onerror = rmtree_onerror)
    os.makedirs(dir)

class TrimStressDriver(Driver):

    def trim_stress(self, trace_file):
        "Execute an apitrace-trim stress test for the given trace."

        (dir, file_name) = os.path.split(trace_file)
        (name, extension) = file_name.rsplit('.', 1)

        ref_dir = name + '-ref/'
        tmp_out_dir = name + '-tmp-out/'
        out_dir = name + '-out/'
        trim_file = name + '.trace.trim'

        rm_and_mkdir(out_dir)
        rm_and_mkdir(tmp_out_dir)

        if (not os.path.exists(ref_dir)):
            rm_and_mkdir(ref_dir)
            subprocess.check_call([self.options.apitrace,
                                   "dump-images", "--call-nos=no",
                                   "--output=" + ref_dir, trace_file]);

        # Count the number of frame snapshots generated
        frames = 0
        for dirpath, dirs, files in os.walk(ref_dir):
            # Don't descend into any sub-directories (not that there
            # should be any)
            del dirs[:]
            pattern = re.compile("[0-9]*\.png")
            def f(x): return re.match(pattern, x)
            files = filter(f, files)
            frames = len(files)

        for frame in range(frames):
            try:
                subprocess.check_call([self.options.apitrace,
                                       "trim", "--auto",
                                       "--frame=%d/frame" % (frame),
                                       "--output=" + trim_file, trace_file])
            except:
                print "An error occurred while trimming " + \
                    "frame %d from %s" % (frame, trace_file)
                fail()
            try:
                subprocess.check_call([self.options.apitrace,
                                       "dump-images", "--call-nos=no",
                                       "--output=" + tmp_out_dir + "frame",
                                       trim_file])
            except:
                print "An error occurred replaying " + \
                    "%s to generate a frame snapshot" % (trim_file)
                fail()
            os.rename("%s/frame0000000000.png" % (tmp_out_dir),
                      "%s/%010d.png" % (tmp_out_dir, frame))
            try:
                subprocess.check_call([self.options.apitrace,
                                       "diff-images", "-v",
                                       "-o", name + "-index.html",
                                       ref_dir, tmp_out_dir])
            except:
                print "Trimmed frame did not match " + \
                    "reference images. See " + name + "-index.html"
                fail()
            os.rename("%s/%010d.png" % (tmp_out_dir, frame),
                      "%s/%010d.png" % (out_dir, frame))
        try:
            subprocess.check_call([self.options.apitrace,
                                   "diff-images", "-v",
                                   "-o", name + "-index.html",
                                   ref_dir, out_dir])
        except:
            print "Trimmed frame did not match " + \
                "reference images. See " + name + "-index.html"
            fail()

    def run(self):
        self.parseOptions()

	self.trim_stress(self.args[0])

        pass_()

if __name__ == '__main__':
    TrimStressDriver().run()
