import os

DIR_STACK = []

def pushd( path ):
    DIR_STACK.append( os.getcwd() )
    os.chdir( path )

def popd( ):
    os.chdir( DIR_STACK.pop() )
    
