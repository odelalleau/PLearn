"""ALL DOCUMENTATION IN THIS MODULE MUST BE REVISED!!!"""
__cvs_id__ = "$Id: PyPLearnObject.py,v 1.8 2005/02/07 21:20:05 dorionc Exp $"

import inspect, string, types

from   pyplearn                   import *
from   plearn.utilities.toolkit   import no_none
import plearn.utilities.metaprog  as     metaprog

__all__ = [ "PyPLearnObject", "PyPLearnList", "frozen_metaclass" ]

##########################################
### Helper functions 
##########################################

def builtin_predicate( name ):
    """Is I{name} a builtin-like name.

    @returns: name.startswith('__') and name.endswith('__')
    """
    return name.startswith('__') and name.endswith('__')

def public_attribute_predicate( name, value ):
    """Return wether or not attribute name is considered a public attribute.

    Are consider public attributes whose I{name} does not an underscore and
    I{value} is not a method, a function or a class.
    """
    return not ( name.startswith("_")
                 or inspect.ismethod(value)
                 or inspect.isfunction(value)
                 or inspect.isclass(value)
                 )

def frozen(set):
    "Raise an error when trying to set an undeclared name."
    def set_attr(self,name,value):
        if not self._frozen or hasattr(self,name):
            set(self,name,value) 
        else:
            raise AttributeError("You cannot add attributes to %s" % self)
    return set_attr

##########################################
### Helper classes
##########################################
class InstancePrinter:
    """This class manages pretty formatting of plearn representations."""
    indent_level   = 0

    def format( openstr, attribute_strings, closestr ):
        separator = ( '\n%s'
                      % string.ljust('', 4*InstancePrinter.indent_level)
                      )
                      
        stringnified_attributes = ' '
        if len(attribute_strings) > 0:
            stringnified_attributes = "%s%s%s" % (
                separator,
                string.join( attribute_strings, ',%s'%separator ),
                separator
                )

        ## Indent level goes down 1
        InstancePrinter.indent_level -= 1

        return ( "%s%s%s" % ( openstr, stringnified_attributes, closestr ) )
    format = staticmethod( format )

    def list_plearn_repr( list_of_attributes ):
        ## Indent level goes up 1
        InstancePrinter.indent_level += 1
        
        attribute_strings = [ _plearn_repr(attr)
                              for attr in list_of_attributes ]
        
        return InstancePrinter.format( "[ ", attribute_strings, " ]" )
    list_plearn_repr = staticmethod( list_plearn_repr )
        
    def plearn_repr( classname, attribute_pairs ):
        ## Indent level goes up 1
        InstancePrinter.indent_level += 1

        pretty    = lambda attr_name: string.ljust(attr_name, 30)
        attribute_strings = [ '%s = %s' % ( pretty(attr),
                                            _plearn_repr(val) )
                              for (attr, val) in attribute_pairs ]

        return InstancePrinter.format( "%s("%classname,
                                       attribute_strings,
                                       ")"
                                       )
    plearn_repr = staticmethod( plearn_repr )

class frozen_metaclass( type ):
    _frozen     = True
    __setattr__ = frozen(type.__setattr__)
    def __init__(cls, name, bases, dict):
        cls._frozen     = True
        cls.__setattr__ = frozen(object.__setattr__)
 
class _manage_attributes:
    """Attributes management for PyPLearnObject.

    This class encapsulates
    """
    __metaclass__ = frozen_metaclass

    class Defaults:
        pass

    def __init__(self, **overrides):
        ## Managing frozen behavior
        self._frozen = False
        
        if overrides.has_key( "Defaults" ):
            self.Defaults = overrides.pop( "Defaults" )
            assert inspect.isclass( self.Defaults )

        ## Managing the attribute ordering protocol
        self._list_of_attributes = []
        if overrides.has_key('__ordered_attr__'):
            self._list_of_attributes.extend( overrides['__ordered_attr__'] )
        elif hasattr(self.Defaults, '__ordered_attr__'):
            self._list_of_attributes.extend( self.Defaults.__ordered_attr__ )

        ## The current length of the attribute list is the length of the
        ## __ordered_attr__ list, if any. Otherwise, it's 0.
        self._ordered_len        = len(self._list_of_attributes)

        ## Attributes defined in default
        default_attributes = inspect.getmembers( self.Defaults )

        ## Setting appropriate values for all default and overriden attributes
        for (attribute_name, attribute_value) in default_attributes:

            ## Builtin attributes are neglected
            if builtin_predicate( attribute_name ):
                continue

            ## The override dominates the default value
            if overrides.has_key( attribute_name ):
                attribute_value = overrides.pop( attribute_name )

            ## Do not use __set_attribute__ in __init__!
            self._init_attribute_protocol( attribute_name, attribute_value )

        ## Overrides may still contain pairs -- attributes with no default
        ## values.
        for (attribute_name, attribute_value) in overrides.iteritems():
            ## Do not use __set_attribute__ in __init__!
            self._init_attribute_protocol( attribute_name, attribute_value )
            
        ## Ensure that the length of ordered attributes list is
        ## either zero or equal to the number of attributes
        assert ( self._ordered_len == 0 or
                 self._ordered_len == len(self._list_of_attributes)
                 )

        ## Managing frozen behavior
        self._frozen = self.__class__._frozen

    def _init_attribute_protocol( self, attribute_name, attribute_value ):
        """Used in __init__() to initialize attributes.
        
        The current protocol is to instanciate any classes provided as
        attributes in Defaults or in overrides. The current version uses no
        arguments at instanciation.

        The next version will manage tuples having a class as first
        element.  The class will be instanciated with the rest of the tuple
        as __init__ arguments.
        """
        assert not inspect.isroutine(attribute_value)
        if self._ordered_len == 0:
            self._list_of_attributes.append(attribute_name)
            self._list_of_attributes.sort()
        
        if inspect.isclass( attribute_value ):
            attribute_value = attribute_value()
    
        self.__dict__[attribute_name] = attribute_value        
            
    def attribute_pairs(self):
        """Returns the list of (name, value) pairs for all attributes."""
        return [ ( att_name, getattr(self, att_name) )
                 for att_name in self._list_of_attributes
                 ]

    def attribute_names( self ):
        """Returns the list of names of all attributes."""
        return copy.deepcopy( self._list_of_attributes )        

    def public_attribute_pairs(self):
        """Returns the list of (name, value) pairs for public attributes.

        Are considered public the attributes whose names does not start
        with an underscore.
        """
        return [ ( att_name, getattr(self, att_name) )
                 for att_name in self._list_of_attributes
                 if not att_name.startswith('_')
                 ]

    def public_attribute_names( self ):
        """Returns the list of names of public attributes.

        Are considered public the attributes whose names does not start
        with an underscore.
        """
        return [ att_name
                 for att_name in self._list_of_attributes
                 if not att_name.startswith('_')
                 ]

    def __str__(self):
        """Prints all attributes pair, not just public ones."""
        return InstancePrinter.plearn_repr( self.classname(), self.attribute_pairs() ) 

    def __repr__(self):
        """Simply returns str(self)."""
        return str(self)
    
            
