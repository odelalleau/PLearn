# pyplearn/__init__.py
# Copyright (C) 2004-2006 Christian Dorion
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#
#   1. Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
#
#   2. Redistributions in binary form must reproduce the above copyright
#      notice, this list of conditions and the following disclaimer in the
#      documentation and/or other materials provided with the distribution.
#
#   3. The name of the authors may not be used to endorse or promote
#      products derived from this software without specific prior written
#      permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
#  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
#  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
#  NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
#  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#  This file is part of the PLearn library. For more information on the PLearn
#  library, go to the PLearn Web site at www.plearn.org

# Author: Christian Dorion
"""The PyPLearn Mecanism."""

import copy, new, os, sys

from plearn.pyplearn.plargs import *
from plearn.pyplearn.plearn_repr import plearn_repr
from plearn.pyplearn.PyPLearnObject import PyPLearnObject, PyPLearnList, PLOption

from plearn.utilities.metaprog import public_attribute_predicate

#######  Helper functions  ####################################################

def getPythonSnippet(module):
    from inspect import getsourcelines
    return "\\n".join([ s.rstrip() for s in getsourcelines(module)[0] ])

def include( filename, replace_list = [] ):
    """Includes the content of a .plearn file.

    Some users that have developed complex chunks of PLearn scripts may not
    be keen to rewrite those in PyPLearn promptly. Hence, this function
    (combined with plvar()) allows to do both the followings:

      1. Including a .plearn file::
        #
        # script.pyplearn
        #
        def main():
            pl_script  = include("dataset.plearn")
            pl_script += pl.PTester( expdir = plargs.expdir,
                                     dataset = plvar("DATASET"),
                                     statnames = ["E[test.E[mse]]", "V[test.E[mse]]"],
                                     save_test_outputs = 1, 
                                     provide_learner_expdir = 1,
                                     
                                     learner = pl.SomeLearnerClassTakingNoOption();
                                     ) # end of PTester
            return pl_script

        where::
          #
          # dataset.plearn
          #
          $DEFINE{DATASET}{ AutoVMatrix( specification = "somefile.pmat";
                                         inputsize     = 10; 
                                         targetsize    = 1              ) }

      2. Importing 'inline' some plearn chunk of code, that is::

        #
        # dataset2.plearn
        #
        AutoVMatrix( specification = ${DATAPATH};
                     inputsize     = 10; 
                     targetsize    = 1              )

        #
        # script2.pyplearn
        #
        def main():
            plvar( 'DATAPATH', 'somefile.pmat' )
            return pl.PTester( expdir = plargs.expdir,
                               dataset = include("dataset2.plearn"),
                               statnames = ["E[test.E[mse]]", "V[test.E[mse]]"],
                               save_test_outputs = 1, 
                               provide_learner_expdir = 1,
                               
                               learner = pl.SomeLearnerClassTakingNoOption();
                               )
    """
    f = open(filename, 'U')
    try:
        include_content = f.read()
        for (search,replace) in replace_list:
            include_content = include_content.replace(search,replace)
    finally:
        f.close()
    return PLearnSnippet( include_content )

def plvar( varname, value ):
    """Emulating PLearn's $DEFINE statement.
    
    An old .plearn script (included with include()) may depend on some
    variables to be defined externally. If one wants to define that
    variable within a PyPLearn script, he must use this function with two
    arguments::

        plvar( 'DATASET',
               pl.AutoVMatrix( specification = "somefile.pmat",
                               inputsize     = 10, 
                               targetsize    = 1
                               )
               )

    which is strictly equivalent to the old::

        $DEFINE{DATASET}{ AutoVMatrix( specification = "somefile.pmat";
                                       inputsize     = 10; 
                                       targetsize    = 1              ) }
    
    If, on the other hand, a variable is defined within the PLearn script
    and must be referenced within PyPLearn, the simple C{plvar('DATASET')}
    will refer to the variable just as C{${DATASET}} would have.
    """
    if value is None:
        snippet = '${%s}' % varname
    else:
        snippet = '$DEFINE{%s}{ %s }' % (
            varname, plearn_repr( value, indent_level+1 )
            )
        
    return PLearnSnippet( snippet )

