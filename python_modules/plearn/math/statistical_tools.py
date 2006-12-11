# statistical_tools.py
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
from arrays import *
from arrays import _as_matrix
from scipy import stats
from scipy.stats import mean, hmean, std, var, cov, skew, kurtosis
from scipy.stats.distributions import norm, chi2

def _chk_asarray(a, axis):
    if axis is None:
        a = ravel(a)
        outaxis = 0
    else:
        a = asarray(a)
        outaxis = axis
    return a, outaxis

# Corrected from the scipy bugged version
def nanmean(x,axis=-1):
    """Compute the mean over the given axis ignoring nans.
    """
    x, axis = _chk_asarray(x,axis)
    x = x.copy()
    Norig = x.shape[axis]
    factor = 1.0-sum(isnan(x),axis)*1.0/Norig
    putmask(x,isnan(x),0)
    return mean(x,axis)/factor


def autocorrelation(series, k=1, biased=True):
    """Returns autocorrelation of order 'k' and corresponding two-tailed pvalue.

    (Inspired by CLM pp.45-47)

    @param series: The series on which to compute autocorrelation
    @param k:      The order to which compute autocorrelation
    @param biased: If False, rho_k will be corrected according to Fuller (1976)

    @return: rho_k, pvalue
    """
    T = len(series)
    mu = mean(series)
    sigma = var(series)

    # Centered observations
    obs = series-mu    
    lagged = lag(obs, k) 
    truncated = obs[:-k]
    assert len(lagged) == len(truncated)

    # Multiplied by 'T' for numerical stability
    gamma_k = T*add.reduce(truncated*lagged)  # Numerator
    gamma_0 = T*add.reduce(obs*obs)           # Denominator
    rho_k   = (gamma_k / gamma_0)
    if rho_k > 1.0: rho_k = 1.0   # Correct for numerical errors

    # The standard normal random variable
    Z = sqrt(T)*rho_k
    
    # Bias correction?
    if not biased:
        rho_k += (1 - rho_k**2) * (T-k)/(T-1)**2
        Z = rho_k * T/sqrt(T-k)

    # The two-tailed p-value is twice the prob that value of a std normal r.v.
    # turns out to be greater than the (absolute) value of Z
    pvalue = 2*( 1 - norm.cdf(abs(Z)) )
    assert pvalue >= 0.0 and pvalue <= 1.0
    return rho_k, pvalue


def skewtest(a,axis=-1):
    """Tests whether the skew is significantly different from a normal distribution.

    Axis can equal None (ravel array first), an integer (the axis over
    which to operate), or a sequence (operate over multiple axes).

    NOTE: This function is mostly copied from scipy.stats.stats, but
    corrects for a major bug: the pvalue returned by SciPy is not valid
    when the skewness is negative! The return values also are slightly
    different: the skew is actually returned will the z-score is not.

    Returns: skewness and 2-tail z-probability
    """
    a, axis = _chk_asarray(a, axis)
    if axis is None:
        a = ravel(a)
        axis = 0
    skewness = skew(a,axis)
    n = float(a.shape[axis])
    if n<8:
        print "skewtest only valid for n>=8 ... continuing anyway, n=",n
    y = skewness * sqrt(((n+1)*(n+3)) / (6.0*(n-2)) )
    beta2 = ( 3.0*(n*n+27*n-70)*(n+1)*(n+3) ) / ( (n-2.0)*(n+5)*(n+7)*(n+9) )
    W2 = -1 + sqrt(2*(beta2-1))
    delta = 1/sqrt(log(sqrt(W2)))
    alpha = sqrt(2.0/(W2-1))
    y = where(equal(y,0),1,y)
    Z = delta*log(y/alpha + sqrt((y/alpha)**2+1))
    
    # The two-tailed p-value is twice the prob that value of a std normal r.v.
    # turns out to be greater than the (absolute) value of Z
    pvalue = 2*( 1 - norm.cdf(abs(Z)) )
    assert pvalue >= 0.0 and pvalue <= 1.0
    return skewness, pvalue
# Prevent user to use SciPy function
stats.skewtest = skewtest


