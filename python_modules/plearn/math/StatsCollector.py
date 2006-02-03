# readMMat.py
# Copyright (C) 2006 by Nicolas Chapados
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


# Author: Nicolas Chapados

import sys
import numarray.ieeespecial      as  ieee
from   numarray    import zeros
from   numarray    import array  as  normal_array
from   numarray.ma import *                         # masked array
from   fpconst     import NegInf, PosInf


class StatsCollector:
    """Similar to a PLearn VecStatsCollector."""

    def __init__(self, fieldnames):
        """The constructor must be passed the fieldnames of the data we
        will be accumulating.
        """
        self.forget(fieldnames)


    def forget(self, fieldnames):
        """Reset all accumulators to zero.
        """
        width = len(fieldnames)
        self.fieldnames = fieldnames
        self.n          = zeros(width)
        self.nnonnan    = zeros(width)
        self.nnan       = zeros(width)
        self.sum        = zeros(width)
        self.sum_sq     = zeros(width)
        self.min        = zeros(width) + PosInf
        self.argmin     = zeros(width) + -1
        self.max        = zeros(width) + NegInf
        self.argmax     = zeros(width) + -1


    def width(self):
        """Return the number of columns (fields).
        """
        return len(self.fieldnames)

    
    def update(self, arr):
        """Update the accumulators of the StatsCollector given a complete matrix;
        assume that all observations have the same weight.
        Properly handle missing values.
        """
        assert self.width() == shape(arr)[1]
        i = 0

        ## Update number of elements counters
        (length,width)= shape(arr)
        initial_n     = self.n[:]          # Keep old n for argmin/argmax
        n             = zeros(width) + length
        missings      = ieee.isnan(arr)
        nnan          = sum(missings)
        self.n       += n
        self.nnan    += nnan
        self.nnonnan += n - nnan

        ## Create masked version of arr and update sums
        ma = masked_array(arr, mask=missings)
        self.sum     = self.sum + sum(ma)          # += does not work...
        self.sum_sq  = self.sum_sq + sum(ma*ma)    # += does not work...

        ## Update (arg)min / make sure old argmin is kept if not updated
        ma_argmin  = argmin(ma,0)
        ma_min     = ma[ma_argmin, range(width)]
        min_newpos = argmin(array([self.min, ma_min]), 0).astype('Bool')
        self.min[min_newpos]    = ma_min[min_newpos]
        self.argmin[min_newpos] = ma_argmin[min_newpos] + initial_n[min_newpos]

        ## Update (arg)max / make sure old argmax is kept if not updated
        ma_argmax  = argmax(ma,0)
        ma_max     = ma[ma_argmax, range(width)]
        max_newpos = argmax(array([self.max, ma_max]), 0).astype('Bool')
        self.max[max_newpos]    = ma_max[max_newpos]
        self.argmax[max_newpos] = ma_argmax[max_newpos] + initial_n[min_newpos]


    def getStats(self):
        """Return a map of computed statistics.
        """
        nn  = normal_array(self.nnonnan).astype('Float64')
        s   = normal_array(self.sum)
        ssq = normal_array(self.sum_sq)
        var = (ssq/nn - (s/nn)*(s/nn)) * (nn / (nn-1))
        
        return {
            "N"           : normal_array(self.n),
            "NMISSING"    : normal_array(self.nnan),
            "NNONMISSING" : nn,
            "E"           : s / nn,
            "V"           : var,
            "STDDEV"      : sqrt(var),
            "STDERR"      : sqrt(var/nn),
            "SUM"         : s,
            "SUMSQ"       : ssq,
            "MIN"         : normal_array(self.min   ),
            "ARGMIN"      : normal_array(self.argmin),
            "MAX"         : normal_array(self.max   ),
            "ARGMAX"      : normal_array(self.argmax)
            }
            

    def printStats(self, os = sys.stdout):
        """Print a nice report with the statistics."""
        stats = self.getStats()
        sk = ["N"      , "NMISSING" , "NNONMISSING" , "E"   ,
              "V"      , "STDDEV"   , "STDERR"      , "SUM" ,
              "SUMSQ"  , "MIN"      , "ARGMIN"      , "MAX" ,  "ARGMAX" ]
        leftlen = max([len(x) for x in sk])
        colwidth = max( [15] + [len(x) for x in self.fieldnames] )

        print >>os, ' | '.join([' '*leftlen] + [ f.rjust(colwidth) for f in self.fieldnames ])
        print >>os, '-+-'.join([ '-'*leftlen ] + [ '-'*colwidth ] * self.width())

        for k in sk:
            stat = stats[k]
            elems = [ ('%.10g' % float(s)) for s in stat ]
            print >>os, ' | '.join( [k.rjust(leftlen)] + [ e.rjust(colwidth) for e in elems ] )


if __name__ == "__main__":
    from plearn.vmat.readAMat import readAMat
    ut,fieldnames = readAMat("top_100_test.amat")
    sc = StatsCollector(fieldnames)
    sc.update(ut)
    sc.printStats()
    print ""
    sc.update(ut)
    sc.printStats()
    sc.forget(fieldnames)
    sc.printStats()
    
