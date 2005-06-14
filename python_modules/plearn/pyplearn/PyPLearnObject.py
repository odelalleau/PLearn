"""Class Emulating PLearn Objects in Python

The class contained in this module is to be used as super class for almost
any Python class emulating a PLearn cousin class.
"""
__cvs_id__ = "$Id: PyPLearnObject.py,v 1.20 2005/06/14 18:08:23 dorionc Exp $"

from   AttributeManager            import AttributeManager
from   plearn.utilities.toolkit    import no_none
from   plearn.pyplearn.plearn_repr import *
import plearn.utilities.metaprog   as     metaprog

__all__ = [ "PyPLearnObject", "PyPLearnList" ]

class PyPLearnObject( AttributeManager ):
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
    subclass. The L{plearn_options()} are considered to be all attributes
    whose names do not start with an underscore. Those are said public,
    while any attribute starting with at least one underscore is considered
    internal (protected '_' or private '__').

    The learn more about the way this class manages attribute, see the
    L{AttributeManager} class defined above.
    """
    __instances = []
    
    #
    #  Classmethods:
    #   See
    #    U{http://www.python.org/doc/2.3.4/lib/built-in-funcs.html}
    #    for an introduction to the classmethod concept.
    #
    def classname( cls ):
        """Classmethod to access the class name. 

        @returns: The (instance) class' name as a string.
        """
        return cls.__name__
    classname = classmethod(classname)

    def instances( cls ):
        ilist = []
        for instance in cls.__instances:
            if isinstance( instance, cls ):
                ilist.append(instance)
        return ilist
    instances = classmethod(instances)

    #
    # PyPLearnObject constructor
    #
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
        automatically managed by the AttributeManager super class.
        """
        AttributeManager.__init__( self, **overrides )
        self.__class__.__instances.append( self )

    def __str__( self ):
        """Calls plearn_repr global function over itself.""" 
        # It is most important no to call this instance's plearn_repr
        # method!!! Indeed, if we did so, we'd neglect to add the current
        # instance in the representations map...
        return plearn_repr( self, indent_level = 0 )

    def __repr__( self ):
        return self.plearn_repr( indent_level = 0,
                                 restricted_to_ploptions = False )

    def plearn_repr( self, indent_level = 0, restricted_to_ploptions = True ):
        """PLearn representation of this python object.

        Are considered as options any 'public' instance attributes.
        """
        if restricted_to_ploptions:
            # PLearn options
            attributes = self.public_attribute_pairs()
        else:
            attributes = self.attribute_pairs()
                        
        def elem_format( elem ):
            k, v = elem
            return '%s = %s' % ( k, plearn_repr(v, indent_level+1) )
        
        return "%s(%s)" % ( self.classname(),
                            format_list_elements( attributes, elem_format, indent_level+1 )
                            )
    


class PyPLearnList( PyPLearnObject ):
    """Emulates a TVec of PyPLearnObject.
    
    This class is to be used whenever the attributes must be considered
    as the elements of a list. Hence, the plearn_repr() returns a list
    representation.

    If the order of the attributes in the plearn_repr is considered
    important, it must be imposed by the order of the member names
    appearance in the __ordered_attr__ list. See the L{AttributeManager}
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

    def __iter__( self ):
        return iter( self.to_list() )

    def _unreferenced( self ):
        return True
    
    def plearn_repr( self, indent_level ):
        """PLearn representation of this python object.

        Are considered as elements any non-None attributes.
        """
        elem_format = lambda elem: plearn_repr( elem, indent_level+1 )
        return '[%s]' % format_list_elements( self.to_list(), elem_format, indent_level+1 )

    def to_list(self):
        return no_none([ elem for (name,elem) in self.iteritems() ])


class PythonObject( PyPLearnObject ):
    """Subclass of PyPLearnObject behaving like Python ones.

    Entrusted with the features of PyPLearnObject, PythonObject instances
    are configured to allow the same flexibility in regards of attributes
    (since instances are not frozen) and are printed with a syntax closer
    to raw Python.
    """
    _frozen = False
    
    def _unreferenced( self ):
        return True

##########################################
### Embedded test/tutorial
##########################################
if __name__ == "__main__":
    from plearn.utilities.toolkit   import boxed_lines
    from plearn.utilities.verbosity import *

    set_verbosity( 1 )
    vprint.highlight( ["PyPLearnObject.py builtin test/tutorial."] )


    def example( s ):
        obj = eval(s)
        return """
    obj = %s
    print obj

    ### Will print
    %s
    """ % ( s, str(obj).replace('\n', (' '*4)+'\n') )

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


