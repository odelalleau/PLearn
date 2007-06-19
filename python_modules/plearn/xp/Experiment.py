import copy, os, time
import plearn.utilities.toolkit as toolkit

from plearn                         import pyplearn
from plearn.pyplearn.plearn_repr    import plearn_repr, python_repr
from plearn.pyplearn.PyPLearnObject import PLOption, PyPLearnObject
from plearn.utilities.moresh        import *
from plearn.utilities.Bindings      import *
from plearn.vmat.PMat               import PMat

rhs_casts = [ int , float ]

def inexistence_predicate( arguments, forget=False ):
    """Predicate checking that the experiment doesn't exist."""
    if forget:
        Experiment.cache_experiments( forget=forget )
    matches = Experiment.match( arguments )
    return len( matches ) == 0
    
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

def xpathfunction( func ):
    def function( obj, *args, **kwargs ):
        if isinstance( obj, Experiment ):
            func( obj.path, *args, **kwargs )
        elif len(args) and isinstance( obj, str ):
            func( obj, *args, **kwargs )
        else:
            raise ValueError( obj )
    return function

def migrate(path, dest, move=False):
    abspath = os.path.abspath(path)
    if move:
        absdest = os.path.abspath(dest)
        os.system('mv %s %s' % (abspath, absdest))
    else:
        relative_link(abspath, dest)            
migrate = xpathfunction( migrate )

class ExpKey( Bindings ):
    _neglected = [ 'expdir', 'expdir_root' ]
    
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
        for neg in self._neglected:
            if neg in self:
                del self[neg]

    def listkey(self):
        keyjoin = lambda key,val: (val is None and key) or "%s=%s"%(key,val)
        return [ keyjoin(key, value) for key,value in self.iteritems() ]
    
    def strkey(self):
        return " ".join(self.listkey())

class Experiment(PyPLearnObject):
    ##
    # Options
    path    = PLOption(None)
    expkey  = PLOption(None)
    abspath = PLOption(None)
            
    ##
    # Class variables
    _expdir_prefix    = 'expdir'
    _metainfos_fname  = 'metainfos.txt'
    _cached_exp_fname = 'Experiment.cache'  
    _lhs_length       = 35
    _cached           = None          # See cache_experiments
    _opened_pmats     = []

    ##
    # PyPLearnObject's classmethod
    _by_value = classmethod( lambda cls: True )

    def cache_experiments( cls, exproot=None, forget=True, name_key=None ):
        if exproot is None:
            roots = pyplearn.config.get_option( 'EXPERIMENTS', 'expdir_root' ).split(',')            
            for exproot in roots:
                cls.cache_experiments( exproot=exproot, forget=False, name_key=name_key )
            return

        if cls._cached is None:
            cls._cached = []
        elif forget:
            for exp in cls._cached:
                exp.close()
            cls._cached = []

        if exproot:
            if not os.path.exists( exproot ):
                return
            dirlist = os.listdir( exproot )
        else:
            dirlist = os.listdir( os.getcwd() )            

        for fname in dirlist:
            candidate_path = os.path.join(exproot, fname)
            candidate_infopath = os.path.join(candidate_path, cls._metainfos_fname)
            if fname.startswith( cls._expdir_prefix ) \
                   and os.path.exists(candidate_infopath):
                x = cls( path = candidate_path )
                if name_key:
                    x.setName(x.expkey[name_key])
                cls._cached.append( x )
        return cls._cached
    cache_experiments = classmethod( cache_experiments )

    def match( cls, expkey=[] ):
        if cls._cached is None:
            cls.cache_experiments()
        return [ exp for exp in cls._cached if exp.isMatched( expkey ) ]
    match = classmethod( match )

    def match_one( cls, expkey ):
        matches = cls.match( expkey )
        assert len(matches) == 1, \
            "Key matches %d experiments\n%s" % (len(matches), expkey)
        return matches[0]    
    match_one = classmethod( match_one )

    #
    #  Instance methods
    #
    def __init__( self, **overrides ):
        PyPLearnObject.__init__( self, **overrides )
        self._name = None
        if self.expkey is None:
            self.expkey = ExpKey( os.path.join( self.path, self._metainfos_fname ) )

        # Update abspath        
        self.abspath = os.path.abspath( self.path )

    def __del__(self):
        self.close()
        
    def close(self):
        for pmat in self._opened_pmats:
            pmat.close()
        
    def __cmp__( self, other ):
        raise NotImplementedError( 'Use keycmp( x1, x2, expkey )' )

    def __str__( self ):
        return self.toString()

    def loadPMat( self, *pmats ):
        for pmat in pmats:
            attr_name = os.path.basename(pmat).replace('.pmat', '')
            if hasattr(self, attr_name):
                assert getattr(self, "_pmat_%s"%attr_name) == pmat
            else:
                setattr(self, "_pmat_%s"%attr_name, pmat)                

                pmat = PMat(os.path.join(self.abspath, pmat))
                setattr(self, attr_name, pmat)
                self._opened_pmats.append(pmat)
        return self._opened_pmats[-1]

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
        
    def isMatched( self, expkey ):
        # Always matching empty expkey
        if not expkey:
            return True 

        # User should probably become aware of the concept of ExpKey.
        if not isinstance( expkey, ExpKey ): 
            expkey = ExpKey( expkey )

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

        # All key element must match (match_predicate)
        for lhs, rhs in expkey.iteritems():
            if not match_predicate(lhs,rhs):
                return False
        
        # All key element matched
        return True

    def toString( self, expkey=None, short=False ):
        s = '%s\n' % self.path

        if short and expkey is None:
            return s
        
        for key, value in self.getKey(expkey).iteritems():
            s += '    %s= %s\n' % (key.ljust(30), value)
        return s

    def running( self ):
        return len(self.expkey) == 0


    ###  set/getName

    def getName(self):
        assert self._name
        return self._name

    def setName(self, name):
        self._name = name
        

    
