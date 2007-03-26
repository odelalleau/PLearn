# plearn_repr.py
# Copyright (C) 2005, 2006 Christian Dorion
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#
#   1. Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
#
#   2. Redistributions in binary form must reproduce the above copyright
#      notice, this list of conditions and the following disclaimer in the
#      documentation and/or other materials provided with the distribution.
#
#   3. The name of the authors may not be used to endorse or promote
#      products derived from this software without specific prior written
#      permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
#  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
#  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
#  NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
#  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#  This file is part of the PLearn library. For more information on the PLearn
#  library, go to the PLearn Web site at www.plearn.org

# Author: Christian Dorion
import warnings
from plearn.pyplearn.context   import actualContext
from plearn.utilities          import toolkit
from plearn.utilities.pobject  import PObject
#from plearn.utilities.metaprog import public_attributes
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

# Class for explicitly giving a PLearn representation as a string in 
# PLearn serialization format. 
class PLearnRepr:
    def __init__( self, reprstring):
        self.reprstring = reprstring

    def plearn_repr(self, indent_level, inner_repr):
        return self.reprstring

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
    def buildClassContext(context):
        assert not hasattr(context, 'pref_map')
        context.pref_map = PRefMap()
    buildClassContext = staticmethod(buildClassContext)    

    def getCurrentPRefMap():
        context = actualContext(PRefMap)
        assert hasattr(context, 'pref_map')
        return context.pref_map
    getCurrentPRefMap = staticmethod(getCurrentPRefMap)    
    
    def __init__( self ):
        Bindings.__init__( self )
        self.references = {}
        
    def __setitem__( self, key, value ):
        assert key not in self.ordered_keys, "Representations can not be updated."
        Bindings.__setitem__( self, key, value )

        #raw_input((len(self.ordered_keys), key, value ))
        self.references[ key ] = PRef( len(self.ordered_keys) )

    def registerPLearnRef(self, plearnref):
        ref = PRef( plearnref )
        ref.expanded = True
        self.references[ plearnref ] = ref

    def __getitem__( self, key ):
        ref = self.references[ key ]
        if ref.expanded:
            return ref.plrepr 
        return ref.expand( Bindings.__getitem__( self, key ) )

def clear_ref_map():
    PRefMap.getCurrentPRefMap().clear()

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
    noref = False
    try:        
        noref = obj._by_value
        if callable(noref):
            noref = noref()
    except AttributeError:
        pass
        
    # To be referenced, object must provide the serial number interface
    if not noref and hasattr(obj, '_serial_number') and callable(obj._serial_number):        
        # The current object can be referenced
        pref_map = PRefMap.getCurrentPRefMap()
        object_id = obj._serial_number()

        # Create a first representation of the object.
        if object_id not in pref_map:
            pref_map[ object_id ] = __plearn_repr( obj, indent_level )
        return pref_map[ object_id ]

    else:
        return __plearn_repr( obj, indent_level )
    
class _PyReprMap(dict):
    def buildClassContext(context):
        assert not hasattr(context, 'pyrepr_map')
        context.pyrepr_map = _PyReprMap()
    buildClassContext = staticmethod(buildClassContext)    

    def getCurrentPyReprMap():
        context = actualContext(_PyReprMap)
        assert hasattr(context, 'pyrepr_map')
        return context.pyrepr_map
    getCurrentPyReprMap = staticmethod(getCurrentPyReprMap)    
    
def python_repr( obj, indent_level = 0 ):
    """Benefits from the plearn_repr() mechanisms but is strict Python.

    Can be used in objects' __repr__ overloads so that::

        eval( repr(obj) ) == obj
    """
    # To be referenced, object must provide the serial number interface
    if hasattr(obj, '_serial_number') and callable(obj._serial_number):        
        # The current object can be referenced
        pyrepr_map = _PyReprMap.getCurrentPyReprMap()
        object_id = obj._serial_number()

        # Create a first representation of the object.
        if object_id not in pyrepr_map:
            pyrepr_map[ object_id ] = __plearn_repr( obj, indent_level, python_repr )

        return pyrepr_map[ object_id ]

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
        ## Indent by 4 spaces for each indent_level
        token  = ",\n%s" % (" " * 4*indent_level)
        elems_as_str  = token[1:]
        elems_as_str += token.join(formatted_list)
        elems_as_str += token[1:]

    return elems_as_str

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

    elif isinstance( obj, tuple ):
        return '(' + ', '.join( [ inner_repr(elem, indent_level+1) for elem in obj] ) + ')'
##         if len(obj) == 2:
##             return inner_repr( obj[0], indent_level+1 ) + ':' + inner_repr( obj[1], indent_level+1 )
##         else:
##             return '(' + ', '.join( [ inner_repr(elem, indent_level+1) for elem in obj] ) + ') '

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


#######  Builtin Tests  #######################################################

def test_pyplearn_object_lists():
    from plearn.pyplearn.PyPLearnObject import PyPLearnObject, PLOption

    def pretty(header, s):
        print header
        print "-"*len(header)
        print
        print s
        print         

    class Test(PyPLearnObject):
        opt1 = PLOption("option1")
        opt2 = PLOption(2)
        opt3 = PLOption({"3":3})

    test_list = [ Test(), Test(opt2 = 2e06), Test(opt3 = 3) ]
    pretty("Direct print:", test_list)
    pretty("Through plearn_repr:", plearn_repr(test_list))
    pretty("Through repr:", repr(test_list))

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
    

    print "#####  Testing deepcopy vs by_value   ########################################"
    def id_print(obj):
        # print "Python Id:", id(obj)
        print "PyPLObj Id:", obj._serial_number()
        print "Object:", obj
        print 
        
    print
    print "***** Deep copy"

    referenced = pl.Referenced(inner_obj = pl.InnerObject(id = 1))
    id_print(referenced)

    print "Printing the same Python object:"
    id_print(referenced)

    print "Printing a deep copy:"
    import copy
    id_print(copy.deepcopy(referenced))

    print
    print "***** By value"
    
    unreferenced = pl.UnReferenced(inner_obj = pl.InnerObject(id = 1), _by_value=True)
    id_print(unreferenced)
    
    print "Printing the same Python object:"
    id_print(unreferenced)

    print "Printing a deep copy:"
    import copy
    id_print(copy.deepcopy(unreferenced))
