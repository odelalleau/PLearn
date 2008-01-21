"""PLearn::PPath emulation.

Except for helper functions, all functions and methods in this module are
copied or strongly inspired of plearn/io/PPath.{h,cc}.

@var  skeldir: The name of the directory used to store skeletons of
code to be managed by I{pyskeleton}. [Special directory]
@type skeldir: String.

@var  pymake_objs: The name of the directory used by pymake to store
compilation results. [Special directory]
@type pymake_objs: String.

@var  pymake_hidden: The name of the hidden directory used by pymake to
store its configuration files.
@type pymake_hidden: String.

@var  cvs_directory: The name of the directory used by cvs to store its
internal state. [Special directory]
@type cvs_directory: String.

@var  pytest_dir: The name of the directory used by pytest to store
test results. [Special directory]
@type pytest_dir: String.

@var special_directories: An array of directory names that are
considered to be internal to specific PLearn (or related)
applications. 
"""
__version_id__ = "$Id$"

import copy, os, string

from plearn.utilities.Bindings import *

__all__ = [
    ## Module Variables
    "home", "plearn_configs", "skeldir", 
    "pymake_objs", "pymake_hidden", "cvs_directory", 
    "pytest_dir", "special_directories", 

    ## Helper Functions
    "exempt_of_subdirectories", "expandEnvVariables",
    
    ## Binding Functions
    "add_binding", "ppath", "write_bindings",     
    ]

########################################
##  Module Variables  ##################
########################################

## Under Windows, the user must define its home prior to running this setup
home                   = os.environ.get( 'HOME', "" )

## The main PLearn     environment variable: PLEARN_CONFIGS
plearn_configs         = os.environ.get( 'PLEARN_CONFIGS',
                                         os.path.join(home, '.plearn') )

## The file path to     the ppath_config file
config_file_path       = os.path.join( plearn_configs, "ppath.config" )

## Probably should be moved some day
skeldir                = "Skeletons"
pymake_objs            = "OBJS"
pymake_hidden          = ".pymake"
cvs_directory          = "CVS"
subversion_hidden      = ".svn"
hg_hidden              = ".hg"
pytest_dir             = ".pytest"

special_directories    = [ skeldir,
                           pymake_objs,   pymake_hidden,
                           cvs_directory, subversion_hidden, hg_hidden,
                           pytest_dir     ]


########################################
##  Helper Functions  ##################
########################################

def get_domain_name():
    from socket import getfqdn
    host_name = getfqdn()
    if host_name == 'localhost':
        return ''
    i = host_name.find('.')
    assert i != -1, "getfqdn didn't return a fully-qualified name (no '.')."
    return host_name[i+1:]

