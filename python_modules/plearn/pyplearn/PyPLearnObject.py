"""Class Emulating PLearn Objects in Python

The class contained in this module is to be used as super class for almost
any Python class emulating a PLearn cousin class.
"""
__version_id__ = "$Id$"

import copy, inspect, re
from   plearn.pyplearn.plearn_repr import *
import plearn.utilities.metaprog   as     metaprog

#
# Helper functions 
#
def option_predicate( name, value ):
    return not ( name.startswith("_")
                 or inspect.ismethod(value)
                 or inspect.isfunction(value)
                 or inspect.isclass(value)
                 or inspect.isbuiltin(value)
                 )

def ploption( function ):
    """Decorator for function declaring PLearn options."""
    class PLOption:
        def __init__( self, _callable, *args, **kwargs ):
            self._callable = _callable
            self.args      = args
            self.kwargs    = kwargs

        def __call__( self ):
            return self._callable( *self._args, **kwargs )

    option = PLOption( function ) # , *args, **kwargs )
    raise NotImplementedError(
        "Will return 'option'. Please get PLOption out of the function"
        )
    
#
#  Classes
#
class PLOptionError( AttributeError ): pass

class PyPLearnObject( object ):
    """A class from which to derive python objects that emulate PLearn ones.

    This class provides any of its instances with a plearn_repr() method
    recognized by PyPLearn's plearn_repr mecanism. The plearn
    representation is defined to be::

        Classname(
            plearn_option1 = option_value1,
            plearn_option2 = option_value2,
            ...
            plearn_optionN = option_valueN
            )

    where <Classname> is the name you give to the PyPLearnObject's
    subclass. The L{PLearn} options are considered to be all attributes
    whose names do not start with an underscore. Those are said public,
    while any attribute starting with at least one underscore is considered
    internal (protected '_' or private '__').

    Protected and private attributes are not affected by the option mecanisms.
    """
    __instances = []

    #
    #  Classmethods:
    #   See
    #    U{http://www.python.org/doc/2.3.4/lib/built-in-funcs.html}
    #    for an introduction to the classmethod concept.
    #
    def allow_unexpected_options( cls ):
        """May overrides contain undefined options?

        The __init__ method accepts any keyword arguments. If a given
        keyword is not class variable, the default behavior is to consider
        that keyword argument as being an extra option to add to the
        instance::

            class Foo( PyPLearnObject ): 
                foo = "the foo option"

            f = Foo( bar = "the bar option" )

            print f.foo         ## prints "the foo option"
            print f.bar         ## prints "the bar option"

        To change that behavior, one may simply override this method and return False::

            class FooNoBar( PyPLearnObject ): 
                foo = "the foo option"
                def allow_unexpected_options( self ): return False 

            f = FooNoBar( bar = "the bar option" )
            ## PLOptionError: Trying to set undefined options {'bar': 'the bar option'} on a FooNoBar
        """
        return True
    allow_unexpected_options = classmethod( allow_unexpected_options )
    
    def classname( cls ):
        """Classmethod to access the class name. 

        @returns: The (instance) class' name as a string.
        """
        return cls.__name__
    classname = classmethod(classname)

    def instances( cls ):
        """Returns a list of this I{cls} instances.

        Notes that if the class has subclass, instances of the subclasses
        will be returned too.
        """
        ilist = []
        for instance in cls.__instances:
            if isinstance( instance, cls ):
                ilist.append(instance)
        return ilist
    instances = classmethod(instances)

    def option_names( klass, ordered = None ):
        """Returns the names of options having default values.
        
        This class method introspect the class to get the names of options
        for which default values were provided in any of the current class
        or its parent classes.

        Note that the mecanism is not valid for idle use: if you subclass
        PyPLearnObject within a Python idle, you should return an explicit
        list of option names.

        @param klass: The class object (classmethod)
        
        @param ordered: The list of ordered option names to fill. Mainly
        for internal recursive use.
        @type  ordered: list

        @returns: Names of options having defaults.
        @rtype: list of str.
        """
        assert issubclass( klass, PyPLearnObject )
        if ordered is None:
            ordered = []

        # Recursively inspect base class first.
        if klass == PyPLearnObject:
            return ordered
        else:
            ordered = klass.__mro__[1].option_names( ordered )

        try:
            lines, lstart = inspect.getsourcelines( klass )
        except Exception, err:
            raise PLOptionError( 'Impossible to parse options for class %s (%s: %s). '
                                 'Please override the classmethod option_names() and '
                                 'provide an explicit list of option names.'
                                 % (klass.__name__, err.__class__.__name__, err)
                                 )

        # Deducing the class indentation from the position of the class
        # keyword.
        class_declaration = lines.pop(0)
        class_indent      = class_declaration.find('class')
        body_indent       = class_indent+4

        # The body lines will be stripped of the body indentation so that
        # class attributes should be defined at the begging of the line ---
        # '^'. The protected and private attributes are neglected ---
        # (?!_). The affectation will be afterward tested to neglect
        # staticmethod and classmethod decorator-like affectations.
        pat = re.compile( r'^(?!_)(\w+)\s*=\s*' )

        for line in lines:
            comment_start = line.find('#')  # neglecting comments
            if comment_start != -1:
                line = line[body_indent:comment_start]
            else:
                line = line[body_indent:]

            # Neglect decorator-like affectations
            decorators = line.find('classmethod') + line.find('staticmethod')
            if decorators != -2:
                continue

            matchobj = pat.match( line )
            if matchobj:
                matches = matchobj.groups()
                if len(matches) == 1:
                    optname = matches[0]
                    if optname not in ordered:
                        # The option name is valid and is not an override
                        # of a previously defined option.
                        ordered.append( optname )
                else:
                    raise ValueError(line)
        return ordered
    option_names = classmethod( option_names )
    
    #
    #  PyPLearnObject's metaclass
    #
    _subclass_filter = classmethod( lambda cls, name: not name.startswith('_') )
    class __metaclass__( type ):
        """Implements some support mecanisms.

        This metaclass defaults right operators to their left counterpart
        if it exists. It also manages the _subclasses option if any;
        subclasses created are added (name -> class object) to the
        dictionnary if _subclass_filter returns True (default version
        neglect classes whose name starts with an underscore.
        """        
        _rop_names = [
        '__radd__', '__rsub__', '__rmul__', '__rdiv__',
        '__rtruediv__', '__rfloordiv__', '__rmod__',
        '__rdivmod__', '__rpow__', '__rlshift__',
        '__rrshift__', '__rand__', '__rxor__', '__ror__'
        ]
        
        def __init__(cls, name, bases, dict):
            super(type, cls).__init__(name, bases, dict) 
            for rop_name in cls._rop_names:
                lop_name = rop_name[0:2] + rop_name[3:]
                if ( not dict.has_key(rop_name)
                     and dict.has_key(lop_name) ):
                    setattr( cls, rop_name, dict[lop_name] )
                    
            if hasattr(cls, '_subclasses'):
                assert isinstance( cls._subclasses, type(dict) ), type( cls._subclasses )
                if cls._subclass_filter( name ):
                    cls._subclasses[name] = cls

    #
    #  PyPLearnObject constructor
    #
    def __init__( self, **overrides ):
        """PyPLearnObject constructor."""
        # Managing the option ordering protocol
        self._ordered_defaults = self.option_names( )

        # Setting appropriate values for all default and overriden options
        for option_name in self._ordered_defaults:            
            option_value = getattr( self.__class__, option_name )

            # The override dominates the default value
            if overrides.has_key( option_name ):
                option_value = overrides.pop( option_name  )
            else:
                option_value = copy.deepcopy( option_value )                            

            self._init_option_protocol( option_name, option_value )

        
        # Overrides may still contain pairs -- options with no default
        # values.
        if self.allow_unexpected_options():
            for option_name, option_value in overrides.iteritems():
                self._init_option_protocol( option_name, option_value )
                
        elif len(overrides):
            raise PLOptionError( "Trying to set undefined options %s on a %s"
                               % ( str(overrides), self.classname() )
                               )
                                  

        # Adding the current instance to the list of instances
        self.__class__.__instances.append( self )

    def _init_option_protocol( self, option_name, option_value ):
        """Used in __init__() to initialize options.
        
        The current protocol is to instanciate any classes provided as
        options. That is, if::

            class A:
                option = SomeClass

        Then the I{option} will be affected an instance of
        I{SomeClass}. The current version uses no arguments at
        instanciation::

            self.default_option = SomeClass() 
        """
        forbidden = False
        if option_name.startswith('__'):
            forbidden = 'private'
        elif option_name.startswith('_'):
            forbidden = 'protected'

        if forbidden:
            raise PLOptionError(
                'Attempt to set a %s member %s through the option mecanism.'
                % (forbidden, option_name)
                )
            
        if inspect.isroutine( option_value ):
            raise TypeError( "Routine types are not supported as PyPLearnObject option "
                             "values (In %s for %s, value is %s)."
                             % ( self.classname(), option_name, option_value )
                             )

        if inspect.isclass( option_value ):
            option_value = option_value( )

        self.__dict__[option_name] = option_value        

    def option_pairs( self, predicate = option_predicate ):
        """Returns the list of (name, value) pairs for all options respecting I{predicate}.

        Note that the list is sorted given the declaration order of default
        option values. PLOptions without default are at the end of the list,
        ordered in alphabetical order.
        """
        def option_sort( opt1, opt2 ):
            optname1 = opt1[0]
            optname2 = opt2[0]            
            
            if optname1 in self._ordered_defaults:
                # Both have defaults --- refer to indices
                if optname2 in self._ordered_defaults:
                    return cmp( self._ordered_defaults.index(optname1),
                                self._ordered_defaults.index(optname2) )
                # The one with a default has priority --- here opt1
                return -1

            # The one with a default has priority --- here opt2
            elif optname2 in self._ordered_defaults:
                return 1

            # Both have no default: alphabetical order
            return cmp( optname1, optname2 )

        #
        #  The option parsing
        #
        optpairs = [ (x,y)
                     for (x,y) in inspect.getmembers(self)
                     if predicate(x, y)
                     ]               
        optpairs.sort( option_sort )

        return optpairs

    def __str__( self ):
        """Calls plearn_repr global function over itself.""" 
        # It is most important no to call this instance's plearn_repr
        # method!!! Indeed, if we did so, we'd neglect to add the current
        # instance in the representations map...
        return plearn_repr( self, indent_level = 0 )

    def __repr__( self ):
        return self.plearn_repr( 0, lambda mname, member: True )

    def plearn_repr( self, indent_level = 0, predicate = option_predicate ):
        """PLearn representation of this python object.

        Are considered as options any 'public' instance attributes.
        """
        def elem_format( elem ):
            k, v = elem
            return '%s = %s' % ( k, plearn_repr(v, indent_level+1) )
        
        return "%s(%s)" % ( self.classname(),
                            format_list_elements( self.option_pairs(predicate),
                                                  elem_format, indent_level+1 )
                            )

