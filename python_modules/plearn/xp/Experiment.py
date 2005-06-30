import copy, os, time
import plearn.utilities.toolkit as toolkit

from plearn.pyplearn.pyplearn       import generate_expdir
from plearn.pyplearn.plearn_repr    import plearn_repr
from plearn.pyplearn.PyPLearnObject import PyPLearnObject
from plearn.utilities.Bindings      import *

__all__ = [
    ## Helper functions
    "option_value_split",

    ## Main class
    "Experiment"
    ]

rhs_casts = [ int , float ]

def get_inexistence_predicate( expkey, cache_once=True ):
    """Returns a predicate checking that the experiment doesn't exist."""
    def inexistence_predicate( arguments ):
        if not (cache_once and Experiment._cached):
            Experiment.cache_experiments( expkey )
        matches = Experiment.match( arguments )
        return len( matches ) == 0

    return inexistence_predicate
    

def option_value_split( s, sep="=" ):
    """Returns a (lhs, rhs) pair given I{sep}."""
    lhs_len = s.find( sep )
    if lhs_len == -1:
        return (s, None)

    ## The left hand side 
    lhs     = s[:lhs_len].strip()

    ## Parsing the right hand side
    rhs = s[lhs_len+1:]    
    for cast in rhs_casts:
        try:
            rhs = cast(rhs)
            break
        except ValueError:
            pass

    ## Keep it as a string
    if isinstance( rhs, str ):
        rhs = rhs.strip()

    return (lhs, rhs)

class ExpKey( Bindings ):
    def __init__( self, keysrc ):
        if isinstance( keysrc, list ):
            expkey = [ option_value_split( key ) for key in keysrc ]
        elif isinstance( keysrc, str ):
            expkey = [ option_value_split( line ) for line in file(keysrc, "r") ]
        Bindings.__init__( self, expkey )

class Experiment(PyPLearnObject):
    ##
    # Options
    path   = None
    expkey  = None
            
    ##
    # Class variables
    _expdir_prefix    = 'expdir'
    _metainfos_fname  = 'metainfos.txt'
    _cached_exp_fname = 'Experiment.cache'  
    _lhs_length       = 35
    _cached           = None          # See cache_experiments

    ##
    # PyPLearnObject's classmethod
    _unreferenced = classmethod( lambda cls: True )

    def load( cls, path ):
        cached = os.path.join( path, cls._cached_exp_fname )
        if os.path.exists( cached ):
            return eval( open( cached, 'r' ).read() )

        # Load from scratch and cache
        exp       = cls( path = path )
        cachefile = open( cached, 'w' )
        print >>cachefile, str(exp)
        cachefile.close()
        
        return exp        
    load = classmethod( load )

    def cache_experiments( cls, dirlist=None ):
        if dirlist is None:
            dirlist = os.listdir( os.getcwd() )            

        cls._cached = []
        for fname in dirlist:
            if fname.startswith( cls._expdir_prefix ):                
                x = cls.load( fname )
                cls._cached.append( x )            
        cls._cached.sort()
    cache_experiments = classmethod( cache_experiments )

    def match( cls, expkey=[] ):
        if cls._cached is None:
            cls.cache_experiments()                        
        return [ exp for exp in cls._cached if exp.is_matched( expkey ) ]
    match = classmethod( match )

    def __init__( self, **overrides ):
        PyPLearnObject.__init__( self, **overrides )
        if self.expkey is None:
            expkey = ExpKey( os.path.join( self.path, self._metainfos_fname ) )
        
    def is_matched( self, expkey ):
        # Always matching empty expkey
        if not expkey:
            return True 

        # For efficiency
        if len(expkey) > len(self.expkey):
            return False

        # A key element from the expkey matches this experiement if
        #
        # 1) the key exists within this experiment ExpKey
        #
        # 2) the value is not restricted (None) or is restricted the same
        #    value than the one in this experiment
        match_predicate = lambda lhs,rhs: \
            lhs in self.expkey and \
            ( rhs is None or self.expkey[lhs]==rhs )                       

        # User should probably become aware of the concept of ExpKey... For now:
        expkey = ExpKey( expkey )

        # All key element must match (match_predicate)
        for lhs, rhs in expkey.iteritems():
            if not match_predicate(lhs,rhs):
                return False
        
        # All key element matched
        return True

    def __cmp__( self, other ):
        if self.path == other.path:
            return 0

        other_it = other.expkey.iteritems()
        for item in self.expkey.iteritems():
            try:
                compare = cmp( item, other_it.next() )                
                if compare != 0:
                    return compare
            except StopIteration:
                return 1 ## >
                
        ## Non-Positive ( < or == )
        return len(self.expkey) - len(other.expkey)
    
