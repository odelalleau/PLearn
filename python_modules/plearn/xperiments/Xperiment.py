import os
import plearn.utilities.toolkit as toolkit

__all__ = [
    ## Helper functions
    "option_value_split",

    ## Main class
    "Xperiment"
    ]

def option_value_split( s, sep="=", rhs_casts=[] ):
    """Returns a (lhs, rhs) pair given I{sep}."""
    lhs_len = s.find( sep )
    if lhs_len == -1:
        return (s, None)

    ## The left hand side 
    lhs     = s[:lhs_len]

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

    def match( cls, expkey=[] ):
        xperiments = []

        files = os.listdir( os.getcwd() )    
        for fname in files:
            if fname.startswith( cls.expdir_prefix ):
                xperiments.append( cls(fname, expkey) )            
            xperiments.sort()
            
        return [ x for x in xperiments
                 if len(expkey) == 0 or
                 len(x.infos) == len(expkey) ]
    match = classmethod( match )
        
    
    def __init__( self, path, expkey=[] ):
        self.path  = path

        fdic = dict([ option_value_split(f, rhs_casts=self.rhs_casts) for f in expkey ])        
        self.infos = self.parse_file( os.path.join(path, self.metainfos_fname),
                                      fdic
                                      )

    def parse_file( self, mipath, expkey ):
        if not os.path.exists( mipath ):
            return [ ("Experiment still running", "") ]

        infos = []
        for line in file(mipath, "r"):
            lhs, rhs = option_value_split( line, rhs_casts=self.rhs_casts)
            do_append = True
            if len(expkey):
                found     = toolkit.find_one(lhs, expkey.keys())                
                do_append = found is not None

                if do_append and expkey[found[0]] is not None:
                    do_append = rhs==expkey[found[0]]

            if do_append:
                infos.append( ("%s"%lhs.strip(), rhs) )

        return infos
               
    def __cmp__( self, other ):
        if self.path == other.path:
            return 0
        
        len_o = len(other.infos)
        
        for (i, info) in enumerate(self.infos):
            if i == len_o:
                return 1

            oinfo = other.infos[i]
            if info != oinfo:
                return cmp(info[1], oinfo[1])

        ## Non-Positive ( < or == )
        return len(self.infos) - len_o

    def __str__( self ):
        return "%s\n%s\n" % ( self.path,
                              "\n".join( [ "    %s= %s"
                                           % ( lhs.ljust(self.lhs_length), str(rhs) )
                                           for (lhs,rhs) in self.infos ] )
                              )
