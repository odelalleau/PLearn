
import modes, string, time
from   ModeAndOptionParser import ModeAndOptionParser, OptionGroup

__all__ = ["modes", "ModeAndOptionParser", "OptionGroup", "pytest_version"]

version_str = "$Id: __init__.py,v 1.6 2004/12/14 04:59:16 dorionc Exp $"

really_all = [ "ModeAndOptionParser", 
               "BasicStats", 
               "IntelligentDiff", 
               "ModeAndOptionParser", 
               "modes", 
               "programs", 
               "test_and_routines" ]

def pytest_version( script_version_string ):
    minv = 1e06
    maxv = -1
    sumv = 0

    for vstr in [script_version_string, version_str]:
        vtup = version_tuple( vstr )
        v    = int(vtup[1])  

        sumv += v
        if v < minv:
            minv = v
        if v > maxv:
            maxv = v

    return "%s.%s.%s.%s" % (1, maxv, minv, sumv)
    
def version_tuple( version_str ):
    bflag = ",v"
    the_year = time.localtime()[0]

    begin = string.find(version_str, bflag) + len(bflag)
    end = string.find(version_str, str(the_year))

    return string.split(string.strip( version_str[begin:end] ), '.')


