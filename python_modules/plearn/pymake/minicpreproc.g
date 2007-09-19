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

%%
parser MiniCPreproc:
    option:   "use-new-regexps"

    ignore: '[ \t]+'

    token END: "$"
    token ENDLINE: "\r\n|\n|\r|$"

    token FALSE: "0"
    token TRUE: "1"
    token UNKNOWN: "@"
    token OR: "\|\||or"
    token AND: "&&|and"
    token NOT: "!|not"

    token IF: "if"
    token IFDEF: "ifdef"
    token IFNDEF: "ifndef"
    token ELIF: "elif"
    token ELSE: "else"
    token ENDIF: "endif"
    token INCLUDE: "include"

    token DEFINED: "defined"
    token STRING: "[.a-zA-Z0-9_]+"
    token FILENAME: "[^\s\"\>]+"

    rule file:  block<<"1">> END                    -> << block >>

    rule block<<cond>>:                             -> << [] >>
        | include_line ENDLINE block<<cond>>        -> << If(cond, include_line) + block >>
        | if_block<<cond>> block<<cond>>            -> << if_block + block>>

    rule if_block<<cond>>: IF disj ENDLINE if_tail<<cond,disj>>                                                    -> << if_tail >>
        | IFDEF variable ENDLINE if_tail<<cond,Defined(variable)>>                                                    -> << if_tail >>
        | IFNDEF variable ENDLINE if_tail<<cond,Not(Defined(variable))>>                                                    -> << if_tail >>

    rule if_tail<<cond,d>>: block<<And(cond,d)>> el_block<<cond,d>>                                                    -> << block + el_block >>

    rule el_block<<cond,d>>: ENDIF ENDLINE                      -> << [] >>
        | ELSE ENDLINE block<<And(cond, Not(d))>> ENDIF ENDLINE -> << block >>
        | ELIF disj ENDLINE if_tail<<And(cond,Not(d)),disj>>    -> << if_tail >>

    rule include_line: INCLUDE quoted_file_name     -> << quoted_file_name >>

    rule quoted_file_name: "<" FILENAME ">"         -> << FILENAME >>
        | "\"" FILENAME "\""                        -> << FILENAME >>

    rule disj:  conj disj_tail<<conj>>              -> << disj_tail >>

    rule disj_tail<<v>>:                            -> << v >>
        | OR conj disj_tail<<Or(v,conj)>>           -> << disj_tail >>

    rule conj:  term conj_tail<<term>>              -> << conj_tail >>

    rule conj_tail<<v>>:                            -> << v >>
        | AND term conj_tail<<And(v,term)>>         -> << conj_tail >>

    rule term:  base_value                          -> << base_value >>
        | "\(" disj "\)"                            -> << disj >>
        | NOT term                                  -> << Not(term) >>

    rule variable:  STRING                          -> << STRING >>
        | "\(" variable "\)"                        -> << variable >>

    rule base_value:
        FALSE                                       -> << "0" >>
        | TRUE                                      -> << "1" >>
        | DEFINED variable                          -> << Defined(variable) >>
        | STRING unknown_operation<<STRING>>        -> << unknown_operation >>

    rule unknown_operation<<s>>:                    -> << Eval(s) >>
        | "<=|<|==|!=|>=|>|\+|-" STRING             -> << "@" >>
%%

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

