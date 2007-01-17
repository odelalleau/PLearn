"""Provides functions to use CVX transparently where NumPy is the standard."""
import numpy
from cvxopt.base import matrix, spmatrix, log

def numpy_to_cvx(*args):
    """Returns CVX matrices from NumPy matrices

    Note that NumPy one-dimensional arrays are transformed as row vectors
    in CVX. See/Run test_matrix_conversions() for details.
    """
    matrices = []
    for a in args:
        a    = numpy.asmatrix(a)
        m, n = a.shape
        a    = numpy.ravel(a.transpose())
        matrices.append( matrix(a, (m,n), 'd') )

    if len(matrices)==1:
        return matrices[0]
    return matrices

def cvx_to_numpy(*args):
    """Returns NumPy matrices from CVX matrices

    Note that CVX vectors are returns as one-dimensional NumPy
    matrices. See/Run test_matrix_conversions() for details.
    """
    matrices = []
    for a in args:
        matrices.append( numpy.array(a) )

    if len(matrices)==1:
        return matrices[0]
    return matrices


#####  Builtin Tests  #######################################################

def test_matrix_conversions():
    _1_10_ = numpy.array( range(1,11) ) 
    test_cases = [
        _1_10_,
        numpy.array([ _1_10_ ]),
        numpy.matrix(_1_10_).transpose(),        
        numpy.array([ _1_10_, range(11, 21) ])
        ]

    for t, a in enumerate(test_cases):
        print "-"*80, '\nTest Case %d\n'%(t+1), "-"*80        
        print "NumPy matrix shaped", a.shape
        print a
        
        a_cvx = numpy_to_cvx(a)
        print "CVX matrix shaped", a_cvx.size
        print a_cvx

        a = numpy.asmatrix(a)
        a_numpy = cvx_to_numpy(a_cvx)
        m, n = a.shape
        assert (m,n)==a_cvx.size, \
               "(m,n) = (%d,%d) != %s = a_cvx.size"%(m, n, a_cvx.size)
        assert (m,n)==a_numpy.shape, \
               "(m,n) = (%d,%d) != %s = a_numpy.size"%(m, n, a_numpy.shape)

        for i in range(m):
            for j in range(n):
                assert a[i,j] == a_cvx[i,j], "a[%d,%d] = %f != %f = a_cvx[%d,%d]"%(
                    i, j, a[i,j], a_cvx[i,j], i, j )
                assert a[i,j] == a_numpy[i,j], "a[%d,%d] = %f != %f = a_numpy[%d,%d]"%(
                    i, j, a[i,j], a_numpy[i,j], i, j )
        print

if __name__ == "__main__":
    test_matrix_conversions()
