__version_id__ = "$Id$"

import os, string, subprocess, types

from plearn.utilities.ppath     import cvs_directory
from plearn.utilities.verbosity import vprint

def run_cmd(cmd):
    """
    Return Popen object that corresponds to running the given command through
    the shell.
    """
    return subprocess.Popen(cmd, shell = True, stdout = subprocess.PIPE,
            stderr = subprocess.PIPE)

def add( path ):
    addCmd = "cvs add %s" % path 
    vprint("Adding: " + addCmd, 2)
    process = run_cmd(addCmd)
    errors = process.stderr.readlines()
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
    commit_process = run_cmd(commit_cmd)

    errors = commit_process.stderr.readlines()
    vprint("%s" % string.join( errors, '' ), 1)

def ignore( path, list_of_paths ):
    raise NotImplementedError

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
    cvs_process = run_cmd("cvs " + option + " " + fname)
    
    lines = cvs_process.stdout.readlines()
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

def recursive_remove(path, options=""):
    rm_cmd = "cvs remove -Rf " + path
    vprint("Removing: " + rm_cmd, 2)
    os.system( rm_cmd )
    
    return True

def update( path ):
    raise NotImplementedError
    
## def remove( path ):
##     rm_cmd = "cvs remove " + path
##     vprint("Removing: " + rm_cmd, 2)
##     os.system( rm_cmd )
    
##     return True