class PyPLearnList( PyPLearnObject ):
    """Emulates a TVec of PyPLearnObject.
    
    This class is to be used whenever the attributes must be considered
    as the elements of a list. Hence, the plearn_repr() returns a list
    representation.

    NOTE THAT THIS CLASS COULD BE DEPRECATED SOON.
    """
    
    def __len__(self):
        return len( self._list_of_attributes )

    def __iter__( self ):
        return iter( self.to_list() )

    def _unreferenced( self ):
        return True
    
    def plearn_repr( self, indent_level = 0 ):
        """PLearn representation of this python object.

        Are considered as elements any non-None attributes.
        """
        elem_format = lambda elem: plearn_repr( elem, indent_level+1 )
        return '[%s]' % format_list_elements( self.to_list(), elem_format, indent_level+1 )

    def to_list(self):
        elem_predicate = lambda name, option: \
            option_predicate(name,option) and option is not None
        
        return [ value for name, value in self.option_pairs( elem_predicate ) ]

if __name__ == "__main__":
    class Test( PyPLearnObject ):
        option1   = 'option1'
        option2   = 'option2'
        #option2a = None
        option3   = 'option3'

        def pouet( self ):
            self.pouet_pouet = 'p'

    class SubTest( Test ):
        option1    = 'OPTION1'
        # option2  = ''
        inner_test = Test
        option4    = 4

        # Not options
        _protected = 'PROTECTED CLASS MEMBER'
        __private  = 'PRIVATE CLASS MEMBER'

        class innerClass:
            pass
        
        def p( cls ):
            pass
        p = classmethod(p)

    print Test( )

    try:
        SubTest( _protected = 'PROTECTED OPTION' )
    except PLOptionError, ae:
        print 'PLOptionError', str(ae)

    try:
        SubTest( __private = 'ATTEMPT TO SET PRIVATE MEMBER' )
    except PLOptionError, ae:
        print 'PLOptionError', str(ae)

    print SubTest( option2 = 2,
                   b       = '',
                   a       = '',
                   c       = '',
                   d       = '',
                   e       = '',
                   f       = '',                   
                   )

    class Foo( PyPLearnObject ): 
        foo = "the foo option"

    f = Foo( bar = "the bar option" )

    print f.foo         ## prints "the foo option"
    print f.bar         ## prints "the bar option"
    try:
        f.new_option = 'NEW'
    except PLOptionError, oe:
        print 'PLOptionError:',str(oe)

    class FooNoBar( PyPLearnObject ): 
        foo = "the foo option"
        def allow_unexpected_options( self ): return False 

    try:
        f = FooNoBar( bar = "the bar option" )
    except PLOptionError, oe:
        print 'PLOptionError:',str(oe)

    from plearn.utilities.ModeAndOptionParser import Mode
    print repr( Mode( None, None ) )
