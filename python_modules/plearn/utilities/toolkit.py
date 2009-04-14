"""Module containing miscellaneous helper functions.

Contains all functions that do fit in any other L{utilities}
submodule. If a considerable number of functions contained in this
module seems to manage similar tasks, it is probably time to create a
I{similar_tasks.py} L{utilities} submodule to move those functions to.
"""
import inspect, os, shutil, string, subprocess, sys, time, types
from os.path import exists, join, abspath
from string import split

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

def command_output(command, stderr = True, stdout = True):
    """Returns the output lines of a shell command.    
    
    @deprecated Please use directly the subprocess module instead.

    @param command: The shell command to execute.
    @type command: String

    @param stderr: Whether or not to include the standard error.
    @type command: Boolean

    @param stdout: Whether or not to include the standard output.
    @type command: Boolean

    @return: Output lines.
    @rtype:  Array of strings.
    """
    if stderr and stdout:
        p = subprocess.Popen(command, stdout = subprocess.PIPE,
                stderr = subprocess.STDOUT, shell = True)
        return p.stdout.readlines()
    else:
        p = subprocess.Popen(command, stdout = subprocess.PIPE,
                stderr = subprocess.PIPE, shell = True)
        if stderr:
            return p.stderr.readlines()
        elif stdout:
            return p.stdout.readlines()
        else:
            return ''

def breakpoint( msg, cond=True ):
    if cond:
        raw_input( msg )
    
def copytree( src, dst, symlinks = True, ignore = [] ):
    """Recursively copy a directory. This function behaves similarly to
    shutil.copytree, except when 'ignore' is provided: in this case, any file
    or directory that matches (exactly) an element of 'ignore' will be skipped.
    """
    if ignore == []:
        shutil.copytree(src, dst, symlinks)
    else:
        if os.path.islink(src):
            if symlinks:
                raise "Not implemented"
            else:
                raise "Not implemented"
        elif os.path.isfile(src):
            shutil.copy2(src, dst)
        elif os.path.isdir(src):
            os.mkdir(dst)
            list_in_dir = os.listdir(src)
            for item in list_in_dir:
                if item not in ignore:
                    copytree( os.path.join(src, item), \
                              os.path.join(dst, item), symlinks, ignore )

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

def doc(obj, docform = 2, join_token = '\n    '):
    """Return documentation associated with an object.

    If the object does not have any documentation, return the empty string.
    Otherwise return the object documentation, in a form that depends on
    doc form.  If docform==0, the first line (short-form) of the
    documentation is returned.  If docform==1, the body of the
    documentation is returned, without the short-form.  If docform==2, the
    whole documentation is returned (both short-form and body).
    """
    docstr = obj.__doc__    
    if docstr is None:
        return ''

    lines = string.split(docstr, '\n')
    if docform==0:
        lines = [ lines[0] ]
    elif docform==1:
        ## Determine a logical starting point that skips blank lines after
        ## the first line of documentation
        #for i in range(1,len(lines)):
        i= 1
        while i < len(lines):
            if lines[i].strip() != "":
                break
            i+= 1
        lines = lines[i:]
    elif docform==2:
        pass
    else:
        raise ValueError,"Argument to 'docform' must be either 0, 1, or 2"
    
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

    as_string = string.join( parsed_lines, join_token )
    return as_string

def exec_path_exists(path):
    """Return True iff the given path is an existing file.
    Under Windows, also returns True when the given path does not actually exist
    but 'path.exe' does.
    """
    if os.path.exists(path):
        return True
    elif sys.platform == 'win32' and os.path.exists(path + '.exe'):
        return True
    else:
        return False

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
    """True if the extension of I{file_path} matches a standard vmat extension.

    Understood extensions: I{.amat}, I{.dmat}, I{.tkmat}, I{.pmat},
    I{.pymat} or I{.vmat}.
    """
    (base,ext) = os.path.splitext(file_path)
    return ext in [ '.amat', '.dmat', '.tkmat', '.pmat', '.pymat', '.vmat' ]

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

def splitDirs(path):
    """
    returns a list of path elements, by successive calls to os.path.split
    """
    dirs= []
    h= 123
    while h not in ['','/']:
        h,t= os.path.split(path)
        dirs= [t]+dirs
        path= h
    if h=='/':
        dirs= [h]+dirs

    return dirs

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
    return doc(obj, 0)

def subset(list1, list2):
    """Returns True if all elements in list1 are elements of list2."""
    for elem in list1:
        if not elem in list2:
            return False
    return True

def symlink( to_path, from_path, force = False, is_exe = False ):
    """Symbolic link to a disk resource.
    This function creates a symbolic link 'from_path' to 'to_path'.
    Under Linux, a regular symbolic link is created, using os.symlink.
    Under Windows, it is actually a copy of the resource that is performed, if
    there is no existing copy already in place that looks like an exact copy of
    the target 'to_path' (by exact copy, we mean same size and timestamp).
    If 'force' is set to True, an existing file, directory or link with name
    'from_path' will be erased, otherwise the function will raise an error if
    such a file, directory or link already exists.
    If 'is_exe' is set to true, then the '.exe' extension will be appended to
    the copy, unless it already exists (only under Windows).

    @param  to_path: The target of the link
    @type   to_path: String

    @param  from_path: The path of the link
    @type   from_path: String

    @param  force: Whether or not we delete an existing file
    @type   force: Boolean

    @param  is_exe: Whether or not the file is executable
    @type   is_exe: Boolean

    @returns: True for success, False for failure.
    @rtype: Boolean
    """
    if sys.platform == "win32":
        import shutil
        if os.path.isdir(to_path):
            # TODO Implement for directories.
            raise "Not implemented yet for directories"
        if is_exe and not from_path.endswith(".exe"):
            from_path += ".exe"
        if os.path.isfile(from_path) or os.path.isdir(from_path):
            if os.path.getmtime(from_path) == os.path.getmtime(to_path) and \
               os.path.getsize(from_path)  == os.path.getsize(to_path):
                # There is an existing resource with same size and timestamp:
                # we can reuse it!
                return True
            if force:
                if os.path.isdir(from_path):
                    os.rmdir(from_path)
                else:
                    os.remove(from_path)
            else:
                raise "File exists: " + from_path
        shutil.copy(to_path, from_path)
        # Set the timestamp of the copied file to the one of the original file,
        # so that it can recognize later it is the same (and thus avoid a
        # useless and costly copy).
        os.utime(from_path,
                 (os.path.getatime(to_path), os.path.getmtime(to_path)))
        return os.path.isfile(from_path)
    else:
        if os.path.islink(from_path) or os.path.isfile(from_path) \
                                     or os.path.isdir(from_path):
            if force:
                if os.path.isdir(from_path):
                    os.rmdir(from_path)
                else:
                    os.remove(from_path)
            else:
                raise "File exists: " + from_path
        os.symlink(to_path, from_path)
        return os.path.islink(from_path)
 
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

#original version: http://aspn.activestate.com/ASPN/Cookbook/Python/Recipe/52224
def search_file(filename, search_path):
    """Given a search path, find file
     Can be used with the PATH variable as search_path
    """
    file_found = 0
    paths = split(search_path, os.pathsep)
    for path in paths:
        if exists(join(path, filename)):
            file_found = 1
            break
    if file_found:
        return abspath(join(path, filename))
    else:
        return None
    
class ListMap(dict):
    def __getitem__(self, key):
        if not key in self:
            self.__setitem__(key, [])
        return super(ListMap, self).__getitem__(key)

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
