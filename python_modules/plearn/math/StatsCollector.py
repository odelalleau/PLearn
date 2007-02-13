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

import os, sys
import numarray.ieeespecial        as  ieee
from   plearn.math import isNaN
from   numarray    import array    as  normal_array
from   numarray    import sometrue as  normal_sometrue
from   numarray    import zeros, matrixmultiply, transpose
from   numarray.ma import *                         # masked array
from   fpconst     import NegInf, PosInf

from   plearn.utilities import ppath


def _printMatrix(m, rownames, colnames, os, pretty = True):
    """Nicely print a matrix given rownames and column names.
    If 'pretty' is set to False, the output will not be so nice, but at least
    will not cause test failures due to a zero test blank tolerance.
    """
    (length, width) = shape(m)
    assert length == len(rownames)
    assert width  == len(colnames)
    leftlen  = max([len(x) for x in rownames])
    colwidth = max( [16] + [len(x) for x in colnames] )

    if pretty:
        print >>os, ' | '.join( [' '*leftlen] + \
                                [ f.ljust(colwidth) for f in colnames ])
        print >>os, '-+-'.join( ['-'*leftlen] + [ '-'*colwidth ] * width )
    else:
        print >>os, ' | '.join( [' ' + f for f in colnames ] )
        print >>os, '-+-'.join( ['-'] + [ '-' ] * width )

    for k in xrange(length):
        currow = m[k]
        elems = [ ('%.10g' % float(s)) for s in currow ]
        if pretty:
            print >>os, ' | '.join( [ rownames[k].rjust(leftlen) ] +
                                [ e.ljust(colwidth) for e in elems ] )
        else:
            print >>os, ' | '.join( [ rownames[k] ] + [ e for e in elems ] )


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
        self.n          = zeros(width)                # Sum of nnonnan and nnan
        self.nnonnan    = zeros(width)                # Nb of non-missing elements
        self.nnan       = zeros(width)                # Nb of missing elements
        self.nxxt       = 0                           # Nb of elements in sum_xxt
        self.sum        = zeros(width)                # Sum of elements
        self.sum_ssq    = zeros(width)                # Sum of square of elements
        self.sum_xxt    = zeros((width, width))       # Accumulator of outer products
        self.sum_nomi   = zeros(width)                # Sum of elements (no missings)
        self.min        = zeros(width) + PosInf       # Minimum element
        self.argmin     = zeros(width) + -1           # Position of minimum
        self.max        = zeros(width) + NegInf       # Maximum element
        self.argmax     = zeros(width) + -1           # Position of maximum


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
        missings      = isNaN(arr)
        nnan          = sum(missings)
        self.n       += n
        self.nnan    += nnan
        self.nnonnan += n - nnan

        ## Create masked version of arr and update accumulators
        ma = masked_array(arr, mask=missings)        # Here, mask missings only
        arr_nomissings = arr[~normal_sometrue(missings,1)]  # Here, strip missing rows
        self.sum     = self.sum + sum(ma)            # += does not work...
        self.sum_ssq = self.sum_ssq + sum(ma*ma)     # += does not work...
        self.sum_xxt = self.sum_xxt + matrixmultiply(transpose(arr_nomissings),
                                                     arr_nomissings)
        self.sum_nomi= self.sum_nomi + sum(arr_nomissings)
        self.nxxt   += shape(arr_nomissings)[0]

        ## Update (arg)min / make sure old argmin is kept if not updated
        ma_argmin  = argmin(ma,0)
        ma_min     = ma[ma_argmin, range(width)]
        min_newpos = argmin(array([self.min, ma_min]), 0).astype('Bool')
        self.min[min_newpos]    = ma_min[min_newpos]
        # XXX Argmin computation needs to be revised! Does not work, at least
        # when passing array of shape (1,1).
        self.argmin[min_newpos] = ma_argmin[min_newpos] + initial_n[min_newpos]

        ## Update (arg)max / make sure old argmax is kept if not updated
        ma_argmax  = argmax(ma,0)
        ma_max     = ma[ma_argmax, range(width)]
        max_newpos = argmax(array([self.max, ma_max]), 0).astype('Bool')
        self.max[max_newpos]    = ma_max[max_newpos]
        # XXX Argmax computation needs to be revised! Does not work, at least
        # when passing array of shape (1,1). Also, is the use of min_newpos
        # correct?
        self.argmax[max_newpos] = ma_argmax[max_newpos] + initial_n[min_newpos]


    def getStats(self):
        """Return a map of computed statistics.
        """
        nn    = normal_array(self.nnonnan).astype('Float64')
        nx    = self.nxxt
        s     = normal_array(self.sum)
        snomi = normal_array(self.sum_nomi)
        ssq   = normal_array(self.sum_ssq)
        xxt   = normal_array(self.sum_xxt)

        var = (ssq/nn - (s/nn)*(s/nn)) * (nn / (nn-1))
        cov = (xxt/nx - outerproduct(snomi/nx,snomi/nx)) * (float(nx) / (nx-1))
        dia = diagonal(cov)
        
        return {
            "N"           : normal_array(self.n),
            "NMISSING"    : normal_array(self.nnan),
            "NNONMISSING" : nn,
            "E"           : s / nn,
            "V"           : var,
            "COV"         : cov,
            "CORR"        : normal_array(cov / outerproduct(sqrt(dia),sqrt(dia))),
            "STDDEV"      : sqrt(var),
            "STDERR"      : sqrt(var/nn),
            "SUM"         : s,
            "SUMSQ"       : ssq,
            "MIN"         : normal_array(self.min   ),
            "ARGMIN"      : normal_array(self.argmin),
            "MAX"         : normal_array(self.max   ),
            "ARGMAX"      : normal_array(self.argmax)
            }
            

    def printStats(self, os = sys.stdout, pretty = True):
        """Print a nice report with the statistics.
        If 'pretty' is set to False, the output will not be so nice, but at least
        will not cause test failures due to a zero test blank tolerance.
        """
        if len(nonzero(self.nnonnan)) != len(self.nnonnan):
            print >>os, "One or more columns in StatsCollector does not contain any data"
            return                      # Nothing accumulated yet
        
        stats = self.getStats()
        sk = ["N"      , "NMISSING" , "NNONMISSING" , "E"   ,
              "V"      , "STDDEV"   , "STDERR"      , "SUM" ,
              "SUMSQ"  , "MIN"      , "ARGMIN"      , "MAX" ,  "ARGMAX" ]

        m = array([[stats[k][i] for i in range(self.width())] for k in sk])
        _printMatrix(m, sk, self.fieldnames, os, pretty)
        
        print "\nCovariance Matrix:"
        _printMatrix(stats["COV"], self.fieldnames, self.fieldnames, os, pretty)
        print "\nCorrelation Matrix:"
        _printMatrix(stats["CORR"], self.fieldnames, self.fieldnames, os, pretty)


if __name__ == "__main__":
    from plearn.vmat.readAMat import readAMat
    ut,fieldnames = readAMat(
            os.path.join(
                ppath.ppath('PLEARNDIR'),
                'examples', 'data', 'test_suite',
                'top_100_test.amat'
                )
            )
    sc = StatsCollector(fieldnames)
    sc.update(ut)
    sc.printStats(sys.stdout, False)
    print "\nAfter accumulating some more:"
    sc.update(ut)
    sc.printStats(sys.stdout, False)
    print "\nAfter forgetting:"
    sc.forget(fieldnames)
    sc.printStats(sys.stdout, False)
