from sys import exc_info
from traceback import format_tb

from plearn.utilities import toolkit
from plearn.pyplearn.PyPLearnObject import PLOption, PyPLearnObject

__exit_code = 0
def getExitCode():
    return __exit_code

def updateExitCode(exit_code):
    global __exit_code
    if exit_code > __exit_code:
        __exit_code = exit_code

class PyTestObject(PyPLearnObject):
    def _unreferenced(self):
        return True

class PyTestError(Exception): 
    def __init__(self, msg):
        self.msg = msg

    def __str__(self):
        return self.msg

    def pretty_str(self):
        return "%s\n%s"%(self,format_tb(exc_info()[2]))
        
    def print_error(self):
        cname  = self.__class__.__name__+':'
        msg    = toolkit.boxed_lines( self.msg, 70 )
        logging.critical('\n'.join(["", cname, ""] + msg + [""]))

if __name__ == '__main__':
    import os, sys
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
