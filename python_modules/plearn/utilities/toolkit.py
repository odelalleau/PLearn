"""Module containing miscellaneous helper functions.

Contains all functions that do fit in any other L{utilities}
submodule. If a considerable number of functions contained in this
module seems to manage similar tasks, it is probably time to create a
I{similar_tasks.py} L{utilities} submodule to move those functions to.
"""
import copy, os, popen2, string, sys, time, types
import epydoc.markup 
import epydoc.markup.epytext

def boxed_string(s, box_width, indent=''):
    if len(s) <= box_width:
        return s

    words = string.split(s)
    return boxed_string_from_words(words, box_width, indent)

def boxed_string_from_words(words, box_width, indent=''):
    box_width = box_width+len(indent)

    boxed = None
    for i, word in enumerate(words):
        if boxed is None:
            boxed = "%s%s" % (indent, word)
        elif len(boxed) < box_width-len(word):
            boxed = "%s %s" % (boxed, word)
        else:
            end   = boxed_string_from_words(words[i:], box_width, indent) 
            boxed = "%s\n%s" % (boxed, end)
            break
        
    return boxed

## def boxed_string(s, box_width):
##     if len(s) > box_width:
##         words = string.split(s)
##         s = ''
##         for word in words:
##             if len(s) == 0:
##                 s = word
##             elif len(s) < box_width-len(word):
##                 s = "%s %s" % (s, word)
##             else:
##                 s = "%s\n%s" % (s, word)
##     return s
    
def command_output(command):
    process = popen2.Popen4( command )
    process.wait()
    return process.fromchild.readlines()

def date_time_string():
    t = time.localtime()
    return ( str(t[0]) + "_" + str(t[1]) + "_" + str(t[2])
             + "_" +
             str(t[3]) + ":" + str(t[4]) + ":" + str(t[5]) )

def doc(obj, short=False):
    docstr = obj.__doc__    
    if docstr is None:
        return ''

    lines        = string.split(docstr, '\n')
    if short:
        lines = [ lines[0] ]
    
    parsed_lines = []
    for line in lines:
        striped = string.lstrip(line)
        errors  = []
        parsed  = epydoc.markup.epytext.parse_docstring( striped, errors ).to_plaintext(None)
        parsed  = string.rstrip( parsed )

        if errors:
            exc = "'%s' contains the following errors\n" % parsed
            for e in errors:
                exc = "%s\n%s" % (exc, str(e))
            raise ValueError(exc)
        
        parsed_lines.append( parsed )

    as_string = string.join( parsed_lines, '\n    ' )
    return as_string

def exempt_list_of(list, undesired_values):
    """Remove all undesired values present in the I{list}.

    @param list: The list to clean.
    @type  list: ListType

    @param undesired_values: Single element B{or} list of elements to
    be removed from I{list} if present.
    @type  undesired_values: Any
    """
    
    if not isinstance(undesired_values, types.ListType):
        undesired_values = [undesired_values]
        
    for undesired in undesired_values:
        if undesired in list:
            list.remove(undesired)

def isccfile(file_path):
    """True if the extension of I{file_path} is one of I{.cc}, I{.CC}, I{.cpp}, I{.c} or I{.C}."""
    (base,ext) = os.path.splitext(file_path)
    return ext in ['.cc','.CC','.cpp','.c','.C']

def istexfile(file_path):
    """True if the extension of I{file_path} is one of I{.tex}, I{.sty} or I{.bib}."""
    (base,ext) = os.path.splitext(file_path)
    return ext in ['.tex','.sty','.bib']

def isvmat( file_path ):
    """True if the extension of I{file_path} is one of I{.amat}, I{.pmat} or I{.vmat}."""
    (base,ext) = os.path.splitext(file_path)
    return ext in [ '.amat','.pmat','.vmat' ]

def is_recursively_empty(directory):
    """Checks if the I{directory} is a the root of an empty hierarchy.

    @param directory: A valid directory path
    @type  directory: StringType

    @return: True if the I{directory} is a the root of an empty
    hierarchy. The function returns False if there exists any file or
    link that are within a subdirectory of I{directory}.
    """
    for path in os.listdir(directory):
        relative_path = os.path.join(directory, path)
        if ( not os.path.isdir(relative_path) or 
             not is_recursively_empty(relative_path) ):
            return False
    return True

def plural(nb, sing='', plur='s'):
    if nb > 1:
        return plur
    return sing

def quote(s):
    if string.find(s, '\n') != -1:
        return '"""%s"""' % s
    return '"%s"' % s

def short_doc(obj):
    return doc(obj, True)

class default_roperators:
    _rop_names = [
        '__radd__', '__rsub__', '__rmul__', '__rdiv__',
        '__rtruediv__', '__rfloordiv__', '__rmod__',
        '__rdivmod__', '__rpow__', '__rlshift__',
        '__rrshift__', '__rand__', '__rxor__', '__ror__'
        ]

    def __getattr__(self, k):
        if k in default_roperators._rop_names:
            rop_name = copy.copy(k)
            lop_name = rop_name[0:2] + rop_name[3:]            
            lop      = getattr(self, lop_name)            
            return lop

        raise AttributeError

if __name__ == "__main__":
    class lop( default_roperators ):
        def test(cls):
            t = cls()
            print t + 10 
            print
            print 10 + t
            print
            try:
                t << 10
                print "SHOULD NOT WORK!!!"
            except Exception:
                print "Failed as expected."

        test = classmethod(test)

        def __init__(self, val=0):
            self.val = val

        def __str__(self):
            return "lop(%d)"%self.val

        def __add__(self, val):
            print val
            return lop( self.val + val )

    lop.test()
