"""Module containing miscellaneous helper functions.

Contains all functions that do fit in any other L{utilities}
submodule. If a considerable number of functions contained in this
module seems to manage similar tasks, it is probably time to create a
I{similar_tasks.py} L{utilities} submodule to move those functions to.
"""
import inspect, os, popen2, string, sys, time, types

try:
    import epydoc.markup 
    import epydoc.markup.epytext
except ImportError:
    pass

def boxed_lines(s, box_width, indent=''):
    if len(s) <= box_width:
        return [s]

    words = string.split(s)
    return boxed_lines_from_words(words, box_width, indent)

def boxed_lines_from_words(words, box_width, indent=''):
    box_width = box_width+len(indent)

    boxed_lines = []
    boxed = None
    for i, word in enumerate(words):
        if boxed is None:
            boxed = "%s%s" % (indent, word)
        elif len(boxed) < box_width-len(word):
            boxed = "%s %s" % (boxed, word)
        else:
            boxed_lines.append( boxed ) 
            boxed = "%s%s" % (indent, word)

    if boxed is not None and boxed != "":
        boxed_lines.append( boxed ) 
            
    return boxed_lines

def boxed_string(s, box_width, indent=''):
    if len(s) <= box_width:
        return s

    words = string.split(s)
    return boxed_string_from_words(words, box_width, indent)

def boxed_string_from_words(words, box_width, indent=''):
    return string.join(boxed_lines_from_words(words, box_width, indent), '\n')    

def centered_square(s, width, ldelim='[', rdelim=']'):
    width -= 4
    return ldelim+" " + string.center(s, width) + " "+rdelim 

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

        if inspect.ismodule(epydoc.markup):
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

def lines_to_file(lines, filepath):
    """Print the lines in a file named I{filepath}.

    @param lines: The lines to be printed in the file. Do not add end of
    lines at the end of your lines; it will be made here.
    @type  lines: List of strings.

    @param filepath: The path to the file in which to output I{lines}. The
    file may not exist, but the directory (if any) must.
    @type  filepath: String.
    """
    output_file = open(filepath, 'w')
    for line in lines:
        output_file.write( line )
    output_file.close()


def listdirs(dirs):
    """Return the list of files in all of I{dirs}.

    @param dirs: A list of directory paths from which to extract file
    entries (using U{os.listdir<http://www.python.org/doc/2.3.4/lib/os-file-dir.html>})
    
    @type  dirs: List of strings

    @return: List of files within all directories in I{dirs}.
    @rtype:  List of strings
    """
    dirs_list = []
    for dirc in dirs:
        if os.path.exists(dirc):
            dirs_list.extend( os.listdir(dirc) )
    return dirs_list

def no_none( orig ):
    """Parses the I{None} elements out of a list.  

    @param orig: The list to parse I{None} elements out of.
    @type  orig: List.

    @return: An array that has the same elements than the original list
    I{orig} except for elements that were I{None}.
    @rtype:  Array.
    """
    nonone = []
    for elem in orig:
        if elem is not None:
            nonone.append(elem)
    return nonone

def plural(nb, sing='', plur='s'):
    if nb > 1:
        return plur
    return sing

def quote(s):
    if string.find(s, '\n') != -1:
        return '"""%s"""' % s
    return '"%s"' % s

def quote_if(s):
    if isinstance(s, types.StringType):
        return quote(s)
    return s

def short_doc(obj):
    return doc(obj, True)

def timed_version_backup( path ):
    backup = "%s.%s" % ( path, date_time_string() )
    os.system( 'mv %s %s' % ( path, backup ) ) 
    return backup

def left_from_right(get):
    def get_attr(self, rop_name):
        if not rop_name in default_roperators._rop_names:
            raise AttributeError
        if hasattr(self, rop_name):
            return get(self, rop_name)
        lop_name = rop_name[0:2] + rop_name[3:]            
        return get(self, lop_name)
    return get_attr


class default_roperators_metaclass( type ):
    _rop_names = [
        '__radd__', '__rsub__', '__rmul__', '__rdiv__',
        '__rtruediv__', '__rfloordiv__', '__rmod__',
        '__rdivmod__', '__rpow__', '__rlshift__',
        '__rrshift__', '__rand__', '__rxor__', '__ror__'
        ]

    def __init__(cls, name, bases, dict):
        frozen = None
        if hasattr(cls, '_frozen'):
            frozen      = cls._frozen
            cls._frozen = False
            
        super(default_roperators_metaclass, cls).__init__(name, bases, dict)
        for rop_name in cls._rop_names:
            lop_name = rop_name[0:2] + rop_name[3:]
            if ( not dict.has_key(rop_name)
                 and dict.has_key(lop_name) ):
                setattr( cls, rop_name, dict[lop_name] )

        if frozen is not None:
            cls._frozen = frozen

                
class default_roperators(object):
    __metaclass__ = default_roperators_metaclass
                
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
            default_roperators.__init__(self)
            self.val = val

        def __str__(self):
            return "lop(%d)"%self.val

        def __add__(self, val):
            print val
            return lop( self.val + val )

    lop.test()

##     class other_super:
##         def __getattr__(self, k):
##             def p(key):
##                 print key
##             return p

##     class lop_first( default_roperators, other_super ):
##         def test(cls):
##             t = cls()
##             print t + 10 
##             print
##             print 10 + t
##             print
##             try:
##                 t << 10
##                 print "SHOULD NOT WORK!!!"
##             except Exception:
##                 print "Failed as expected."

##         test = classmethod(test)

##         def __init__(self, val=0):
##             self.val = val

##         def __str__(self):
##             return "lop_first(%d)"%self.val

##         def __add__(self, val):
##             print val
##             return lop_first( self.val + val )

##     lop_first.test()

##     class lop_second( other_super, default_roperators ):
##         def test(cls):
##             t = cls()
##             print t + 10 
##             print
##             print 10 + t
##             print
##             try:
##                 t << 10
##                 print "SHOULD NOT WORK!!!"
##             except Exception:
##                 print "Failed as expected."

##         test = classmethod(test)

##         def __init__(self, val=0):
##             self.val = val

##         def __str__(self):
##             return "lop_second(%d)"%self.val

##         def __add__(self, val):
##             print val
##             return lop_second( self.val + val )

##     lop_second.test()
