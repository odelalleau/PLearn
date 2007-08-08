## Automatically adapted for numpy.numarray Jun 13, 2007 by python_numarray_to_numpy (-xsm)

# missings_robust_covariance
# Copyright (C) 2007 by Nicolas Chapados
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
from fpconst     import NaN
from numpy.numarray.ma import *
from plearn.math import isNaN

def missingsRobustCovariance(arr, unbiased=False, epsilon=1e-8):
    """Compute the covariance matrix in a way that's robust to missing values.

    This function computes the covariance matrix between the columns of
    'arr' in a way that is resistant to missing values (NaN).  It does not
    just 'drop missing rows', like would be the easy thing to do.  Instead,
    it uses as much information as it can, and can be used, for instance,
    when EVERY ROW contains one or more missings.

    If the matrix contains no missings whatsoever, it makes sense to use
    the option 'unbiased=True', which produces the same results as the
    standard bias-corrected maximum likelihood estimator.  Otherwise,
    numerical problems may ensue with the covariance matrix (e.g. lack of
    positive-definiteness).

    The option 'epsilon' is used to add a small regularization constant to
    the diagonal.
    """
    ## Second-order statistics
    mask   = isnan(arr)
    arr_ma = array(arr, mask=mask)
    count  = array(1 - mask)

    ## First-order statistics to center
    rowsum  = sum(arr_ma)
    row_c   = sum(count)
    rowmean = rowsum / row_c

    ## Maximum likelihood cov
    xxt       = dot(transpose(arr_ma),arr_ma)
    cov_count = dot(transpose(count),count)
    e_xxt     = xxt / cov_count
    outer_m   = outerproduct(rowmean, rowmean)
    cov_mle   = e_xxt - outer_m

    ## Unbias the thing.  Subtle since the count may be 1 in some places
    ## and we don't want to divide by zero.  Use masking again.
    masked_count = masked_less_equal(cov_count, 1.0)
    cov_mle *= masked_count / (masked_count-1.)

    for i in range(cov_mle.shape[0]):
        cov_mle[i,i] += epsilon         # numarray cannot assign to diagonal!!!!

    return filled(cov_mle, NaN)
