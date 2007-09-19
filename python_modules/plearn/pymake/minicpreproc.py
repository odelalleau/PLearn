# minicpreproc.g contains the grammar of a minimalist parser, intended
# to process C-preprocessor-like code produced by (and used inside of)
# pymake, in order to detect the dependency to included header files.
#
# minicpreproc.py contains the parser itself, generated from the grammar
# by Yapps (http://theory.stanford.edu/~amitp/Yapps/), that should also
# be distributed with pymake.
#
# The command line to build minicpreproc.py is:
# % python yapps.py -fembed-error-printer -fembed-scanner minicpreproc.g
#
# Copyright (C) 2006 Pascal Lamblin
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#
#   1. Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
#
#   2. Redistributions in binary form must reproduce the above copyright
#      notice, this list of conditions and the following disclaimer in the
#      documentation and/or other materials provided with the distribution.
#
#   3. The name of the authors may not be used to endorse or promote
#      products derived from this software without specific prior written
#      permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
#  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
#  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
#  NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
#  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#  This file is part of the PLearn library. For more information on the PLearn
#  library, go to the PLearn Web site at www.plearn.org


def Or( arg1, arg2 ):
    if arg1 == "0":
        return arg2
    elif arg1 == "1" or arg2 == "1":
        return "1"
    else:
        return "@"

def And( arg1, arg2 ):
    if arg1 == "1":
        return arg2
    elif arg1 == "0" or arg2 == "0":
        return "0"
    else:
        return "@"

def Not( arg1 ):
    if arg1 == "0":
        return "1"
    elif arg1 == "1":
        return "0"
    else:
         return "@"

def Defined( var ):
    if var in defined_var_list:
        return "1"
    elif var in undefined_var_list:
        return "0"
    else:
        return "@"

def Eval( var ):
    if var in var_dict:
        return var_dict[var]
    else:
        return "@"

def If( cond, expr ):
    if cond != "0":
        return [expr]
    else:
        return []


## Begin parser
from string import *

class SyntaxError:
    "When we run into an unexpected token, this is the exception to use"
    def __init__(self, pos=-1, msg="Bad Token"):
        self.pos = pos
        self.msg = msg
    def __repr__(self):
        if self.pos < 0: return "#<syntax-error>"
        else: return "SyntaxError[@ char " + `self.pos` + ": " + self.msg + "]"

class NoMoreTokens:
    "Another exception object, for when we run out of tokens"
    pass

import re

class Scanner:
    def __init__(self, patterns, ignore, input):
        """Patterns is [(terminal,regex)...];
        Ignore is [terminal,...];
        Input is a string"""
        self.tokens = []
        self.restrictions = []
        self.input = input
        self.pos = 0
        self.patterns = []
        self.terminals = []
        self.ignore = ignore
        # The stored patterns are a pair (compiled regex,source regex)
        for k,r in patterns:
            self.terminals.append(k)
            self.patterns.append( (k, re.compile(r)) )

    def token(self, i, restrict=0):
        """Get the i'th token, and if i is one past the end, then scan
        for another token; restrict is a list of tokens that
        are allowed, or 0 for any token."""
        if i == len(self.tokens): self.scan(restrict)
        if i < len(self.tokens):
            # Make sure the restriction is more restricted
            if restrict:
                for r in restrict:
                    if r not in self.restrictions[i]:
                        raise "Unimplemented: restriction set changed"
            return self.tokens[i]
        raise NoMoreTokens()

    def __repr__(self):
        "Print the last 10 tokens that have been scanned in"
        output = ''
        for t in self.tokens[-10:]:
            output = '%s\n  (@%s)  %s  =  %s' % (output,t[0],t[2],`t[3]`)
        return output

    def scan(self, restrict):
        """Should scan another token and add it to the list, self.tokens,
        and add the restriction to self.restrictions"""
        # Keep looking for a token, ignoring any in self.ignore
        while 1:
            # Search the patterns for the longest match, with earlier
            # tokens in the list having preference
            best_match = -1
            best_pat = '(error)'
            for p, regexp in self.patterns:
                # First check to see if we're ignoring this token
                if restrict and p not in restrict and p not in self.ignore:
                    continue
                m = regexp.match(self.input, self.pos)
                if m and len(m.group(0)) > best_match:
                    # We got a match that's better than the previous one
                    best_pat = p
                    best_match = len(m.group(0))

            # If we didn't find anything, raise an error
            if best_pat == '(error)' and best_match < 0:
                msg = "Bad Token"
                if restrict:
                    msg = "Trying to find one of "+join(restrict,", ")
                raise SyntaxError(self.pos, msg)

            # If we found something that isn't to be ignored, return it
            if best_pat not in self.ignore:
                # Create a token with this data
                token = (self.pos, self.pos+best_match, best_pat,
                         self.input[self.pos:self.pos+best_match])
                self.pos = self.pos + best_match
                # Only add this token if it's not in the list
                # (to prevent looping)
                if not self.tokens or token != self.tokens[-1]:
                    self.tokens.append(token)
                    self.restrictions.append(restrict)
                return
            else:
                # This token should be ignored ..
                self.pos = self.pos + best_match

