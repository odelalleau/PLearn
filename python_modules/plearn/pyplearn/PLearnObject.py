"""Contains the PLearnObject class from which to derive python objects that emulate PLearn ones.

X{Tutorial: Using PLearnObject}
===============================

Coming soon...
"""
__cvs_id__ = "$Id: PLearnObject.py,v 1.5 2004/12/21 15:31:15 dorionc Exp $"

from pyplearn import *
from plearn.utilities.FrozenObject import *

class PLearnObject( FrozenObject ):
    """Class from which to derive python objects that emulate PLearn ones.

    This abstract class is to be the superclass of any python object
    that has a plearn representation, which is managed through
    plearn_repr().

    This class also sets the basics for managing classes defining default
    values for PLearnObject fields. It also provides, through
    plearn_options(), an easy way to retreive all attributes to be forwarded
    to the plearn snippet through the 'pl' object (see plearn_repr()).

    B{IMPORTANT}: Subclasses of PLearnObject MUST declare any internal
    member, i.e. B{any member that SHOULD NOT be considered by plearn_options},
    with a name that starts with at least one underscore '_'.

    B{NOTE} that B{ALL INTERNAL VARIABLES} must be set throught the
    set_attribute() method, the default values class or by the
    'override' dictionnary argument passed to the PLearnObject ctor.
    Indeed, out of this method, the PLearnObject are frozen to avoid
    accidental external manipulations.
    """
    def __init__(self, defaults=None, overrides=None):
        FrozenObject.__init__(self, defaults, overrides)
        
    def plearn_options(self):
        """Returns a dictionnary of all members of this instance that are considered to be plearn options.

        B{IMPORTANT}: Subclasses of PLearnObject MUST declare any
        internal member, i.e. B{any member that SHOULD NOT be
        considered by plearn_options()}, with a name that starts with
        at least one underscore '_'.

        Indeed, this method returns a dictionnary containing all and
        only the members whose names do not start with an
        underscore. Note, however, that no given order may be assumed
        in the members dictionnary returned.

        @return: Dictionnary of all members of this instance that are
        considered to be plearn options.
        """
        return dict( [ (x,y) for (x,y) in inspect.getmembers(self)
                       if ( x[0] != "_"
                            and not inspect.ismethod(y)
                            and not inspect.isfunction(y) )       ] )
        
    def plearn_repr(self):
        """Any object overloading this method will be recognized by the pyplearn_driver

        NOTE THAT in most cases, given the plearn_options() method just above,
        the proper and easy way to implement this method is::
        
            return pl.NameOfTheCorespondingPLearnObject( **self.plearn_options() )
        """
##         raise NotImplemented("PLearnObject.plearn_repr must be overrided. (type(self): %s)"
##                              % type(self))
        return eval("pl.%s( **self.plearn_options() )" % self.classname())
