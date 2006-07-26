# Yapps 1.1 - yet another python parser system

# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:

# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

# Amit J Patel, February 1998
# See http://theory.stanford.edu/~amitp/Yapps/ for documentation and updates

# 10-Aug-1998: Made the resulting file test itself if it's __main__
# 25-Feb-1998: Added option NOT to use the context sensitive scanner,
#              which can decrease error detection capabilities
# 25-Feb-1998: Added option to use Python 1.5 style regexps ('re' module)
# 17-Feb-1998: Fixed bug with empty productions

from string import *
import regex, sys

## These are more readable when we're doing boolean stuff ###########
true = 1
false = 0

## First, some helper functions ######################################

def append(lst,x):
    lst.append(x)
    return lst

def setadd(s, x):
    """Add x to s if it's not already there"""
    if x not in s: s.append(x)

def setunion(s, t):
    """Add elements of t into s"""
    for x in t: setadd(s,x)

def union_list(sets):
    """Combine the elements and return the union"""
    s = []
    for t in sets: setunion(s,t)
    return s

def union(*sets):
    """Combine the elements and return the union"""
    return union_list(sets)

## Command line or parser options ####################################
yapps_options = [
    ('embed-scanner','scanner',
     'Include the scanner code in output.py'),
    ('embed-error-printer','error-printer',
     'Include a nice error display routine'),
    ('use-new-regexps','use-new-regexps',
     'Use Python 1.5 style regexps'),
    ('context-insensitive-scanner','context-insensitive-scanner',
     'Scan all tokens (see docs)')
]

## Small helper classes ##############################################

class Patterns:
    """Define some common regexps; these are also in the tokenize module"""
    ID = '^[a-zA-Z_][a-zA-Z_0-9]*$'
    NUM = '^[0-9]+$'
    STR = '^[rR]?\'\\(\\\\.\\|[^\\\012\']\\)*\'\\|[rR]?"\\(\\\\.\\|[^\\\012"]\\)*"$'

class Clause:
    """Keep each clause in its own object"""
    def __init__(self, subs, args, action):
        self.subs = subs
        self.args = args
        self.action = action
    def __repr__(self):
        return 'Clause(%s,%s,%s)' % (`self.subs`,`self.args`,`self.action`)

# Ugh, triple quoted strings don't get along with Emacs font-lock. Hey Barry!
verbatim_exceptions = '''
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
'''

verbatim_scanner = '''
import regex

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
            self.patterns.append( (k, regex.compile(r)) )

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
            output = '%s\\n  (@%s)  %s  =  %s' % (output,t[0],t[2],`t[3]`)
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
                if m > best_match:
                    # We got a match that's better than the previous one
                    best_pat = p
                    best_match = m

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
'''

verbatim_print_error = '''
def print_error(input, err, scanner):
    """This is a really dumb long function to print error messages nicely."""
    p = err.pos
    # Figure out the line number
    line = count(input[:p], '\\n')
    print err.msg+" on line "+`line+1`+":"
    # Now try printing part of the line
    text = input[max(p-80,0):p+80]
    p = p - max(p-80,0)

    # Strip to the left
    i = rfind(text[:p],'\\n')
    j = rfind(text[:p],'\\r')
    if i < 0 or (j < i and j >= 0): i = j
    if i >= 0 and i < p:
        p = p - i - 1
        text = text[i+1:]

    # Strip to the right
    i = find(text,'\\n',p)
    j = find(text,'\\r',p)
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
'''

def get_verbatim_scanner(options):
    """Make changes for Python 1.5, if needed"""
    v = verbatim_scanner
    if 'use-new-regexps' in options:
        # Patch the scanner code above to work with 1.5-style regexps
        subs = [('import regex', 'import re'),
                ('regex.', 're.'),
                ('m > best_match', 'm and len(m.group(0)) > best_match'),
                ('best_match = m', 'best_match = len(m.group(0))')]
        for s in subs:
            i = find(v, s[0])
            if i < 0: raise "Verbatim code has been changed."
            v = v[:i]+s[1]+v[i+len(s[0]):]
    return v

