# arrays.py
# Copyright (C) 2005,2006 Christian Dorion
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
from math import *
from numarray import *
from numarray.linear_algebra import inverse as __numarray_inverse
from numarray.linear_algebra import determinant

def _2D_shape(M):
    if len(M.shape)==1:
        return M.shape[0], 1
    elif len(M.shape)==2:
        return M.shape
    else:
        raise ValueError(
            "Matrices are expected to be uni- or bidimensional. "
            "Current matrices is shaped %s" % shape(M) )

def _as_matrix(*matrices):
    matrices = list(matrices)
    for m, M in enumerate(matrices):        
        if len(M.shape)==1:
            matrices[m] = transpose( array([ M ]) )
        elif len(M.shape)==2:
            pass
        else:
            raise ValueError(
                "Matrices are expected to be uni- or bidimensional. "
                "Current matrices is shaped %s" % shape(M) )
    return matrices
    
def inverse(a):
    """Built over numarray's inverse() function while handling scalars."""
    if isinstance(a, (int, long, float, complex)) or a.shape == (1,):
        return 1.0/a
    return __numarray_inverse(a)

# __numarray_transpose = transpose
# def transpose(a):
#     print a.shape
#     if len(a.shape) == 1:
#         return reshape(a, (1,a.shape[0]))
#     return __numarray_transpose(a)

def kronecker(m1, m2):    
    M1, M2 = _as_matrix(m1, m2) 
    l1, w1 = shape(M1)
    l2, w2 = shape(M2)
    K = array(shape=(l1*l2, w1*w2), type=Float64)
    for i in range(l1):
        for j in range(w1):
            K[i*l2:(i+1)*l2, j*w2:(j+1)*w2] = M1[i,j] * m2

    if w1==1 and w2==1:
        K = K.getflat()
    return K

def lag(series, k=1, fill=(lambda shape : array([]))):
    """Returns a lagged version of 'series'.

    Default behavior is to return an array with abs(k) elements less than
    'series'. This behavior can be modified by providing a user-defined
    'fill' function which accepts a shape tuple as single argument. To
    have, for instance, last 'k' elements equal to 0 (read first 'k'
    elements if 'k' is negative), 'fill' could be the zeros() function from
    numarray.
    """
    assert len(shape(series)) == 1, "Manages row vectors only"
    if k > 0:
        return concatenate([series[k:], fill((k,))])
    elif k < 0:
        return concatenate([fill((-k,)), series[:k]])
    else:
        return series

def matrix_distance(m1, m2):
    return maximum.reduce( fabs(ravel(m1-m2)) )

def matrix2vector(mat):
    shp = shape(mat)
    assert len(shp)==2
    length = shp[0]*shp[1]

    mat.setshape((length,))
    return shp

def mmult(*args):
    """Shorthand for matrix multiplication

    Numarray's 'matrixmultiply' function takes only two matrices at a time,
    which can make statements quite heavy when multiplying many matrices. This
    function accepts as many matrices as one wants, processing multiplication
    'from left to right'.
    """
    #mmult_shapes(*args)
    return reduce(matrixmultiply, args)

def mmult_shapes(*args):
    """For debugging purposes; print shapes."""
    for a in args:
        print shape(a),
    print

def rrange(start, stop, step, include_stop=False):
    """Behaves like the builtin range() function but with real arguments.

    Note that since 'step' is here mandatory since there is no obvious
    default choice. The 'include_stop' is False, so that the default
    behaviour is similar to all Python range* functions, but can be set
    True for convenience.
    """
    npoints = int((stop-start) / step)
    if include_stop:
        npoints += 1
    return [ start+step*d for d in range(npoints) ]

def grid_around(center, width, step):        
    start = center-(width/2.0)
    npoints = int(width/step) + 1
    return array([ start + step*d for d in range(npoints) ])
        
def sign(m):
    m = asarray(m)
    return zeros(shape(m))-ufunc.less(m,0)+ufunc.greater(m,0)

def symmetric(m, precision=1e-6, rel_precision=1e-6):
    return (asymmetry(m, precision, rel_precision) is None)
    
def asymmetry(m, precision=1e-6, rel_precision=1e-6):
    n, nn = shape(m)
    if n != nn:
        return n, nn
    for i in range(n):
        for j in range(i+1, n):
            if fabs(m[i,j]-m[j,i]) > max(precision, rel_precision*m[i,j]):
                return i, j
    return None

def assert_symmetric(m, callback=None, precision=1e-6, rel_precision=1e-6):
    asym = asymmetry(m, precision, rel_precision)
    if asym is not None:
        i, j = asym
        if callback: callback()
        raise AssertionError("Matrix is not symmetric (%d, %d):\n%s"%(i,j,m))

def to_diagonal(a):
    """Returns a diagonal matrix with elements in 'a' on the diagonal."""
    assert len(a.shape)==1
    n = len(a)
    A = zeros(shape=(n,n), type=a.type())
    for i in range(n):
        A[i,i] = a[i]
    return A

def fast_softmax( x ):
    s = 0
    res = []
    for r in x:
        e = exp(r)
        s += e
        res.append( e )

    return [ r/s for r in res ]

def hasNaN(f):
    f = ravel(f)
    f = choose(isNaN(f), (0, 1))
    return sum(f)
    
def isNaN(f):
    """Return 1 where f contains NaN values, 0 elsewhere."""
    from numarray import ieeespecial
    return ieeespecial.mask(f, ieeespecial.NAN)

def isNotNaN(f):
    """Return 0 where f contains NaN values, 1 elsewhere."""
    return ufunc.equal(isNaN(f), 0.0)

def replace_nans(a, with=0.0):
    return choose(isNotNaN(a), (with, a))

def average(x, axis=0):
    arrx = array(x)
    return sum(arrx,axis) / arrx.shape[axis]

def any(x):
    return logical_or.reduce(ravel(x))

def all(x):
    return logical_and.reduce(ravel(x))

if __name__ == '__main__':
    a = array(range(10))
    print "lag(%s): %s"%(a, lag(a))
    print 
    
    print fast_softmax([ 0, 0, 100 ])
    print
    print isNaN(float('NaN'))
    print isNaN(2.0)
    print isNaN([1.0, float('NaN'), 3.0])
    print hasNaN([1.0, float('NaN'), 3.0])
    print isNaN([1.0, 2.0, 3.0])
    print hasNaN([1.0, 2.0, 3.0])
    print

    print "2d"
    print hasNaN([[1.0, 2.0, 3.0],
                  [1.0, 2.0, 3.0]])
    print hasNaN([[1.0, 2.0,          3.0],
                  [1.0, float('NaN'), 3.0]])
    print hasNaN([[1.0, float('NaN'), 3.0],
                  [1.0, 2.0,          3.0]])
    print

    
    a = [1.0, float('NaN'), 3.0, float('NaN')]
    print a
    print hasNaN(a)
    print replace_nans(a)

    print 
    print "Kronecker Product"
    print kronecker(array([1, 0]), identity(3))
    print
    print kronecker(array([[1, 2],[3, 4]]), identity(3))

    print
    print "matrix2vector"
    a = array([[1,2,3], [4,5,6]])
    print a
    a_shape = matrix2vector(a)
    print a
    a.setshape(a_shape)
    print a
    
    
    
