"""Cluster task arguments iterator.

See http://www.python.org/doc/2.3.4/lib/typeiter.html for an introduction to Python's iterator protocol.
"""
import os, re

class ArgumentsOracle:
    """A base class for task arguments iterators.

    The ArgumentsOracle is iterator whose next() method returns a
    formatted string representing the task arguments.
    """
    name_value_joining_token = '='
    arguments_joining_token  = " "
    
    def __iter__( self ):
        return self

    def next( self ):
        """Returns a formatted string representing the task arguments."""
        raise NotImplementedError

    def reset( self ):
        raise NotImplementedError
        
class Cursor:
    def __init__( self, start = 0, restart = None ):
        self._value   = start
        self.restart  = restart

    def inc( self ):
        """Returns True if the cursor was reset to 0."""
        self._value  += 1

        if ( self.restart is not None and
             self.restart == self._value ):
            self._value = 0
            return True

        ## Return False otherwise
        return False        

    def set_value( self, val ):
        self._value = val
        
    def value( self ):
        return self._value
    
class CrossProductOracle( ArgumentsOracle ):
    def __init__( self, name_values_pairs ):
        """Sets the internal state of the iterator.

        @param name_values_pairs: A list of pairs having (I{name},
        I{values}) form where I{name} must be a string. The I{values} entry
        can be a single value or a list of values.
        @type  name_values_pairs: List of pairs.
        """
        self.nargs             = len(name_values_pairs)
        self.cursors           = []
        self.name_values_pairs = []
        
        for (name, values) in name_values_pairs:
            ## Support for non-list values
            if not isinstance(values, list):
                values = [values]

            ## Initialize cursors
            self.cursors.append( Cursor(0, len(values)) )
            self.name_values_pairs.append((name,values))

        ## Starting before the first sequence
        self.cursors[-1].set_value(-1)
        
    def next( self ):
        argc               = self.nargs        
        reversed_arguments = []
        do_inc             = True  ## Always increment the last cursor
        while argc > 0:
            argc -= 1
            (name, values) = self.name_values_pairs[argc]
            cursor         = self.cursors[argc]
            
            if do_inc:
                do_inc = cursor.inc()

            reversed_arguments.append( "%s%s%s"
                                       % ( name, self.name_value_joining_token,
                                           str(values[cursor.value()])
                                           ))

        ## If the first list of values has expired, the old set of
        ## arguments was processed
        if do_inc:
            self.cursors[-1].set_value( None )
            raise StopIteration

        reversed_arguments.reverse()
        return self.arguments_joining_token.join( reversed_arguments )

    def reset( self ):
        for i, pair in enumerate(self.name_values_pairs):
            self.cursors[i].set_value(0)
        ## Starting before the first sequence
        self.cursors[-1].set_value(-1)

class DirectorySplittedOracle( ArgumentsOracle ):
    def __init__( self, arguments_oracle, *build_dirname_from ):                
        self.underlying_oracle = arguments_oracle

        nvjoin      = self.underlying_oracle.name_value_joining_token
        self.ajoin  = self.underlying_oracle.arguments_joining_token

        regexp = "" 
        for param in build_dirname_from:
            regexp += "%s%s\S%s" % (param, nvjoin, self.ajoin)
        self.regexp = "(%s)"%regexp

        self.creation_hook         = lambda : None
        self.created_directories   = []
        self.directory_when_called = os.getcwd()

    def set_creation_hook( self, hook ):
        self.creation_hook = hook

    def next( self ):
        _next   = self.underlying_oracle.next()
        match   = re.match( self.regexp, _next ).groups()[0]
        dirname = match.rstrip(self.ajoin)

        if ( len(self.created_directories) == 0 or
             self.created_directories[-1]  != dirname ):
            os.chdir( self.directory_when_called )
            os.mkdir( dirname )
            os.chdir( dirname )
            self.creation_hook( )
            self.created_directories.append( dirname )

        return _next
        
    def reset( self ):
        self.underlying_oracle.reset()

class PredicateOracle( ArgumentsOracle ):
    def __init__( self, arguments_oracle, predicate ):                
        self.underlying_oracle = arguments_oracle
        self.predicate         = predicate

    def next( self ):
        _next = self.underlying_oracle.next()
        while not self.predicate( _next ):
            _next = self.underlying_oracle.next()
        
        return _next

    def reset( self ):
        self.underlying_oracle.reset()

if __name__ == '__main__':
    pairs = []
    pairs.append(( "first",  [1,2,3] ))
    pairs.append(( "second", 10 ))
    pairs.append(( "third",  [100, 200] ))

    cporacle = CrossProductOracle( pairs )
    print "CrossProductOracle( %s )" % pairs
    
    for arguments in cporacle:
        print arguments
    print "## End of cporacle arguments"


    print
    print "DirectorySplittedOracle"
    cporacle.reset()
    dsoracle = DirectorySplittedOracle( cporacle, "first" )
    
    for arguments in dsoracle:
        print arguments
    print "## End of dsoracle arguments"
