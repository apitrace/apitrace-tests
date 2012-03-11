#!/usr/bin/env python
##########################################################################
#
# Copyright 2008-2012 Jose Fonseca
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


import sys
import re


class ValueMatcher:

    def match(self, value):
        raise NotImplementedError

    def __str__(self):
        raise NotImplementerError


class WildcardMatcher(ValueMatcher):

    def match(self, value):
        return true

    def __str__(self):
        return '*'


class LiteralValueMatcher(ValueMatcher):

    def __init__(self, refValue):
        self.refValue = refValue

    def match(self, value):
        return self.refValue == value

    def __str__(self):
        return repr(self.refValue)


class ApproxValueMatcher(ValueMatcher):


    def __init__(self, refValue, tolerance = 2**-23):
        self.refValue = refValue
        self.tolerance = tolerance

    def match(self, value):
        error = abs(self.refValue - value)
        if self.refValue:
            error = error / self.refValue
        return error <= self.tolerance

    def __str__(self):
        return repr(self.refValue)


class ArrayMatcher(ValueMatcher):

    def __init__(self, refElements):
        self.refElements = refElements

    def match(self, value):
        if not isinstance(value, list):
            return False

        if len(value) != len(self.refElements):
            return False

        for refElement, element in zip(self.refElements, value):
            if not refElement.match(element):
                return False

        return True

    def __str__(self):
        return '{' + ', '.join(map(str, self.refElements)) + '}'


class StructMatcher(ValueMatcher):

    def __init__(self, refMembers):
        self.refMembers = refMembers

    def match(self, value):
        if not isinstance(value, dict):
            return False

        if len(value) != len(self.refMembers):
            return False

        for name, refMember in self.refMembers.iteritems():
            try:
                member = value[name]
            except KeyError:
                return False
            else:
                if not refMember.match(member):
                    return False

        return True

    def __str__(self):
        print self.refMembers
        return '{' + ', '.join(['%s = %s' % refMember for refMember in self.refMembers.iteritems()]) + '}'


class CallMatcher:

    def __init__(self, refFunctionName, refArgs, refRet = None):
        self.refFunctionName = refFunctionName
        self.refArgs = refArgs
        self.refRet = refRet

    def match(self, functionName, args, ret = None):
        if refFunctionName != functionName:
            return False

        if len(self.refArgs) != len(args):
            return False

        for (refArgName, refArg), (argName, arg) in zip(self.refArgs, args):
            if not refArg.match(arg):
                return False

        if self.refRet is None:
            if ret is not None:
                return False
        else:
            if not self.refRet.match(ret):
                return False

        return True

    def __str__(self):
        s = self.refFunctionName
        s += '(' + ', '.join(['%s = %s' % refArg for refArg in self.refArgs]) + ')'
        if self.refRet is not None:
            s += ' = ' + str(self.refRet)
        return s


class TraceMatcher:

    def __init__(self, refCalls):
        self.refCalls = refCalls

    def __str__(self):
        return ''.join(['%s\n' % refCall for refCall in self.refCalls])


EOF = -1
SKIP = -2


class ParseError(Exception):

    def __init__(self, msg=None, filename=None, line=None, col=None):
        self.msg = msg
        self.filename = filename
        self.line = line
        self.col = col

    def __str__(self):
        return ':'.join([str(part) for part in (self.filename, self.line, self.col, self.msg) if part != None])
        

class Scanner:
    """Stateless scanner."""

    # should be overriden by derived classes
    tokens = []
    symbols = {}
    literals = {}
    ignorecase = False

    def __init__(self):
        flags = re.DOTALL
        if self.ignorecase:
            flags |= re.IGNORECASE
        self.tokens_re = re.compile(
            '|'.join(['(' + regexp + ')' for type, regexp, test_lit in self.tokens]),
             flags
        )

    def next(self, buf, pos):
        if pos >= len(buf):
            return EOF, '', pos
        mo = self.tokens_re.match(buf, pos)
        if mo:
            text = mo.group()
            type, regexp, test_lit = self.tokens[mo.lastindex - 1]
            pos = mo.end()
            if test_lit:
                type = self.literals.get(text, type)
            return type, text, pos
        else:
            c = buf[pos]
            return self.symbols.get(c, None), c, pos + 1


