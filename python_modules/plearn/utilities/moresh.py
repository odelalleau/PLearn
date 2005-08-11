import os
from plearn.utilities import ppath

DIR_STACK = []

def cd( path = None ):
    """Emulate shell's cd --- forwards the call to os.chdir().

    Mainly called as cd() as a shorthand to os.chdir( os.getenv('HOME') ).  
    """
    if path is None:
        path = ppath.ppath('HOME')
    os.chdir( path )
    
def ls( path = None ):
    """Emulate shell's ls --- forwards the call to os.listdir().

    Mainly called as ls() as a shorthand to os.listdir( os.getcwd() ).  
    """
    if path is None:
        path = os.getcwd()
    return os.listdir( path )
    
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

def relative_path( path, basepath ):
    path = path.replace( basepath, '' )
    if path.startswith('/'):
        return path[1:]
    return path
        
def softlink( src, dest ):
    if os.path.exists( src ):
        if not os.path.exists( dest ):
            os.symlink( src, dest )
        return True
    return False

