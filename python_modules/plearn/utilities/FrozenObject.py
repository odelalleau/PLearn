import inspect, string, types

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

class MandatoryOverrideError(ValueError):
    def __init__(self, frozen_object, attribute_name):
        self.frozen_object
        self.attribute_name = attribute_name

    def __str__(self):
        classname = self.frozen_object.classname()
        return ( "The %s.%s value must be overriden in the keyword "
                 "arguments provided to the %s constructor."
                 %(classname, self.attribute_name, classname)
                 )

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
        
        self.__members    = []
        self.__str_spacer = ' '
        self._defaults    = defaults
        members_list      = inspect.getmembers(defaults)
        declared_members   = []
        
        if defaults is not None:
            defaults_dict = {}
            for (x,y) in members_list:
                if not(x[0:2] == "__" and x[-2:]=="__"):
                    self.__dict__[x] = y
                elif x == '__declare_members__':
                    declared_members = y
        
        if overrides is not None:
            self.__dict__.update( overrides )

        for (att, typ) in declared_members:
            self.__members.append(att)
            if typ is not None:
                self.type_check(att, typ)
        
        self._frozen = True

    __setattr__=frozen(object.__setattr__)
    class __metaclass__(type):
        __setattr__=frozen(type.__setattr__)

    def __str__(self):
        members = [ ]
        
        for member in self.classmembers():
            value = getattr(self, member)            
            if isinstance(value, types.StringType):
                members.append( '%s = "%s"' % (member,value) )
            else:
                members.append( '%s = %s' % (member,value) )

        sp = self.__str_spacer
        indent = ''
        if sp == '\n':
            indent = '    '

        return ( "%s%s(%s%s%s%s)"
                 % ( sp, self.classname(),
                     sp, indent,
                     string.join(members, ',%s%s'%(sp, indent)),
                     sp
                     )
                 )
    
    def __repr__(self):
        return str(self)

    def set_str_spacer(self, sp):
        self.__str_spacer = sp
    
    def classname(self):
        """Simple shortcut method to get the classname of an instance object.

        @returns: The class' name as a string.
        """
        return self.__class__.__name__

    def classmembers(self):
        return self.__members

    def set_attribute(self, key, value):
        self._frozen = False
        setattr(self, key, value)
        self._frozen = True

##     def mandatory_override(self, attribute_name):
##         if getattr(self, attribute_name) is None:
##             raise MandatoryOverrideError(self, attribute_name)

    def type_check(self, attribute_name, expected_type):
        att = getattr(self, attribute_name)
        if not isinstance(att, expected_type):
            raise TypeError(
                "The %s.%s type is expected to be %s but currently is %s."
                % (self.classname(),   attribute_name,
                   str(expected_type), type(att)      )
                )
