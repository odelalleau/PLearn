"""Contains the PLearnObject class from which to derive python objects that emulate PLearn ones."""
import inspect

class PLearnObject:
    """Class from which to derive python objects that emulate PLearn ones.

    This abstract class is to be the superclass of any python object
    that has a plearn representation, which is managed through
    plearn_repr().
    
    This class sets the basics for managing classes defining default
    values for PLearnObject fields. It also provides, through
    plearn_options(), an easy way to retreive all attributes to be forwarded
    to the plearn snippet through the 'pl' object (see plearn_repr()).

    IMPORTANT: Subclasses of PLearnObject MUST declare any internal
    member, i.e. *any member that SHOULD NOT be considered by plearn_options*,
    with a name that starts with at least one underscore '_'.
    """
    def __init__(self, defaults=None, overrides=None):
        """Sets, if any, the default field values provided through 'defaults'

        defaults <- Class
        """
        if defaults is not None:
            defaults_dict = dict( [(x,y) for (x,y) in inspect.getmembers(defaults)
                                   if not(x[0:2] == "__" and x[-2:]=="__")        ] )
            self.__dict__.update( defaults_dict )

        if overrides is not None:
            self.__dict__.update( overrides )

    def plearn_options(self):
        """Return a dictionnary of all members of this instance that are considered to be plearn options.

        IMPORTANT: Subclasses of PLearnObject MUST declare any
        internal member, i.e. *any member that SHOULD NOT be
        considered by plearn_options()*, with a name that starts with at least
        one underscore '_'.

        Indeed, this method returns a dictionnary containing all and
        only the members whose names do not start with an
        underscore. Note, however, that no given order may be assumed
        in the members dictionnary returned.
        """
        return dict( [ (x,y) for (x,y) in inspect.getmembers(self)
                       if ( x[0] != "_"
                            and not inspect.ismethod(y)
                            and not inspect.isfunction(y) )       ] )
        
    def plearn_repr(self):
        """Any object overloading this method will be recognized by the pyplearn_driver

        NOTE THAT in most cases, given the plearn_options() method just above,
        the proper and easy way to implement this method is

            return pl.NameOfTheCorespondingPLearnObject( **self.plearn_options() )
        """
        raise NotImplemented("PLearnObject.plearn_repr must be overloaded. (type(self): %s)"
                             % type(self))