def kurtosistest(a,axis=-1):
    """Tests whether a dataset has normal kurtosis

    That is, test whether kurtosis=3(n-1)/(n+1). Valid only for n>20.  Axis
    can equal None (ravel array first), an integer (the axis over which to
    operate), or a sequence (operate over multiple axes).

    NOTE: This function is mostly copied from scipy.stats.stats, but
    corrects for a major bug: the pvalue returned by SciPy is not valid
    when the kurtosis is negative! The return values also are slightly
    different: the kurtosis is actually returned will the z-score is not.

    Returns: kurtosis and 2-tail z-probability.
    """
    a, axis = _chk_asarray(a, axis)
    n = float(a.shape[axis])
    if n<20:
        print "kurtosistest only valid for n>=20 ... continuing anyway, n=",n
    kurt = kurtosis(a,axis)
    E = 3.0*(n-1) /(n+1)
    varkurt = 24.0*n*(n-2)*(n-3) / ((n+1)*(n+1)*(n+3)*(n+5))
    x = (kurt-E)/sqrt(varkurt)
    sqrtbeta1 = 6.0*(n*n-5*n+2)/((n+7)*(n+9)) * sqrt((6.0*(n+3)*(n+5))/
                                                       (n*(n-2)*(n-3)))
    A = 6.0 + 8.0/sqrtbeta1 *(2.0/sqrtbeta1 + sqrt(1+4.0/(sqrtbeta1**2)))
    term1 = 1 -2/(9.0*A)
    denom = 1 +x*sqrt(2/(A-4.0))
    denom = where(less(denom,0), 99, denom)
    term2 = where(equal(denom,0), term1, power((1-2.0/A)/denom,1/3.0))
    Z = ( term1 - term2 ) / sqrt(2/(9.0*A))
    Z = where(equal(denom,99), 0, Z)
    
    # The two-tailed p-value is twice the prob that value of a std normal r.v.
    # turns out to be greater than the (absolute) value of Z
    pvalue = 2*( 1 - norm.cdf(abs(Z)) )
    assert pvalue >= 0.0 and pvalue <= 1.0
    return kurt, pvalue    
# Prevent user to use SciPy function
stats.kurtosistest = kurtosistest
    

def Qstatistic(series, m, ljung_box=False):
    """Returns 'Q_m' and the corresponding pvalue.

    Q here refers to the Box and Pierce (1970) statistic. (Inspired by CLM p.49)

    @param series:    The series on which to compute Q.
    @param m:         The order up to which sum squared autocorrelations.
    @param ljung_box: If True, Q will be corrected according to Ljung and Box (1978).

    @return: Q_m, pvalue
    """
    T = len(series)

    # Handling Ljung and Box correction
    correction = lambda k: 1.0
    if ljung_box:
        correction = lambda k: 1.0/(T-k)

    Q_m = 0.0
    for k in range(1, m+1): # Last index is not returned by range...
        Q_m += correction(k) * autocorrelation(series, k, biased=True)[0]**2

    Q_m *= T
    if ljung_box:
        Q_m *= (T+2)

    # Q_m is asymptotically distributed Chi^2(m).
    return Q_m, 1-chi2.cdf(abs(Q_m), m)


def variance_ratio(series, q, rw_hypothesis=1):
    """Returns 'VR(q)' and the corresponding pvalue.

    VR(q) here refers to the variance ratio suggested in Campbell, Lo and
    MacKinlay (1997), pp.49-57.

    @param series:        The series on which to compute VR.
    @param q:             Number of periods of the long-horizon return in VR
    @param rw_hypothesis: Which null hypothesis to test against. The value
      must be in [0, 1, 3]. Zero is a special value under which no pvalue is
      reported. One and three lead to the use of RW1 and RW3.

    @return: VR_q [, pvalue -- if rw_hypothesis!=0 ]
    """
    assert q > 1
    T = len(series)
    qf = float(q)
    VR_q = 1.0
    for k in range(1, q): # Will sum till q-1 as desired
        VR_q += 2.0 * (1.0 - (k/q)) * autocorrelation(series, k, biased=True)[0]   

    # Zero is a special value under which no pvalue is reported
    if rw_hypothesis==0:
        return VR_q

    # TBReplaced by n*q in pp.52-55's version...        
    nq = float(T-1)
    Z = sqrt(nq) * (VR_q - 1.0)

    if rw_hypothesis==1:
        Z /= sqrt(2.0*(2*q - 1)*(q-1) / (3.0*q))
        return VR_q, 2*( 1 - norm.cdf(abs(Z)) )

    if rw_hypothesis==3:
        raise NotImplementedError
    raise ValueError("'rw_hypothesis' must be in [0,1,3].")

