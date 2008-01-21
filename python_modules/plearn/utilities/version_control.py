import cvs, svn, os, hg

from plearn.utilities.ppath     import cvs_directory, subversion_hidden, hg_hidden
from plearn.utilities.verbosity import vprint

class VersionControlError( Exception ): pass

def get_vc_module():
    if os.path.isdir(cvs_directory):
        return cvs
    elif os.path.isdir(subversion_hidden):
        return svn
    elif os.path.isdir(hg_hidden):
        return hg
    else:
        raise VersionControlError("Could not figure out version control system to use!")


def add( path ):
    get_vc_module().add( path )

def commit( path, msg ):
    get_vc_module().commit( path, msg )

def recursive_add( path ):
    get_vc_module().recursive_add( path )

def recursive_remove(path, options=""):
    get_vc_module().recursive_remove(path, options)
    
def is_under_version_control( path ):
    return ( os.path.isdir( os.path.join(path, cvs_directory) ) or
             os.path.isdir( os.path.join(path, subversion_hidden) ) )        

# Ignore in 'path' all files given by 'list_of_paths'.
def ignore( path, list_of_paths ):
    get_vc_module().ignore( path, list_of_paths )

# Update the given path.
def update( path ):
    get_vc_module().update( path )

# Return current revision of repository in 'repository_path'.
# The current revision is stored in the file
# 'repository_revision_dir'/'repository_name'_'svn or cvs'_repository_revision
# Finally, if (and only if) the previously stored revision is not the same as
# the current one, the file 'revision_file_path' is touched.
def update_repository_revision( repository_name, repository_path,
                                repository_revision_dir, revision_file_path ):
    old_path      = os.getcwd()
    os.chdir(repository_path)    # Necessary before calling get_vc_module()
    vc_module     = get_vc_module()
    vc_name       = vc_module.__name__.split('.')[-1]
    current_rev   = vc_module.repository_revision( repository_path )
    rev_file_path = os.path.join(repository_revision_dir,
                                 repository_name + '_' + vc_name + '_repository_revision')
    is_up_to_date = False
    if os.path.isfile(rev_file_path):
        try:
            rev_file = open(rev_file_path)
            last_rev = rev_file.read().strip()
            rev_file.close()
            if last_rev == current_rev:
                is_up_to_date = True
        except:
            pass

    if not is_up_to_date:
        rev_file = open(rev_file_path, "w")
        rev_file.write( str(current_rev) )
        rev_file.close()
        os.utime(revision_file_path, None)

    os.chdir(old_path)    # Get back to previous directory

    return str(current_rev)