class ScannerGenerator:
    """This class generates and writes out a scanner"""
    def __init__(self, name, patterns, ignore, embed):
        self.name = name
        self.patterns = patterns
        self.ignore = ignore
        self.terminals = map(lambda x: x[0], patterns)
        self.embed = embed

    def pre_output(self, file):
        # Declarations
        file.write("from string import *\n")

        if 'scanner' in self.embed:
            # Put a new copy of the exceptions into this module
            file.write(verbatim_exceptions)
        else:
            # Get the exception objects from the Yapps module
            file.write("from yapps import SyntaxError, NoMoreTokens\n")

        if 'scanner' in self.embed or 'use-new-regexps' in self.embed:
            # Put a new copy of the scanner into this module;
            # this is *necessary* if we want to use 1.5-style regexps
            # because the scanner class in the Yapps module uses old regexps
            file.write(get_verbatim_scanner(self.embed))
        else:
            # Get the scanner from the Yapps module
            file.write("from yapps import Scanner\n")

        file.write("\n")
        file.write("class "+self.name+"Scanner(Scanner):\n")
        file.write("    def __init__(self, str):\n")
        file.write("        Scanner.__init__(self,[\n")
        for p in self.patterns:
            file.write("            " + `p` + ",\n")
        file.write("            ], "+`self.ignore`+", str)\n")
        file.write("\n")

    def post_output(self, file):
        # Call to the scanner
        file.write(self.name+"Scanner(input)")

