import os
from ppath import ppath
from verbosity import vprint

def __report_status( cmd ):
    return os.WEXITSTATUS( os.system( cmd ) ) == 0
    
def add(path):
    add_cmd = "svn add -N %s" % path
    vprint("Adding: %s" % add_cmd, 2)    

    return __report_status( add_cmd )

def commit(files, msg):
    if isinstance( files, str ):
        files = [files]
    elif not isinstance(files, type([])):
        raise TypeError("The commit procedure accepts argument of type string of"
                        "array of string: type (%s) is not valid.\n" % type(files))
    
    commit_cmd = "svn commit -m '" + msg + "' " + " ".join(files)

    vprint("\n+++ Commiting (from "+ os.getcwd() +"):\n" + commit_cmd, 1)
    os.system(commit_cmd) 
    
def last_user_to_commit(file_path):
    raise NotImplementedError

def project_version(project_path, build_list):
    """Automatic version control.
    
    Builds are tuples of the form (major, minor, plearn_revision) where
    plearn_revision is the SVN revision of the PLearn project when the
    project's version was released.
    """    
    #plearn_version  = int(repository_revision(ppath('PLEARN')))
    #project_version = int(repository_revision(ppath(project_path)))
    version_number, release_version = build_list[-1]
    return "%s (Released on PL%d)"%(version_number, release_version)

def query(option, fname, lookingFor, delim = "\n"):
    raise NotImplementedError

def recursive_add( path ):
    add_cmd = "svn add %s" % path
    vprint("Adding: %s" % add_cmd, 2)    

    return __report_status( add_cmd )

def recursive_remove( path ):
    rm_cmd = "svn remove %s" % path
    vprint("Removing: %s" % rm_cmd, 2)

    return __report_status( rm_cmd )


def repository_revision( path ):
    "Return the SVN repository version (as a string) within 'path'."
    try:
        current_path = os.getcwd()

        # First move to the given path, as svn does not always like absolute
        # directories.
        os.chdir(path)
        h = os.popen("env LC_ALL=C svn info | grep 'Revision:' | cut -d' ' -f2")

        # Move back to the previous current directory.
        os.chdir(current_path)

        x = h.readlines()
        h.close()
        return x[0].strip()
    except Exception, e:
        print 'Could not find SVN revision number:', e
        return "NO_REVISION"


## def remove( path ):
##     rm_cmd = "svn remove -N %s" % path
##     vprint("Removing: %s" % rm_cmd, 2)

##     return __report_status( rm_cmd )


    
