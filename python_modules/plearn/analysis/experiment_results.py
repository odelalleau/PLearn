# experiment_results.py
# Copyright (C) 2007 Nicolas Chapados, Christian Dorion
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
import sys, os, os.path, glob, fnmatch, csv, numpy.numarray

import os, glob, fnmatch, numpy.numarray
from plearn.vmat.PMat          import PMat
from plearn.vmat.smartReadMat  import smartReadMat
from plearn.utilities.moresh   import relative_path
from plearn.utilities.Bindings import Bindings

#####  ExperimentDirectory  #################################################

class ExperimentDirectory( object ):
    """Class representing a subdirectory within a Finlearn3 experiment.

    This object loads on-demand the .pmat, .amat and .csv making up the
    subdirectory, and allows recursive access to sub-subdirectory through
    simple member access. So if you have, for instance, an expdir
    containing a PerformanceReport, you can write:

        xp.PerformanceByKind.test_raw_costs['rel_return']

    and if test_raw_costs is not yet loaded, it is at that point, and you
    can access all columns of the underlying file by name.  Note that the
    returned data is a VECTOR (1-D).

    As a special case, passing the string '__all__' returns the whole
    array.
    """
    def __init__(self, expdir):
        if os.path.isdir(expdir):
            self.expdir = expdir
        else:
            raise RuntimeError, "Cannot construct ExperimentDirectory because " \
                  "path '%s' is not a readable directory" % expdir

        ## Remember files of interest
        self.pmats = self._get_files('pmat')
        self.csv   = self._get_files('csv')
        self.amats = self._get_files('amat')

        ## Remember the subdirectories
        self.subdirs = [ f for f in os.listdir(expdir)
                         if not f.endswith('.metadata')
                            and os.path.isdir(os.path.join(expdir, f)) ]

    def __str__(self):        
        return "\n    ".join([ self.expdir ] + self.subdirs)

    def _get_files(self, extension):
        return [ os.path.splitext(os.path.basename(fname))[0]
                 for fname in glob.glob(os.path.join(self.expdir, '*.' + extension)) ]

    def find(self, pattern):
        """Recursively find files that match a shell-like pattern in the
        expdir tree
        
        For example, to find all generated png files, you can write:

            xp.find('*.png')
        """
        return [ os.path.join(r,f)
                 for (r,d,files) in os.walk(self.expdir)
                   for f in files if fnmatch.fnmatch(f, pattern) ]

    def _add_array(self, name, arr, fieldnames):
        columns = {'__all__': arr}
        for i, f in enumerate(fieldnames):
            columns[f] = arr[:,i]
        setattr(self, name, columns)
        return columns

    def __getattr__(self, name):
        if name in self.subdirs:
            subdir = ExperimentDirectory(os.path.join(self.expdir, name))
            setattr(self, name, subdir)
            return subdir

        elif name in self.pmats or name in self.amats or name in self.csv:
            # minorly ugly, but will do for now...
            if name in self.pmats: filename = os.path.join(self.expdir, name+'.pmat')
            if name in self.amats: filename = os.path.join(self.expdir, name+'.amat')
            if name in self.csv  : filename = os.path.join(self.expdir, name+'.csv' )

            arr, fieldnames = smartReadMat(filename)
            return self._add_array(name, arr, fieldnames)

        else:
            raise ValueError, "ExperimentDirectory '%s' does not contain a component '%s'" \
                  % (self.expdir, name)

            
#####  ExperimentResults  ###################################################

