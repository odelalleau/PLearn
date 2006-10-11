from numarray import *
from numarray.random_array import normal

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
    if epsilon is None:
        epsilon = normal(0, 1, (T,))
    elif T is None:
        raise ValueError("You must provide T or epsilon.")

    if T is None:        
        T = shape(epsilon)[0]
    else:
        assert T == shape(epsilon)[0], "T and shape(epsilon) are incoherent."

    u = array(type=Float32, shape=(T,))
    for t in range(T):
        u[t] = epsilon[t]

        for k in range(1, min(t, len(ma_weights))+1):
            alpha_k = ma_weights[k-1] # offset by 1 since lists start at 0 in Python 
            u[t] += alpha_k * epsilon[t-k]

        for p in range(1, min(t, P)+1):
            u[t] += RHO**p * u[t-p]

    return u

def autoregressive(P, RHO, T=None, epsilon=None):
    """Returns an AR(P) process.

    Returns $X$ such that, for $t \in {0, \ldots, T-1}$, 

    \[
        X_t = \sum_{k=1}^P \rho_k X_{t-k} + \epsilon_t 
    \]

    where $\rho_k = \rho^k$ (for now). Of course, the summation is
    truncated whenever $t < P$.
    """
    return arma(P, RHO, T=T, epsilon=epsilon,
                ma_weights=[] ) # Disabling MA                
            
def moving_average(ma_weights, T=None, epsilon=None):
    """Returns a MA(Q) process (Q <- len(ma_weights)).

    Returns $X$ such that, for $t \in {0, \ldots, T-1}$, 

    \[
        X_t = \epsilon_t + \sum_{k=1}^Q \alpha_k \epsilon_{t-k}
    \]

    where $Q = len(ma_length)$ and $\alpha_k = ma_weights[k-1]$. Of course,
    the summation is truncated whenever $t < Q$.
    """
    return arma(-1, float('NaN'), # Disabling AR 
                ma_weights, T, epsilon=epsilon)


#####  Builtin Tests  #######################################################

if __name__ == "__main__":
    import matplotlib
    from pylab import *

    T = 1000
    x_range = range(0, T)
    plot(x_range, moving_average([0.25], T))
    plot(x_range, autoregressive(1, 0.85, T))
    plot(x_range, arma(1, 0.85, [0.25], T))
    show()
