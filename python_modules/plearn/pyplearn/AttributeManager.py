import copy, inspect

##########################################
### Helper functions 
##########################################

def builtin_predicate( name ):
    """Is I{name} a builtin-like name.

    @returns: name.startswith('__') and name.endswith('__')
    """
    return name.startswith('__') and name.endswith('__')

def frozen_public_interface(set):
    "Raise an error when trying to set an undeclared 'public' name."
    def set_attr(self,name,value):
        if not self._frozen or name.startswith('_') or hasattr(self,name):
            set(self,name,value) 
        else:
            raise AttributeError("You cannot add attributes to an instance of %s" % self.__class__.__name__)
    return set_attr

class AttributeManager:
    """Attributes management for high level objects.

    This class encapsulates the attributes' related concerns.

        1. The metaclass: This class (and therefore PyPLearnObject) is a
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
        
        2. The Defaults class: At init, the Defaults class is parsed and
        its attributes are used to set default attributes to the new
        instance. The current protocol is to instanciate any classes
        provided as attributes in Defaults or in overrides. The current
        version uses no arguments at instanciation.

        B{The Defaults class to use for a new instance may be overriden by
        using the Defaults keyword argument at the instance's initialization}
        
        3. The __str__ and __repr__ methods are defaulted to return a
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
        __setattr__ = frozen_public_interface(type.__setattr__)

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
            cls.__setattr__ = frozen_public_interface(object.__setattr__)

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
                attribute_value = overrides.pop( attribute_name  )
            else:
                attribute_value = copy.deepcopy( attribute_value ) 

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

    def add_attribute( self, attname, attvalue ):
        if hasattr( self, attname ):
            raise AssertionError( "Can not add %s to object; attribute already exists." % attname )
        self.__dict__[ attname ] = attvalue
        self._list_of_attributes.append( attname )
            
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

    class __attribute_iterator:
        def __init__(self, attr_pairs):
            self.pos        = -1
            self.attr_pairs = attr_pairs

        def __iter__(self):
            return self

        def next(self):
            self.pos += 1
            if self.pos == len(self.attr_pairs):
                raise StopIteration

            ## Returning the value
            return self.attr_pairs[self.pos]
        
    def iteritems( self ):        
        return self.__attribute_iterator( self.public_attribute_pairs() )

    def __str__( self ):
        return object.__str__( self )

    def __repr__(self):
        """Simply returns str(self)."""
        return str(self)
    
if __name__ == "__main__":
    class Test:
        test_attr = "attribute"
        
    print AttributeManager( emulate = Test() )

    
