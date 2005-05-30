import os, time
import plearn.utilities.toolkit as toolkit

from plearn.pyplearn.pyplearn       import PLearnRepr, generate_expdir
from plearn.pyplearn.PyPLearnObject import PyPLearnObject
from plearn.utilities.Bindings      import *

__all__ = [
    ## Helper functions
    "option_value_split",

    ## Main class
    "Xperiment"
    ]

def get_inexistence_predicate( expkey, cache_once=True ):
    "Returns a predicate checking that the experiment doesn't exist."
    def inexistence_predicate( arguments ):
        if not (cache_once and Xperiment._cached_experiments):
            Xperiment.load_experiments( expkey )
        matches = Xperiment.match( arguments )
        return len( matches ) == 0

    return inexistence_predicate
    

def option_value_split( s, sep="=", rhs_casts=[] ):
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

class Xperiment(PyPLearnObject):
    ## The order is important! See rhs() implementation.
    rhs_casts        = [ int , float ]
    expdir_prefix    = "expdir" 
    metainfos_fname  = "metainfos.txt"
    cached_exp_fname = 'Xperiment.py'  
    lhs_length       = 35

    ## See load_experiments
    _cached_experiments = None

    def load_single_exp( cls, path, expkey ):
        cached = os.path.join( path, cls.cached_exp_fname )
        if os.path.exists( cached ):
            return eval( open( cached, 'r' ).read() )

        exp       = cls( path = path , expkey = expkey )

        # THE KEY SHOULD NOT BE PASSED TO THE FIRST LOAD!!!
        ## cachefile = open( cached, 'w' )
        ## print >>cachefile, exp.plearn_repr()
        ## cachefile.close()
        
        return exp
        
    load_single_exp = classmethod( load_single_exp )

    def load_experiments( cls, expkey=[], dirlist=None ):
        assert isinstance( expkey, list )
        xperiments = []

        if dirlist is None:
            dirlist = os.listdir( os.getcwd() )            

        for fname in dirlist:
            if fname.startswith( cls.expdir_prefix ):                
                x = cls.load_single_exp( fname, expkey )
                xperiments.append( x )            
            xperiments.sort()

        cls._cached_experiments = \
            [ x for x in xperiments 
              if len(expkey) == 0 or
              len(x.infos) == len(expkey) ]
    
        return cls._cached_experiments            
    load_experiments = classmethod( load_experiments )

    def load_filed_experiments( cls, filename ):
        if not os.path.exists( filename ):            
            raise AssertionError( "Xperiments.load_filed_experiments() expects a "
                                  "valid file; %s is not" % filename
                                  )
        
        exps = open( filename, "r" )
        cls._cached_experiments = eval( exps.read() )
        exps.close()
        return cls._cached_experiments
    load_filed_experiments = classmethod( load_filed_experiments )

    def match( cls, expkey=[] ):
        if cls._cached_experiments is None:
            raise AssertionError("Xperiment.load_experiments must be called before Xperiment.match.")
                        
        return [ exp for exp in cls._cached_experiments
                 if exp.is_matched( expkey )
                 ]    
    match = classmethod( match )

    def save_cache( cls, filename ):
        cache_file = open( filename, "w" )
        cache_file.write( PLearnRepr.repr(cls._cached_experiments) )
        cache_file.close()
    save_cache = classmethod( save_cache )

    class Defaults:
        path   = None
        infos  = None
        expkey = None
            
    def __init__( self, **overrides ):
        PyPLearnObject.__init__( self, **overrides )

        if self.infos is None:            
            self.infos = self.parse_file( os.path.join(self.path, self.metainfos_fname), self.expkey )
        
    def is_matched( self, expkey ):
        match_predicate = lambda key,val: val is None or self.infos[key]==val
        for string_key in expkey:
            key, val = option_value_split(string_key, rhs_casts=self.rhs_casts)
            if ( key not in self.infos or 
                 not match_predicate(key, val) ):
                return False

        ## Always matching empty expkey
        return True

    def parse_file( self, mipath, expkey ):
        if not os.path.exists( mipath ):
            return Bindings()

        infos  = Bindings([ option_value_split( line, rhs_casts=self.rhs_casts )
                            for line in file(mipath, "r")
                            ])

        ## Restricting to the keys asked for
        if expkey:
            expkey = Bindings([ option_value_split(key, rhs_casts=self.rhs_casts)
                                for key in expkey
                                ])
            validity_predicate = lambda lhs,rhs: rhs is None or infos[lhs]==rhs

            subset = Bindings()
            for (lhs,rhs) in expkey.iteritems():
                if lhs in infos and validity_predicate(lhs,rhs):
                    subset[lhs] = infos[lhs]
            infos = subset

        return infos

    def __cmp__( self, other ):
        if self.path == other.path:
            return 0

        other_it = other.infos.iteritems()
        for item in self.infos.iteritems():
            try:
                compare = cmp( item, other_it.next() )                
                if compare != 0:
                    return compare
            except StopIteration:
                return 1 ## >
                
        ## Non-Positive ( < or == )
        return len(self.infos) - len(other.infos)

    def __str__( self ):
        return "%s\n%s\n" % ( self.path,
                              "\n".join( [ "    %s= %s"
                                           % ( lhs.ljust(self.lhs_length), str(rhs) )
                                           for (lhs,rhs) in self.infos.iteritems() ] )
                              )
