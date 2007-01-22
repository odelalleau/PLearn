# cvx_utils.py
# Copyright (C) 2007 Christian Dorion
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
"""Provides functions to use CVX transparently where NumPy or NumArray is the standard."""
import numpy
import numarray 
from cvxopt import base as cvx


#####  NumArray vs CVX  #####################################################

def numarray_asmatrix(a):
    if len(a.shape) == 1:
        return numarray.array([ a ]) # One row matrix
    elif len(a.shape) == 2:
        return a
    raise ValueError, a

def numarray_to_cvx(*args):
    """Returns CVX matrices from NumArray matrices

    Note that NumArray one-dimensional arrays are transformed as row vectors
    in CVX. See/Run test_numarray_cvx_conversions() for details.
    """
    matrices = []
    for a in args:
        a    = numarray_asmatrix(a)
        m, n = a.shape
        a    = numarray.ravel( numarray.transpose(a) )
        matrices.append( cvx.matrix(a, (m,n), 'd') )

    if len(matrices)==1:
        return matrices[0]
    return matrices

def cvx_to_numarray(*args):
    """Returns NumArray matrices from CVX matrices

    Note that CVX vectors are returns as one-dimensional NumArray
    matrices. See/Run test_numarray_cvx_conversions() for details.
    """
    matrices = []
    for a in args:
        matrices.append( numarray.array(a) )

    if len(matrices)==1:
        return matrices[0]
    return matrices


#####  NumPy vs CVX  ########################################################

def numpy_to_cvx(*args):
    """Returns CVX matrices from NumPy matrices

    Note that NumPy one-dimensional arrays are transformed as row vectors
    in CVX. See/Run test_numpy_cvx_conversions() for details.
    """
    matrices = []
    for a in args:
        a    = numpy.asmatrix(a)
        m, n = a.shape
        a    = numpy.ravel(a.transpose())
        matrices.append( cvx.matrix(a, (m,n), 'd') )

    if len(matrices)==1:
        return matrices[0]
    return matrices

def cvx_to_numpy(*args):
    """Returns NumPy matrices from CVX matrices

    Note that CVX vectors are returns as one-dimensional NumPy
    matrices. See/Run test_numpy_cvx_conversions() for details.
    """
    matrices = []
    for a in args:
        matrices.append( numpy.array(a) )

    if len(matrices)==1:
        return matrices[0]
    return matrices


#####  Numerical Derivatives  ###############################################

def numerical_gradient(x_k, func, epsilon, *args):
    n_params, one = x_k.size
    assert one==1
    
    ei = cvx.matrix(0.0, x_k.size)
    grad = cvx.matrix(0.0, x_k.size)
    for i in range(n_params):
        ei[i]   = epsilon
        grad[i] = ( func(x_k+ei, *args) - func(x_k-ei, *args) ) / (2*epsilon)
        ei[i]   = 0.0
    return grad.T

def numerical_hessian(x_k, f_k, func, epsilon, *args):
    n_params, one = x_k.size
    assert one==1
    H = cvx.matrix(0.0, (n_params,n_params))
    epsilon2 = epsilon**2
    
    ei = cvx.matrix(0.0, x_k.size)
    esame = cvx.matrix(0.0, x_k.size)
    ecross = cvx.matrix(0.0, x_k.size)
    for i in range(n_params):
        esame[i], ecross[i] = +epsilon, +epsilon
        for j in range(i):            
            esame[j], ecross[j] = +epsilon, -epsilon

            lhs = func(x_k+esame, *args) - func(x_k+ecross, *args)
            rhs = func(x_k-ecross, *args) - func(x_k-esame, *args)            
            H[i,j] = (lhs - rhs) / (4*epsilon2)
            H[j,i] = H[i,j]

            esame[j], ecross[j] = 0.0, 0.0
        esame[i], ecross[i] = 0.0, 0.0
                
        ei[i] = epsilon        
        H[i,i] = (func(x_k+ei, *args) + func(x_k-ei, *args) - 2*f_k) / epsilon2        
        ei[i] = 0.0        

    return H

def use_numerical_derivatives_nc(objective, starting_values, epsilon):
    """NC stands for 'no nonlinear constraints'"""
    def _using_numerical_derivatives(params=None, z=None):
        # Access pattern for starting values
        if params is None:
            return 0, starting_values

        # If objective returns None, this function shall return
        OBJ = objective(params)
        if OBJ is None: return None
        
        # Compute numerical gradient
        OBJ_D = numerical_gradient(params, objective, epsilon)
        if z is None:
            return cvx.matrix(OBJ, (1,1)), OBJ_D

        # Compute the numerical Hessian
        assert z.size==(1,1)
        OBJ_H = z[0,0]*numerical_hessian(params, OBJ, objective, epsilon)
        return cvx.matrix(OBJ, (1,1)), OBJ_D, OBJ_H

    def using_numerical_derivatives(params=None, z=None):
        out = _using_numerical_derivatives(params, z)
        print "OUT",
        for o in out:
            print o
        if len(out)==3:
            from cvxopt.lapack import syev
            W = cvx.matrix(0.0, (6,1), 'd')
            syev(out[-1], W)
            print "EIGEN", W
        return out
        
    return using_numerical_derivatives


#####  Builtin Tests  #######################################################

def test_numarray_cvx_conversions():
    _1_10_ = numarray.array( range(1,11) )
    test_cases = [
        _1_10_,
        numarray.array([ _1_10_ ]),
        numarray.transpose( numarray.array([ _1_10_ ]) ),        
        numarray.array([ _1_10_, range(11, 21) ])
        ]

    for t, a in enumerate(test_cases):
        print "-"*80, '\nTest Case %d\n'%(t+1), "-"*80        
        print "NumArray matrix shaped", a.shape
        print a
        
        a_cvx = numarray_to_cvx(a)
        print "CVX matrix shaped", a_cvx.size
        print a_cvx

        a = numarray_asmatrix(a)
        a_numarray = cvx_to_numarray(a_cvx)
        m, n = a.shape
        assert (m,n)==a_cvx.size, \
               "(m,n) = (%d,%d) != %s = a_cvx.size"%(m, n, a_cvx.size)
        assert (m,n)==a_numarray.shape, \
               "(m,n) = (%d,%d) != %s = a_numarray.size"%(m, n, a_numarray.shape)

        for i in range(m):
            for j in range(n):
                assert a[i,j] == a_cvx[i,j], "a[%d,%d] = %f != %f = a_cvx[%d,%d]"%(
                    i, j, a[i,j], a_cvx[i,j], i, j )
                assert a[i,j] == a_numarray[i,j], "a[%d,%d] = %f != %f = a_numarray[%d,%d]"%(
                    i, j, a[i,j], a_numarray[i,j], i, j )
        print

def test_numpy_cvx_conversions():
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
    test_numarray_cvx_conversions()
