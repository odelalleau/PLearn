"""Tools for emulating PLearn objects.

The string representation of a PyPLearnObject simply looks like calling the
constructor for this object. So that::

    obj = PyPLearnObject()
    print obj

    ### Will print
    PyPLearnObject( )
    
and::

    obj = PyPLearnObject( foo='bar' )
    print obj

    ### Will print
    PyPLearnObject(
        foo                            = "bar"
        )
    

+++ The Defaults class:

At init, the Defaults class is parsed and its attributes are used to set
default attributes to the new instance. The current protocol is to
instanciate any classes provided as attributes in Defaults or in
overrides. The current version uses no arguments at instanciation.

For instance, if::

    class Dummy( PyPLearnObject ):
        class Defaults:
            hello = 'hello'
then::

    obj = Dummy()
    print obj

    ### Will print
    Dummy(
        hello                          = "hello"
        )
    

Note that you could well derive from this default class and provide the
derived class as an override to the embedded one. That is, if::

    class SubDefaults( Dummy.Defaults ):
        hi = 'hi'

then::

    obj = Dummy( Defaults = SubDefaults )
    print obj

    ### Will print
    Dummy(
        hello                          = "hello",
        hi                             = "hi"
        )
    
while::

    obj = Dummy( Defaults = SubDefaults, hello = 'HELLO' )
    print obj

    ### Will print
    Dummy(
        hello                          = "HELLO",
        hi                             = "hi"
        )
    
The hello keyword argument in the above initialization is treated as an
override to the default value of the hello attribute.

The overrides may also contain attributes that have no default values::

    obj = Dummy( Defaults = SubDefaults, kwarg = 'new_attr' )
    print obj

    ### Will print
    Dummy(
        hello                          = "hello",
        hi                             = "hi",
        kwarg                          = "new_attr"
        )
    

Since instances of any PyPLearnObject's subclass may not be added new
attributes after the instanciation, the above mecanism is a simple way to
get an instance to have specific attributes. It is simpler than defining a
Defaults subclass for a sinlge use::

    ## Frozen test
    try:
        obj = Dummy() 
        obj.add_member = "add"
        return "obj._frozen is %s" % str(obj._frozen)
        raise RuntimeError("Add succeeded on a frozen object!")
    except AttributeError, e:
        return "Add failed as expected."
        
    ## Will return        
    Add failed as expected.
An important method of the PyPLearnObject is the plearn_repr() method,
which is recognized by the pyplearn._plearn_repr mecanism. The plearn
representation is defined to be::

    Classname(
        plearn_option1 = option_value1,
        plearn_option2 = option_value2,
        ...
        plearn_optionN = option_valueN
        )

where <Classname> is the name you give to the PyPLearnObject's
subclass. That format is very alike the __str__ behavior except that the
L{plearn_options()} are considered to be all attributes whose names do not
start with an underscore. Those are said public, while any attribute
starting with at least one underscore is considered internal (protected '_'
or private '__').

    obj = Dummy( _internal1 = "internal",
                 _internal2 = "very internal",
                 public1    = [1, 2, 3],
                 public2    = "public again"
                 )

    ## str(obj) returns
    Dummy(
        _internal1                     = "internal",
        _internal2                     = "very internal",
        hello                          = "hello",
        public1                        =  [1, 2, 3],
        public2                        = "public again"
        )

    ## While obj.plearn_repr() returns
    Dummy(
        hello                          = "hello",
        public1                        =  [1, 2, 3],
        public2                        = "public again"
        )
"""
__cvs_id__ = "$Id: PyPLearnObject.py,v 1.13 2005/02/18 22:19:06 dorionc Exp $"

import inspect, string, types

from   pyplearn                   import PLearnRepr
from   plearn.utilities.toolkit   import no_none
import plearn.utilities.metaprog  as     metaprog

__all__ = [ "PyPLearnObject", "PyPLearnList",
            "frozen_metaclass", "public_attribute_predicate" ]

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
            raise AttributeError("You cannot add attributes to an instance of %s" % self.__class__.__name__)
    return set_attr

##########################################
### Helper classes
##########################################
class frozen_metaclass( type ):
    _frozen     = True
    __setattr__ = frozen(type.__setattr__)
    def __init__(cls, name, bases, dict):
        cls._frozen     = True
        cls.__setattr__ = frozen(object.__setattr__)
 
