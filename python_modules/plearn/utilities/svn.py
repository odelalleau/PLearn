import os
from   verbosity import vprint

def __report_status( cmd ):
    return os.WEXITSTATUS( os.system( cmd ) ) == 0
    
def add(path):
    add_cmd = "svn add -N %s" % path
    vprint("Adding: %s" % add_cmd, 2)    

    return __report_status( add_cmd )
    
def last_user_to_commit(file_path):
    raise NotImplementedError

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

## def remove( path ):
##     rm_cmd = "svn remove -N %s" % path
##     vprint("Removing: %s" % rm_cmd, 2)

##     return __report_status( rm_cmd )


    
