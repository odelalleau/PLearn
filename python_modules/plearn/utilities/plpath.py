"""Utilities to manage paths under the PLearn architecture.

Defines some PLearn path variables and somehow extends
U{os<http://www.python.org/doc/current/lib/module-os.html>} and
U{os.path<http://www.python.org/doc/current/lib/module-os.path.html>}.

In the B{Class Variable Details}, all values are to be taken as
examples since the values are user dependent.

@var  home: For convinience; the user's home.
@type home: String

@var plearndir: The PLearn directory absolute path.
The value of plearndir is the value of the PLEARNDIR environment
variable, if it exists. Otherwise, it is set to os.path.join(home, 'PLearn').
@type plearndir: String

@var lisaplearndir: The LisaPLearn directory absolute path.
The value of lisaplearndir is the value of the LISAPLEARNDIR environment
variable, if it exists. Otherwise, it is set to os.path.join(home, 'LisaPLearn').
@type lisaplearndir: String

@var apstatdir: The apstatsoft directory absolute path.
The value of apstatdir is the value of the APSTATDIR environment
variable, if it exists. Otherwise, it is set to os.path.join(home, 'apstatsoft').
@type apstatdir: String

@var plbranches: We call plearn branches the PLearn, LisaPLearn and apstatsoft libraries.
@type plearndir: Dictionnary; branch path (string) to branch name (string).

@var plearn_scripts: Simply os.path.join( plearndir, 'scripts' ).
@type plearn_scripts: String

@var  pymake_objs: The name of the directory used by pymake to store
compilation results.
@type pymake_objs: String.

@var  cvs_directory: The name of the directory used by cvs to store its
internal state.
@type cvs_directory: String.

@var  pytest_dir: The name of the directory used by pytest to store
test results.
@type pytest_dir: String.
"""
import os, string

home                   = os.getenv('HOME')

plearndir              = os.getenv( 'PLEARNDIR',
                                    os.path.join(home, 'PLearn') )
lisaplearndir          = os.getenv( 'LISAPLEARNDIR',
                                    os.path.join(home, 'LisaPLearn') )
apstatdir              = os.getenv( 'APSTATDIR',
                                    os.path.join(home, 'apstatsoft') )

env_mappings           = { home                : "$HOME",
                           plearndir           : "$PLEARNDIR",
                           lisaplearndir       : "$LISAPLEARNDIR",
                           apstatdir           : "$APSTATDIR"
                           }

plbranches             = { plearndir     : 'PLearn',
                           lisaplearndir : 'LisaPLearn',
                           apstatdir     : 'apstatsoft'  }

plearn_scripts         = os.path.join( plearndir, 'scripts' )

pymake_objs            = "OBJS"
pymake_hidden          = ".pymake"
cvs_directory          = "CVS"
pytest_dir             = "pytest"

special_directories    = [ pymake_objs,   pymake_hidden,
                           cvs_directory, pytest_dir    ]

def exempt_of_subdirectories( directories ):
    """Remove any path in list that is a subdirectory of some other directory in the list.

    @param list: The list exempt.
    @type  list: ListType
    """    
    ldirs = len(directories)
    to_remove = []
    for i in range(ldirs):
        for j in range(ldirs):
            if i==j:
                continue
            if string.find(directories[i], directories[j]) != -1:
                to_remove.append(directories[i])

    for r in to_remove:
        directories.remove(r)
    return directories


def user_independant_path(directory):
    """
    Eventuellement, il faut que je fasse quelque chose pour les tests qui ne sont sous aucune branche...
    """
    pass

def keep_only(dir, to_keep, hook=None):
    """Given a directory, this function removes all files and directories except I{to_keep}.

    @param dir: A valid directory name.
    @type  dir: String

    @param to_keep: A valid I{dir} subdirectory or I{dir}/file name.
    @type  to_keep: String

    @param hook: The I{hook} function is called on all directory/file name after removal.
    @type  hook: Any callable function accepting a string as single argument.
    """
    dirlist = os.listdir(dir)
    for file in dirlist:
        if file != to_keep:
            fpath = os.path.join(dir, file)
            if os.path.isdir(fpath):
                keep_only(fpath, to_keep, hook)
            else:
                os.remove(fpath)
                if callable(hook):
                    hook(fpath)

