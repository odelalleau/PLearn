"""Bindings: Acts like a Python dictionary but keeps the addition order."""

from plearn.utilities.toolkit import quote_if, doc

def c_iterator( container, itype="iteritems" ):
    if hasattr(container, itype ):
        return container.iteritems()
    return iter(container)

# Should be CAREFULLY moved to toolkit..        
def cross_product( *sets ):
    return [ item for item in iter_cross_product( *sets ) ]

def iter_cross_product( *sets ):
    cp_iter = lambda struct: \
        ( isinstance( struct, list ) and iter(struct) ) \
        or iter([struct])
    
    wheels = map( cp_iter, sets ) # wheels like in an odometer
    digits = [it.next() for it in wheels]
    while True:
        yield digits[:]
        for i in range(len(digits)-1, -1, -1):
            try:
                digits[i] = wheels[i].next()
                break
            except StopIteration:
                wheels[i] = cp_iter(sets[i])
                digits[i] = wheels[i].next()
        else:
            break        


class Bindings( object ):
    """Acts like a Python dictionary but keeps the addition order."""

    class __no_default: pass
    
    def __init__(self, container=[], value=None):
        self.ordered_keys  = []
        self.internal_dict = {}

        iterator = c_iterator( container )                    
        for item in iterator:
            pair = None

            try:
                ilen = len(item)
                if ilen != 2:
                    raise ValueError(
                        "The Bindings constructor should be provided a list of keys "
                        "or pairs or a mapping object supporting iteritems." )
                pair = item
            except TypeError, err: # len() of unsized object
                pair = (item, value)

            self.__setitem__( *pair )

    ## Emulating containers ##########################################    
    def __len__(self):
        return len(self.ordered_keys)

    def __contains__(self, key):
        return key in self.ordered_keys
    
    def __setitem__(self, key, value):
        self.internal_dict[key] = value
        if not key in self.ordered_keys:
            self.ordered_keys.append(key)

    def __getitem__( self, key ):
        return self.internal_dict[key]

    def __delitem__(self, key):
        self.pop(key)

    def __str__(self):
        if len(self) == 0:
            return "{}"

        s = None        
        for (key, val) in self.iteritems( ):
            if s is None:
                s = "{%s: %s"
            else:
                s += ", %s: %s"
            s = s % (quote_if(key), quote_if(val))
        s += "}"
        return s        

    def __repr__( self ):
        return "%s([ %s ])" % ( self.__class__.__name__,
                                ', '.join([ str(item) for item in self.items() ])
                                )

    ## Iterators  ####################################################
    class item_iterator:
        def __init__( self, bindings, return_pairs=True ):
            self.bindings     = bindings
            self.return_pairs = return_pairs

            self.cur      = -1
            self.keys     = bindings.ordered_keys

        def __iter__(self):
            return self

        def next(self):
            self.cur += 1
            if len(self.bindings) <= self.cur:
                raise StopIteration

            curkey = self.keys[self.cur]
            curval = self.bindings[curkey] 

            if self.return_pairs:
                return (curkey, curval)
            else:
                return curval
        
    def __iter__(self):
        return self.iterkeys()

    def iteritems(self):
        return Bindings.item_iterator( self )

    def iterkeys(self):
        return iter(self.ordered_keys)
            
    def itervalues(self):
        return Bindings.item_iterator( self, False )

    ## Iterator related methods ######################################
    def items( self ):
        return [ item for item in self.iteritems() ]

    def keys( self ):
        return [ k for k in self.iterkeys( ) ]

    def values( self ):
        return [ value for value in self.itervalues() ]

    ## Other dictionnary methods #####################################
    def clear( self ):
        del self.ordered_keys[:]
        self.internal_dict.clear()

    def copy( self ):
        return Bindings( self )
        
    def fromkeys( cls, container, value=None ):               
        return cls( c_iterator( container, "iterkeys" ), value )               
    fromkeys = classmethod( fromkeys )

    def get( self, key, x=None ):
        if key in self:
            return self.__getitem__( key )
        return x

    def has_key( self, key ):
        return key in self.ordered_keys

    def pop( self, key, x=__no_default() ):
        if key in self:
            self.ordered_keys.remove(key)
            return self.internal_dict.pop(key)

        if isinstance( x, self.__no_default ):
            raise KeyError("Binding object does not contain any '%s' key." % key ) 
        return x
        
    def popitem( self ):
        (key, val) = self.internal_dict.popitem(key)
        self.ordered_keys.remove(key)
        return (key, val)

    def setdefault( self, k, x=None ):
        if key in self:
            return self.__getitem__( key )
        self.__setitem__( k, x )
        return x        
        
    def update( self, pairs ):
        try:
            # Dictionnary
            iterator = pairs.iteritems()
        except AttributeError:
            # List of pairs
            iterator = iter( pairs )
            
        for k, val in iterator:
            self.__setitem__( k, val )

    #
    #  Added instance method
    #
    def explode_values( self ):
        cls = self.__class__

        exploded = []
        for value_set in iter_cross_product( *self.values() ):
            exploded.append( cls( zip(self.ordered_keys, value_set) ) )
        return exploded

    def getKey(self, index):
        assert isinstance(index, int)
        return self.ordered_keys[index]

    def getValue(self, index):
        assert isinstance(index, int)
        key = self.ordered_keys[index]
        return self[key]
    
    def getItem(self, index):
        assert isinstance(index, int)
        key = self.ordered_keys[index]
        return key, self[key]
        

