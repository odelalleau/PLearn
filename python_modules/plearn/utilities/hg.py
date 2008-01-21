import logging, os, sys, tempfile
from ppath import ppath

def __report_status( cmd ):
    if sys.platform == 'win32':
        # os.WEXITSTATUS is not implemented under Windows. However, on recent
        # Windows platform, this should be the command exit status.
        return os.system( cmd ) == 0
    else:
        return os.WEXITSTATUS( os.system( cmd ) ) == 0
    
def add(path):
    raise NotImplementedError
    add_cmd = "svn add -N %s" % path
    logging.debug("Adding: %s" % add_cmd)    

    return __report_status( add_cmd )

def commit(files, msg):
    raise NotImplementedError
    
    if isinstance( files, str ):
        files = [files]
    elif not isinstance(files, type([])):
        raise TypeError("The commit procedure accepts argument of type string of"
                        "array of string: type (%s) is not valid.\n" % type(files))
    
    commit_cmd = "svn commit -m '" + msg + "' " + " ".join(files)

    logging.info("\n+++ Commiting (from "+ os.getcwd() +"):\n" + commit_cmd)
    os.system(commit_cmd) 

def ignore( path, list_of_paths ):
    raise NotImplementedError
    
    get_ign_prop_cmd = "svn propget --strict svn:ignore %s" % path
    h = os.popen(get_ign_prop_cmd)
    already_ignored = h.readlines()
    h.close()
    to_ignore = []

    # Remove trailing carriage returns.
    for file in already_ignored:
        to_ignore.append(file.strip(os.linesep))

    for added in list_of_paths:
        if added not in to_ignore:
            to_ignore.append(added)

    to_ignore_full_string = ""
    for file in to_ignore:
        to_ignore_full_string += file + os.linesep
    propfile = tempfile.NamedTemporaryFile()
    propfile.write(to_ignore_full_string)
    propfile.flush()
    # Remove trailing carriage return.
    to_ignore_full_string = to_ignore_full_string.strip(os.linesep)
    ignore_cmd = "svn propset svn:ignore -F %s %s" % (propfile.name, path)
    result = __report_status( ignore_cmd )
    propfile.close()
    return result

def last_user_to_commit(file_path):
    raise NotImplementedError

def project_version(project_path, build_list):
    raise NotImplementedError

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
    raise NotImplementedError

    add_cmd = "svn add %s" % path
    logging.debug("Adding: %s" % add_cmd)

    return __report_status( add_cmd )

def recursive_remove(path, options=""):
    raise NotImplementedError

    rm_cmd = "svn remove %s %s"%(path, options)
    logging.debug("Removing: %s" % rm_cmd)

    return __report_status( rm_cmd )


def repository_revision( path ):
    "Return the hg repository version (as a string) within 'path'."
    try:
        current_path = os.getcwd()

        # First move to the given path, as svn does not always like absolute
        # directories.
        os.chdir(path)
        h = os.popen("hg identify | cut -d' ' -f1")

        # Move back to the previous current directory.
        os.chdir(current_path)

        x = h.readlines()
        h.close()
        return x[0].strip()
    except Exception, e:
        print 'Could not find hg revision number:', e
        return "NO_REVISION"

def update( path ):
    raise NotImplementedError

    up_cmd = "svn update %s" % path
    logging.debug("Updating: %s" % up_cmd)
    return __report_status( up_cmd )


## def remove( path ):
##     rm_cmd = "svn remove -N %s" % path
##     vprint("Removing: %s" % rm_cmd, 2)

##     return __report_status( rm_cmd )


    
