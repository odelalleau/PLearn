import copy, os, time
import plearn.utilities.toolkit as toolkit

from plearn                         import pyplearn
from plearn.pyplearn.pyplearn       import generate_expdir
from plearn.pyplearn.plearn_repr    import plearn_repr, python_repr
from plearn.pyplearn.PyPLearnObject import PyPLearnObject
from plearn.utilities.moresh        import *
from plearn.utilities.Bindings      import *

rhs_casts = [ int , float ]

def get_inexistence_predicate( expkey, cache_once=True ):
    """Returns a predicate checking that the experiment doesn't exist."""
    def inexistence_predicate( arguments ):
        if not (cache_once and Experiment._cached):
            Experiment.cache_experiments( )
        matches = Experiment.match( arguments )
        return len( matches ) == 0

    return inexistence_predicate
    
def keycmp( exp, other, expkey ):
    if exp.path == other.path:
        return 0

    exp_subkey   = exp.getKey( expkey )
    exp_it       = exp_subkey.iteritems()

    other_subkey = other.getKey( expkey )
    other_it     = other_subkey.iteritems()
    
    for item in exp_it:
        try:
            compare = cmp( item, other_it.next() )                
            if compare != 0:
                return compare
        except StopIteration:
            return 1 ## >

    ## Non-Positive ( < or == )
    return len(exp_subkey) - len(other_subkey)

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
    def __init__( self, keysrc=[] ):
        expkey = []
        if isinstance( keysrc, list ):
            if keysrc and isinstance( keysrc[0], tuple ):
                expkey = keysrc
            else:
                expkey = [ option_value_split( key ) for key in keysrc ]
        elif isinstance( keysrc, str ) and os.path.exists( keysrc ):
            expkey = [ option_value_split( line ) for line in file(keysrc, "r") ]        
        Bindings.__init__( self, expkey )

    def strkey( self ):
        return " ".join([ '='.join([str(key),str(value)]) for key,value in self.iteritems() ])

class Experiment(PyPLearnObject):
    ##
    # Options
    path    = None
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

        # CACHING DISABLED!!!
        # if not exp.running():
        #     cachefile = open( cached, 'w' )
        #     print >>cachefile, repr(exp)
        #     cachefile.close()
        
        return exp        
    load = classmethod( load )

    def cache_experiments( cls, exproot=None, forget=True ):
        if exproot is None:
            roots = pyplearn.config.get_option( 'EXPERIMENTS', 'expdir_root' ).split(',')            
            for exproot in roots:
                cls.cache_experiments( exproot=exproot, forget=False )
            return

        if cls._cached is None or forget:
            cls._cached = []

        if exproot:
            if not os.path.exists( exproot ):
                return
            dirlist = os.listdir( exproot )
        else:
            dirlist = os.listdir( os.getcwd() )            

        for fname in dirlist:
            if fname.startswith( cls._expdir_prefix ):                
                x = cls.load( os.path.join(exproot, fname) )
                cls._cached.append( x )            
    cache_experiments = classmethod( cache_experiments )

    def match( cls, expkey=[] ):
        if cls._cached is None:
            cls.cache_experiments()                        
        return [ exp for exp in cls._cached if exp.is_matched( expkey ) ]
    match = classmethod( match )

    def __init__( self, **overrides ):
        PyPLearnObject.__init__( self, **overrides )
        if self.expkey is None:
            self.expkey = ExpKey( os.path.join( self.path, self._metainfos_fname ) )
        
    def __cmp__( self, other ):
        raise NotImplementedError( 'Use keycmp( x1, x2, expkey )' )
    
    def __str__( self ):
        return self.toString()

    def getKey( self, expkey = None ):
        if expkey is None:
            return self.expkey

        subset = ExpKey()
        for key in expkey:
            if key in self.expkey:
                subset[key] = self.expkey[key]
            else:
                subset[key] = None
        return subset
        
    def toString( self, expkey=None ):
        s = '%s\n' % self.path
        for key, value in self.getKey(expkey).iteritems():
            s += '    %s= %s\n' % (key.ljust(30), value)
        return s

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

        # User should probably become aware of the concept of ExpKey.
        if not isinstance( expkey, ExpKey ): 
            expkey = ExpKey( expkey )

        # All key element must match (match_predicate)
        for lhs, rhs in expkey.iteritems():
            if not match_predicate(lhs,rhs):
                return False
        
        # All key element matched
        return True

    def running( self ):
        return len(self.expkey) == 0
