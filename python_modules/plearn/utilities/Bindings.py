__cvs_id__ = "$Id: Bindings.py,v 1.2 2004/12/21 15:31:50 dorionc Exp $"

from toolkit import quote_if, doc

class dictionary_emulator_metaclass( type ):
    """To emulate the Python dictionaries behavior.

    Any class having this class as metaclass must provide any of its
    instance with a member I{internal_dictionary}.
    """
    def __init__(cls, name, bases, dict):
        super(dictionary_emulator_metaclass, cls).__init__(name, bases, dict)
        
class Bindings( dict ):
    """Acts like a Python dictionary but keeps the addition order."""
    def __init__(self, list_of_pairs=[]):
        dict.__init__(self, list_of_pairs)

        self.ordered_keys = []        
        for pair in list_of_pairs:
            if len(pair) != 2:
                raise ValueError("The Bindings constructor should be provided a list of pairs.")
            self.__setitem__( *pair )

    def __setitem__(self, key, value):
        dict.__setitem__(self, key, value)
        if not key in self.ordered_keys:
            self.ordered_keys.append(key)

    def __delitem__(self, key):
        dict.__delitem__(self, key)
        self.ordered_keys.remove(key)

    def __iter__(self):
        return self.iterkeys()

    def iterkeys(self):
        return self.ordered_keys
            
    class iterator:
        def __init__(self, bindings, values_else_item):
            self.bindings = bindings
            self.values   = values_else_item

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

            if self.values:
                return curval
            else:
                return (curkey, curval)
        
    def itervalues(self):
        return Bindings.iterator( self, True )

    def iteritems(self):
        return Bindings.iterator( self, False )

    def __str__(self):
        if len(self) == 0:
            return "{}"
        s = None        
        for (key, val) in self.iteritems():
            if s is None:
                s = "{%s: %s"
            else:
                s += ", %s: %s"
            s = s % (quote_if(key), quote_if(val))
        s += "}"
        return s
        
        
if __name__ == "__main__":
    print "\nEmbedded test/tutorial for Bindings.py.\n"

    print "Bindings: "
    print doc(Bindings)
    print 
    
    def print_bindings(b):
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

    bind = Bindings( [(1, "a"), (2, "b"), (3, "c")] )
    print 'After\n    bind = Bindings( [(1, "a"), (2, "b"), (3, "c")] )'
    print_bindings(bind)
    
    bind[4] = "d"
    print 'Then doing\n   bind[4] = "d"' 
    print_bindings(bind)

    bind[3] = "*C*"
    print 'Note that binding an existing argument keeps the original order of keys, i.e. after'
    print 'bind[3] = "*C*"'    
    print_bindings(bind)

    del bind[3]
    bind[3] = "newC"
    print "Therefore, to change the order, one should do"
    print '    del bind[3]'
    print '    bind[3] = "newC"'
    print_bindings(bind)

    