class _manage_attributes:
    """Attributes management for PyPLearnObject.

    This class encapsulates the attributes' related concerns.

        1) The metaclass: This class (and therefore PyPLearnObject) is a
        new style class: it uses a metaclass. Here, this metaclass modifies
        the __setattr__ method so that no new attributes can be added to an
        instance after its initialization is done. Each instance will
        therefore have for attributes the default attributes in Defaults,
        if any -- see 2), and the attributes defined in the keyword
        arguments of the __init__ method, referred to as overrides. The
        metaclass also defines default right operators for all defined left
        operators::

            ## With this declaration
            class AffineTransform( PyPLearnObject ):
                def __init__(self, pipe_stage, a=1, b=0):
                    PyPLearnObject.__init__(
                        self,
                        pipe_stage = pipe_stage,
                        a          = a,
                        b          = b
                        )

                def __add__(self, val):
                     if not operator.isNumberType(val):
                         raise TypeError
        
                     return AffineTransform( self.pipe_stage,
                                             self.a,
                                             self.b + val   )

            ## The following statement, calling __radd__, works
            10 + AffineTransform(pipe_stage, 1, 0)
            ## even though __radd__ is not explicitly defined
        
        2) The Defaults class: At init, the Defaults class is parsed and
        its attributes are used to set default attributes to the new
        instance. The current protocol is to instanciate any classes
        provided as attributes in Defaults or in overrides. The current
        version uses no arguments at instanciation.

        B{The Defaults class to use for a new instance may be overriden by
        using the Defaults keyword argument at the instance's initialization}
        
        3) The __str__ and __repr__ methods are defaulted to return a
        string that could be used for serialization purpose (public and
        internal members are wrote to a initialization-like syntax.)
    """
    class __metaclass__( type ):
        """...

        This metaclass also manages the _subclasses attribute if any; all
        subclasses created are added (name -> class object) to the
        dictionnary. Classes whose name starts with at least one underscore
        are neglected.
        """        
        _frozen     = True
        __setattr__ = frozen(type.__setattr__)

        _rop_names = [
        '__radd__', '__rsub__', '__rmul__', '__rdiv__',
        '__rtruediv__', '__rfloordiv__', '__rmod__',
        '__rdivmod__', '__rpow__', '__rlshift__',
        '__rrshift__', '__rand__', '__rxor__', '__ror__'
        ]
        
        def __init__(cls, name, bases, dict):
            cls._frozen = False
            
            super(type, cls).__init__(name, bases, dict) 
            for rop_name in cls._rop_names:
                lop_name = rop_name[0:2] + rop_name[3:]
                if ( not dict.has_key(rop_name)
                     and dict.has_key(lop_name) ):
                    setattr( cls, rop_name, dict[lop_name] )
                    
            cls._frozen     = True
            cls.__setattr__ = frozen(object.__setattr__)

            if hasattr(cls, '_subclasses'):
                assert isinstance( cls._subclasses, type(dict) ), type( cls._subclasses )
                if not name.startswith('_'):
                    cls._subclasses[name] = cls
        
        
    class Defaults:
        pass

    def _allow_undefined_overrides( self ):
        """Should overrides having no defaults be allowed.

        The __init__ method accepts any keyword arguments. If a given
        keyword is not class variable in Defaults, the default behavior is
        to consider that keyword argument as being an extra attribute to
        add to the instance::

            class Foo( PyPLearnObject ): 
                class Defaults:
                    foo = "the foo attribute"

            f = Foo( bar = "the bar attribute" )

            print f.foo         ## prints "the foo attribute"
            print f.bar         ## prints "the bar attribute"

        To change that behavior, one may simply override this method and return False::

            class FooNoBar( PyPLearnObject ): 
                class Defaults:
                    foo = "the foo attribute"

                def __allow_undefined_overrides( self ): return False 

            f = FooNoBar( bar = "the bar attribute" )
            ## AttributeError: Trying to set undefined attributes {'bar': 'the bar attribute'} on a FooNoBar
        """
        return True

    def __init__(self, **overrides):
        ## Managing frozen behavior
        self._frozen = False
        
        if overrides.has_key( "Defaults" ):
            self.Defaults = overrides.pop( "Defaults" )
            ## assert inspect.isclass( self.Defaults )

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

        if self._allow_undefined_overrides():
            ## Overrides may still contain pairs -- attributes with no default
            ## values.
            for (attribute_name, attribute_value) in overrides.iteritems():
                ## Do not use __set_attribute__ in __init__!
                self._init_attribute_protocol( attribute_name, attribute_value )
        elif len(overrides):
            raise AttributeError( "Trying to set undefined attributes %s on a %s"
                                  % ( str(overrides), self.classname() )
                                  )
                                  
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
        attributes in Defaults. That is, if::

            class Defaults:
                default_attribute = SomeClass

        Then the 'default_attribute' will be affected an instance of subclass. The current version
        uses no arguments at instanciation::

            self.default_attribute = SomeClass() 

        The same will be done for overrides if the current class does not
        contain an internal class having the override's name. If such a
        class exists, however, it is assumed that the internal class is
        properly managed as a class.

        The next version will manage tuples having a class as first
        element.  The class will be instanciated with the rest of the tuple
        as __init__ arguments.
        """
        if inspect.isroutine(attribute_value):
            raise TypeError( "Routine types are not supported as PyPLearnObject attribute "
                             "values (In %s for %s, value is %s)."
                             % ( self.classname(), attribute_name, attribute_value )
                             )

        ## See the method docstring for the class management protocol.
        internal_class = False
        
        if inspect.isclass( attribute_value ):
            if hasattr( self, attribute_name ):
                internal_class  = True
            else:
                attribute_value = attribute_value()

        if self._ordered_len == 0 and not internal_class:
            self._list_of_attributes.append(attribute_name)
            self._list_of_attributes.sort()

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
        return PLearnRepr.plearn_repr( self.classname(), self.attribute_pairs() ) 

    def __repr__(self):
        """Simply returns str(self)."""
        return str(self)
    
            
##########################################
### Main class
##########################################
class PyPLearnObject( _manage_attributes ):
    """Class from which to derive python objects that emulate PLearn ones.

    This class provides any of its instances with a plearn_repr() method
    recognized by the pyplearn._plearn_repr mecanism. The plearn
    representation is defined to be::

        Classname(
            plearn_option1 = option_value1,
            plearn_option2 = option_value2,
            ...
            plearn_optionN = option_valueN
            )

    where <Classname> is the name you give to the PyPLearnObject's
    subclass. The L{plearn_options()} are considered to be all attributes
    whose names do not start with an underscore. Those are said public,
    while any attribute starting with at least one underscore is considered
    internal (protected '_' or private '__').

    The learn more about the way this class manages attribute, see the
    L{_manage_attributes} class defined above.
    """
    ###########################################################
    ## Classmethods:
    ## See
    ## U{http://www.python.org/doc/2.3.4/lib/built-in-funcs.html}
    ## for an introduction to the classmethod concept.
    def classname(cls):
        """Classmethod to access the class name. 

        @returns: The (instance) class' name as a string.
        """
        return cls.__name__
    classname = classmethod(classname)
    
    ###########################################################
    ### PyPLearnObject constructor
    def __init__(self, **overrides):
        """PyPLearnObject constructor.
        
        Please note that the proper way to override the
        L{PyPLearnObject} constructor is::
        
            class Inherited( L{PyPLearnObject} ):
                class Defaults:
                    some_default_int = 10
                    
                def __init__(self, **override):
                    PyPLearnObject.__init__(self, **overrides)
                    some_other_things()
        
        Note the double stars before I{overrides} both in the declaration
        AND the forwarding.  Also note that the Defaults class is
        automatically managed by the _manage_attributes super class.
        """
        
        _manage_attributes.__init__(self, **overrides)

    ###########################################################
    ### PyPLearnObject feature: As a cousin of a PLearn object
    def plearn_options(self):
        """Returns a list of pairs of all attributes of this instance that are considered to be plearn options.

        B{IMPORTANT}: Subclasses of PLearnObject MUST declare any
        internal member, i.e. B{any member that SHOULD NOT be
        considered by plearn_options()}, with a name that starts with
        at least one underscore '_'.

        Indeed, this method returns a list of pairs containing all and
        only the attributes whose names do not start with an
        underscore. Note, however, that no given order may be assumed
        in the attributes dictionnary returned.

        @return: List of pairs of all attributes of this instance that are
        considered to be plearn options.
        """
        return self.public_attribute_pairs()
        
    def plearn_repr(self):
        """Method recognized by the pyplearn._plearn_repr mecanism.

        That method should never be overriden.
        """
        try:
            return PLearnRepr.plearn_repr( self.classname(), self.plearn_options() )
        except TypeError, e:            
            raise TypeError( "Instance of %s forwarded an invalid member to plearn_repr"
                             " [ %s ]" % ( self.classname(), str(e) )
                             )

    ## Old and uncommented functionnality...
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
    
    This class is to be used whenever the attributes must be considered
    as the elements of a list. Hence, the plearn_repr() returns a list
    representation.

    If the order of the attributes in the plearn_repr is considered
    important, it must be imposed by the order of the member names
    appearance in the __ordered_attr__ list. See the L{_manage_attributes}
    class declaration for more information on __ordered_attr__ and the
    protocol associated with it.
    """
    
    def __len__(self):
        return len( self._list_of_attributes )

    def __getitem__( self, key ):
        raise NotImplementedError

    def __setitem__( self, key, value ):
        raise NotImplementedError

    def __delitem__( self, key ):
        raise NotImplementedError

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
        return PLearnRepr.list_plearn_repr( self.to_list() )

    def to_list(self):
        return no_none([ elem for elem in iter(self) ])