class ParserGenerator:
    """This class generates and writes out a parser"""
    epsilon = 0

    # Grammar should be {nonterminal:(params,[clause,...])}
    # A clause is a record with .subs = [sub], .action = string
    # A sub is a record with .name = string, .args = string
    # where each name is a nonterminal or a terminal
    def __init__(self, name, scanner, grammar, embed):
        # Do sanity check on nonterminal names
        for g in grammar.keys():
            if regex.match(Patterns.ID, g) == -1:
                raise 'Bad Grammar', `g` + " must be a valid identifier"
        # Set up this object
        self.parsername = name
        self.scanner = scanner
        self.grammar = grammar
        self.embed = embed
        # Compute some needed information about the grammar
        first,follow = self.compute_first_and_follow()
        self.first = first
        self.follow = follow

    def __repr__(self):
        """Return some information about the grammar, with newlines in it"""
        output = ''
        longest = max(map(len, self.grammar.keys()))
        for g in self.grammar.keys():
            params = self.grammar[g][1]
            if params: params = '<<%s>>' % params
            label = (' '*longest + g + params + ': ')[-(longest+2):]
            for clause in self.grammar[g][0]:
                output = '%s\n%s%s' % (output, label, join(clause.subs,' '))
                label = ' '*(len(label)-2) + '| '
        return output

    def print_details(self):
        """Print some information about the grammar"""
        def add_brackets(s):
            if s: return "<<"+s+">>"
            else: return s

        nonterminals = self.grammar.keys()
        terminals = self.scanner.terminals
        first = self.first
        follow = self.follow
        for n in nonterminals+terminals:
            print '%s%s:' % (n, add_brackets(self.grammar[n][1]))
            if n in nonterminals:
                for clause_i in range(len(self.grammar[n][0])):
                    clause = self.grammar[n][0][clause_i]
                    print "  ",
                    for i in range(len(clause.subs)):
                        print clause.subs[i] + add_brackets(
                            clause.args[i])

    def compute_first_and_follow(self):
        """The FIRST sets are used to create the parser.
        The FOLLOW sets are used in creating the FIRST sets.

        FIRST, for each clause, is the set of tokens that can appear
        at the beginning of that clause.  FOLLOW, for each nonterminal,
        is the set of tokens that can appear after that nonterminal
        has been matched.  See a compiler book for more details."""

        grammar = self.grammar
        nonterminals = grammar.keys()
        terminals = self.scanner.terminals
        # Set up the initial first and follow sets
        first = {}
        follow = {}
        accepts_epsilon = {}
        for n in nonterminals:
            # There's one first set for every clause in the grammar
            first[n] = []
            for clause_i in range(len(grammar[n][0])):
                first[n].append([])
            # There's just one follow set for the nonterminal
            follow[n] = []
            # Start everything out an epsilon accepter
            accepts_epsilon[n] = true
        for n in terminals:
            first[n] = [[n]]
            follow[n] = []
            accepts_epsilon[n] = false

        # Repeat this loop until things stabilize
        #   We want to figure out accepts_epsilon in this stage
        changed = true
        while changed:
            changed = false
            for n in nonterminals:
                if accepts_epsilon[n]:
                    # For each clause, check to see if ANY element does not
                    # allow epsilon -- i.e., ALL elements but allow epsilon.
                    # This is called eps_one.  eps_any is true of any of
                    # the clauses allow epsilon.
                    eps_any = false
                    for clause in grammar[n][0]:
                        eps_one = true
                        for g in clause.subs:
                            if not accepts_epsilon[g]:
                                eps_one = false
                                break
                        eps_any = (eps_any or eps_one)
                    if not eps_any:
                        accepts_epsilon[n] = false
                        changed = true

        # Repeat the loop until things stabilize
        #   We want to figure out first and follow in this stage
        changed = true
        while changed:
            changed = false

            for n in nonterminals:
                clause_list = grammar[n][0]
                # Adjust the first set for each clause
                for clause_i in range(len(clause_list)):
                    # Start with the old first set, and add to it
                    new_first = first[n][clause_i]
                    # We need to look at the follow set if none of the
                    # parts of this clause sequence block the clause
                    # from accepting epsilon
                    need_follow = true
                    subs = clause_list[clause_i].subs
                    for g in subs:
                        new_first = union(new_first, union_list(first[g]))
                        if not accepts_epsilon[g]:
                            need_follow = false
                            break
                    if need_follow: new_first = union(new_first,follow[n])
                    if first[n][clause_i] != new_first:
                        changed = true
                        first[n][clause_i] = new_first

                    for sub_i in range(len(subs)-1):
                        g = subs[sub_i]
                        new_follow = union(follow[g],
                                           union_list(first[subs[sub_i+1]]))
                        if new_follow != follow[g]:
                            changed = true
                            follow[g] = new_follow
                    if subs:
                        new_follow = union(follow[subs[-1]], follow[n])
                        if new_follow != follow[subs[-1]]:
                            changed = true
                            follow[subs[-1]] = new_follow

        # Now verify the first sets to make sure they don't have repeat info
        #  (If they do, then there's left recursion or ambiguity)
        bad = false
        for n in nonterminals:
            # Check to see if any of the firsts are empty
            for clause_i in range(len(first[n])):
                if not first[n][clause_i]:
                    print 'Warning, rule',n,'clause (',
                    print join(grammar[n][0][clause_i].subs, ' '),
                    print ') never matches.'

            for g in union_list(first[n]):
                # g is a symbol, and we want to see if it occurs many times
                all_matches = []
                for clause_i in range(len(first[n])):
                    if g in first[n][clause_i]: setadd(all_matches, clause_i)
                if len(all_matches) > 1:
                    # There's a duplicate
                    print 'Warning, rule',n,'when',g,'occurs on left:'
                    lefts = []
                    left_factoring = []
                    for a in all_matches:
                        g = grammar[n][0][a].subs
                        print '  Matches: ',join(g,' ')
                        if not g: continue
                        if g[0] == n:
                            # Simple left recursion detected
                            print '  (Left recursion not allowed)'
                        if g[0] in lefts:
                            # Left factoring needed
                            setadd(left_factoring, g[0])
                        setadd(lefts, g[0])
                    if left_factoring:
                        # Display which symbols must be left factored
                        print '  (Left factoring needed for',
                        print join(left_factoring,', '),'..)'
                    bad = true
        if bad:
            print 'An LL(1) parser requires the left tokens to be unambiguous.'
            print '  A parser will be generated, but it is unlikely to work.'

        return first, follow

    def pick_name(self, subs, sub_i):
        """This routine picks a name for the sub_i'th part of clause `subs'
        If part X is the only one by that name, it gets X as a name;
        If there are parts 1..N and X is the i'th one, then it's named Xi;
        If X isn't a valid identifier, then _ is used"""
        if regex.match(Patterns.ID, subs[sub_i]) >= 0:
            # At least it's a decent name
            # Now figure out where it is in the list, and how many of that name
            count = 0
            pos = 0
            for i in range(len(subs)):
                if i == sub_i: pos = count
                if subs[i] == subs[sub_i]:  count = count+1
            # (pos is 0-based)
            if count == 1:
                # The name is unique, so don't number it
                return subs[sub_i]
            else:
                # Attach the number to the name
                return subs[sub_i] + `pos+1`
        # Just return a generic name
        return "_"

    def pre_output(self, file):
        """Generate the parser class"""

        # First output the scanner
        self.scanner.pre_output(file)

        # Write the beginning of the class
        file.write("class " + self.parsername + ":\n")
        file.write("    def __init__(self, scanner):\n")
        file.write("        self.scanner = scanner\n")
        file.write("\n")
        if 'context-insensitive-scanner' in self.embed:
            file.write("    def token(self, _pos_, type):\n")
            file.write("        tok = self.scanner.token(_pos_)\n")
            file.write("        if tok[2] != type:\n")
            file.write("            raise SyntaxError(tok[0], ")
            file.write("'Trying to find '+type)\n")
            file.write("        return tok[3]\n")
        else:
            file.write("    def token(self, _pos_, type):\n")
            file.write("        return self.scanner.token(_pos_, [type])[3]\n")
        file.write("\n")

        nonterminals = self.grammar.keys()
        terminals = self.scanner.terminals

        # For each nonterminal, create a method
        for n in nonterminals:
            clauses, params = self.grammar[n]
            if params: params = ', '+params
            else: params = '=0'
            file.write("    def "+n+"(self,_pos_"+params+"):\n")
            fl_all = []
            tail_recursion = false
            for clause_i in range(len(clauses)):
                clause = clauses[clause_i]
                fl_all = union(fl_all, self.first[n][clause_i])
                if clause.subs and strip(clause.action) == \
                   self.pick_name(clause.subs, len(clause.subs)-1) \
                   and clause.subs[-1] == n: tail_recursion = true

            # If there's tail recursion, the whole thing is in a loop
            indent0 = "        "
            if tail_recursion:
                file.write(indent0 + "while 1:\n")
                indent0 = indent0 + "    "

            # Get the first token
            file.write(indent0+"_start_,_,_token_,_text_ = self.scanner.token")
            if 'context-insensitive-scanner' in self.embed:
                file.write("(_pos_)\n")
            elif len(`fl_all`) > 25-len(indent0):
                # This is too long, so wrap it
                file.write("(_pos_,\n" + indent0 + "    " + `fl_all` + ")\n")
            else:
                file.write("(_pos_," + `fl_all` + ")\n")

            # Now, for each clause in the nonterminal, put in tests
            for clause_i in range(len(clauses)):
                fl = self.first[n][clause_i]
                # Remove these tokens from the set of all the tokens
                for f in fl:
                    if f in fl_all: fl_all.remove(f)
                clause = clauses[clause_i]
                # Now generate a test
                indent1 = indent0 + "    "
                if len(fl) == 1: test = "== " + `fl[0]`
                else:            test = "in " + `fl`
                if len(clauses) == 1:
                    # This is the ONLY one
                    indent1 = indent0
                elif clause_i == len(clauses)-1:
                    # This is the LAST one
                    file.write(indent0)
                    if 'context-insensitive-scanner' in self.embed:
                        # We must test the last case if the scanner didn't
                        file.write("elif _token_ "+test)
                    else:
                        file.write("else")
                    file.write(":\n")
                elif clause_i == 0:
                    # This is the FIRST one
                    file.write(indent0 + "if _token_ " + test + ":\n")
                else:
                    # This is in the middle
                    file.write(indent0 + "elif _token_ " + test + ":\n")

                # Now, for each piece of this clause, add scanning rules
                pos = '_pos_'
                tail_call = false
                for sub_i in range(len(clause.subs)):
                    name = self.pick_name(clause.subs, sub_i)
                    # Optimization that could be extended to cover
                    # more than just the last clause:
                    if sub_i == len(clause.subs)-1 and \
                       find(clause.action, name) < 0:
                        # This name will never get used
                        name = '_'

                    g = clause.subs[sub_i]
                    if g in nonterminals:
                        args = clause.args[sub_i]
                        # Check for tail recursion
                        if (g == n and sub_i == len(clause.subs)-1
                            and strip(clause.action) == name):
                            tail_call = true
                            if pos != '_pos_':
                                file.write(indent1 + "_pos_ = " + pos + "\n")
                            if args != self.grammar[n][1]:
                                # The parameters have changed
                                file.write(indent1 + self.grammar[n][1] + \
                                           " = " + args + "\n")
                            # No call has to be made, since it'll loop
                        else:
                            if args: args = ', ' + args
                            if (sub_i == len(clause.subs)-1
                                and strip(clause.action) == name):
                                # A tail call is still being made
                                tail_call = true
                                file.write(indent1 + "return")
                            else:
                                file.write(indent1 + name + ",_pos_ =")
                            file.write(" self." + g + "(" + pos + args + ")\n")
                            pos = '_pos_'
                    elif g in terminals:
                        if sub_i > 0:
                            file.write(indent1)
                            if name != '_': file.write(name + " = ")
                            file.write("self.token(" + pos + "," + `g` + ")\n")
                        elif name != '_':
                                file.write(indent1 + name + " = _text_\n")
                        # Do "partial evaluation" to increment 'pos'
                        if find(pos, '+') >= 0:
                            pos = `1+atoi(pos[:find(pos, '+')])` + '+_pos_'
                        else:
                            pos = "1+" + pos

                # If it's not tail recursion, so we should return a value
                if not tail_call:
                    # Put parentheses around the action if it's not simple
                    action = strip(clause.action)
                    if not action: action = 'None'
                    if (regex.match(Patterns.ID, action) < 0 and
                        regex.match(Patterns.STR, action) < 0 and
                        regex.match(Patterns.ID, action) < 0):
                        action = "(%s)" % action
                    file.write(indent1+"return "+action+", "+pos+"\n")

            # Write the end of the method
            if 'context-insensitive-scanner' in self.embed and len(clauses)>1:
                # Since the scanner didn't check all cases, we must here
                file.write(indent0+"else:\n")
                file.write(indent0+"    raise SyntaxError(_start_, ")
                file.write("'Could not match "+n+"')\n")
            file.write("\n")

    def post_output(self,file):
        """Generate the constructor call to the Parser class"""
        if 'error-printer' in self.embed:
            file.write(verbatim_print_error)
            file.write('\n')

        file.write("def parse(rule, input):\n")
        file.write("    parser = " + self.parsername + "(")
        self.scanner.post_output(file)
        file.write(")\n")
        file.write("    try: return getattr(parser, rule)()[0]\n")
        file.write("    except SyntaxError, s:\n")
        if 'error-printer' in self.embed:
            file.write("        print_error(input, s, parser.scanner)\n")
        else:
            file.write("        try:\n")
            file.write("            from yapps import print_error\n")
            file.write("            print_error(input, s, parser.scanner)\n")
            file.write("        except ImportError:\n")
            file.write("            print 'Syntax Error',s.msg,'on line',")
            file.write(            "1+count(input[:s.pos], '\\n')\n")
        file.write("    except NoMoreTokens:\n")
        file.write("        print 'Ran out of input'\n")
        file.write("\n")

        file.write("if __name__=='__main__':\n")
        file.write("    from sys import argv\n")
        file.write("    if len(argv) >= 3: print "
                   + "parse(argv[1], open(argv[2],'r').read())\n")
        file.write("    else: print 'Args:  <rule> <filename>'\n")
        file.write("\n")

