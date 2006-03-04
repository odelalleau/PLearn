import sets, sys
from plearn.utilities import toolkit
from plearn.pyplearn.PyPLearnObject import PLOption, PyPLearnObject

#######  PyTestObject  ########################################################

class PyTestObject(PyPLearnObject):
    def _unreferenced(self):
        return True

#######  Exceptions  ##########################################################

def traceback(exception):
    import traceback
    return "Traceback (most recent call last):\n" + \
           ''.join(traceback.format_tb(sys.exc_info()[2])) + \
           "%s: %s"%(exception.__class__.__name__, exception)

class PyTestUsageError(Exception):
    pass

#######  Exit Code  ###########################################################

#   0 : Tests were ran and all enabled tests passed (no skipped test)
#   1 : At least one test was skipped
#   2 : At least one test failed
#   4 : At least one program did not compile 
#  32 : PyTest usage error (wrong command line arguments, ...)
#  64 : PyTest internal error
__code_mappings = {
    ""                   : 0, # For convenience
    "PASSED"             : 0,
    "DISABLED"           : 0,
    "SKIPPED"            : 1,
    "FAILED"             : 2,
    "COMPILATION FAILED" : 4,
    "USAGE ERROR"        : 32,
    "INTERNAL ERROR"     : 64
    }

__exit_flags = sets.Set()
def exitPyTest(flag=""):
    updateExitCode(flag)
    sys.exit( sum(__exit_flags) )

def updateExitCode(flag):
    __exit_flags.add(__code_mappings[flag])

#######  Module's Test  #######################################################

if __name__ == '__main__':
    import os
    import modes
    from plearn.utilities.ModeAndOptionParser import Mode

    def vsystem(cmd):        
        print >>sys.stderr, '#\n#  %s\n#' % cmd
        os.system( cmd + ' > /dev/null' )
        print >>sys.stderr, ''
        
    vsystem('pytest')
    vsystem('pytest -h')

    for mode in Mode.mode_list():
        mode_name = mode.__name__
        vsystem('pytest %s -h' % mode_name)
