from math import *
from numarray import asarray, choose, zeros, shape, ufunc

def sign(m):
    m = asarray(m)
    return zeros(shape(m))-ufunc.less(m,0)+ufunc.greater(m,0)

def fast_softmax( x ):
    s = 0
    res = []
    for r in x:
        e = exp(r)
        s += e
        res.append( e )

    return [ r/s for r in res ]

def floats_are_equal(a, b, numtol=1e-6):
    # print "Comparing floats ",a,b,numtol
    minabs = min(abs(a),abs(b))
    if minabs<1.0:
        return abs(a-b) <= numtol
    return abs(a-b) <= numtol*minabs

def isNaN(f):
    """Return True where f contains NaN values"""
    from numarray import ieeespecial
    return ieeespecial.mask(f, ieeespecial.NAN)

def isNotNaN(f):
    """Return False where f contains NaN values"""
    return ufunc.equal(isNaN(f), 0.0)

def replace_nans(a, with=0.0):
    return choose(isNotNaN(a), (with, a))


if __name__ == '__main__':
    print fast_softmax([ 0, 0, 100 ])
    print
    print isNaN(float('NaN'))
    print isNaN(2.0)
    print isNaN([1.0, float('NaN'), 3.0])
    print isNaN([1.0, 2.0, 3.0])
    print
    
    a = [1.0, float('NaN'), 3.0, float('NaN')]
    print a
    print replace_nans(a)