class Token:

    def __init__(self, type, text, line, col):
        self.type = type
        self.text = text
        self.line = line
        self.col = col


class Lexer:

    # should be overriden by derived classes
    scanner = None
    tabsize = 8

    newline_re = re.compile(r'\r\n?|\n')

    def __init__(self, buf = None, pos = 0, filename = None, fp = None):
        if fp is not None:
            try:
                fileno = fp.fileno()
                length = os.path.getsize(fp.name)
                import mmap
            except:
                # read whole file into memory
                buf = fp.read()
                pos = 0
            else:
                # map the whole file into memory
                if length:
                    # length must not be zero
                    buf = mmap.mmap(fileno, length, access = mmap.ACCESS_READ)
                    pos = os.lseek(fileno, 0, 1)
                else:
                    buf = ''
                    pos = 0

            if filename is None:
                try:
                    filename = fp.name
                except AttributeError:
                    filename = None

        self.buf = buf
        self.pos = pos
        self.line = 1
        self.col = 1
        self.filename = filename

    def next(self):
        while True:
            # save state
            pos = self.pos
            line = self.line
            col = self.col

            type, text, endpos = self.scanner.next(self.buf, pos)
            assert pos + len(text) == endpos
            self.consume(text)
            type, text = self.filter(type, text)
            self.pos = endpos

            if type == SKIP:
                continue
            elif type is None:
                msg = 'unexpected char '
                if text >= ' ' and text <= '~':
                    msg += "'%s'" % text
                else:
                    msg += "0x%X" % ord(text)
                raise ParseError(msg, self.filename, line, col)
            else:
                break
        return Token(type = type, text = text, line = line, col = col)

    def consume(self, text):
        # update line number
        pos = 0
        for mo in self.newline_re.finditer(text, pos):
            self.line += 1
            self.col = 1
            pos = mo.end()

        # update column number
        while True:
            tabpos = text.find('\t', pos)
            if tabpos == -1:
                break
            self.col += tabpos - pos
            self.col = ((self.col - 1)//self.tabsize + 1)*self.tabsize + 1
            pos = tabpos + 1
        self.col += len(text) - pos


class Parser:

    def __init__(self, lexer):
        self.lexer = lexer
        self.lookahead = self.lexer.next()

    def match(self, type):
        return self.lookahead.type == type

    def skip(self, type):
        while not self.match(type):
            self.consume()

    def error(self):
        raise ParseError(
            msg = 'unexpected token %r' % self.lookahead.text, 
            filename = self.lexer.filename, 
            line = self.lookahead.line, 
            col = self.lookahead.col)

    def consume(self, type = None):
        if type is not None and not self.match(type):
            self.error()
        token = self.lookahead
        self.lookahead = self.lexer.next()
        return token


ID = 0
NUMBER = 1
HEXNUM = 2
STRING = 3

LPAREN = 4
RPAREN = 5
LCURLY = 6
RCURLY = 7
COMMA = 8
AMP = 9
EQUAL = 11

BLOB = 12


class CallScanner(Scanner):

    # token regular expression table
    tokens = [
        # whitespace
        (SKIP, r'[ \t\f\r\n\v]+', False),

        # Alphanumeric IDs
        (ID, r'[a-zA-Z_\x80-\xff][a-zA-Z0-9_\x80-\xff]*', True),

        # Numeric IDs
        (HEXNUM, r'-?0x[0-9a-fA-F]+', False),
        
        # Numeric IDs
        (NUMBER, r'-?(?:\.[0-9]+|[0-9]+(?:\.[0-9]*)?)', False),

        # String IDs
        (STRING, r'"[^"\\]*(?:\\.[^"\\]*)*"', False),
    ]

    # symbol table
    symbols = {
        '(': LPAREN,
        ')': RPAREN,
        '{': LCURLY,
        '}': RCURLY,
        ',': COMMA,
        '&': AMP,
        '=': EQUAL,
    }

    # literal table
    literals = {
        'blob': BLOB
    }


class CallLexer(Lexer):

    scanner = CallScanner()

    def filter(self, type, text):
        if type == STRING:
            text = text[1:-1]

            # line continuations
            text = text.replace('\\\r\n', '')
            text = text.replace('\\\r', '')
            text = text.replace('\\\n', '')
            
            # quotes
            text = text.replace('\\"', '"')

            type = ID

        return type, text


class CallParser(Parser):

    def __init__(self, stream):
        lexer = CallLexer(fp = stream)
        Parser.__init__(self, lexer)

    def parse(self):
        while not self.match(EOF):
            self.parse_call()

    def parse_call(self):
        if self.lookahead.type == NUMBER:
            token = self.consume()
            callNo = int(token.text)
        else:
            callNo = None
        
        functionName = self.consume(ID).text

        args = self.parse_sequence(LPAREN, RPAREN, self.parse_pair)

        if self.match(EQUAL):
            self.consume(EQUAL)
            ret = self.parse_value()
        else:
            ret = None

        self.handle_call(callNo, functionName, args, ret)

    def parse_pair(self):
        '''Parse a `name = value` pair.'''
        name = self.consume(ID).text
        self.consume(EQUAL)
        value = self.parse_value()
        return name, value

    def parse_opt_pair(self):
        '''Parse an optional `name = value` pair.'''
        if self.match(ID):
            name = self.consume(ID).text
            if self.match(EQUAL):
                self.consume(EQUAL)
                value = self.parse_value()
            else:
                value = name
                name = None
        else:
            name = None
            value = self.parse_value()
        if name is None:
            return value
        else:
            return name, value

    def parse_value(self):
        if self.match(AMP):
            self.consume()
            value = [self.parse_value()]
            return ArrayMatcher(value)
        elif self.match(ID):
            token = self.consume()
            value = token.text
            return LiteralValueMatcher(value)
        elif self.match(STRING):
            token = self.consume()
            value = token.text
            return LiteralValueMatcher(value)
        elif self.match(NUMBER):
            token = self.consume()
            value = float(token.text)
            return ApproxValueMatcher(value)
        elif self.match(HEXNUM):
            token = self.consume()
            value = int(token.text, 16)
            return LiteralValueMatcher(value)
        elif self.match(LCURLY):
            value = self.parse_sequence(LCURLY, RCURLY, self.parse_opt_pair)
            if len(value) and isinstance(value[0], tuple):
                return StructMatcher(dict(value))
            else:
                return ArrayMatcher(value)
        elif self.match(BLOB):
            token = self.consume()
            self.consume(LPAREN)
            length = self.consume()
            self.consume(RPAREN)
            # TODO
            return WildcardMatcher()
        else:
            self.error()

    def parse_sequence(self, ltype, rtype, elementParser):
        '''Parse a comma separated list'''

        elements = []

        self.consume(ltype)
        sep = None
        while not self.match(rtype):
            if sep is None:
                sep = COMMA
            else:
                self.consume(sep)
            element = elementParser()
            elements.append(element)
        self.consume(rtype)

        return elements

    def handle_call(self, callNo, functionName, args, ret):
        matcher = CallMatcher(functionName, args, ret)

        if callNo is not None:
            sys.stdout.write('%u ' % callNo)
        sys.stdout.write(str(matcher))
        sys.stdout.write('\n')


def main():
    parser = CallParser(sys.stdin)
    parser.parse()


if __name__ == '__main__':
    main()
