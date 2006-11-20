# gmm.py
# Copyright (C) 2006 Christian Dorion
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

# Author: Christian Dorion
"""Module for GMM estimation.

Notation:

X (T x k), \beta \in \RSet[k], y,u \in \RSet[T]
   y = X \beta + u
     where E[ u u\Tr ] = \Omega (T x T)

Instruments W (T x l), T > l, l >= k, 
   E[ u_t | W_t ] = 0;  E[ u_t u_s | W_t W_s ] = \omega_{ts}

The above implies that:
   E[ W_t\Tr ( y_t - X_t \beta ) ] = 0 \forall t

Suppose J (l x k) full rank is s.t.
   J\Tr W\Tr (y- X\beta) = 0
=> Orthogonality conditions!!
=> J = (W\Tr \Omega_0 W)\inv W\Tr X
 where the 0 subscript denotes the TRUE value of \Omega

GMM criterion:

 Q(\beta, y)
  = (y - X \beta)\Tr W (W\Tr \Omega_0 W)\inv W\Tr (y - X\beta)
  = T^{-0.5} (y - X \beta)\Tr W (T\inv W\Tr \Omega_0 W)\inv W\Tr (y - X\beta) T^{-0.5}

 => Q(\beta_0, y_0) \simASY \Chi^2(l)

\betahat_{GMM}
    = \BETA(\Omega_0)
    = (X\Tr W (W\Tr \Omega_0 W)\inv W\Tr X)\inv X\Tr W (W\Tr \Omega_0 W)\inv W\Tr y

Inefficient
  \betahat_{iGMM} = (X\Tr W \Lambda W\Tr X)\inv X\Tr W \Lambda W\Tr y

IV with W:  \uhat
  => Let \Omegahat = Diag(\uhat)
  => \betahat_{FGMM} = \BETA(\Omegahat)

\Varhat( \betahat_{FGMM} ) = (X\Tr W (W\Tr \Omegahat W)\inv W\Tr X)\inv
"""
from numarray import *
from numarray.linear_algebra import inverse

# Shorthand for matrix multiplication
def mm(*args):
    return reduce(matrixmultiply, args)

class GMM(object):
    """A GMM implementation assuming X is predetermined."""

    def __init__(self, y, X, W):
        self.y = y
        self.X = X
        self.W = W

        T = len(y)
        self.T = T
        assert X.shape[0] == T
        assert W.shape[0] == T
        assert W.shape[1] >= X.shape[1]

        # Performing IV with W as instruments
        Xt       = transpose(X)
        Wt       = transpose(W)
        WtW      = mm(Wt, W)
        WtW_inv  = inverse(WtW)
        ProjW    = mm(W, WtW_inv, Wt) 
        XtProjW  = mm(Xt, ProjW)

        self.beta_iv = mm(inverse(mm(XtProjW, X)), XtProjW, y)

        # The IV residuals
        self.uhat_iv = y - mm(X, self.beta_iv)
        
        # Consistent estimator of the covariance matrix of the error terms
        Omegahat = zeros((T, T), type=Float64)
        for t in range(T):
            Omegahat[t,t] = self.uhat_iv[t]**2

        # Feasible GMM
        XtW         = mm(Xt, W)
        WtX         = transpose(XtW)
        Lambda      = inverse( mm(Wt, Omegahat, W) )
        XtWLambda   = mm(XtW, Lambda)
        XtWLambdaWt = mm(XtWLambda, Wt)

        # This is the (feasible) GMM beta!
        self.beta = mm(inverse(mm(XtWLambdaWt, X)), XtWLambdaWt, y)

        # And these are the residuals
        self.uhat = y - mm(X, self.beta)

if __name__ == "__main__":        
    def matrix_distance(m1, m2):
        return maximum.reduce( fabs(ravel(m1-m2)) )

    from numarray.random_array import seed, random, normal
    seed(02, 25)
    
    # Setting the problem
    T = 5
    u = normal(0, 1, (T,))
    beta = range(T)
    
    X = 100 * random((T, T))
    y = mm(X, beta) + u    
    
    # Performing OLS
    Xt = transpose(X)
    XtX = mm(Xt, X)
    XtY = mm(Xt, y)
    beta_ols = mm( inverse(XtX), XtY)
    print "Distance beta_0 and beta_ols:", matrix_distance(beta, beta_ols)
    
    gmm = GMM(y, X, X)
    print "Distance beta_ols and beta_IV:", matrix_distance(beta_ols, gmm.beta_iv)
    print "Distance beta_ols and beta_GMM:", matrix_distance(beta_ols, gmm.beta)

    print "GMM residuals:"
    print gmm.uhat
