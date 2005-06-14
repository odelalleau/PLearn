from plearn.utilities               import toolkit
from plearn.utilities.verbosity     import vprint
from plearn.pyplearn.PyPLearnObject import PyPLearnObject

class PyTestObject( PyPLearnObject ):
    _frozen = False
    
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

if __name__ == '__main__':
    import os, sys
    import modes
    from plearn.utilities.ModeAndOptionParser import Mode

    def vsystem( cmd ):        
        print >>sys.stderr, '#\n#  %s\n#' % cmd
        os.system( cmd + ' > /dev/null' )
        print >>sys.stderr, ''
        
    vsystem( 'pytest' )
    vsystem( 'pytest -h' )

    for mode_name in Mode.mode_list():
        vsystem( 'pytest %s -h' % mode_name )
