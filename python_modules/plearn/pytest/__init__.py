
import modes, os, string, time
from   ModeAndOptionParser import ModeAndOptionParser, OptionGroup

__all__ = ["modes", "ModeAndOptionParser", "OptionGroup", "pytest_version"]

version_str = "$Id: __init__.py,v 1.7 2004/12/14 05:09:17 dorionc Exp $"

## really_all = [ "ModeAndOptionParser", 
##                "BasicStats", 
##                "IntelligentDiff", 
##                "ModeAndOptionParser", 
##                "modes", 
##                "programs", 
##                "test_and_routines" ]

def version_strings():
    entries_path = os.path.abspath( os.path.dirname(__file__) )
    entries_path = os.path.join( entries_path, "CVS/Entries" )
    cvs_entries  = toolkit.command_output("cat %s" % entries_path)

    for line in cvs_entries:
        splited_line = string.split(line, '/')
        if len(splited_line) < 1:
            continue
        
        filename = splited_line[0]
        if filename.endswith('.py'):
            pass

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


