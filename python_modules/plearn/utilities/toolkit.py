"""Module containing miscellaneous helper functions.

Contains all functions that do fit in any other L{utilities}
submodule. If a considerable number of functions contained in this
module seems to manage similar tasks, it is probably time to create a
I{similar_tasks.py} L{utilities} submodule to move those functions to.
"""
import inspect, os, popen2, string, sys, time, types

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
    """Returns the output lines of a shell command.    
    
    The U{commands<http://docs.python.org/lib/module-commands.html>} module
    provides a similar function, getoutput(), that returns a single
    string. Since the
    U{commands<http://docs.python.org/lib/module-commands.html>} module
    relies on U{popen<http://docs.python.org/lib/module-popen2.html>}
    objects, it is more efficient to use directly
    U{popen<http://docs.python.org/lib/module-popen2.html>} here that
    splitting the results of the commands.getoutput() in lines.

    @param command: The shell command to execute.
    @type command: String

    @return: Output lines.
    @rtype:  Array of strings.
    """
    breakpoint(command, False and command.startswith('diff'))
    process = popen2.Popen4(command)

    return process.fromchild.readlines()

def breakpoint( msg, cond=True ):
    if cond:
        raw_input( msg )
    
def cross_product( list1,  list2,
                   joinfct = lambda i,j: (i, j) ):
    """Returns the cross product of I{list1} and I{list2}.

    The default behavior is to return pairs made for elements in list1 and list2::

       cross_product( ['a', 'b'], [1, 2, 3] )
       ### [('a', 1), ('b', 1), ('a', 2), ('b', 2), ('a', 3), ('b', 3)]

    but one can specify the I{joinfct} to obtain a different behavior::
    
       cross_product( ['a', 'b'], [1, 2, 3], joinfct = lambda s,i: '%s_%d' % (s, i) )
       ### ['a_1', 'b_1', 'a_2', 'b_2', 'a_3', 'b_3']

    @param list1: First elements of the cross-product pairs (inner-loop).
    @type  list1: List

    @param list2: Second elements of the cross-product pairs (outer-loop).
    @type  list2: List

    @param joinfct: The pairing procedure.
    @type  joinfct: Any function-like object accepting two arguments.

    @returns: The list of joinfct-paired cross-product elements.
    """
    cross = []
    for elem2 in list2:
        cross.extend([ joinfct(elem1, elem2)
                       for elem1 in list1
                       ])
    return cross

def date_time_string(date_separator = '_', time_separator = ':', \
                     date_time_separator = '_'):
    t = time.localtime()
    year  = str(t[0])

    def length2( i ):
        if i < 10:
            return "0%d" % i
        return str(i)
    
    month = length2( t[1] )
    day   = length2( t[2] )

    hour  = length2( t[3] )
    mins  = length2( t[4] )
    secs  = length2( t[5] )

    return "%s%s%s%s%s%s%s%s%s%s%s" % ( year, date_separator, month,
                                              date_separator, day,
                                        date_time_separator,
                                        hour, time_separator, mins,
                                              time_separator, secs ) 

def date_time_random_string(date_separator = '_', time_separator = ':', \
                            date_time_separator = '_'):
    import random
    s = date_time_string(date_separator, time_separator, date_time_separator)
    s += "_%d" % random.randint(1e03, 1e09)    
    return s

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

        parsed = striped
        try:
            from epydoc.markup import epytext
            parsed  = epytext.parse_docstring( striped, errors ).to_plaintext(None)
        except:
            pass
        
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

def find_one(s, substrings):
    """Searches string I{s} for any string in I{substrings}.

    The I{substrings} list is iterated using iter(substrings).

    @param  s: The string to parse.
    @type   s: String

    @param  substrings: The strings to look for in s
    @type   substrings: List of strings

    @returns: A pair formed of the first substring found in I{s} and its
    position in I{s}. If none of I{substrings} is found, None is
    returned. 
    """
    for sub in iter(substrings):
        index = string.find(s, sub)
        if index != -1:
            return (sub, index)
    return None

def function_body( func ):
    func_source, lstart = inspect.getsourcelines( func )
    body_indent = func_source.pop(0).find('def') + 4

    return "".join([ srcline[body_indent:] for srcline in func_source ])

