import logging, os
from plearn.utilities import ppath
from plearn.utilities.toolkit import exempt_list_of, re_filter_list

DIR_STACK = []

def cd( path = None ):
    """Emulate shell's cd --- forwards the call to os.chdir().

    Mainly called as cd() as a shorthand to os.chdir( os.environ.get('HOME') ).  
    """
    if path is None:
        path = ppath.ppath('HOME')
    os.chdir( path )

def compare_trees(former, later, ignored_files_re=["\.svn", ".\.metadata"]):
    """Compare the directory tree starting at \I{former} to the one starting at I{later}.
    
    Returns tree lists containing 

        1) (former_file, later_file) pairs of paths of common files
        2) paths of directories or files unique to former
        3) paths of directories or files unique to later

    Paths in the lists will be relative or absolute following the form in
    which I{former} and I{later} are. Files are listed in 'topdown' order.
    """
    assert os.path.isdir(former)
    assert os.path.isdir(later)

    common_files = []
    unique_to_former = []
    unique_to_later = []
    for former_dirpath, former_subdirs, former_files in os.walk(former):
        later_dirpath = former_dirpath.replace(former, later, 1)
        later_subdirs, later_files = split_listdir(later_dirpath)
        
        # Filters the subdirs given the 'ignored_files_re' argument
        re_filter_list(former_subdirs, ignored_files_re)
        re_filter_list(later_subdirs, ignored_files_re)

        ######  Directories  #########################################################

        uniquedirs = []
        for subdir in former_subdirs:
            if subdir in later_subdirs:                
                # can be modified without effects, unlike former_subdirs
                later_subdirs.remove(subdir)
            else:
                uniquedirs.append(subdir)
                unique_to_former.append(os.path.join(former_dirpath, subdir))

        # Avoiding recursion in 'uniquedirs'
        exempt_list_of(former_subdirs, uniquedirs)
        
        # later_subdirs should now contain only dirs that were not in former_subdirs
        for subdir in later_subdirs:
            unique_to_later.append(os.path.join(later_dirpath, subdir))

        ######  Files  ###############################################################

        for fname in former_files:
            if fname in later_files:
                later_files.remove(fname)
                common_files.append(( os.path.join(former_dirpath,fname),
                                      os.path.join(later_dirpath,fname) ))

            else: # if not os.path.islink(fname):
                unique_to_former.append(os.path.join(former_dirpath, fname))

        # later_files should now contain only files that were not in former_files
        for fname in later_files:
            unique_to_later.append(os.path.join(later_dirpath, fname))

    return common_files, unique_to_former, unique_to_later

def is_recursively_empty(directory):
    """Checks if the I{directory} is a the root of an empty hierarchy.

    @param directory: A valid directory path
    @type  directory: StringType

    @return: True if the I{directory} is a the root of an empty
    hierarchy. The function returns False if there exists any file or
    link that are within a subdirectory of I{directory}.
    """
    for path in os.listdir(directory):
        relative_path = os.path.join(directory, path)
        if ( not os.path.isdir(relative_path) or 
             not is_recursively_empty(relative_path) ):
            return False
    return True
    
def ls( path = None ):
    """Emulate shell's ls --- forwards the call to os.listdir().

    Mainly called as ls() as a shorthand to os.listdir( os.getcwd() ).  
    """
    if path is None:
        path = os.getcwd()
    return os.listdir( path )

def path_type(path=None):
    TYPES = { "link" : os.path.islink,
              "dir"  : os.path.isdir,
              "file" : os.path.isfile }
    if path is None:
        return TYPES.keys()
    
    for typ, check in TYPES.iteritems():
        if check(path):
            return typ
    raise ValueError("Invalid path '%s'."%path)
    
def pushd( path = None ):
    if path is None:
        path = ppath.ppath('HOME')
    
    DIR_STACK.append( os.getcwd() )
    os.chdir( path )

def popd( ):
    os.chdir( DIR_STACK.pop() )
    
def pwd( ):
    print os.getcwd( )

def relative_link( src, dest ):
    src  = os.path.realpath( src )
    dest = os.path.realpath( dest )
    common = os.path.commonprefix([ src, dest ])

    # Even if common is a directory, it does no exclude that it may not be
    # a valid common path:
    #
    #   src:  /home/dorionc/tmp/file.txt
    #   dest: /home/dorionc/tmp2/file.txt
    #
    # will have '/home/dorionc/tmp' common while the common path is clearly
    # '/home/dorionc'. Hence, we always take the dirname.
    common = os.path.dirname( common )
    pushd( common )

    relsrc  = relative_path( src, common )
    reldest = relative_path( dest, common )
    if os.path.isdir( reldest ):
        if reldest.endswith('/'):
            reldest = reldest[:-1]
        dest = os.path.basename( src )
        pushd( reldest )
    else:
        reldest, dest = os.path.split( reldest )
        pushd( reldest )        

    src = ''
    while reldest:
        src = os.path.join( src, '..' )
        reldest = os.path.dirname( reldest )
    src = os.path.join( src, relsrc )
        
    os.symlink( src, dest )
    popd()
    popd()

def relative_path( path, basepath=None ):
    if basepath is None:
        basepath = os.getcwd()
        
    path = path.replace( basepath, '' )
    if path.startswith('/'):
        return path[1:]
    return path
        
def softlink( src, dest ):
    """Create a link only if I{src} exists and I{dest} does not.

    @return: True if a link was created, False otherwise.
    """
    if os.path.exists( src ):
        if not os.path.exists( dest ):
            os.symlink( src, dest )
        return True
    return False

def split_listdir(path):
    """Split the content of os.listdir() in dirs and nondirs lists."""
    dirs = []
    nondirs = []
    for name in os.listdir(path):
        if os.path.isdir(os.path.join(path,name)):
            dirs.append(name)
        else:
            nondirs.append(name)
    return dirs, nondirs

def system_symlink(source, target):
    """Create a link if the system correctly support it; copy otherwise.
    
    Under Cygwin, links are not appropriate as they are ".lnk" files,
    not properly opened by PLearn. Thus we need to copy the files.
    """
    import shutil, sys
    if (sys.platform == "cygwin"):
        if (os.path.isdir(source)):
            logging.debug("Recursively copying source: %s <- %s."%(target, source))
            shutil.copytree(source, target, symlinks=False)
        else:
            logging.debug("Copying source: %s <- %s."%(target, source))
            shutil.copy(source, target)
    else:
        logging.debug("Linking source: %s -> %s."%(target,source))
        # print "ln -s %s %s"%(source, target)
        rcode = os.system("ln -s '%s' '%s'"%(source, target))
        # print rcode


if __name__ == "__main__":
    from plearn.utilities.ppath import home
    testdir = os.path.join(home,"PLearn/plearn_learners/distributions/test/.pytest/PL_GaussMix_Spherical")
    os.chdir(testdir)
    current = os.getcwd()
    print current

    print "#####  split_listdir  #######################################################"
    print split_listdir(current)
    assert os.getcwd()==current
    print "#####  end split_listdir  ###################################################"

    print "#####  compare_trees  #######################################################"
    for res in compare_trees("expected_results", "run_results"):
        print res
    assert os.getcwd()==current

    cd()
    print
    # Absolute version
    for res in compare_trees(os.path.join(testdir, "expected_results"), os.path.join(testdir, "run_results")):
        print res    
    print "#####  end compare_trees  ###################################################"

