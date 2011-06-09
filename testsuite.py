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


import fnmatch
import optparse
import os.path
import sys

from test import Report, TestCase


def runtest(report, demo):
    app = os.path.join(options.mesa_demos, 'src', demo)
    dirname, basename = os.path.split(app)
    name = demo.replace('/', '-')
    args = [os.path.join('.', basename)]

    test = TestCase(
        name = name,
        args = args,
        cwd = dirname,
        build = options.build,
        results = options.results,
    )
    test.run(report)


def parse_spec(filename):
    testlist = []
    for line in open(filename, 'rt'):
        if line.lstrip().startswith('#') or not line.strip():
            # comment / empty
            continue
        fields = line.split()

        name = fields.pop(0)
        cwd = fields.pop(0)
        args = fields

        if cwd:
            if options.cwd is not None:
                cwd = os.path.join(options.cwd, cwd)
            if not os.path.dirname(args[0]):
                args[0] = os.path.join('.', args[0])
        else:
            cwd = None

        test = TestCase(
            name = name,
            args = args,
            cwd = cwd,
            build = options.build,
            results = options.results,
        )

        testlist.append(test)
    return testlist


def main():
    global options

    # Parse command line options
    optparser = optparse.OptionParser(
        usage='\n\t%prog [options] testspec [glob] ...',
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
        optparser.error('a test spec must be specified')

    spec = args.pop(0)
    testlist = parse_spec(spec)

    if args:
        new_testlist = []
        for test in testlist:
            for pattern in args:
                if fnmatch.fnmatchcase(test.name, pattern):
                    new_testlist.append(test)
        testlist = new_testlist

    report = Report(options.results)
    for test in testlist:
        test.run(report)


if __name__ == '__main__':
    main()
