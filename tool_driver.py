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

'''Tool test driver.'''


import os.path
import subprocess
import sys


from base_driver import *


class AsciiComparer:

    def __init__(self, srcStream, refFileName, verbose=False):
        self.srcStream = srcStream
        self.refFileName = refFileName
        if refFileName:
            self.refStream = open(refFileName, 'rt')
        else:
            self.refStream = None
    
    def readLines(self, stream):
        lines = []
        for line in stream:
            line = line[:-1]
            lines.append(line)
        return lines

    def compare(self):
        refLines = self.readLines(self.refStream)
        srcLines = self.readLines(self.srcStream)

        numLines = max(len(refLines), len(srcLines))

        for lineNo in xrange(numLines):
            try:
                refLine = refLines[lineNo]
            except IndexError:
                fail('unexpected junk: %r' % self.srcLines[lineNo])

            try:
                srcLine = srcLines[lineNo]
            except IndexError:
                fail('unexpected EOF: %r expected' % refLine)

            if refLine != srcLine:
                fail('mismatch: expected %r but got %r' % (refLine ,srcLine))



class TestCase:

    cmd = None
    cwd = None
    ref_dump = None

    verbose = False

    def __init__(self):
        pass
    
    def runTool(self):
        '''Run the application standalone, skipping this test if it fails by
        some reason.'''

        if not self.cmd:
            return

        if self.ref_dump:
            stdout = subprocess.PIPE
        else:
            stdout = None

        cmd = [options.apitrace] + self.cmd
        p = popen(cmd, cwd=self.cwd, stdout=stdout)

        if self.ref_dump:
            comparer = AsciiComparer(p.stdout, self.ref_dump, self.verbose)
            comparer.compare()

        p.wait()
        if p.returncode != 0:
            fail('tool returned code %i' % p.returncode)

    def run(self):
        self.runTool()

        pass_()


class ToolMain(Main):

    def createOptParser(self):
        optparser = Main.createOptParser(self)

        optparser.add_option(
            '--ref-dump', metavar='PATH',
            type='string', dest='ref_dump', default=None,
            help='reference dump')

        return optparser

    def main(self):
        global options

        (options, args) = self.parseOptions()

        test = TestCase()
        test.verbose = options.verbose

        test.cmd = args
        test.cwd = options.cwd
        test.ref_dump = options.ref_dump

        test.run()


if __name__ == '__main__':
    ToolMain().main()
