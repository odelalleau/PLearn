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

##########################################
### Main class
##########################################

class FrozenObject:
    """Superclass for Python objects that should not be added fields externally. 

    B{NOTE} that B{ALL INTERNAL VARIABLES} must be set throught the
    set_attribute() method, the default values class or by the
    'override' dictionnary argument passed to the PLearnObject ctor.
    Indeed, out of this method, the FrozenObject instances are frozen
    to avoid accidental external manipulations.
    """
    _frozen = True
    
    def __init__(self, defaults=None, overrides=None):
        """Sets, if any, the default field values provided through 'defaults'

        Typically, subclasses' constructor looks like::
            class Subclass( FrozenObject ):
                def __init__(self, arg1, arg2, defaults=SubclassDefaults, **overrides):
                    FrozenObject.__init__(self, defaults, overrides)
                    self.set_attribute('option1', arg1)
                    self.set_attribute('option2', arg2)
                    
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

    def set_attribute(self, key, value):
        self._frozen = False
        setattr(self, key, value)
        self._frozen = True
        

