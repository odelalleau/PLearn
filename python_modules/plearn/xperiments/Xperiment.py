import os, time
import plearn.utilities.toolkit as toolkit

from plearn.utilities.toolkit  import date_time_string
from plearn.utilities.Bindings import *

__all__ = [
    ## Helper functions
    "generate_expdir",
    "option_value_split",

    ## Main class
    "Xperiment"
    ]

def generate_expdir( ):
    expdir = Xperiment.expdir_prefix
    if os.getenv('PyTest', '') != 'Running':                        
        expdir = '%s_%s' % ( expdir, date_time_string() )

    return expdir

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

class Xperiment:
    ## The order is important! See rhs() implementation.
    rhs_casts       = [ int , float ]
    expdir_prefix   = "expdir" 
    metainfos_fname = "metainfos.txt"
    lhs_length      = 35

    ## See load_experiments
    _cached_experiments = None

    def load_experiments( cls, expkey=[], dirlist=None ):
        xperiments = []

        if dirlist is None:
            dirlist = os.listdir( os.getcwd() )            

        for fname in dirlist:
            if fname.startswith( cls.expdir_prefix ):
                xperiments.append( cls( fname , expkey ) )            
            xperiments.sort()

        cls._cached_experiments = \
            [ x for x in xperiments 
              if len(expkey) == 0 or
              len(x.infos) == len(expkey) ]
    
        return cls._cached_experiments            
    load_experiments = classmethod( load_experiments )

    def match( cls, expkey=[], dirlist=None ):
        if cls._cached_experiments is None:
            raise AssertionError("Xperiment.load_experiments must be called before Xperiment.match.")
                        
        return [ exp for exp in cls._cached_experiments
                 if exp.is_matched( expkey )
                 ]    
    match = classmethod( match )
            
    def __init__( self, path, expkey=[] ):
        self.path  = path
        self.infos = self.parse_file( os.path.join(path, self.metainfos_fname), expkey )

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