class MiniCPreprocScanner(Scanner):
    def __init__(self, str):
        Scanner.__init__(self,[
            ('"\\""', '"'),
            ('">"', '>'),
            ('"<"', '<'),
            ('"<=|<|==|!=|>=|>|\\+|-"', '<=|<|==|!=|>=|>|\\+|-'),
            ('"\\)"', '\\)'),
            ('"\\("', '\\('),
            ("'[ \\t]+'", '[ \t]+'),
            ('END', '$'),
            ('ENDLINE', '\r\n|\n|\r|$'),
            ('FALSE', '0'),
            ('TRUE', '1'),
            ('UNKNOWN', '@'),
            ('OR', '\\|\\||or'),
            ('AND', '&&|and'),
            ('NOT', '!|not'),
            ('IF', 'if'),
            ('IFDEF', 'ifdef'),
            ('IFNDEF', 'ifndef'),
            ('ELIF', 'elif'),
            ('ELSE', 'else'),
            ('ENDIF', 'endif'),
            ('INCLUDE', 'include'),
            ('DEFINED', 'defined'),
            ('STRING', '[.a-zA-Z0-9_]+'),
            ('FILENAME', '[^\\s"\\>]+'),
            ], ["'[ \\t]+'"], str)

class MiniCPreproc:
    def __init__(self, scanner):
        self.scanner = scanner

    def token(self, _pos_, type):
        return self.scanner.token(_pos_, [type])[3]

    def conj(self,_pos_=0):
        _start_,_,_token_,_text_ = self.scanner.token(_pos_,
            ['"\\("', 'NOT', 'FALSE', 'TRUE', 'DEFINED', 'STRING'])
        term,_pos_ = self.term(_pos_)
        return self.conj_tail(_pos_, term)

    def term(self,_pos_=0):
        _start_,_,_token_,_text_ = self.scanner.token(_pos_,
            ['FALSE', 'TRUE', 'DEFINED', 'STRING', '"\\("', 'NOT'])
        if _token_ in ['FALSE', 'TRUE', 'DEFINED', 'STRING']:
            return self.base_value(_pos_)
        elif _token_ == '"\\("':
            disj,_pos_ = self.disj(1+_pos_)
            self.token(_pos_,'"\\)"')
            return disj, 1+_pos_
        else:
            NOT = _text_
            term,_pos_ = self.term(1+_pos_)
            return (Not(term)), _pos_

    def include_line(self,_pos_=0):
        _start_,_,_token_,_text_ = self.scanner.token(_pos_,['INCLUDE'])
        INCLUDE = _text_
        return self.quoted_file_name(1+_pos_)

    def if_block(self,_pos_, cond):
        _start_,_,_token_,_text_ = self.scanner.token(_pos_,
            ['IF', 'IFDEF', 'IFNDEF'])
        if _token_ == 'IF':
            IF = _text_
            disj,_pos_ = self.disj(1+_pos_)
            ENDLINE = self.token(_pos_,'ENDLINE')
            return self.if_tail(1+_pos_, cond,disj)
        elif _token_ == 'IFDEF':
            IFDEF = _text_
            variable,_pos_ = self.variable(1+_pos_)
            ENDLINE = self.token(_pos_,'ENDLINE')
            return self.if_tail(1+_pos_, cond,Defined(variable))
        else:
            IFNDEF = _text_
            variable,_pos_ = self.variable(1+_pos_)
            ENDLINE = self.token(_pos_,'ENDLINE')
            return self.if_tail(1+_pos_, cond,Not(Defined(variable)))

    def disj_tail(self,_pos_, v):
        while 1:
            _start_,_,_token_,_text_ = self.scanner.token(_pos_,
                ['"\\)"', 'ENDLINE', 'OR'])
            if _token_ in ['"\\)"', 'ENDLINE']:
                return v, _pos_
            else:
                OR = _text_
                conj,_pos_ = self.conj(1+_pos_)
                v = Or(v,conj)

    def unknown_operation(self,_pos_, s):
        _start_,_,_token_,_text_ = self.scanner.token(_pos_,
            ['AND', 'OR', '"\\)"', 'ENDLINE', '"<=|<|==|!=|>=|>|\\+|-"'])
        if _token_ in ['AND', 'OR', '"\\)"', 'ENDLINE']:
            return (Eval(s)), _pos_
        else:
            self.token(1+_pos_,'STRING')
            return "@", 2+_pos_

    def conj_tail(self,_pos_, v):
        while 1:
            _start_,_,_token_,_text_ = self.scanner.token(_pos_,
                ['OR', '"\\)"', 'ENDLINE', 'AND'])
            if _token_ in ['OR', '"\\)"', 'ENDLINE']:
                return v, _pos_
            else:
                AND = _text_
                term,_pos_ = self.term(1+_pos_)
                v = And(v,term)

    def file(self,_pos_=0):
        _start_,_,_token_,_text_ = self.scanner.token(_pos_,
            ['END', 'INCLUDE', 'IF', 'IFDEF', 'IFNDEF', 'ENDIF', 'ELSE', 'ELIF'])
        block,_pos_ = self.block(_pos_, "1")
        self.token(_pos_,'END')
        return block, 1+_pos_

    def variable(self,_pos_=0):
        _start_,_,_token_,_text_ = self.scanner.token(_pos_,
            ['STRING', '"\\("'])
        if _token_ == 'STRING':
            STRING = _text_
            return STRING, 1+_pos_
        else:
            variable,_pos_ = self.variable(1+_pos_)
            self.token(_pos_,'"\\)"')
            return variable, 1+_pos_

    def if_tail(self,_pos_, cond,d):
        _start_,_,_token_,_text_ = self.scanner.token(_pos_,
            ['END', 'INCLUDE', 'IF', 'IFDEF', 'IFNDEF', 'ENDIF', 'ELSE', 'ELIF'])
        block,_pos_ = self.block(_pos_, And(cond,d))
        el_block,_pos_ = self.el_block(_pos_, cond,d)
        return (block + el_block), _pos_

    def disj(self,_pos_=0):
        _start_,_,_token_,_text_ = self.scanner.token(_pos_,
            ['"\\("', 'NOT', 'FALSE', 'TRUE', 'DEFINED', 'STRING'])
        conj,_pos_ = self.conj(_pos_)
        return self.disj_tail(_pos_, conj)

    def base_value(self,_pos_=0):
        _start_,_,_token_,_text_ = self.scanner.token(_pos_,
            ['FALSE', 'TRUE', 'DEFINED', 'STRING'])
        if _token_ == 'FALSE':
            return "0", 1+_pos_
        elif _token_ == 'TRUE':
            return "1", 1+_pos_
        elif _token_ == 'DEFINED':
            DEFINED = _text_
            variable,_pos_ = self.variable(1+_pos_)
            return (Defined(variable)), _pos_
        else:
            STRING = _text_
            return self.unknown_operation(1+_pos_, STRING)

    def quoted_file_name(self,_pos_=0):
        _start_,_,_token_,_text_ = self.scanner.token(_pos_,['"<"', '"\\""'])
        if _token_ == '"<"':
            FILENAME = self.token(1+_pos_,'FILENAME')
            self.token(2+_pos_,'">"')
            return FILENAME, 3+_pos_
        else:
            FILENAME = self.token(1+_pos_,'FILENAME')
            self.token(2+_pos_,'"\\""')
            return FILENAME, 3+_pos_

    def block(self,_pos_, cond):
        _start_,_,_token_,_text_ = self.scanner.token(_pos_,
            ['END', 'ENDIF', 'ELSE', 'ELIF', 'INCLUDE', 'IF', 'IFDEF', 'IFNDEF'])
        if _token_ in ['END', 'ENDIF', 'ELSE', 'ELIF']:
            return ([]), _pos_
        elif _token_ == 'INCLUDE':
            include_line,_pos_ = self.include_line(_pos_)
            ENDLINE = self.token(_pos_,'ENDLINE')
            block,_pos_ = self.block(1+_pos_, cond)
            return (If(cond, include_line) + block), _pos_
        else:
            if_block,_pos_ = self.if_block(_pos_, cond)
            block,_pos_ = self.block(_pos_, cond)
            return (if_block + block), _pos_

    def el_block(self,_pos_, cond,d):
        _start_,_,_token_,_text_ = self.scanner.token(_pos_,
            ['ENDIF', 'ELSE', 'ELIF'])
        if _token_ == 'ENDIF':
            ENDIF = _text_
            self.token(1+_pos_,'ENDLINE')
            return ([]), 2+_pos_
        elif _token_ == 'ELSE':
            ELSE = _text_
            ENDLINE1 = self.token(1+_pos_,'ENDLINE')
            block,_pos_ = self.block(2+_pos_, And(cond, Not(d)))
            ENDIF = self.token(_pos_,'ENDIF')
            self.token(1+_pos_,'ENDLINE')
            return block, 2+_pos_
        else:
            ELIF = _text_
            disj,_pos_ = self.disj(1+_pos_)
            ENDLINE = self.token(_pos_,'ENDLINE')
            return self.if_tail(1+_pos_, And(cond,Not(d)),disj)


