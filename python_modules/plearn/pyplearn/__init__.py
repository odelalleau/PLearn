"""This package contains all modules related to the pyplearn mecanism.

Note that including this package simply includes the pyplearn module it contains.
"""
__cvs_id__ = "$Id: __init__.py,v 1.10 2005/06/13 15:56:49 dorionc Exp $"

import new, string, numarray
from pyplearn                  import *
from PyPLearnObject            import *
from plearn.utilities.metaprog import public_attribute_predicate

class __pyplearn_magic_module:
    """PyPLearn Magic Module.

    An instance of this class (instanciated as pl) is used to provide
    the magic behavior whereas bits of Python code like:

        pl.SequentialAdvisorSelector(comparison_type='foo', etc.)

    On any attempt to access a function from pl, the magic module creates,
    on the fly, a PLearn-like class (derived from PyPLearnObject) named
    after the function asked for. The named arguments provided to the
    function are forwarded to the constructor of this new class to create
    an instance of the class. Hence, names of the function's arguments are
    considered as the PLearn object's option names.
    """
    def __getattr__(self, name):
        if name.startswith('__'):
            raise AttributeError

        klass = new.classobj(name, (PyPLearnObject,), {})
        assert issubclass( klass, PyPLearnObject )

        def initfunc(**kwargs):
            obj = klass(**kwargs)
            assert isinstance( obj, PyPLearnObject )
            return obj
        
        return initfunc

pl = __pyplearn_magic_module()

class PyPLearnScript( PyPLearnObject ):
    class Defaults:
        expdir        = None
        metainfos     = None
        plearn_script = None

    def __init__(self, main_object, **overrides):
        PyPLearnObject.__init__(self, **overrides)

        self.expdir        = plargs.expdir
        self.metainfos     = self.get_metainfos()
        self.plearn_script = str( main_object )

    def get_metainfos(self):
        import inspect
        def parse( obj ):
            return [ (attr_name, attr_val)
                     for (attr_name, attr_val)
                     in inspect.getmembers( obj )
                     if public_attribute_predicate(attr_name, attr_val) ]
        
        plarg_attrs = dict(parse( plarg_defaults ))
        plarg_attrs.update( dict(parse( plargs )) )
        
        ## Alphabetical iteration
        keys = plarg_attrs.keys()
        keys.sort()
        
        pretty            = lambda attr_name: string.ljust(attr_name, 30)
        attribute_strings = [ '%s = %s'
                              % ( pretty(attr_name), plarg_attrs[attr_name] ) 
                              for attr_name in keys
                              if public_attribute_predicate(attr_name, plarg_attrs[attr_name])
                              ]
        return "\n".join( attribute_strings )

class __TMat:
    def __init__( self, nrows, ncols, content ):
        self.nrows   = nrows
        self.ncols   = ncols
        self.content = content

    def _unreferenced( self ):
        return True
    
    def plearn_repr( self, indent_level=0 ):
        return "%d %d %s" % (
            self.nrows, self.ncols, 
            plearn_repr.plearn_repr( self.content, indent_level+1 ) 
            )
    
def TMat( *args ):
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
            i = j = 0
            while i < nrows:
                row = []
                while j < ncols:
                    element = content[i*ncols + j]
                    assert is_real(element)
                    row.append( element )
                mat.append( row )
            return numarray.array( mat )
            
    # TMat<T> where T is not real
    return __TMat( nrows, ncols, content )

        

if __name__ == "__main__":
    print TMat(2, 2, ["allo", "mon", "petit", "coco"])
