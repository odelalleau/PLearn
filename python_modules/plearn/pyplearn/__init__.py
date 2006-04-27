"""The PyPLearn Mecanism."""
__version_id__ = "$Id$"

import new, string, sys
from pyplearn                  import *
from PyPLearnObject            import *
from plearn.utilities.metaprog import public_attribute_predicate

class __pyplearn_magic_module:
    """PyPLearn Magic Module.

    An instance of this class (instanciated as pl) is used to provide
    the magic behavior whereas bits of Python code like::

        pl.SequentialAdvisorSelector( comparison_type='foo', ... )

    On any attempt to access a function from I{pl}, the magic module creates,
    on the fly, a PLearn-like class (derived from PyPLearnObject) named
    after the function asked for. The named arguments provided to the
    function are forwarded to the constructor of this new class to create
    an instance of the class. Hence, names of the function's arguments are
    considered as the PLearn object's option names.
    """
    def __getattr__(self, name):
        if name.startswith('__'):
            raise AttributeError

        def initfunc(**kwargs):
            klass = new.classobj(name, (PyPLearnObject,), {})
            assert issubclass( klass, PyPLearnObject )

            obj = klass(**kwargs)
            assert isinstance(obj, PyPLearnObject)
            return obj
        
        return initfunc

pl = __pyplearn_magic_module()

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
        self.metainfos     = self.get_metainfos()

    def get_metainfos(self):
        def parse( obj, prefix='' ):
            results = []
            for key in dir(obj):
                if not key.startswith('_'):
                    value = getattr(obj, key)
                    if ( public_attribute_predicate(key, value) and
                         not isinstance( value, plargs.namespace_overrides ) ):
                        results.append((prefix+key, value))
            results.sort()
            return results

        from plearn.utilities.Bindings import Bindings
        plarg_attrs = Bindings( parse(plarg_defaults) )
        plarg_attrs.update( parse(plargs) )
        for clsname, cls in plargs_namespace._subclasses.iteritems():
            if cls.__dict__['__accessed']:
                plarg_attrs.update( parse(cls, '%s.'%clsname) )
        
        ## Pretty printing
        pretty            = lambda attr_name: string.ljust(attr_name, 30)
        def backward_cast( value ):
            if isinstance( value, list ):
                return ",".join([ str(e) for e in value ])
            return str(value)

        attribute_strings = [ '%s = %s'
                              % ( pretty(attr_name), backward_cast(attr_value) ) 
                              for attr_name, attr_value in plarg_attrs.iteritems() ]
        return "\n".join( attribute_strings )

class __TMat:
    """Python representation of TMat of non-numeric elements."""
    def __init__( self, nrows, ncols, content ):
        self.nrows   = nrows
        self.ncols   = ncols
        self.content = content

    def _unreferenced( self ):
        return True
    
    def plearn_repr( self, indent_level=0, inner_repr=plearn_repr ):
        return "%d %d %s" % (
            self.nrows, self.ncols, 
            inner_repr( self.content, indent_level+1 ) 
            )

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

def getPythonSnippet(module):
    from inspect import getsourcelines
    return "\\n".join([ s.rstrip() for s in getsourcelines(module)[0] ])

if __name__ == "__main__":
    print TMat(2, 2, ["allo", "mon", "petit", "coco"]).plearn_repr()