##########################################
### Embedded test/tutorial
##########################################
if __name__ == "__main__":
    from plearn.utilities.toolkit   import boxed_lines
    from plearn.utilities.verbosity import *

    set_verbosity( 1 )
    vprint.highlight( ["PyPLearnObject.py builtin test/tutorial."] )

    ## For examples
    PLearnRepr.indent_level += 1 

    def example( s ):
        obj = eval(s)
        return """
    obj = %s
    print obj

    ### Will print
    %s
    """ % ( s, str(obj) )

    class Dummy( PyPLearnObject ):
        class Defaults:
            hello = 'hello'

    class SubDefaults( Dummy.Defaults ):
        hi = 'hi'


    def frozen_test():
        try:
            obj = Dummy() 
            obj.add_member = "add"
            return "obj._frozen is %s" % str(obj._frozen)
            raise RuntimeError("Add succeeded on a frozen object!")
        except AttributeError, e:
            return "Add failed as expected."


    def repr_vs_str():
        obj = Dummy( _internal1 = "internal",
                     _internal2 = "very internal",
                     public1    = [1, 2, 3],
                     public2    = "public again"
                     )
        return (str(obj), obj.plearn_repr())

    tutorial = ("""
The string representation of a PyPLearnObject simply looks like calling the
constructor for this object. So that::
""" + example( "PyPLearnObject()" ) + """
and::
""" + example( "PyPLearnObject( foo='bar' )" ) + """

+++ The Defaults class:

At init, the Defaults class is parsed and its attributes are used to set
default attributes to the new instance. The current protocol is to
instanciate any classes provided as attributes in Defaults or in
overrides. The current version uses no arguments at instanciation.

For instance, if::

    class Dummy( PyPLearnObject ):
        class Defaults:
            hello = 'hello'
then::
""" + example( "Dummy()" ) + """

Note that you could well derive from this default class and provide the
derived class as an override to the embedded one. That is, if::

    class SubDefaults( Dummy.Defaults ):
        hi = 'hi'

then::
""" + example( "Dummy( Defaults = SubDefaults )" ) + """
while::
""" + example( "Dummy( Defaults = SubDefaults, hello = 'HELLO' )" ) + """
The hello keyword argument in the above initialization is treated as an
override to the default value of the hello attribute.

The overrides may also contain attributes that have no default values::
""" + example( "Dummy( Defaults = SubDefaults, kwarg = 'new_attr' )" ) + """

Since instances of any PyPLearnObject's subclass may not be added new
attributes after the instanciation, the above mecanism is a simple way to
get an instance to have specific attributes. It is simpler than defining a
Defaults subclass for a sinlge use::

    ## Frozen test
    try:
        obj = Dummy() 
        obj.add_member = "add"
        return "obj._frozen is %s" % str(obj._frozen)
        raise RuntimeError("Add succeeded on a frozen object!")
    except AttributeError, e:
        return "Add failed as expected."
        
    ## Will return        
    """ + frozen_test() + """
An important method of the PyPLearnObject is the plearn_repr() method,
which is recognized by the pyplearn._plearn_repr mecanism. The plearn
representation is defined to be::

    Classname(
        plearn_option1 = option_value1,
        plearn_option2 = option_value2,
        ...
        plearn_optionN = option_valueN
        )

where <Classname> is the name you give to the PyPLearnObject's
subclass. That format is very alike the __str__ behavior except that the
L{plearn_options()} are considered to be all attributes whose names do not
start with an underscore. Those are said public, while any attribute
starting with at least one underscore is considered internal (protected '_'
or private '__').

    obj = Dummy( _internal1 = "internal",
                 _internal2 = "very internal",
                 public1    = [1, 2, 3],
                 public2    = "public again"
                 )

    ## str(obj) returns
    %s

    ## While obj.plearn_repr() returns
    %s
    """ % repr_vs_str() 
    )
    vprint(tutorial)    


##     class lop( default_roperators ):
##         def test(cls):
##             t = cls()
##             print t + 10 
##             print

##             print 10 + t
##             print
##             try:
##                 t << 10
##                 print "SHOULD NOT WORK!!!"
##             except Exception:
##                 print "Failed as expected."

##         test = classmethod(test)

##         def __init__(self, val=0):
##             default_roperators.__init__(self)
##             self.val = val

##         def __str__(self):
##             return "lop(%d)"%self.val

##         def __add__(self, val):
##             print val
##             return lop( self.val + val )

##     lop.test()
