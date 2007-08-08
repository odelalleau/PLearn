## Automatically adapted for numpy.numarray Jun 13, 2007 by python_numarray_to_numpy (-xsm)

#!/usr/bin/env python
from math import ceil, floor
from numpy.numarray import *
from numpy.numarray.random_array import normal

#####  Utility Functions  ###################################################
#####    Instanciate the process' class and extract the process          ####
#####    array from it. Implemented only for the discreet processes      ####

def arma(P, RHO, ma_weights, T=None, epsilon=None):
    """Returns an ARMA(P, Q) process. (Q <- len(ma_weights))

    Returns $X$ such that, for $t \in {0, \ldots, T-1}$, 

    \[
        X_t = \sum_{k=1}^P \rho_k X_{t-k} +
                \sum_{k=1}^Q \alpha_k \epsilon_{t-k} + \epsilon_t 
    \]

    where $\rho_k = \rho^k$ (for now) and $\alpha_k = ma_weights[k-1]$. Of
    course, the summation is truncated whenever $t < P$.
    """
    return ARMA(P, RHO, ma_weights, T, epsilon).x

def autoregressive(P, RHO, T=None, epsilon=None):
    """Returns an AR(P) process.

    Returns $X$ such that, for $t \in {0, \ldots, T-1}$, 

    \[
        X_t = \sum_{k=1}^P \rho_k X_{t-k} + \epsilon_t 
    \]

    where $\rho_k = \rho^k$ (for now). Of course, the summation is
    truncated whenever $t < P$.
    """
    return AutoRegressive(P, RHO, T, epsilon).x

def moving_average(ma_weights, T=None, epsilon=None):
    """Returns a MA(Q) process (Q <- len(ma_weights)).

    Returns $X$ such that, for $t \in {0, \ldots, T-1}$, 

    \[
        X_t = \epsilon_t + \sum_{k=1}^Q \alpha_k \epsilon_{t-k}
    \]

    where $Q = len(ma_length)$ and $\alpha_k = ma_weights[k-1]$. Of course,
    the summation is truncated whenever $t < Q$.
    """
    return MovingAverage(ma_weights, T, epsilon).x


#####  Random Processes  ####################################################

class ARMA:
    """Generates and represents the evoluation of a ARMA(P, Q) through time.

    Returns $X$ such that, for $t \in {0, \ldots, T-1}$, 

    \[
        X_t = \sum_{k=1}^P \rho_k X_{t-k} +
                \sum_{k=1}^Q \alpha_k \epsilon_{t-k} + \epsilon_t 
    \]

    where $\rho_k = \rho^k$ (for now) and $\alpha_k = ma_weights[k-1]$ and
    $epsilon$ denotes innovations.  Hence, $Q = len(ma_weights)$. Of
    course, the summations are truncated whenever $t < P$ or $t < Q$.

    Use the getitem protocol to access the value of the ARMA process at
    time $t$, i.e.

        a = ARMA(1, 0.94, T=100)
        print a[97]               # Will print the value at t=97

    Note that iteration on an instance of this class is equivalent to
    iterating the internal 'x' array containing the generated process.
    """
    def __init__(self, P, RHO, ma_weights, T=None, epsilon=None):        
        # If not provided, epsilon is an array of N(0, 1).
        if epsilon is None:
            epsilon = normal(0, 1, (T,))
        elif T is None:
            raise ValueError("You must provide T or epsilon.")
        self.epsilon = epsilon
        
        # If not provided, T will be len(epsilon)
        if T is None:
            T = shape(epsilon)[0]
        else:
            assert T == shape(epsilon)[0], "T and shape(epsilon) are incoherent."
        self.T = T

        # Generating the ARMA process
        Q = len(ma_weights)
        X = array(typecode=Float32, shape=(T,))
        for t in range(T):
            X[t] = epsilon[t]
        
            for k in range(1, min(t, Q)+1):
                alpha_k = ma_weights[k-1] # offset by 1 since lists start at 0 in Python 
                X[t] += alpha_k * epsilon[t-k]
        
            for p in range(1, min(t, P)+1):
                X[t] += RHO**p * X[t-p]
        
        self.x = X
        self.p = P
        self.q = Q
        self.rho = RHO 
        self.ma_weights = ma_weights

    def __getitem__(self, t):
        return self.x[t]

    def __iter__(self):
        return iter(self.x)

class AutoRegressive(ARMA):
    def __init__(self, P, RHO, T=None, epsilon=None):
        ARMA.__init__(self, P, RHO, [], T, epsilon)

class MovingAverage(ARMA):
    def __init__(self, ma_weights, T=None, epsilon=None):
        ARMA.__init__(self, 0, float('NaN'), ma_weights, T, epsilon)
        
class BrownianMotion:
    """Generates and represents the evoluation of a Brownian motion through time.

    Use the call protocol to access the value of the Brownian motion at
    time $t$, i.e.

        b = BrownianMotion(100)
        print  b(97.22)         # Will print the value at t=97.22

    NOTE: Back-check this code with Numerical Methods!!!
    """
    def __init__(self, T, n=1000):
        self.T = T
        self.n = n
        self.n_inverse      = 1.0/n
        self.increments     = normal(0, self.n_inverse, (n*T,))
        self.discretization = cumsum(self.increments)

    def __call__(self, t):
        assert t >= 0 and t <= self.T, "Invalid t=%d"%t
        if t==T:
            return self.discretization[-1]        

        nt = self.n * t
        floor_nt = int(floor(nt))
        frac_nt  = nt - floor_nt         # between 0 and 1        
        if frac_nt < self.n_inverse:     # nt is an (almost) integer...
            return self.discretization[floor_nt]

        ceil_nt = int(ceil(nt))
        return ( (1-frac_nt) * self.discretization[floor_nt]
                 + frac_nt   * self.discretization[ceil_nt] )

    def getIncrements(self, partition=[]):
        if not partition:
            return self.increments
        
        assert partition[0]==0 and partition[-1]==self.T
        increments = array(typecode=Float32, shape=(len(partition)-1,))
        for i,t1 in enumerate(partition[:-1]):
            t2 = partition[i+1]
            assert t2 > t1, "t2 = %d <= %d = t1"%(t2, t1)
            increments[i] = self(t2)-self(t1)

        return increments

#####  Builtin Tests  #######################################################

if __name__ == "__main__":
    from plearn.graphical.tools import *

    # ARMA and subclasses
    if False:
        T = 1000
        x_range = range(0, T)

        getNewFigure()
        pylab.plot(x_range, moving_average([0.25], T))
        
        getNewFigure()
        pylab.plot(x_range, autoregressive(1, 0.85, T))

        getNewFigure()
        pylab.plot(x_range, arma(1, 0.85, [0.25], T))

        pylab.show()

    # Brownian Motion
    if True:
        T = 100
        x_range = [ 0.01*d for d in range(0, 100*T) ]
        x_range.append(T) # To test that it works...

        b = None
        getNewFigure()
        for i in range(4):
            b = BrownianMotion(T)    
            pylab.plot(x_range, [ b(t) for t in x_range ])        

        getNewFigure()
        increments = b.getIncrements(x_range)
        pylab.plot(x_range[:-1], increments)

        getNewFigure()
        pylab.hist(increments, bins=100)

        pylab.show()
        