######################################################################
# This code was generated by the parser generator!

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

class Scanner:
    def __init__(self, patterns, ignore, input):
        "Patterns is [(terminal,regex)...];"
        "Ignore is [terminal,...];"
        "Input is a string"
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
            self.patterns.append( (k, regex.compile(r)) )

    def token(self, i, restrict=0):
        "Get the i'th token, and if i is one past the end, then        scan "
        "for another token; restrict is a list of tokens that"
        "are allowed, or 0 for any token."
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
        "Should scan another token and add it to the list, self.tokens,"
        "and add the restriction to self.restrictions"
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
                if m > best_match:
                    # We got a match that's better than the previous one
                    best_pat = p
                    best_match = m

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

class ParserDescScanner(Scanner):
    def __init__(self, str):
        Scanner.__init__(self,[
            ('"ignore"', 'ignore'),
            ('"token"', 'token'),
            ('"->"', '->'),
            ('"rule"', 'rule'),
            ('"|"', '|'),
            ('"parser"', 'parser'),
            ('":"', ':'),
            ('"option"', 'option'),
            ('"[ \\t\\n\\r]+"', '[ \011\012\015]+'),
            ('END', '$'),
            ('ATTR', '<<\\([^>]+\\|>[^>]\\)*>>'),
            ('ID', '[a-zA-Z_][a-zA-Z_0-9]*'),
            ('STR', '\\("\\([^\\"]+\\|\\\\.\\)*"\\)\\|\\(\'\\([^\\\']+\\|\\\\.\\)*\'\\)'),
            ], ['"[ \\t\\n\\r]+"'], str)

