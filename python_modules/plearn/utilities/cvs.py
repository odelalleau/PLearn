__cvs_id__ = "$Id: cvs.py,v 1.3 2005/01/25 03:15:58 dorionc Exp $"

import os, popen2, string, types

from plearn.utilities.ppath     import cvs_directory
from plearn.utilities.verbosity import vprint

def add( path ):
    status = query("status", path, "Status: ")
    vprint(path + " status: " + status + "\n", 2)
    if status != '' and string.find(status, "Unknown") == -1:
        return False
    
    addCmd = "cvs add %s" % path 
    vprint("Adding: " + addCmd, 2)
    process = popen2.Popen4(addCmd)

    errors = process.fromchild.readlines()
    vprint("%s" % string.join( errors, '' ), 1)

    return True

def commit(files, msg):
    if isinstance(files, types.StringType):
        files = [files]
    elif not isinstance(files, type([])):
        raise TypeError("The commit procedure accepts argument of type string of"
                        "array of string: type (%s) is not valid.\n" % type(files))
    
    commit_cmd = ("cvs commit -m '" + msg + "' ")
    for f in files:
        commit_cmd += f + " " 
        
    vprint("\n+++ Commiting (from "+ os.getcwd() +"):\n" + commit_cmd, 1)
    commit_process = popen2.Popen4(commit_cmd)    

    errors = commit_process.fromchild.readlines()
    vprint("%s" % string.join( errors, '' ), 1)

def last_user_to_commit(file_path):
    """Returns username of the last person to commit the file corresponding to I{file_path}."""
    file_path = os.path.abspath(file_path)
    (dir, fname) = os.path.split(file_path)
    os.chdir(dir)

    author = "NEVER BEEN COMMITED"
    a = query("log", fname, "author: ", ";")
    if a != '':
        author = a
    
    return author

def query(option, fname, lookingFor, delim = "\n"):
    cvs_process = popen2.Popen4("cvs " + option + " " + fname)
    
    lines = cvs_process.fromchild.readlines()
    for line in lines :
        index = string.find(line, lookingFor)
        if index != -1:
            result = line[index+len(lookingFor):]
            result = result[:string.find(result, delim)]
            return string.rstrip(result)
    return ''

def recursive_add( path ):
    ## First add directory
    path_added = add( path )
    
    if path_added and os.path.isdir( path ):
        ## Then recursive_add each files within
        for sub_path in os.listdir( path ):
            if sub_path == cvs_directory:
                continue

            relative_path = os.path.join( path, sub_path )
            path_added = ( path_added and
                           recursive_add( relative_path ) )

    return path_added
    
def remove(file):
    status = query("status", file, "Status: ")
    if status == '' or string.find(status, "Unknown") != -1:
        return False

    rmCmd = "cvs remove " + file
    vprint("Removing: " + rmCmd, 2)
    process = popen2.Popen3(rmCmd, True)
    process.wait()
    
    errors = process.childerr.readlines()
    map(lambda err: vprint(err, 1), errors)
    return True

