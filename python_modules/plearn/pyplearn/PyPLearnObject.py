"""ALL DOCUMENTATION IN THIS MODULE MUST BE REVISED!!!"""
import inspect, types

from   pyplearn                   import *
import plearn.utilities.metaprog  as     metaprog

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

class frozen_metaclass( type ):
    __setattr__ = frozen(type.__setattr__)
    

##########################################
### Main class
##########################################
class PyPLearnObject:
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

    ## See set_attribute()
    _frozen = True

    def classname(cls):
        """Classmethod to access the class name.

        See U{http://www.python.org/doc/2.3.4/lib/built-in-funcs.html}
        for an introduction to the classmethod concept. 

        @returns: The (instance) class' name as a string.
        """
        return cls.__name__
    classname = classmethod(classname)
    
    ###########################################################
    ### PyPLearnObject constructor
    def __init__(self, defaults=None, **overrides):
        """PyPLearnObject constructor.
        
        Previously, the syntax was::
        
            class InheritedDefaults:
                some_default_int = 10
        
            class Inherited( PyPLearnObject ):            
                def __init__(self, defaults=InheritedDefaults, **overrides):
                    PyPLearnObject.__init__(self, defaults, overrides)
                    some_other_things()
        
        where the double stars before I{overrides} were only in the
        declaration. Also note that a class was provided as default
        value to the I{defaults} argument.
        
        Please note that the proper way to override the
        L{PyPLearnObject} constructor is::
        
            class Inherited( L{PyPLearnObject} ):
                class Defaults:
                    some_default_int = 10
                    
                def __init__(self, defaults=None, **override):
                    L{PyPLearnObject}.__init__(self, defaults, **overrides)
                    some_other_things()
        
        Note the double stars before I{overrides} both in the
        declaration AND the forwarding.  Also, the I{defaults}
        argument takes I{None} as default instead of a classname.
        """

        ## See set_attribute()
        self._frozen = False

        to_declare = self.use_defaults(defaults)

        if overrides is not None:
            self.__dict__.update( overrides )
        
        if to_declare:
            self.declare_members( to_declare )

        ## See set_str_spacer()
        self.__str_spacer = ' '

        ## See set_attribute()
        self._frozen = self.__class__._frozen 

    ###########################################################
    ### PyPLearnObject feature: classes for default values
    def use_defaults(self, defaults):
        if defaults is None:
            if ( hasattr(self, 'Defaults') and
                 inspect.isclass(self.Defaults) ):
                defaults = self.Defaults
            else:
                return ## No defaults: nothing to do

        to_declare    = []
        defaults_dict = {}
        members_list  = inspect.getmembers(defaults)
        for (x,y) in members_list:
            if not( x[0:2] == "__" and x[-2:]=="__" ):
                self.__dict__[x] = y
                
            ## Special PyPLearnObject features
            elif x == '__declare_members__':
                ## No: some members are not declared yet
                ## self.declare_members(y)
                to_declare = y
        return to_declare
        
    ###########################################################
    ### PyPLearnObject feature: as a frozen object
    __setattr__   = frozen(object.__setattr__)
    __metaclass__ = frozen_metaclass

    def set_attribute(self, key, value):
        self._frozen = False
        setattr(self, key, value)
        self._frozen = True

    ###########################################################
    ### PyPLearnObject feature: As a cousin of a PLearn object
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
        return metaprog.public_members( self )
        
    def plearn_repr(self):
        """Any object overloading this method will be recognized by the pyplearn_driver

        NOTE THAT in most cases, given the plearn_options() method just above,
        the proper and easy way to implement this method is::
        
            return pl.NameOfTheCorespondingPLearnObject( **self.plearn_options() )

        and that is what this default version does.
        """
        return eval("pl.%s( **self.plearn_options() )" % self.classname())

    ###########################################################
    ### PyPLearnObject feature: __declare_members__ 
    def __str__(self):
        members = self.classmembers()
        if len(members) == 0:
            members = metaprog.public_members( self )
        return metaprog.instance_to_string(self, members, self.__str_spacer) 

    def __repr__(self):
        return str(self)
    
    def set_str_spacer(self, sp):
        self.__str_spacer = sp

    def classmembers(self):
        if hasattr(self, '__member'):
            return self.__members
        return []

    def declare_members(self, declared_members):
        self.__members = []
        for (att, typ) in declared_members:
            self.__members.append(att)
            if typ is not None:
                self.type_check(att, typ)

    def type_check(self, attribute_name, expected_type):
        att = getattr(self, attribute_name)
        if not isinstance(att, expected_type):
            raise TypeError(
                "The %s.%s type is expected to be %s but currently is %s."
                % (self.classname(),   attribute_name,
                   str(expected_type), type(att)      )
                )
    ###########################################################                

    def __lshift__(self, other):
        cls = self.__class__
        if not isinstance(other, cls):
            raise TypeError( "%s type expected in __lshift__: %s received."
                             % (cls, type(other))
                             )

        ## Unfreeze the object and keep track of the internal state
        frozen = self._frozen
        self._frozen = False
        
        ## The default values for this instance
        defaults  = metaprog.public_members(cls.Defaults)

        ## The current values for the other object
        othervals = metaprog.public_members( other )

        ## For each public members
        for (member, otherval) in othervals.iteritems():

            default = None
            if defaults.has_key(member):
                default = defaults[member]

            override = None
            if hasattr(self, member):
                override = getattr(self, member)

            ## If the default value was not overriden
            if override == default:
                ## Then override it with the other
                ## object's override
                setattr(self, member, otherval)

        ## Set back to the internal state
        self._frozen = frozen
        return self



##########################################
### Embedded test/tutorial
##########################################
if __name__ == "__main__":
    print "---"
    print "PyPLearnObject.py builtin test/tutorial."
    print "---"
    
    obj = PyPLearnObject()
    print obj

    print "+++ Management of 'Defaults'"
    class Bidon( PyPLearnObject ):
        class Defaults:
            allo = 'allo'
    print "\t",Bidon()

    class OtherDefaults( Bidon.Defaults ):
        hi = 'hi'
    print "\t",Bidon( OtherDefaults )
    print "or"
    print "\t",Bidon( defaults = OtherDefaults )
    print "and its also possible to add members within the "
    print "constructor's keyword arguments"
    print "\t",Bidon( defaults = OtherDefaults, from_ctor = "some other member" )
    print

    print "+++ A major difference with the prior version when inheriting:"
    class Inherited( PyPLearnObject ):
        class Defaults:
            some_default_int = 10
            
        def __init__(self, defaults=None, **overrides):
            PyPLearnObject.__init__(self, defaults, **overrides)
            print(PyPLearnObject.__init__.__doc__)
    Inherited()

    print
    print "+++ Testing 'frozen' behaviour"
    try:
        obj.add_member = "add"
        print "obj._frozen is ",obj._frozen
        raise RuntimeError("Add succeeded on a frozen object!")
    except AttributeError, e:
        print "Default behavior: Add failed as expected."
        print

    class NotFrozen( PyPLearnObject ):
        _frozen = False
    not_frozen = NotFrozen()
    not_frozen.add_member = 'added'
    print not_frozen
    
    print

    print "+++ Classmethod classname()"
    print "Classname: %s" % PyPLearnObject.classname()
    print "Classname from instance: %s" %obj.classname()
    print

    
