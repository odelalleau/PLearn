
import modes, string, time
from   ModeAndOptionParser import ModeAndOptionParser, OptionGroup

__all__ = ["modes", "ModeAndOptionParser", "OptionGroup", "pytest_version"]

version_str = "$Id: __init__.py,v 1.5 2004/12/14 04:43:07 dorionc Exp $"

really_all = [ "ModeAndOptionParser", 
               "BasicStats", 
               "IntelligentDiff", 
               "ModeAndOptionParser", 
               "modes", 
               "programs", 
               "test_and_routines" ]

def pytest_version( script_version_string ):
    raw_input( version_tuple( script_version_string ) )

def version_tuple( version_str ):
    bflag = "pytest,v"
    the_year = time.localtime()[0]

    begin = string.find(version_str, bflag) + len(bflag)
    end = string.find(version_str, str(the_year))

    return string.split(string.strip( version_str[begin:end] ), '.')


