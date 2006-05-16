import warnings
from plearn.utilities.pobject  import PObject
from plearn.utilities          import toolkit
from plearn.utilities.metaprog import public_attributes
from plearn.utilities.Bindings import Bindings

try:
    import numarray
except ImportError:
    numarray = None

#
#  Deprecated functions
#
def __deprecated( msg ):
    msg += '\n*** Use warnings.filterwarnings( action = "ignore" ) to get rid of warnings. ***'
    warnings.warn( msg, DeprecationWarning, stacklevel = 3 )
    
def bind( refname, obj ):
    __deprecated("bind(refname, obj): assign a python object instead.")
    globals()[refname] = obj
    
def ref( refname ):
    __deprecated("ref(refname): refer to a python object instead.")
    return globals()[refname]

def bindref( refname, obj ):
    __deprecated("bindref(refname, obj): assign and refer to a python object instead.")
    globals()[refname] = obj
    return obj

#
#  Helper classes
#
class PRef:
    def __init__( self, refno ):
        self.plrepr   = "*%d" % refno
        self.expanded = False

    def expand( self, value ):
        expansion = self.plrepr + ' -> ' + value
        self.plrepr += ';'
        self.expanded = True

        return expansion

class PRefMap( Bindings ):
    def __init__( self ):
        Bindings.__init__( self )
        self.references = {}
        
    def __setitem__( self, key, value ):
        assert key not in self.ordered_keys, "Representations can not be updated."
        Bindings.__setitem__( self, key, value )

        #raw_input((len(self.ordered_keys), key, value ))
        self.references[ key ] = PRef( len(self.ordered_keys) )

    def __getitem__( self, key ):
        ref = self.references[ key ]
        if ref.expanded:
            return ref.plrepr 
        return ref.expand( Bindings.__getitem__( self, key ) )        
__pref_map = PRefMap()

def clear_ref_map():
    __pref_map.clear()

#
#  Main function
#
def plearn_repr( obj, indent_level = 0 ):    
    """Returns a string that is a valid PLearn representation of I{obj}.

    This function is somehow the core of the whole PyPLearn mechanism. It
    maps most Python objects to a representation understood by the PLearn
    serialization mechanism.
    """ 
    # Classes may specify themselves as unreferenced.
    noref = hasattr( obj, '_unreferenced' ) and obj._unreferenced()
    
    # To be referenced, object must provide the serial number interface
    if not noref and hasattr(obj, '_serial_number') and callable(obj._serial_number):        
        # The current object can be referenced
        object_id = obj._serial_number()

        # Create a first representation of the object.
        if object_id not in __pref_map:
            __pref_map[ object_id ] = __plearn_repr( obj, indent_level )

        return __pref_map[ object_id ]

    else:
        return __plearn_repr( obj, indent_level )
    

__pyrepr_map = {}
def python_repr( obj, indent_level = 0 ):
    """Benefits from the plearn_repr() mechanisms but is strict Python.

    Can be used in objects' __repr__ overloads so that::

        eval( repr(obj) ) == obj
    """
    # To be referenced, object must provide the serial number interface
    if hasattr(obj, '_serial_number') and callable(obj._serial_number):        
        # The current object can be referenced
        object_id = obj._serial_number()

        # Create a first representation of the object.
        if object_id not in __pref_map:
            __pyrepr_map[ object_id ] = __plearn_repr( obj, indent_level, python_repr )

        return __pyrepr_map[ object_id ]

    else:
        return repr(obj)    
    
#
#  Helper functions
#
def format_list_elements( the_list, element_format, indent_level ):
    formatted_list = []
    for elem in the_list:        
        formatted = element_format(elem)
        if len(formatted):
            formatted_list.append(formatted)
    
    n_elems = len(formatted_list)
    if n_elems == 0:
        elems_as_str = " "

    elif n_elems == 1 and formatted_list[0].find("\n") == -1:
        elems_as_str = " %s " % formatted_list[0]
        
    else:
        token  = ",\n%s" % (" " * 4*indent_level)
        elems_as_str  = token[1:]
        elems_as_str += token.join(formatted_list)
        elems_as_str += token[1:]

    return elems_as_str

