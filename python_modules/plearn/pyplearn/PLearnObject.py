"""Contains the PLearnObject class from which to derive python objects that emulate PLearn ones.

X{Tutorial: Using PLearnObject}
===============================

Coming soon...
"""
import inspect

##########################################
### Helper functions 
##########################################

def frozen(set):
    "Raise an error when trying to set an undeclared name."
    def set_attr(self,name,value):
        if not self._frozen or hasattr(self,name):
            set(self,name,value) 
        else:
            raise AttributeError("You cannot add attributes to %s" % self)
    return set_attr

class PLearnObject:
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
    _set_attribute() method, the default values class or by the
    'override' dictionnary argument passed to the PLearnObject ctor.
    Indeed, out of this method, the PLearnObject are frozen to avoid
    accidental external manipulations.
    """
    _frozen = True
    
    def __init__(self, defaults=None, overrides=None):
        """Sets, if any, the default field values provided through 'defaults'

        Typically, subclasses' constructor looks like::
            class Subclass( PLearnObject ):
                def __init__(self, arg1, arg2, defaults=SubclassDefaults, **overrides):
                    PLearnObject.__init__(self, defaults, overrides)
                    self._set_attribute('option1', arg1)
                    self._set_attribute('option2', arg2)
                    
        @param defaults: A class whose class variables are the default
        values for this object's fields.
        @type  defaults: Class

        @param overrides: A dictionnary containing the keyword
        arguments passed to to subclass constructor.
        @type  overrides: Dictionnary
        """
        self._frozen = False

        if defaults is not None:
            defaults_dict = dict( [(x,y) for (x,y) in inspect.getmembers(defaults)
                                   if not(x[0:2] == "__" and x[-2:]=="__")        ] )
            self.__dict__.update( defaults_dict )

        if overrides is not None:
            self.__dict__.update( overrides )

        self._frozen = True

    __setattr__=frozen(object.__setattr__)
    class __metaclass__(type):
        __setattr__=frozen(type.__setattr__)

    def _set_attribute(self, key, value):
        self._frozen = False
        setattr(self, key, value)
        self._frozen = True
        
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
        raise NotImplemented("PLearnObject.plearn_repr must be overrided. (type(self): %s)"
                             % type(self))