##########################################
### Main class
##########################################
class PyPLearnObject( _manage_attributes ):
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
    def __init__(self, **overrides):
        """PyPLearnObject constructor.
        
        Previously, the syntax was::
        
            class InheritedDefaults:
                some_default_int = 10
        
            class Inherited( PyPLearnObject ):            
                def __init__(self, defaults=InheritedDefaults, **overrides):
                    PLearnObject.__init__(self, defaults, overrides)
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
                    PyPLearnObject.__init__(self, defaults, **overrides)
                    some_other_things()
        
        Note the double stars before I{overrides} both in the
        declaration AND the forwarding.  Also, the I{defaults}
        argument takes I{None} as default instead of a classname.
        """
        _manage_attributes.__init__(self, **overrides)

    def set_attribute(self, key, value):
        raise DeprecationWarning
        self._frozen = False
        setattr(self, key, value)
        self._frozen = True

    ###########################################################
    ### PyPLearnObject feature: As a cousin of a PLearn object
    def plearn_options(self):
        """Returns a list of pairs of all members of this instance that are considered to be plearn options.

        B{IMPORTANT}: Subclasses of PLearnObject MUST declare any
        internal member, i.e. B{any member that SHOULD NOT be
        considered by plearn_options()}, with a name that starts with
        at least one underscore '_'.

        Indeed, this method returns a dictionnary containing all and
        only the members whose names do not start with an
        underscore. Note, however, that no given order may be assumed
        in the members dictionnary returned.

        @return: List of pairs of all members of this instance that are
        considered to be plearn options.
        """
        return self.public_attribute_pairs()
        
    def plearn_repr(self):
        """Any object overloading this method will be recognized by the pyplearn_driver

        NOTE THAT in most cases, given the plearn_options() method just above,
        the proper and easy way to implement this method is::
        
            return pl.NameOfTheCorespondingPLearnObject( **self.plearn_options() )

        and that is what this default version does.
        """
        return InstancePrinter.plearn_repr( self.classname(), self.plearn_options() )

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


class PyPLearnList( PyPLearnObject ):
    """Emulates a TVec of PyPLearnObject.
    
    This class is to be used whenever the members must be considered
    as the elements of a list. Hence, the plearn_repr() returns a list
    representation.

    If the order of the members in the plearn_repr is considered
    important, it must be imposed by the order of the member names
    appearance in the __declare_members__ list. See the PyPLearnObject
    class declaration for more information on __declare_members__ and the
    protocol associated with it.
    """
    def __len__(self):
        return len( self.__member_pairs() )

    def __getitem__( self, key ):
        return getattr( self, key )

    def __setitem__( self, key, value ):
        setattr( self, key, value )

    def __delitem__( self, key ):
        setattr( self, key, None )

    class _iterator:
        def __init__(self, pairs):
            self.pos   = -1
            self.end   = len(pairs)
            self.pairs = pairs

        def __iter__(self):
            return self

        def next(self):
            self.pos += 1
            if self.pos == self.end:
                raise StopIteration

            ## Returning the value
            current_pair = self.pairs[self.pos]
            return current_pair[1]
        
    def __iter__(self):        
        return self._iterator( self.public_attribute_pairs() )
        
    def plearn_repr(self):
        return InstancePrinter.list_plearn_repr( self.to_list() )

    def to_list(self):
        return no_none([ elem for elem in iter(self) ])

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

    