def path_in_branches(directory, return_tuple=False):
    """Returns the path the I{directory} within the appropriate plearn branch.

    Will raise a L{ValueError} if the directory is not under any plearn branch.

    @param directory: A valid directory path. 
    @type  directory: String.

    @param return_tuple: If True, the method returns a (branch,
    path_in_branch) tuple. Else, it returns
    'branch/path_in_branch'. Default: False.
    @type  return_tuple: bool

    @returns: If I{return_tuple} is True, the method returns a
    (branch, path_in_branch) tuple. Else, it returns
    'branch/path_in_branch'.
    """
    path = None
    for plbranch in plbranches.iterkeys():
        try:
            path = path_in_branch( plbranch, directory,
                                   (not return_tuple) )
            
        except ValueError:
            pass
        else:
            if return_tuple:
                path = ( plbranch, path )
            break

    if path is None:
        raise ValueError("Directory %s is under none of the plearn branches."
                         % directory )
        
    return path
    
def path_in_branch(plbranch, directory, prepend_branch=True):
    """Returns the path the I{directory} within the provided I{plbranch}.

    Will raise a L{ValueError} if the directory is not under I{plbranch}.

    @param plbranch: A valid PLearn branch.
    @type  plbranch: String.

    @param directory: A valid directory path. 
    @type  directory: String.

    @param prepend_branch: If True, the method returns
    'branch/path_in_branch'. Else it returns only 'path_in_branch'.
    Default: True.
    @type  prepend_branch: bool

    @return: If I{prepend_branch} is True, the method returns
    'branch/path_in_branch'. Else it returns only 'path_in_branch'.
    """
    d = os.path.abspath(directory)
    i = string.find(d, plbranch)
    if i == -1:
        raise ValueError( "Directory %s is not under the %s directory."
                          % (directory, plbranches[plbranch]) )
    assert i == 0
    if prepend_branch:
        return os.path.join(plbranch, d[len(plearndir):])
    return d[len(plearndir):]

def plcommand(command_name):
    """The absolute path to the command named I{command_name}.

    @param command_name: The name of a command the user expect to be
    found in the 'commands' directory of one of the plearn branches.
    @type  command_name: String

    @return: A string representing the path to the command. The
    function returns None if the command is not found.
    """
    command_path = None
    for plbranch in plbranches.iterkeys():
        cmd_path = os.path.join(plbranch, 'commands', command_name)
        path     = cmd_path+'.cc'
        if os.path.exists( path ):
            command_path = cmd_path
            break

    return command_path


def process_with_mappings( path, mappings ):
    ## Do not process links
    if os.path.islink( path ):
        return

    ## For directories, recursively process subpaths.
    if os.path.isdir( path ):
        for subpath in os.listdir( path ):
            if subpath in [pymake_objs, pymake_hidden, cvs_directory]:
                continue
            
            relative_path = os.path.join( path, subpath )
            process_with_mappings( relative_path, mappings )
        return

    fcontent  = file( path ).readlines()
    key_paths = mappings.keys()
    key_paths.sort( lambda r, s: len(s)-len(r) )

    processed = open( path, 'w' )
    for line in fcontent:
        for key in key_paths:
            ##raw_input( "LINE: %s\nKEY: %s \n\n" % (line, key) )
            line = string.replace(line, key, mappings[key])            
        processed.write(line)
    processed.close()
    
def splitprev(dir):
    """Returns the directory of which I{dir} is a subdirectory.

    @param dir: A directory name.
    @type  dir: String

    @return: The directory of which I{dir} is a subdirectory. If
    I{dir} doesn't end by '/', then the result will be the same than
    L{os.path.split}(I{dir})
    """
    if dir[-1] == "/":
        dir.pop()
    return os.path.split(dir)
