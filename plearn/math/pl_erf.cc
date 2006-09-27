// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
//

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
//  1. Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
// 
//  2. Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
// 
//  3. The name of the authors may not be used to endorse or promote
//     products derived from this software without specific prior written
//     permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
// NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// This file is part of the PLearn library. For more information on the PLearn
// library, go to the PLearn Web site at www.plearn.org

#include <plearn/base/general.h>
// #include <iostream>
// #include <cmath>

namespace PLearn {
using namespace std;

#define ITMAX 150
#define EPS 3.0e-7
#define FPMIN 1.0e-30
#define Pi 3.141592653589793
#define Log2Pi 1.837877066409
#define Sqrt2Pi 2.506628274631

static double pl_gammln_cof[7]={ 1.000000000190015     ,
				 76.18009172947146     ,
				 -86.50532032941677     ,
				 24.01409824083091     ,
				 -1.231739572450155    ,
				 0.1208650973866179e-2,
				 -0.5395239384953e-5   };

// function gamma returns log(Gamma(z)), where
// Gamma(z) = \int_0^infty t^{z-1}*e^{-t} dt
real pl_gammln(real z)
{
    double gz,tmp;
    static double gamma = 5.0;
    gz = (z+0.5)*pl_log(z+gamma+0.5);
    gz -= z+gamma+0.5;
    gz += 0.5*Log2Pi;
    tmp = pl_gammln_cof[0];
    for(int i=1;i<7;i++) tmp += pl_gammln_cof[i]/(z+i);
    gz += pl_log(tmp/z);
    return(gz);
}

// returns d(pl_gammln(z))/dz also known as the digamma function
real pl_dgammlndz(real z)
{
    real tmp0= pl_gammln_cof[0],
        tmp1= 0.0;
    for(int i= 1; i<7; ++i)
    {
        tmp0+= pl_gammln_cof[i]/(z+i);
        tmp1-= pl_gammln_cof[i]/((z+i)*(z+i));
    }
    return (0.5+z)/(5.5+z)-1 + z*(-tmp0/(z*z) + tmp1/z)/tmp0 + pl_log(5.5+z);
}


// returns the series value of
// the incomplete gamma function
real pl_gser(real a, real x) {
    real EPSILON = 1e-7;
    real g = pl_gammln(a);  
    real sum,term;
    if (x<0 || a<0)
        PLERROR("Error in function pl_gser. Bad argument.");
    else if (fast_exact_is_equal(x, 0)) 
        return 0;

    sum = term = 1/a;  
    for(int i=1;i<ITMAX;i++) {
        term *= x/(a+i);
        sum += term;
        if (term < sum*EPSILON) break;
    }
    return exp(-x+a*pl_log(x)-g)*sum;
}


// returns the continued fraction representation of
// the incomplete gamma function
real  pl_gcf(real a, real x)
{
    assert( !is_missing(a) && !is_missing(x) );
  
    int i;
    real an,b,c,d,del,h;

    real gln=pl_gammln(a);
    b=x+1.0-a;
    c=1.0/FPMIN;
    d=1.0/b;
    h=d;
    for (i=1;i<=ITMAX;i++) {
        an = -i*(i-a);
        b += 2.0;
        d=an*d+b;
        if (fabs(d) < FPMIN) d=FPMIN;
        c=b+an/c;
        if (fabs(c) < FPMIN) c=FPMIN;
        d=1.0/d;
        del=d*c;
        h *= del;
        if (fabs(del-1.0) < EPS) break;
    }
    if (i > ITMAX) {
        PLWARNING("\"a\" is too large, ITMAX too small in "
                  "calling pl_gcf(%f,%f)", a,x);
    }
    return exp(-x+a*pl_log(x)-(gln))*h;
}


// returns the incomplete gamma function Q(a,x) = 1 - P(a,x)
// it either uses the series or the continued fraction formula
real pl_gammq(real a, real x) {
    if (x<0 || a<0)
        PLERROR("Error in function gammax. Bad arguments.");
    if (x<a+1) 
        return 1-pl_gser(a,x);
    return pl_gcf(a,x);
}


// returns the error function "erf"
real pl_erf(real x) {
    return (x<0?-1:1)*(1-pl_gammq(0.5,x*x));
}


//returns the gaussian cumulative function
// For X ~ Normal(0,1), cumulative probability function P(X<x)
real gauss_01_cum(real x) {
    return 0.5*(1+pl_erf(x/1.414214));
}

// For X ~ Normal(0,1), inverse of cumulative probability function P(X<x)
// i.e. approximately gauss_01_quantile(gauss_01_cum(x)) ~=~ x
// (the inverse is computed with a binary search, the bisection method)
real gauss_01_quantile(real q) {
    // first find a reasonable interval (a,b) s.t. cum(a)<q<cum(b)
    real a=-2;
    real b=2;
    real cum_a=gauss_01_cum(a);
    real cum_b=gauss_01_cum(b);
    while (cum_a>q) { a*=1.5; cum_a=gauss_01_cum(a); }
    while (cum_b<q) { b*=1.5; cum_b=gauss_01_cum(b); }
    // then start the bisection loop itself
    for (;;) {
        real c=0.5*(a+b);
        real precision = fabs(b-a);
        // PRECISION HERE:
        if (precision < 1e-6) 
            return c;
        real cum_c = gauss_01_cum(c);
        if (cum_c < q)
            a=c;
        else
            b=c;
    }
}


// for X ~ Normal(0,1), return density of X at x
real gauss_01_density(real x)
{
    return exp(-0.5*x*x) / Sqrt2Pi;
}

real gauss_01_log_density(real x)
{
    return -0.5*x*x - 0.5*Log2Pi;
}

real gauss_log_density_var(real x, real mu, real var)
{
    real dx=x-mu;
    return -0.5*(dx*dx/var + Log2Pi + pl_log(var));

}

real gauss_density_var(real x, real mu, real var) {
    real dx = x - mu;
    return exp(-0.5 * dx * dx / var) / Sqrt2Pi;
}

real gauss_log_density_stddev(real x, real mu, real sigma)
{
    real dx = (x-mu) / sigma;
    return -0.5*(dx*dx + Log2Pi) - pl_log(sigma);
}

real p_value(real mu, real vn)
{
    if (is_missing(mu) || is_missing(vn))
        return MISSING_VALUE;
    return 1 - gauss_01_cum(fabs(mu/sqrt(vn)));
}


} // end of namespace PLearn


/*
  Local Variables:
  mode:c++
  c-basic-offset:4
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:79
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=79 :