class ExperimentResults( ExperimentDirectory ):
    """Class allowing the inspection of Finlearn3 experiment results.

    Note: an expdir is considered to contain valid results only if contains
    a 'metainfos.txt' file, indicating that the experiment completed
    successfully.
    """
    cached = []
    metainfos_path = 'metainfos.txt'
    
    def __init__(self, expdir):
        super(ExperimentResults,self).__init__(expdir)
        metainfos_name = os.path.join(expdir, self.metainfos_path)
        if os.path.isfile(metainfos_name):
            self.metainfos = self.parseMetaInfos(metainfos_name)
        else:
            # Experiment is still running
            self.metainfos = ExpKey()

        self.path = expdir
        self.cached.append(self)

    def __cmp__(self, other):
        raise ExpKey.keycmp(x1, x2, None)

    def __str__( self ):
        return self.toString()

    def getKey(self, expkey=None):
        if expkey is None:
            return self.metainfos

        subset = ExpKey()
        for key in expkey:
            if key in self.metainfos:
                subset[key] = self.metainfos[key]
            else:
                subset[key] = None
        return subset
        
    def getSubKey(self, keys=None):
        """Returns a subset of the metainfos dict for the given keys."""
        if keys is None:
            return self.metainfos.copy()

        subset = ExpKey()
        for key in keys:
            if key in self.metainfos:
                subset[key] = self.metainfos[key]
            else:
                subset[key] = None
        return subset
        
    def isMatched(self, expkey=[]):
        # Always matching empty expkey
        if not expkey:
            return True 

        # User should probably become aware of the concept of ExpKey.
        if not isinstance(expkey, ExpKey): 
            expkey = ExpKey(expkey)

        # For efficiency
        if len(expkey) > len(self.metainfos):
            return False

        # A key element from the expkey matches this experiement if
        #
        # 1) the key exists within this experiment ExpKey
        #
        # 2) the value is not restricted (None) or is restricted the same
        #    value than the one in this experiment
        match_predicate = lambda lhs,rhs: \
            lhs in self.metainfos and \
            ( rhs is None or self.metainfos[lhs]==rhs )                       

        # All key element must match (match_predicate)
        for lhs, rhs in expkey.iteritems():
            if not match_predicate(lhs,rhs):
                return False
        
        # All key element matched
        return True

    def isRunning(self):
        return len(self.metainfos) == 0

    def toString(self, expkey=None, short=False):
        s = '%s\n' % relative_path(self.expdir)
        if short and expkey is None:
            return s
        
        for key, value in self.getKey(expkey).iteritems():
            s += '    %s= %s\n' % (key.ljust(30), value)
        return s


    #####  Static Methods  ##############################################

    def loadResults(exproot=None, forget=True):
        # Clean cache if required
        if forget:
            ExperimentResults.cached = []
    
        # Use CWD as default exproot
        if exproot is None:
            exproot = os.getcwd()
    
        # Don't barf on inexistant exproots
        if not os.path.exists(exproot):
            return
    
        # Load all experiments in the provided exproot
        dirlist = os.listdir(exproot)
        for fname in dirlist:
            candidate_path = os.path.join(exproot, fname)
            candidate_infopath = os.path.join(candidate_path,
                                              ExperimentResults.metainfos_path)
            if fname.startswith("expdir") and os.path.exists(candidate_infopath):
                ExperimentResults(candidate_path) # Cached in __init__ 
    loadResults = staticmethod(loadResults)

    def match(expkey=[]):
        if not ExperimentResults.cached:
            ExperimentResults.loadResults()
        return [ exp for exp in ExperimentResults.cached if exp.isMatched(expkey) ]
    match = staticmethod(match)

    def match_one(expkey=[]):
        matches = ExperimentResults.match(expkey)
        assert len(matches) == 1, \
               "Key matches %d experiments\n%s" % (len(matches), expkey)
        return matches[0]
    match_one = staticmethod(match_one)

    def parseMetaInfos(metainfos_name):
        metainfos = ExpKey()
        f = open(metainfos_name)
        for line in f:
            [lhs,rhs] = line.split('=',1)
            metainfos[lhs.strip()] = rhs.strip()
        return metainfos
    parseMetaInfos = staticmethod(parseMetaInfos)
    

#####  ExpKey  ##############################################################

class ExpKey( Bindings ):
    """Utility class for matching experiments (ExperimentResults.isMatched())"""
    _neglected = [ 'expdir', 'expdir_root' ]
    
    def __init__(self, keysrc=[]):
        expkey = []
        if isinstance(keysrc, str): # Handle single string
            keysrc = [ keysrc ]
        assert isinstance(keysrc, list)
        
        # List of tuples
        if keysrc and isinstance(keysrc[0], tuple):
            expkey = keysrc

        # List of strings
        elif keysrc and isinstance(keysrc[0], str):
            for key in keysrc:
                lhs_rhs = key.split('=',1)
                if len(lhs_rhs)==1:
                    expkey.append( (lhs_rhs[0].strip(), None) )
                else:
                    expkey.append( (lhs_rhs[0].strip(), lhs_rhs[1].strip()) )

        # Create the dict
        Bindings.__init__(self, expkey)
        for neg in self._neglected:
            if neg in self:
                del self[neg]

    def listkey(self):
        keyjoin = lambda key,val: (val is None and key) or "%s=%s"%(key,val)
        return [ keyjoin(key, value) for key,value in self.iteritems() ]
    
    def strkey(self):
        return " ".join(self.listkey())


    #####  Static Methods  #################################################

    def keycmp(exp, other, expkey):
        """Compare two experiments along a given key"""
        if exp.expdir == other.expdir:
            return 0
        
        exp_subkey   = exp.getSubKey(expkey)
        exp_it       = exp_subkey.iteritems()
        
        other_subkey = other.getSubKey(expkey)
        other_it     = other_subkey.iteritems()
        
        for item in exp_it:
            try:
                compare = cmp(item, other_it.next())
                if compare != 0:
                    return compare
            except StopIteration:
                return 1 ## >
        
        ## Non-Positive ( < or == )
        return len(exp_subkey) - len(other_subkey)
    keycmp = staticmethod(keycmp)


