"""Module containing miscellaneous helper functions.

Contains all functions that do fit in any other L{utilities}
submodule. If a considerable number of functions contained in this
module seems to manage similar tasks, it is probably time to create a
I{similar_tasks.py} L{utilities} submodule to move those functions to.
"""
import os, popen2, string, sys, time, types
import epydoc.markup 
import epydoc.markup.epytext

def command_output(command):
    process = popen2.Popen4( command )
    process.wait()
    return process.fromchild.readlines()

def cvs_add(file):
    status = cvs_query("status", file, "Status: ")
    vprint(file + " status: " + status + "\n", 2)
    if status != '' and string.find(status, "Unknown") == -1:
        return False
    
    addCmd = "cvs add " + file
    vprint("Adding: " + addCmd, 2)
    process = Popen3(addCmd, True)
    errors = process.childerr.readlines()
    map(lambda err: vprint(err, 1), errors)

    return True

def cvs_commit(files, msg):
    if isinstance(files, types.StringType):
        files = [files]
    elif not isinstance(files, type([])):
        raise TypeError("The cvs_commit procedure accepts argument of type string of"
                        "array of string: type (%s) is not valid.\n" % type(files))
    
    commitCmd = ("cvs commit -m '" + msg + "' ")
    for f in files:
        commitCmd += f + " " 
        
    vprint("\n+++ Commiting (from "+ os.getcwd() +"):\n" + commitCmd, 1)
    commitProcess = Popen3(commitCmd, True)
    vprint(commitProcess.childerr.read(1024), 1)

def cvs_query(option, fname, lookingFor, delim = "\n"):
    #print fname
    cvsProcess = Popen3("cvs " + option + " " + fname, True)
    lines = cvsProcess.fromchild.readlines()
    #print lines
    for line in lines :
        #print line
        index = string.find(line, lookingFor)
        #print("string.find(" + line + ", " + lookingFor + ") : ")
        #print index
        if index != -1:
            result = line[index+len(lookingFor):]
            result = result[:string.find(result, delim)]
            return string.rstrip(result)
    return ''

def cvs_remove(file):
    status = cvs_query("status", file, "Status: ")
    if status == '' or string.find(status, "Unknown") != -1:
        return False

    rmCmd = "cvs remove " + file
    vprint("Removing: " + rmCmd, 2)
    process = Popen3(rmCmd, True)
    errors = process.childerr.readlines()
    map(lambda err: vprint(err, 1), errors)
    return True

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

def isvmat( file_path ):
    """True if the extension of I{file_path} is one of I{.amat}, I{.pmat} or I{.vmat}."""
    (base,ext) = os.path.splitext(file_path)
    return ext in [ '.amat','.pmat','.vmat' ]
    
    
def last_user_to_commit(file_path):
    """Returns username of the last person to commit the file corresponding to I{file_path}."""
    file_path = os.path.abspath(file_path)
    (dir, fname) = os.path.split(file_path)
    os.chdir(dir)

    author = "NEVER BEEN COMMITED"
    a = cvs_query("log", fname, "author: ", ";")
    if a != '':
        author = a
    
    return author

def quote(s):
    if string.find(s, '\n') != -1:
        return '"""%s"""' % s
    return '"%s"' % s

def short_doc(obj):
    return doc(obj, True)