def as_stat_pair(*stats):
    formatted = []
    for stat,pvalue in stats:
        assert pvalue >= 0.0 and pvalue <= 1.0, "Invalid probability %.3f"%pvalue
        pair = stat, report_significance(pvalue, "%.3f")
        formatted.append(stacked_pair(pair, '%.2f', r'\text{\scriptsize (%s)}'))
        #    r"$\underset{\text{(%s)}}{%.2f}$"%(pair[1], pair[0]) )
    if len(formatted)==1:
        return formatted[0]
    return formatted

def stacked_pair(pair, fmt1='%s', fmt2='%s'):
    #return (r"$\overset{"+fmt1+'}{\underset{'+fmt2+'}{}}$')%pair
    return (r"$\begin{matrix}"+fmt1+r' \\ '+fmt2+r"\end{matrix}$")%pair
    
def report_significance(pvalue, format, threshold=0.05):
    if pvalue < threshold:
        format = r"\textbf{%s}"%format
    return format%pvalue


def studentized_range(means, variances, sample_sizes):
    nh = hmean(sample_sizes)
    mse = sum(variances) / len(variances)
    return (max(means) - min(means)) / sqrt(mse/nh)

def ols_regression(X, Y, intercept=True):
    """Perform an OLS regression (Robust to NaNs in targets).
    
        Y = alpha + X beta + epsilon
    
    If the 'intercept' argument is False, 'alpha' is enforced to be
    zero. The 'sigma' output is

        mmult(epsilon, transpose(epsilon)) / T .

    @return: alpha, beta, epsilon, sigma.
    """
    X, Y  = _as_matrix(X, Y)    
    T, K  = X.shape
    T2, N = Y.shape
    assert T==T2, "T,T2,N,K=%s"%[T, T2, N, K]
    
    Xorig = X
    Yorig = Y
    y_column = lambda ycol : Yorig[:,ycol]
    iota = lambda length: ones(shape=(length,1), type=Float64) 

    # Add an intercept if required
    if intercept:
        X = concatenate([ iota(T), Xorig ], 1)
        assert X.shape==(T,K+1)

    # Seeking computational efficiency...
    Xt         = transpose(X)
    XtX        = mmult(Xt, X)
    XtX_inv    = inverse(XtX)
    XtX_inv_Xt = mmult(XtX_inv, Xt)

    # To be estimated
    beta = array(shape=(N,K), type=Float64)
    alpha = array(shape=(N,), type=Float64)
    epsilon = zeros(shape=(N,T), type=Float64)    
    for ycol in range(N):
        Ycol = y_column(ycol)
        Y    = Ycol[ where(isNotNaN(Ycol)) ] 
        
        _T_ = len(Y)
        if _T_ != T: # There were NaNs...
            _X_   = X[where(isNotNaN(Ycol))]  # Filtered X
            _Xt_  = transpose(_X_)
            _XtX_ = mmult(_Xt_, _X_)            
            aug_beta = mmult(inverse(_XtX_), _Xt_, Y)
        else:
            _X_ = X
            aug_beta = mmult(XtX_inv_Xt ,Y)
        
        # Extract the intercept if any
        if intercept:
            alpha[ycol] = aug_beta[0]
            beta[ycol]  = aug_beta[1:]
        else:
            alpha[ycol] = 0.0
            beta[ycol]  = aug_beta

        prediction = mmult(_X_, aug_beta)
        epsilon[ycol][where(isNotNaN(Ycol))] = Y - prediction

    # Is sigma really supposed to be NxN?
    sigma = mmult(epsilon, transpose(epsilon)) / T    

    # if hccm is not None:
    #     Sigma = mmult(transpose(epsilon), epsilon) / N
    #     print shape(XtX_inv), shape(Xt), shape(Sigma), shape(X), shape(XtX_inv)
    #     H = mmult(XtX_inv, Xt, Sigma, X, XtX_inv) 
    #     resize(hccm, H.shape)
    #     hccm[:] = H[:]
    #     print H

    # for convenience...
    if K==1:
        beta = beta.getflat()
    if N==1:
        alpha   = alpha.  getflat()
        beta    = beta.   getflat()
        epsilon = epsilon.getflat()
        
    return alpha, beta, epsilon, sigma

