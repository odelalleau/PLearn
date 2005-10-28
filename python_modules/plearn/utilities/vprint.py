__version_id__ = "$Id: verbosity.py 3631 2005-06-21 20:53:33Z dorionc $"

import os, string, sys
from plearn.utilities               import toolkit
from plearn.pyplearn.PyPLearnObject import PLOption, PyPLearnObject

def box(line, box_width, prefix='', postfix=''):
    # Adjust the box width for prefix and postfix
    box_width -= len(prefix) + len(postfix)     
    
    _boxed = [ ]
    boxed = lambda s: _boxed.append(prefix + s.ljust(box_width) + postfix)
    
    while len(line) > box_width:
        if line[box_width] in string.whitespace:
            cut = box_width
        else:            
            sub, cut = toolkit.rfind_one(line[:box_width], string.whitespace)

        # Cut the line, neglecting the whitespace
        b, line = line[:cut], line[cut+1:]
        boxed(b)
            
    # Append the last line to the list
    boxed(line)
    return _boxed

#
#  The main object provided by this module
# 
class __global_vprint:
    vpr = None
    def setVPR(cls, vpr):
        assert cls.vpr is None, "A global vprinter is already open. Close it first."
        cls.vpr = vpr
    setVPR = classmethod(setVPR)
    
    def close(cls):
        if cls.vpr is not None:
            cls.vpr = None 
    close = classmethod(close)

    def __call__(self, *args, **kwargs):
        if self.vpr is not None:
            self.vpr(*args, **kwargs)

    def __getattr__(self, attr):
        if self.vpr is not None:
            return getattr(self.vpr, attr)
        raise AttributeError(attr)
    
    def __setattr__(self, attr, val):
        if self.vpr is not None:
            return setattr(self.vpr, attr, val)
        raise AttributeError(attr)
    
vprint = __global_vprint()
                                
#
# Subclass of PyPLearnObject to benefit from the option mechanism
#
class VPrinter(PyPLearnObject):
    """Manages verbosity option for Python applications."""

    ## 
    # Class variables
    _vlevels  = { 'quiet'   : -1,
                  'normal'  : 0,
                  'verbose' : 5,
                  'debug'   : 10, }

    ##
    # Options
    box_width = PLOption(100)
    is_global = PLOption(False)
    output    = PLOption(lambda : sys.stderr) 
    verbosity = PLOption('normal')
    
    def __init__(self, **overrides):
        super(VPrinter, self).__init__(**overrides)        
        self._vlevel = self.__class__._vlevels[self.verbosity]
        self._indent = ''
        self._indent_stack = []

        if self.is_global:
            globals()['__global_vprint'].setVPR(self)

    def __call__(self, message='', priority='normal', highlight=''):
        if isinstance(priority, str):
            priority = self._vlevels[priority]
        assert priority >= 0

        if priority > self._vlevel:
            return

        if highlight:
            assert len(highlight) == 1
            self._print()
            self._print(highlight*self.box_width)
            self.indent(highlight+' ')

        lines = str(message).split('\n')
        for line in lines:
            for subline in box( line, self.box_width, self._indent, ' '+highlight ):
                self._print(subline)

        if highlight:
            self.dedent()
            self._print(highlight*self.box_width)
            self._print() 

    def _print(self, s=''):
        print >>self.output, s

    def dedent(self):
        self._indent = self._indent[:-self._indent_stack.pop()]

    def indent(self, s=" "*4):
        self._indent += s
        self._indent_stack.append(len(s))

    def setOutput(self, output):
        self.output = output
        


if __name__ == '__main__':
    VPrinter(verbosity='normal', is_global=True)

    vprint('vprint.box_width = %d'%vprint.box_width, highlight='*')
    msg = ' '.join( [ '-0%d-'%d for d in range(1, 10 ) ] +
                    [ '-%d-' %d for d in range(10,100) ]
                    )    
    vprint( msg )

    vprint.box_width = 50
    vprint( 'vprint.box_width = %d'%vprint.box_width, highlight='*' )
    vprint( msg )

    vprint( )
    msg = ( 'This very long (and repetitive) message should be cut'
            ' properly at each 46 chars (given a four chars indent). ' )
    vprint.indent()
    vprint( msg*10 )
    vprint.dedent()

    wstr = toolkit.WString()    
    vprint.setOutput( wstr )
    print '\nCall:'
    vprint( 'The message' )    
    print 'String content:', wstr

    vprint.setOutput( sys.stderr )
    vprint( 'Done.' )
    vprint.close()
    