# Factory function
def TMat( *args ):
    """Returns the PyPLearn representation of a TMat.

    Within PyPLearn scripts, TMat instances may have two representations::

       1. A numarray for TMat of numeric types
       2. A representation wrapper for other types.

    @param args: Arguments may be a list of list or a (I{nrows}, I{ncols},
    I{content}) tuple where I{content} must be a list of length I{nrows*ncols}.

    @returns: PyPLearn's TMat representation
    @rtype: A numarray or L{ __TMat} wrapper
    """
    import numarray
    
    nargs   = len(args)
    is_real = lambda r: isinstance( r, int   ) or isinstance( r, float )
    
    # Support for a list of lists
    if nargs == 1:
        mat   = args[0]                
        nrows = len(mat)
        ncols = 0
        if nrows:
            ncols = len( mat[0] )

        # TMat< real >: Will return a numarray
        if nrows == 0 or is_real( mat[0][0] ):
            return numarray.array( mat )
        content = reduce( lambda v1, v2: v1+v2, mat )


    # Support for (length, width, content) tuples
    elif nargs == 3:
        nrows   = args[0]
        ncols   = args[1]
        content = args[2]
        assert nrows*ncols == len(content)

        # TMat< real >: Will return a numarray
        if nrows==0 or is_real( content[0] ):
            mat = []
            i = 0
            while i < nrows:
                row = []
                j = 0
                while j < ncols:
                    element = content[i*ncols + j]
                    assert is_real(element)
                    row.append( element )
                    j = j + 1
                mat.append( row )
                i = i + 1
            return numarray.array( mat )
            
    # TMat<T> where T is not real
    return __TMat( nrows, ncols, content )


#######  Core classes  ########################################################

class PyPLearnError( Exception ):
    """For unexpected or incorrect use of some PyPLearn mechanism."""
    pass

class UnknownArgumentError( PyPLearnError ):
    """Attribute known by neither plargs or plarg_defaults.

    This exception is raised when attempting to use a PLearn argument
    that was not defined, either on the command-line or with
    plarg_defaults.
    """
    def __init__(self, arg_name):
        self.arg_name = arg_name

    def __str__(self):
        return "Unknown PyPLearn argument: '%s'." % self.arg_name

class PLearnSnippet:
    """Wrapper for PLearn code snippets.

    Objects of this class are used to wrap the parts of the Python code
    that have already be converted to a PLearn string, so they don't
    get the same treatment as Python strings (which will get wrapped in
    double quotes by plearn_repr() ).
    """
    def __init__(self, s):
        self.s = s

    def __str__(self):
        return self.s

    def __repr__(self):
        return self.s

    def __add__(self, o):
        raise NotImplementedError("See dorionc")
    
    def __iadd__(self, snippet):
        """Overrides the operator+= on plearn_snippet instances.

        snippet <- plearn_snippet
        -> plearn_snippet
        """
        if not isinstance(snippet, plearn_snippet):
            raise TypeError("plearn_snippet.__add__ expects a plearn_snippet instance")
        self.s += snippet.s
        return self

    def _by_value( self ):
        return True

    def plearn_repr(self, indent_level=0, inner_repr=plearn_repr):
        return self.s

### Alias the two things for now
RealRange = PLearnSnippet


class PyPLearnScript( PyPLearnObject ):
    """Feeded by the PyPLearn driver to PLearn's main.

    This class has a PLearn cousin of the same name that is used to wrap
    PyPLearn scripts feeded to PLearn application. Most user need not to
    care about the behaviour of this class and its cousin since their
    effects are hidden in the core of the PLearn main program.

    Basically, feeding a PyPLearnScript to PLearn allows management of
    files to be writen in the experiment directory after the experiment was
    ran.
    """
    expdir        = PLOption(None)
    metainfos     = PLOption(None)
    plearn_script = PLOption(None)

    def __init__(self, main_object, **overrides):
        PyPLearnObject.__init__(self, **overrides)

        self.expdir        = plargs.expdir
        self.plearn_script = str( main_object )
        self.metainfos     = self.getMetainfos()

    def getMetainfos(self):
        bindings = plargs.getContextBindings()

        def backward_cast(value):
            if isinstance(value, list):
                return ",".join([ str(e) for e in value ])
            return str(value)
        pretty = lambda opt, val: (opt.ljust(45), backward_cast(val))
        
        cmdline = [ "%s = %s"%pretty(opt, bindings[opt]) for opt in bindings ]
        return "\n".join(cmdline)