#####  Very First Sketch...  ################################################

class LinearGMM(object):
    """A GMM implementation assuming a linear model and a predetermined 'X'.

    NOTE:
    This thing is a *very first* sketch toward a more general GMM class...
    
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
        WtW      = mmult(Wt, W)
        WtW_inv  = inverse(WtW)
        ProjW    = mmult(W, WtW_inv, Wt) 
        XtProjW  = mmult(Xt, ProjW)

        self.beta_iv = mmult(inverse(mmult(XtProjW, X)), XtProjW, y)

        # The IV residuals
        self.uhat_iv = y - mmult(X, self.beta_iv)
        
        # Consistent estimator of the covariance matrix of the error terms
        Omegahat = zeros((T, T), type=Float64)
        for t in range(T):
            Omegahat[t,t] = self.uhat_iv[t]**2

        # Feasible GMM
        XtW         = mmult(Xt, W)
        WtX         = transpose(XtW)
        Lambda      = inverse( mmult(Wt, Omegahat, W) )
        XtWLambda   = mmult(XtW, Lambda)
        XtWLambdaWt = mmult(XtWLambda, Wt)

        # This is the (feasible) GMM beta!
        self.beta = mmult(inverse(mmult(XtWLambdaWt, X)), XtWLambdaWt, y)

        # And these are the residuals
        self.uhat = y - mmult(X, self.beta)



def test_ols_regression(T, K, alpha=0.0, scale=10.0, plot=False):
    u = normal(0, 1, (T,))
    beta = range(1, K+1)
    
    X = scale * random((T, K))
    Y = alpha + mmult(X, beta) + u    

    
    print "Beta:", beta

    print 
    olsR = ols_regression(X, Y, False)
    print "Beta OLS (no intercept):", olsR[1], "(%.3f)"%matrix_distance(olsR[1],beta)

    print 
    ols = ols_regression(X, Y, True)
    print "Beta OLS (with intercept %s):"%ols[0], ols[1], "(%.3f)"%matrix_distance(ols[1],beta)

    print
    if K==1:
        X = X.getflat()
    scipy = stats.linregress(X, Y)    
    print "Beta SciPy (with intercept %s):"%scipy[1],
    print scipy[0], "(%.3f)"%matrix_distance(array(scipy[0]),beta)

    if plot:
        import pylab
        pylab.hold(True)
        pylab.scatter(X, Y)
        xlims = pylab.xlim()
        pylab.plot([0, xlims[1]], [0, olsR[1][0]*xlims[1]], label="No intercept")

        intercept = ols[0][0]
        line_end  = intercept + ols[1][0]*xlims[1]
        pylab.plot([0, xlims[1]], [intercept, line_end], label="With intercept")

        intercept = scipy[1]
        line_end  = intercept + scipy[0]*xlims[1]
        pylab.plot([0, xlims[1]], [intercept, line_end], label="SciPy")

        pylab.legend(loc=0)
        pylab.show()

    return X, Y, olsR, ols, scipy

if __name__ == "__main__":        
    from numarray.random_array import seed, random, normal
    seed(02, 25)
    
    # Setting the problem
    print "T=10, K=1, (a, b)=(50, 1)"
    test_ols_regression(10, 1, 50.0, plot=True)

    print
    print "T=100, K=1, (a, b)=(25, 1)"
    test_ols_regression(100, 1, 25.0, plot=True)

    print
    print "T=100, K=3 (a, b)=(25, 1)"
    try:
        test_ols_regression(100, 3, 25.0)
    except:
        print "SciPy FAILS!"

    # # Performing OLS
    # Xt = transpose(X)
    # XtX = mmult(Xt, X)
    # XtY = mmult(Xt, y)
    # beta_ols = mmult( inverse(XtX), XtY)
    # print "Distance beta_0 and beta_ols:", matrix_distance(beta, beta_ols)
    # 
    # gmm = LinearGMM(y, X, X)
    # print "Distance beta_ols and beta_IV:", matrix_distance(beta_ols, gmm.beta_iv)
    # print "Distance beta_ols and beta_GMM:", matrix_distance(beta_ols, gmm.beta)
    # 
    # print "GMM residuals:"
    # print gmm.uhat
