"""This package contains all modules related to the pyplearn mecanism.

Note that including this package simply includes the pyplearn module it contains.
"""
__cvs_id__ = "$Id: __init__.py,v 1.7 2005/02/15 15:08:33 dorionc Exp $"

import new, string
from pyplearn        import *
from PyPLearnObject  import *

class __pyplearn_magic_module:
    """An instance of this class (instanciated as pl) is used to provide
    the magic behavior whereas bits of Python code like:
    pl.SequentialAdvisorSelector(comparison_type='foo', etc.) become
    a string that can be fed to PLearn instead of a .plearn file."""
    def __getattr__(self, name):
        if name.startswith('__'):
            raise AttributeError

        klass = new.classobj(name, (PyPLearnObject,), {})
                             
        def initfunc(**kwargs):
            return klass(**kwargs)
        
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

        from pyplearn import _postprocess_refs
        self.plearn_script = _postprocess_refs( str( main_object ) )

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

        ## plarg_defaults
