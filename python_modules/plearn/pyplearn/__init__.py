"""This package contains all modules related to the pyplearn mecanism.

Note that including this package simply includes the pyplearn module it contains.
"""
__cvs_id__ = "$Id: __init__.py,v 1.4 2005/02/07 21:20:05 dorionc Exp $"

import new
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
