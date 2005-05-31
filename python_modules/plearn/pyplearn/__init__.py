"""This package contains all modules related to the pyplearn mecanism.

Note that including this package simply includes the pyplearn module it contains.
"""
__cvs_id__ = "$Id: __init__.py,v 1.9 2005/05/31 14:32:58 dorionc Exp $"

import new, string, numarray
from pyplearn                  import *
from PyPLearnObject            import *
from operator                  import isNumberType
from plearn.utilities.metaprog import public_attribute_predicate

class __pyplearn_magic_module:
    """An instance of this class (instanciated as pl) is used to provide
    the magic behavior whereas bits of Python code like:
    pl.SequentialAdvisorSelector(comparison_type='foo', etc.) become
    a string that can be fed to PLearn instead of a .plearn file."""
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


def TMat( num_rows, num_cols, mat_contents ):
    mat = []
    i = j = 0
    while i < num_rows:
        row = []
        while j < num_cols:
            element = mat_contents[i*num_cols + j]
            assert isNumberType(element)
            row.append( element )
        mat.append( row )
    return numarray.array( )

if __name__ == "__main__":
    print TMat(2, 2, ["allo", "mon", "petit", "coco"])