class pl:
    """PyPLearn Magic Module.

    This class is used to provide the magic behavior whereas bits of Python
    code like::

        pl.SequentialAdvisorSelector( comparison_type='foo', ... )

    On any attempt to access I{method} SUBCLASS from I{pl}, the magic
    module creates, on the fly, a subclass of PyPLearnObject named
    subclass. This class is totally independent of any actual subclass of
    PyPLearnObject which would be named SUBCLASS.

    The named arguments provided to the function are forwarded to the
    constructor of this new class to create an instance of the
    class. Hence, names of the function's arguments are considered as the
    PLearn object's option names.
    """
    class __metaclass__(type):
        def __getattr__(cls, name):
            if name.startswith('__'):
                raise AttributeError

            def initfunc(**kwargs):
                klass = new.classobj(name, (PyPLearnObject,), {})
                assert issubclass( klass, PyPLearnObject )
        
                obj = klass(**kwargs)
                assert isinstance(obj, PyPLearnObject)
                return obj
            
            return initfunc

class __TMat:
    """Python representation of TMat of non-numeric elements."""
    def __init__(self, nrows, ncols, content):
        self.nrows   = nrows
        self.ncols   = ncols
        self.content = content

    def _by_value(self):
        return True
    
    def plearn_repr(self, indent_level=0, inner_repr=plearn_repr):
        return "%d %d %s" % (
            self.nrows, self.ncols, 
            inner_repr( self.content, indent_level+1 ) 
            )

class Storage:
    instance_count = 0
    
    def __init__(self, data):        
        self.data = data
        self.__class__.instance_count += 1
        self.serial_number = self.instance_count

    def plearn_repr(self, indent_level=0, inner_repr=plearn_repr):
        return inner_repr(self.data, indent_level)
    
    def __deepcopy__(self, memo):
        """For the deepcopy mechanism."""
        return self.__class__( copy.deepcopy(self.data, memo) ) 

    def _serial_number(self):
        # This is slightly hackish but otherwise instances of storage would
        # clash with PyPLearnObject instances...
        return "Storage%d"%self.serial_number

class TVec:
    def __init__(self, vector):        
        self.storage = Storage(vector)

    def plearn_repr(self, indent_level=0, inner_repr=plearn_repr):
        return "TVec(%d, 0, %s)" % (
            len(self.storage.data), inner_repr(self.storage, indent_level+1) )


#######  Builtin Tests  #######################################################

def test_pl_magic_module():
    sub = pl.Subclass(option1="opt1", option2=2)
    print sub.__class__.__name__, id(sub.__class__)
    print sub
    print
    
    class Subclass(PyPLearnObject):
        option1 = PLOption("opt1")
        option2 = PLOption(2)

    sub2 = Subclass()
    print sub2.__class__.__name__, id(sub2.__class__)
    print sub2
    print
    
    sub3 = pl.Subclass()
    print sub3.__class__.__name__, id(sub3.__class__)
    print sub3
    print

if __name__ == "__main__":
    print TMat(2, 2, ["allo", "mon", "petit", "coco"]).plearn_repr()
    print 

    print "Simple list:\n"
    a = [1, 2, 3]
    print plearn_repr(a)
    print plearn_repr(a)
    print "---"
    print
    
    print "TVec( list ):\n"
    b = TVec(a)
    print plearn_repr(b)
    print plearn_repr(b)
    print "---"
    print
    
    print "copy.deepcopy( tvec ):\n"
    c = copy.deepcopy(b)
    print plearn_repr(c)
    print plearn_repr(c)
    print "---"
    print
    
