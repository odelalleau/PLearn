# spline.py
# Copyright (C) 2008 Xavier Saint-Mleux
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

# Author: Xavier Saint-Mleux


from numpy import *


class Spline(object):
    """
    Cubic spline
        (adapted from Numerical Recipes in Fortran 77, 2nd ed.)
    """
    def __init__(self, x, y):
        self.calc_ypp(x, y)
        
    def calc_ypp(self, x, y):
        """
        find cubic spline coeffs.
        N.B. assumes slope of 0 at both ends
        """
        l= len(x)
        y2= zeros(l, dtype=float)
        u= zeros(l, dtype=float)
        for i in range(1,l-1):
            px, cx, nx= map(float,x[i-1:i+2])
            py, cy, ny= map(float,y[i-1:i+2])
            dx= cx-px
            dx2= nx-px
            sig= dx/dx2
            p= sig * y2[i-1] + 2.
            y2[i]= (sig-1.) / p
            u[i]= (6.*((ny-cy)/(nx-cx) - (cy-py)/dx) / dx2 - sig*u[i-1])/p
        for i in range(l-2,-1,-1):
            y2[i]= y2[i]*y2[i+1]+u[i]
        self.ox= x
        self.oy= y
        self.y2= y2

    def __call__(self, x):
        if isinstance(x, ndarray):
            return vectorize(self.call)(x)
        else:
            return self.call(x)

    def call(self, x):
        """
        calc. interpolated value from tabulated fn. + spline coeffs.
        """
        ox, oy, y2= self.ox, self.oy, self.y2
        x= float(x)
        l= len(ox)
        i0= 0
        i1= l-1
        while i1-i0 > 1:
            i= (i0+i1)/2
            if ox[i] > x:
                i1= i
            else:
                i0= i
        h= ox[i1]-ox[i0]
        a= (ox[i1]-x) / h
        b= (x-ox[i0]) / h
        y= a*oy[i0] + b*oy[i1] + ((a**3-a)*y2[i0] + (b**3-b)*y2[i1])*(h**2)/6.
        return y
