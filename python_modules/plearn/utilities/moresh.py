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