def exempt_of_subdirectories( directories ):
    """Remove any path in list that is a subdirectory of some other directory in the list.

    @param directories: The list exempt.
    @type  directories: ListType
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

def expandEnvVariables( path ):
    expanded = copy.copy(path)
    
    begvar = string.find(expanded, "${")
    endvar = string.find(expanded,  "}")
  
    while begvar != -1 and endvar != -1:        
        start    = begvar+2;
        envvar   = expanded[start:endvar]
        envpath  = os.environ.get(envvar, "");

        if envpath == "":
            raise ValueError( "Unknown environment variable %s in %s."
                              % (envvar, path)
                              )

        expanded = expanded.replace("${%s}"%envvar, envpath);

        ## Look for other environment variables
        begvar = string.find(expanded, "${")
        endvar = string.find(expanded,  "}")

    return expanded;

########################################
##  Binding Functions  #################
########################################

class PPathBindings( Bindings ):
    """Emulates the PLearn::PPath object behavior."""
    config_file = None

    def load(cls):
        bindings = PPathBindings()
        if os.path.exists(config_file_path):
            for line in file(config_file_path, "r"):
                line = line.strip()
                if len(line) == 0:
                    continue

                mprotocol, mpath = line.split(None, 1)
                # Remove matched double quotes if present
                if mpath[0] == '"' and mpath[-1] == '"':
                    mpath = mpath[1:-1]
                bindings[mprotocol] = mpath

        return bindings        
    load = classmethod(load)

    def config_file_writer(cls):
        ## Creating the plearn_configs directory
        if not os.path.exists( plearn_configs ):
            os.makedirs( plearn_configs )

        ## Backing up the old configuration
        if os.path.exists( config_file_path ):
            backup = "%s.backup" % config_file_path
            os.system("mv %s %s" % (config_file_path, backup))
            print "Older %s found:\n    Moved to %s." % (config_file_path, backup)

        cls.config_file = open(config_file_path, "w")
        return cls.config_file.write
    config_file_writer = classmethod(config_file_writer)

    def close_writer(cls):
        if cls.config_file is not None:
            cls.config_file.close()
    close_writer = classmethod(close_writer)
    
    def __setitem__(self, key, canonic_path):
        if key in self.ordered_keys:
            ## Preserving order but avoiding metaprotocol loops
            Bindings.__setitem__(self, key, "")
##         print
##         raw_input( (key, canonic_path) )

        ## The environment variables within the path are replaced by their value
        canonic_path = expandEnvVariables( canonic_path )
        
        metaprotocol = None
        metapath     = "" 
        for mprotocol, mpath in self.iteritems():
            begpath = string.find(canonic_path, mpath);
            ##raw_input( "%s, %s: beg=%d" % (mprotocol, mpath, begpath) )

            ## The path does not start with the current candidate or is shorter
            ## than the previous metapath found.
            if begpath != 0 or len(mpath) < len(metapath):
                continue

            endpath = len(mpath);
            ##raw_input( "%s, %s: end=%d" % (mprotocol, mpath, endpath) )

            ## The current candidate is only a subtring of the canonic path.
            ## Ex:
            ##    /home/dorionc/hey
            ## dans
            ##    /home/dorionc/heyYou. 
            if endpath != len(canonic_path) and canonic_path[endpath] != '/':
                continue

            ##raw_input( "%s, %s: after end" % (mprotocol, mpath) )

            ## The current candidate is indeed a subpath of canonic_path.
            metaprotocol = mprotocol;
            metapath     = mpath;


        ##raw_input( "Kept: %s, %s" % (metaprotocol, metapath) )
        
        ## If any metapath was found, it must be replaced by its metaprotocol
        ## equivalent.
        if len(metapath) > 0:
          if len(canonic_path) == len(metapath):
            canonic_path = canonic_path.replace( metapath, metaprotocol+':' );

          ## Replace the '/' by a ':' (metapath never contain trailingSlash --
          ## see ensureMappings)
          else: 
            canonic_path = canonic_path.replace( metapath+'/', metaprotocol+':' );

        ## We may now call the parent setitem version
        Bindings.__setitem__(self, key, canonic_path)
        
bindings = PPathBindings.load()

def add_binding(mprotocol, mpath):
    bindings[mprotocol] = mpath

def ppath(path):
    # It may be that many protocols follow each other: use rfind
    end_of_metaprotocol = path.rfind(':')

    if end_of_metaprotocol == -1:
        candidate_ppath = path
        basepath        = ""
    else:
        candidate_ppath = path[:end_of_metaprotocol]
        basepath        = path[end_of_metaprotocol+1:]
        
    try:            
        path = os.path.join(__ppath(candidate_ppath), basepath)
    except KeyError:
        fallback = os.environ.get(candidate_ppath, "")
        if fallback:
            path = os.path.join(fallback, basepath)

    return path
    
def __ppath(metaprotocol):
    metapath = None    
    try:
        #print("*** Looking for ppath %s" % metaprotocol)
        metapath = bindings[metaprotocol]
    except KeyError:
        raise KeyError("The %s metaprotocol does not exists. "
                       "Existing mappings are: %s"
                       % ( metaprotocol, string.join(bindings.keys(), ", ") )
                       )

    endpr = protocol_end( metapath )
    while endpr != -1:
        mprotocol = metapath[:endpr]

        if bindings.has_key(mprotocol):
            ## Keeping the rest of the metapath
            after_colon = ""
            if endpr+1 < len(metapath):
                after_colon = metapath[endpr+1:]

            ## Replacing the MPROTOCOL:after_colon by MPath/after_colon
            metapath = os.path.join( bindings[mprotocol], after_colon )
            endpr    = protocol_end( metapath )
        else:
            raise KeyError('Could not resolve %s', mprotocol)

    return metapath

def protocol_end( mpath ):
    ## DOS absolute paths contain ':' !!!
    if os.path.isabs( mpath ):
        return -1

    ## Looking for the next metaprotocol
    return string.find(mpath, ':')
    
def remove_binding(mprotocol):
    """Remove a metaprotocol to metapath binding.

    @param mprotocol: The metaprotocol key to remove from bindings.
    @type  mprotocol: String.

    @returns: The metapath that was associated to the remove metaprotocol.
    @rtype: String.
    """
    mpath = bindings[mprotocol]
    del bindings[mprotocol]
    return mpath

def write_bindings( writer = None ):
    if writer is None:
        writer = PPathBindings.config_file_writer()
    
    for (metaprotocol, metapath) in bindings.iteritems():
        if metaprotocol == "HOME":
            metapath = "${HOME}"
            
        if metapath != "":
            binding = '%s "%s"\n' % ( string.ljust(metaprotocol, 49),
                                  metapath.replace('\\', '\\\\')
                                  )
            writer(binding)

    ## Will close the writer iff it was provided by the PPathBindings'
    ## classmethod config_file_writer()
    PPathBindings.close_writer()

if __name__ == "__main__":
    print "As a dictionary:\n   ",bindings

    print "\nAs a config file:"
    def writer(line):
        print "    %s" % line,
    write_bindings(writer)

    print
    print string.ljust("As Path", 49),"Exists?\n"
    for mprotocol in bindings.iterkeys():
        mpath  = ppath(mprotocol)
        print string.ljust(mpath, 49),os.path.exists(mpath)