# def format_list_elements( the_list, element_format, indent_level ):
#     n_elems = len(the_list)       
#     if n_elems == 0:
#         elems_as_str = " "    
# 
#     elif n_elems == 1:
#         elemstr = element_format( the_list[0] )
#         if elemstr.find('\n') == -1:
#             token = ' '
#         else:
#             token  = "\n%s" % (" " * 4*indent_level )
# 
#         elems_as_str = "%s%s%s" % (token, elemstr, token)
#         
#     else:
#         token  = ",\n%s" % (" " * 4*indent_level )
#         elems_as_str  = token[1:]
#         elems_as_str += token.join([ element_format(elem) for elem in the_list ])
#         elems_as_str += token[1:]
# 
#     return elems_as_str
# 

def __plearn_repr( obj, indent_level, inner_repr = plearn_repr ):
    """Returns a string that is the PLearn representation of obj."""

    if hasattr( obj, 'plearn_repr' ) and callable( obj.plearn_repr ):
        try:            
            return obj.plearn_repr(indent_level, inner_repr)
        except TypeError:
            raise TypeError('%s.plearn_repr signature should be:'
                            'plearn_repr(self, indent_level=0, inner_repr=plearn_repr)'
                            % obj.__class__.__name__)

    # Don't use repr for numeric type, so we don't get 0.20000000000000001
    # for 0.2
    elif ( isinstance( obj, int )
           or isinstance( obj, float ) ):
        return str(obj)

    elif isinstance(obj, str):
        return '"%s"' % obj.replace('"', r'\"') # toolkit.quote( obj )

    elif isinstance(obj, list):
        elem_format = lambda elem: inner_repr( elem, indent_level+1 )
        return '[%s]' % format_list_elements( obj, elem_format, indent_level+1 )

    elif isinstance( obj, dict ):
        def elem_format( elem ):
            k, v = elem
            return '%s : %s' % ( inner_repr(k, indent_level+1),
                                 inner_repr(v, indent_level+1) )        
        return '{%s}' % format_list_elements( obj.items(), elem_format, indent_level+1 )

    elif isinstance( obj, tuple ) and len(obj) == 2:
        return inner_repr( obj[0], indent_level+1 ) + ':' + inner_repr( obj[1], indent_level+1 )

    # Stands for TMat<real>
    elif numarray is not None and isinstance( obj, numarray.numarraycore.NumArray ):
        shape = obj.getshape()
        if len(shape) == 1:
            listrepr = [ elem for elem in obj ]
            return "%d %s" % ( shape[0], inner_repr(listrepr, indent_level+1) )

        elif len(shape) == 2:
            l,w = shape
            listrepr = [ f for row in obj for f in row ]
            return "%d %d %s" % ( l, w, inner_repr(listrepr, indent_level+1) )

        raise ValueError( "Only numarrays of dimension 1 and 2 are understood by plearn_repr." )
            
    
    elif obj is None:
        return "*0;"

    elif isinstance( obj, PObject ):
        # A PObject is a lightweight PLearn object received from the PLearn
        # server mechanism (plservice/plio)
        return repr(obj)

    # No specific mechanism known for that type
    raise TypeError( 'Does not know how to handle type %s (obj = %s)'
                     % ( type(obj), str(obj) )
                     )    
    

if __name__ == "__main__":
    from plearn.pyplearn import pl

    object1  = pl.SomeClass( name = "Object1" )    
    object2  = pl.SomeOtherClass( first_option = "first_option", internal = object1 )
    toplevel = pl.TopLevel( list_option = [object1, object2] )
    print toplevel

    toplevel2 = pl.TopLevel( name = "Embedded", embedded = toplevel,
                             some_dict = { "a" : 1, "b" : 2 },
                             some_mat  = numarray.array([[1,2], [3, 4]])
                             )
    print toplevel2 

    print
    print "toplevel3"
    toplevel3 = pl.TopLevel( some_dict = { "a" : 1, "b" : 2 }, # should be another dict object
                             str_list  = [ "str1", "str2" ],
                             )
    print toplevel3

    #
    #  Test deprecated
    #
    print
    print "+++ Deprecated functions (deprecation warnings disabled):"
    warnings.filterwarnings( action = "ignore" )
    
    bind( 'allo-toi', pl.AlloToi( p = "p", a = 1 ) )
    refvalue = ref( 'allo-toi' )
    print refvalue

    br = bindref( 'peanut', pl.Peanut( inside = refvalue ) )
    print br
    
