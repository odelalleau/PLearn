
import modes, os, string, time
from   ModeAndOptionParser           import ModeAndOptionParser, OptionGroup
import plearn.utilities.toolkit      as     toolkit

import plearn.utilities.versionning  as     versionning
versionning.declare_project( "PyTest" )
versionning.project_module ( "PyTest", __name__,
                             "$Id: __init__.py,v 1.8 2004/12/15 13:51:07 dorionc Exp $"
                             )
def declare_pytest_cvs_id( script_cvs_id ):
    versionning.declare_module("PyTest", "__pytest_script__", script_cvs_id)
    
def pytest_version(  ):
    return versionning.project_version("PyTest")

__all__ = [ "modes", "ModeAndOptionParser", "OptionGroup",
            "declare_pytest_cvs_id", "pytest_version"
            ]
    