class ParserDesc:
    def __init__(self, scanner):
        self.scanner = scanner

    def token(self, _pos_, type):
        tok = self.scanner.token(_pos_)
        if tok[2] != type:
            raise SyntaxError(tok[0], 'Trying to find '+type)
        return tok[3]

    def Options(self,_pos_, v):
        while 1:
            _start_,_,_token_,_text_ = self.scanner.token(_pos_)
            if _token_ == '"option"':
                self.token(1+_pos_,'":"')
                STR = self.token(2+_pos_,'STR')
                _pos_ = 3+_pos_
                v = append(v,eval(STR))
            elif _token_ in ['"token"', '"ignore"', '"rule"', 'END']:
                return v, _pos_
            else:
                raise SyntaxError(_start_, 'Could not match Options')

    def Subs(self,_pos_, u,v):
        while 1:
            _start_,_,_token_,_text_ = self.scanner.token(_pos_)
            if _token_ == 'STR':
                STR = _text_
                _pos_ = 1+_pos_
                u,v = append(u,STR), append(v,'')
            elif _token_ == 'ID':
                ID = _text_
                OptParam,_pos_ = self.OptParam(1+_pos_)
                u,v = append(u,ID), append(v,OptParam)
            elif _token_ == '"->"':
                return ((u, v)), _pos_
            else:
                raise SyntaxError(_start_, 'Could not match Subs')

    def Parser(self,_pos_=0):
        _start_,_,_token_,_text_ = self.scanner.token(_pos_)
        ID = self.token(1+_pos_,'ID')
        self.token(2+_pos_,'":"')
        Options,_pos_ = self.Options(3+_pos_, [])
        Tokens,_pos_ = self.Tokens(_pos_, [])
        Rules,_pos_ = self.Rules(_pos_, [])
        self.token(_pos_,'END')
        return ((ID,Options,Tokens,Rules)), 1+_pos_

    def OptParam(self,_pos_=0):
        _start_,_,_token_,_text_ = self.scanner.token(_pos_)
        if _token_ == 'ATTR':
            return self.Attr(_pos_)
        elif _token_ in ['STR', 'ID', '":"', '"->"']:
            return '', _pos_
        else:
            raise SyntaxError(_start_, 'Could not match OptParam')

    def Clauses(self,_pos_, v):
        while 1:
            _start_,_,_token_,_text_ = self.scanner.token(_pos_)
            if _token_ == '"|"':
                Clause,_pos_ = self.Clause(1+_pos_)
                v = append(v,Clause)
            elif _token_ in ['"rule"', 'END']:
                return v, _pos_
            else:
                raise SyntaxError(_start_, 'Could not match Clauses')

    def Rules(self,_pos_, v):
        while 1:
            _start_,_,_token_,_text_ = self.scanner.token(_pos_)
            if _token_ == '"rule"':
                ID = self.token(1+_pos_,'ID')
                OptParam,_pos_ = self.OptParam(2+_pos_)
                self.token(_pos_,'":"')
                Clause,_pos_ = self.Clause(1+_pos_)
                Clauses,_pos_ = self.Clauses(_pos_, [Clause])
                v = append(v,(ID,OptParam,Clauses))
            elif _token_ == 'END':
                return v, _pos_
            else:
                raise SyntaxError(_start_, 'Could not match Rules')

    def Clause(self,_pos_=0):
        _start_,_,_token_,_text_ = self.scanner.token(_pos_)
        Subs,_pos_ = self.Subs(_pos_, [],[])
        self.token(_pos_,'"->"')
        Attr,_pos_ = self.Attr(1+_pos_)
        return (Clause(Subs[0], Subs[1], Attr)), _pos_

    def Attr(self,_pos_=0):
        _start_,_,_token_,_text_ = self.scanner.token(_pos_)
        ATTR = _text_
        return (ATTR[2:-2]), 1+_pos_

    def Tokens(self,_pos_, v):
        while 1:
            _start_,_,_token_,_text_ = self.scanner.token(_pos_)
            if _token_ == '"token"':
                ID = self.token(1+_pos_,'ID')
                self.token(2+_pos_,'":"')
                STR = self.token(3+_pos_,'STR')
                _pos_ = 4+_pos_
                v = append(v,(ID,STR))
            elif _token_ == '"ignore"':
                self.token(1+_pos_,'":"')
                STR = self.token(2+_pos_,'STR')
                _pos_ = 3+_pos_
                v = append(v,('#ignore',STR))
            elif _token_ in ['"rule"', 'END']:
                return v, _pos_
            else:
                raise SyntaxError(_start_, 'Could not match Tokens')