def print_error(input, err, scanner):
    """This is a really dumb long function to print error messages nicely."""
    p = err.pos
    # Figure out the line number
    line = count(input[:p], '\n')
    print err.msg+" on line "+`line+1`+":"
    # Now try printing part of the line
    text = input[max(p-80,0):p+80]
    p = p - max(p-80,0)

    # Strip to the left
    i = rfind(text[:p],'\n')
    j = rfind(text[:p],'\r')
    if i < 0 or (j < i and j >= 0): i = j
    if i >= 0 and i < p:
        p = p - i - 1
        text = text[i+1:]

    # Strip to the right
    i = find(text,'\n',p)
    j = find(text,'\r',p)
    if i < 0 or (j < i and j >= 0): i = j
    if i >= 0:
        text = text[:i]

    # Now shorten the text
    while len(text) > 70 and p > 60:
        # Cut off 10 chars
        text = "..." + text[10:]
        p = p - 7

    # Now print the string, along with an indicator
    print '> ',text
    print '> ',' '*p + '^'
    print 'List of nearby tokens:', scanner

def parse(rule, input):
    parser = MiniCPreproc(MiniCPreprocScanner(input))
    try: return getattr(parser, rule)()[0]
    except SyntaxError, s:
        print_error(input, s, parser.scanner)
    except NoMoreTokens:
        print 'Ran out of input'

if __name__=='__main__':
    from sys import argv
    if len(argv) >= 3: print parse(argv[1], open(argv[2],'r').read())
    else: print 'Args:  <rule> <filename>'




def naive_include_list( input ):
    include_regexp = re.compile(r'^include\s*(?:"|<)([^\s\"\>]+)(?:"|>)',re.M)
    return include_regexp.findall(input)

def get_include_list( input, undefined_cpp_vars, defined_cpp_vars, cpp_vars_values ):
    global undefined_var_list, defined_var_list, var_dict
    undefined_var_list = undefined_cpp_vars
    defined_var_list = defined_cpp_vars
    var_dict = cpp_vars_values
    parser = MiniCPreproc(MiniCPreprocScanner(input))
    try:
        return getattr(parser, "file")()[0]
    except SyntaxError, s:
        print_error(input, s, parser.scanner)
        return naive_include_list( input )
    except NoMoreTokens:
        print "Error: No more tokens"
        return naive_include_list( input )

