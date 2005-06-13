from plearn.utilities               import toolkit
from plearn.utilities.verbosity     import vprint
from plearn.pyplearn.PyPLearnObject import PyPLearnObject

class PyTestObject( PyPLearnObject ):
    def _unreferenced( self ):
        return True

class PyTestUsageError(Exception): 
    def __init__(self, msg):
        self.msg = msg

    def __str__(self):
        return self.msg

    def print_error(self):
        cname  = self.__class__.__name__+':'
        msg    = toolkit.boxed_lines( self.msg, 70 )
        vprint.highlight( ["", cname, ""] + msg + [""], '!' )
