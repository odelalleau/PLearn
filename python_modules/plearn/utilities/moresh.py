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
    # raw_input( src )
    # raw_input( dest )
        
    common = os.path.commonprefix([ src, dest ])
    if not os.path.isdir( common ):
        common = os.path.dirname( common )
    pushd( common )
    # raw_input( common )

    relsrc  = src.replace( common, '' )
    reldest = dest.replace( common, '' )
    # raw_input( relsrc )
    # raw_input( reldest )

    
    if os.path.isdir( reldest ):
        if reldest.endswith('/'):
            reldest = reldest[:-1]
        dest = os.path.basename( src )
        pushd( reldest )
    else:
        reldest, dest = os.path.split( reldest )
        pushd( reldest )        
    # raw_input( reldest )

    src = ''
    while reldest:
        src = os.path.join( src, '..' )
        reldest = os.path.dirname( reldest )
    src = os.path.join( src, relsrc )
        
    os.symlink( src, dest )
    popd()
    popd()

def softlink( src, dest ):
    if os.path.exists( src ):
        if not os.path.exists( dest ):
            os.symlink( src, dest )
        return True
    return False

