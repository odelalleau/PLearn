import cvs, svn, os

from plearn.utilities.ppath     import cvs_directory, subversion_hidden
from plearn.utilities.verbosity import vprint

class VersionControlError( Exception ): pass

def get_vc_module():
    if os.path.isdir(cvs_directory):
        return cvs
    elif os.path.isdir(subversion_hidden):
        return svn
    else:
        raise VersionControlError("Could not figure out version control system to use!")


def add( path ):
    get_vc_module().add( path )

def commit( path, msg ):
    get_vc_module().commit( path, msg )

def recursive_add( path ):
    get_vc_module().recursive_add( path )

def recursive_remove( path ):
    get_vc_module().recursive_remove( path )
    
def is_under_version_control( path ):
    return ( os.path.isdir( os.path.join(path, cvs_directory) ) or
             os.path.isdir( os.path.join(path, subversion_hidden) ) )        
