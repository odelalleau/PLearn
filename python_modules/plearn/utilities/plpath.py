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

@var pytest_variables: The paths to the user-specific pytest
variables file. The value is set to os.path.join( plearn_scripts, '.pytest_variables' ).
@type pytest_variables: String
"""
import os, string

home                   = os.getenv('HOME')

plearndir              = os.getenv( 'PLEARNDIR',
                                    os.path.join(home, 'PLearn') )
lisaplearndir          = os.getenv( 'LISAPLEARNDIR',
                                    os.path.join(home, 'LisaPLearn') )
apstatdir              = os.getenv( 'APSTATDIR',
                                    os.path.join(home, 'apstatsoft') )

plbranches             = { plearndir     : 'PLearn',
                           lisaplearndir : 'LisaPLearn',
                           apstatdir     : 'apstatsoft'  }

plearn_scripts         = os.path.join( plearndir, 'scripts' )

pytest_variables       = os.path.join( plearn_scripts, '.pytest_variables' )


def user_independant_path(directory):
    """
    Éventuellement, il faut que je fasse quelque chose pour les tests qui ne sont sous aucune branche...
    """
    pass

def path_in_branches(directory, return_tuple=False):
    path = None
    for plbranch in plbranches.iterkeys():
        try:
            path = path_in_branch( plbranch, directory )
            
        except ValueError:
            pass
        else:
            if return_tuple:
                path = ( plbranch, path )
            else:
                path = plbranches[plbranch] + path
            break

    if path is None:
        raise ValueError("Directory %s is under none of the plearn branches.")
        
    return path
    
def path_in_branch(plbranch, directory):
    d = os.path.abspath(directory)
    i = string.find(d, plbranch)
    if i == -1:
        raise ValueError( "Directory %s is not under the %s directory."
                          % (directory, plbranches[plbranch]) )
    assert i == 0    
    return d[len(plearndir):]
    
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
