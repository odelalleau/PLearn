
### As the versionning is used by some of the modules imported, this chunk
### of code *MUST* be executed prior to any other imports
import plearn.utilities.versionning  as     versionning
versionning.declare_project( "PyTest" )
versionning.project_module ( "PyTest", __name__,
                             "$Id: __init__.py,v 1.10 2004/12/20 17:04:42 dorionc Exp $"
                             )

### When an official build will be ready...
## versionning.official_build( "PyTest",
##                             build_version = [1, 0],
##                             fixlevels     = [5, 28, 89]
##                             )
                            
### The versionning tools are now properly enabled.
import modes
from   ModeAndOptionParser           import ModeAndOptionParser, OptionGroup
import plearn.utilities.toolkit      as     toolkit

def declare_pytest_cvs_id( script_cvs_id ):
    versionning.project_module("PyTest", "__pytest_script__", script_cvs_id)
    
def pytest_version():
    return versionning.project_version("PyTest")

__all__ = [ "modes", "ModeAndOptionParser", "OptionGroup",
            "declare_pytest_cvs_id", "pytest_version"
            ]
    