# End of generated code
######################################################################

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

def generate(inputfilename, outputfilename='', embed=[]):
    """Generate a grammar, given an input filename (X.g)
    and an output filename (defaulting to X.py)."""

    if not outputfilename:
        if inputfilename[-2:]=='.g': outputfilename = inputfilename[:-2]+'.py'
        else: raise "Invalid Filename", outputfilename

    # Read in the entire file
    input = open(inputfilename, 'r').read()

    preparser = ""
    postparser = ""
    DIVIDER = "\n%%\n"
    # See if there's a separation between the pre-parser and parser
    f = find(input, DIVIDER)
    if f >= 0:        preparser, input = input[:f]+'\n\n', input[f+len(DIVIDER):]
    # See if there's a separation between the parser and post-parser
    f = find(input, DIVIDER)
    if f >= 0:  input, postparser = input[:f], '\n\n'+input[f+len(DIVIDER):]

    # Create scanner and parser objects, and try to parse
    parserparser = ParserDesc(ParserDescScanner(input))
    try:
        name, options, tokens, rules = parserparser.Parser()[0]
        # Add parser specified options to command line options
        embed = embed + options
        # Make sure that they're understood
        for opt in options:
            for _, n, _ in yapps_options:
                if opt in options: break
            else:
                print 'Warning: unrecognize parser option ',opt

    except SyntaxError, s:
        print_error(input, s, parserparser.scanner)
        return
    except NoMoreTokens:
        print inputfilename, 'could not be parsed completely.'

    # Create the list of tokens, along with the ones to ignore
    patterns = []
    ignore = []
    terminals = []
    for k,t in tokens:
        if k == '#ignore':
            k = t
            ignore.append(k)
        terminals.append(k)
        patterns.append( (k, eval(t, {}, {})) )

    grammar = {}
    nonterminals = []
    for n,a,clause_list in rules:
        grammar[n] = (clause_list, a)
        nonterminals.append(n)
    # Add inline tokens to the token list
    for n in grammar.keys():
        for clause in grammar[n][0]:
            for g in clause.subs:
                if (g not in nonterminals) and (g not in terminals):
                    if regex.match(Patterns.STR, g) >= 0:
                        # Only do this if it's a string literal
                        patterns.insert( 0, (g, eval(g, {}, {})) )
                        terminals.append(g)
                    else:
                        # What the heck is it?
                        print 'Warning (rule %s):' % n,
                        print 'Could not understand', `g`

    scanner = ScannerGenerator(name, patterns, ignore, embed)
    parser = ParserGenerator(name, scanner, grammar, embed)
    output = open(outputfilename, 'w')
    output.write(preparser)
    output.write('## Begin parser\n')
    parser.pre_output(output)
    parser.post_output(output)
    output.write(postparser)

if __name__=='__main__':
    import getopt
    optlist, args = getopt.getopt(sys.argv[1:], 'f:')
    if not args or len(args) > 2:
        print 'Usage:'
        print '  python yapps.py [flags] input.g [output.py]'
        print 'Flags:'
        for flag, _, doc in yapps_options:
            print ('  -f' + flag + ' '*40)[:35] + doc
    else:
        embed = []
        for opt in optlist:
            for flag, name, _ in yapps_options:
                if opt == ('-f', flag):
                    embed.append(name)
                    break
            else:
                print 'Warning - unrecognized option:  ', opt[0], opt[1]
        apply(generate, tuple(args), {'embed':embed})