#######  Embedded Tests  ######################################################

def _print_bindings(b):
    print "\nBindings: %s" % b

    print "Keys: ",
    for bkey in b.iterkeys():
        print bkey,
    print

    print "Values: ",
    for bval in b.itervalues():
        print quote_if(bval),
    print

    print "Items: ",
    for bitem in b.iteritems():
        print bitem,
    print

    print "\n"


def _test_ctors():
    print "#####  Testing __init__ behaviours"    

    bind = Bindings( [(1, "a"), (2, "b"), (3, "c")] )
    _print_bindings(bind)

    bind = Bindings( range(5) )
    _print_bindings(bind)

    bind = Bindings(range(5), "DefaultValue")
    _print_bindings(bind)

    bind = Bindings({"a":1, "b":2.5, "c":[]})
    _print_bindings(bind)
    
def _test_ordering():
    print "#####  Testing addition ordering"    

    bind = Bindings( [(1, "a"), (2, "b"), (3, "c")] )
    print 'After\n    bind = Bindings( [(1, "a"), (2, "b"), (3, "c")] )'
    _print_bindings(bind)
    
    bind[4] = "d"
    print 'Then doing\n   bind[4] = "d"' 
    _print_bindings(bind)

    bind[3] = "*C*"
    print 'Note that binding an existing argument keeps the original order of keys, i.e. after'
    print 'bind[3] = "*C*"'    
    _print_bindings(bind)

    del bind[3]
    bind[3] = "newC"
    print "Therefore, to change the order, one should do"
    print '    del bind[3]'
    print '    bind[3] = "newC"'
    _print_bindings(bind)    

def _test_iterators():
    print "#####  Testing iterators (hence ordering again...)"    

    list_of_pairs = [(1, "a"), (2, "b"), (3, "c"), (4, "d")]
    bindings = Bindings(list_of_pairs)

    iterkeys   = bindings.iterkeys()
    itervalues = bindings.itervalues()
    iteritems  = bindings.iteritems()
    for pair in list_of_pairs:
        assert pair[0] == iterkeys.next()
        assert pair[1] == itervalues.next()
        assert pair    == iteritems.next()
    print "Success!"
    print "\n"

if __name__ == "__main__":
    print "\nEmbedded test/tutorial for Bindings.py.\n"

    print "Bindings: "
    print doc(Bindings)
    print 
    
    _test_ctors()
    _test_ordering()
    _test_iterators()