def rfind_one(s, substrings):
    """Searches string I{s} for any string in I{substrings}.

    The I{substrings} list is iterated using iter(substrings).

    @param  s: The string to parse.
    @type   s: String

    @param  substrings: The strings to look for in s
    @type   substrings: List of strings

    @returns: A pair formed of the first substring found B{(using rfind)} in
    I{s} and its position in I{s}. If none of I{substrings} is found, None
    is returned.
    """
    for sub in iter(substrings):
        index = string.rfind(s, sub)
        if index != -1:
            return (sub, index)
    return None

def isccfile(file_path):
    """True if the extension of I{file_path} is one of I{.cc}, I{.CC}, I{.cpp}, I{.c} or I{.C}."""
    (base,ext) = os.path.splitext(file_path)
    return ext in ['.cc','.CC','.cpp','.c','.C']

def istexfile(file_path):
    """True if the extension of I{file_path} is one of I{.tex}, I{.sty} or I{.bib}."""
    (base,ext) = os.path.splitext(file_path)
    return ext in ['.tex','.sty','.bib']

def isvmat( file_path ):
    """True if the extension of I{file_path} is one of I{.amat}, I{.dmat}, I{.pmat} or I{.vmat}."""
    (base,ext) = os.path.splitext(file_path)
    return ext in [ '.amat', '.dmat', '.pmat','.vmat' ]

def keep_a_timed_version( path ):
    backup = "%s.%s" % ( path, date_time_string() )
    os.system( 'mv %s %s' % ( path, backup ) ) 
    return backup

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

def parse_indent(s):
    indent = ""
    for ch in s:
        if ch == ' ':
            indent += ' '
        else:
            break
    return indent
    
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

def re_filter_list(strlist, undesired_regexp):
    """Remove all elements in the I{strlist} matching any I{undesired_regexp}.

    @param strlist: The list to clean.
    @type  strlist: list of strings

    @param undesired_regexp: re pattern(s)
    @type  undesired_values: list of strings
    """
    import re
    if not isinstance(undesired_regexp, types.ListType):
        undesired_regexp = [undesired_regexp]

    patterns = [ re.compile(u) for u in undesired_regexp ]

    check = strlist[:]
    for elem in check:
        for p in patterns:
            if p.search(elem):
                strlist.remove(elem)
                break

def short_doc(obj):
    return doc(obj, True)

def version( project_path, build_list ):
    """Automatic version control.
    
    Builds are tuples of the form (major, minor, plearn_revision) where
    plearn_revision is the SVN revision of the PLearn project when the
    project's version was released.
    """
    
    if os.path.isdir( project_path ):
        dirname, fname = project_path, '.'
    else:
        dirname, fname = os.path.split( project_path )

    from moresh import pushd, popd
    pushd( dirname )
    info_lines = command_output( 'svn info %s' % fname )
    popd( )

    plearn_version  = int( info_lines[3].split(':')[1] )
    project_version = int( info_lines[7].split(':')[1] )

    major, minor, release_version = build_list[-1]
    
    revision = plearn_version - release_version
    if revision == 0:
        return [ major, minor ]

    return [ major, minor, revision, plearn_version ]

def vsystem( cmd, prefix='+++', quiet=False ):
    if quiet:
        os.system( "%s > /dev/null" % cmd )
    else:
        print prefix, cmd
        os.system( cmd )

class WString:
    """Writable String.

    Has a write() method.
    """
    def __init__( self, s='' ):
        self.s = s

    def __str__( self ):
        return self.s

    def write( self, s ):
        self.s += s
    
if __name__ == "__main__":

    def header( s ):
        print "==========\n\n",s,"\n----------"

    def eval_test( as_str ):
        print "\n",as_str
        print "###",eval( as_str )
        print

    header( "cross_product" )
    eval_test( "cross_product( ['a', 'b'], [1, 2, 3] )" )
    eval_test( "cross_product( ['a', 'b'], [1, 2, 3], "
               "joinfct = lambda s,i: '%s_%d' % (s, i) )"
               )
