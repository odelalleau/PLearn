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


/* *******************************************************      
   * $Id: pl_math.h,v 1.14 2004/02/20 21:11:47 chrish42 Exp $
   * This file is part of the PLearn library.
   ******************************************************* */


/*! \file PLearnLibrary/PLearnCore/pl_math.h */

#ifndef pl_math_INC
#define pl_math_INC

#include <cmath>
#include <cfloat>
#include <climits>
#include "plerror.h"

namespace PLearn {
using namespace std;

#ifdef USEDOUBLE
#define real double
#endif

#ifdef USEFLOAT
#define real float
#endif

// it is not really the max, but it's large enough for most applications
#ifdef USEFLOAT
#define REAL_MAX FLT_MAX
#else
#define REAL_MAX DBL_MAX
#endif

union _plearn_nan_type { unsigned char c[4]; float d; };
extern _plearn_nan_type plearn_nan;

//!  Quiet NaN (float pattern)
#define MISSING_VALUE (plearn_nan.d)

  using namespace std;

  using std::log;
  using std::sqrt;
  using std::pow;
  using std::exp;
  using std::tanh;
  using std::abs;

  //! Deprecated, use std::min and std::max instead
#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

#define SIGN(a) ((a)>=0?1:-1)

  inline real sign(real a) { 
    if (a>0) return 1; 
    if (a<0) return -1; 
    return 0; 
  }
  inline real positive(real a) { if (a>0) return a; return 0; }
  inline real negative(real a) { if (a<0) return a; return 0; }

#define ABS(x) ((x)>0 ?(x) :-(x))
#define random() rand()

#define Pi 3.141592653589793
#define Log2Pi 1.837877066409
#define LOG_2 0.693147180559945
#define LOG_INIT -FLT_MAX
#define MINUS_LOG_THRESHOLD -18.42

#if defined(_MSC_VER) || defined(_MINGW_)
//!  drand48 does not exist in NT... because ANSI
//!  declared it obsolete in 1990 or so
#define drand48() ( rand()/ (double)(RAND_MAX+1) )
//!  Non-ansi, BSD function
#define log1p(a) log((real)1.0+(real)a)
#define rint(a) (int)(a+(real)0.5)
#define isnan(x) _isnan(x)
#define finite(x) _finite(x)
#endif

#if defined(DARWIN)
#define isnan(x) __isnan(x)
#endif

  template<class T>
  inline T square(const T& x)
  { return x*x; }

  // Wish I could get rid of this, but it's used a s some function pointer
  // in calls to some apply function in RandomVar. So I'd need to change
  // those calls first to sth more stl like.
  real square_f(real x);

  template<class T>
  inline T two(const T& x)
  { return x+x; } 

#define TANHTABLESIZE 5000
#define MAXTANHX 10.

  class PLMathInitializer
  {
  public:
    PLMathInitializer();
    ~PLMathInitializer();
  };

  /*
#define DOUBLE2INT(i,d) {maniax = ((d)+6755399441055744.0); i=*((int *)(&maniax));}

  inline int double2int(double d)
  {
    double maniax = d+6755399441055744.0;
    return *((int *)&maniax);
  }
  */

#ifdef LINUX
#define DOUBLE_TO_INT(in,out) __asm__ __volatile__ ("fistpl %0" : "=m" (out) : "t" (in) : "st")  
#else
#define DOUBLE_TO_INT(in, out)  out = int(in)
#endif

  extern float tanhtable[TANHTABLESIZE];
  extern PLMathInitializer pl_math_initializer;

  inline real fasttanh(const real& x)
  {
    if(x>0)
      {
        if(x>MAXTANHX)
          return real(tanhtable[TANHTABLESIZE-1]);
        else
          {
            int i;
            DOUBLE_TO_INT( double(x*((TANHTABLESIZE-1)/MAXTANHX)), i);
            return real(tanhtable[i]);
          }
      }
    else
      {
        real nx = -x;
        if(nx>MAXTANHX)
          return real(-tanhtable[TANHTABLESIZE-1]);
        else
          {
            int i;
            DOUBLE_TO_INT( double(nx*((TANHTABLESIZE-1)/MAXTANHX)), i);
            return real(-tanhtable[i]);
          }
      }
  }

  inline real fastsigmoid(const real& x)
  { return (real)0.5*(fasttanh(0.5*x)+1.); }


  // These are quadratic approximations to tanh and sigmoid 

  /*
inline real ultrafasttanh(const real& x)
{
  if(x>1.92033) return 0.96016;
  else if (x>0) return 0.96016 - 0.26037 * square(x - 1.92033);
  else if (x<=-1.92033) return -0.96016;
  else return 0.26037 * square(x + 1.92033) - 0.96016;
}

  inline real ultrafastsigmoid(const real& x)
  { return (real)0.5*(ultrafasttanh(0.5*x)+1.); }
*/

  inline real ultrafasttanh(const real& x)
  {
    if(x>=0)
      return (x<1.7 ? (1.5*x/(1+x)) : ( x<3 ? (0.935409070603099 + 0.0458812946797165*(x-1.7)) :0.99505475368673));
    else
      {
        real xx = -x;
        return -(xx<1.7 ? (1.5*xx/(1+xx)) : ( xx<3 ? (0.935409070603099 + 0.0458812946797165*(xx-1.7)) :0.99505475368673));
      }
  }

  /*
  inline real ultrafasttanh(const real& x)
  {
    return x/(1+fabs(x));
  }
  */

  inline real ultrafastsigmoid(const real& x)
  {
    //    return 0.5*x / (1. + fabs(x)) + 0.5;
    return (real)0.5*(ultrafasttanh(0.5*x)+1.);
    // return fastsigmoid(x);
  }

  //! Tells if the passed value means "missing" for its data-type.
  //! The default version of returns false (not a "missing value")
  template<class T>
  inline bool is_missing(const T& x) { return false; }

  //! Missing value for double and float are represented by NaN
  inline bool is_missing(double x) { return isnan(x)!=0; }

  //! Missing value for double and float are represented by NaN
  inline bool is_missing(float x) { return isnan(x)!=0; }
  
  inline bool is_integer(real x) { return real(int(x))==x; }

  inline real FABS(real x)
    { return x>=0. ?x :-x; }

#define FSWAP(a,b) do {real _c; _c = *(a); *(a) = *(b); *(b) = _c;} while(0)
#define FLOAT_THRESHOLD 0.00001
#define FEQUAL(a,b) (FABS((a)-(b))<FLOAT_THRESHOLD)


  inline real mypow(real x, real p)
    { return x==0 ?0 :pow(x,p); }

  inline real ipow(real x, int p)
    { 
      real result = 1.0;
      while(p--)
        result *= x;
      return result;
    }

//!  numerically stable version of sigmoid(x) = 1.0/(1.0+exp(-x))
  inline real sigmoid(real x)
    { return (real)0.5*(tanh(0.5*x)+1.); }

//!  "hard" version of the sigmoid, i.e. the indicator function that
//!  is 1 if its argument is STRICTLY positive, and 0 otherwise
  inline real is_positive(real x) { return x>0? 1 : 0; }

//!  numerically stable version of inverse_sigmoid(x) = log(x/(1-x))
  inline real inverse_sigmoid(real x)
    {
#ifdef BOUNDCHECK
      if (x < 0. || x > 1.)
        PLERROR("In inv_sigmoid_value: a should be in [0,1]");
#endif
      if (FEQUAL(x,0.))
        return -88.;
      else if (FEQUAL(x,1.))
        return 14.5;
      else
        return real(-log(1./x - 1.));
    }

//!  numerically stable computation of log(1+exp(x))
  inline real softplus(real x)
    { 
      if(x<=-30.)
        return 0.0;
      else if(x>=30.)
        return x;
      else
        return log1p(exp(x));
      //!  return 0.5*x + LOG_2 - log(1./cosh(0.5*x)); 
    }

//! inverse of softplus function
inline real inverse_softplus(real y)
{
  if (y<0) 
    return MISSING_VALUE;
  if (y>=30)
    return y;
  if (y==0)
    return -30;
  return log(exp(y)-1);
}

inline real hard_slope(real x, real left=0, real right=1)
{
  if (x<left) return 0;
  if (x>right) return 1;
  return (x-left)/(right-left);
}

// as smoothness-->infty this becomes the linear by part function that
// is 0 in [-infty,left], linear in [left,right], and 1 in [right,infty].
// For finite smoothness, it is a smoother function, always with value in the interval [0,1].
// It is always monotonically increasing wrt x (positive derivative in x).
inline real soft_slope(real x, real smoothness=1, real left=0, real right=1)
{
  if (smoothness==0)
    return 0.5;
  if (smoothness>1000)
    return hard_slope(x,left,right);
  return 1 + (softplus(-smoothness*(x-left))-softplus(-smoothness*(x-right)))/(smoothness*(right-left));
}

  
// This is the derivative of soft_slope with respect to x.
inline real d_soft_slope(real x, real smoothness=1, real left=0, real right=1)
{
  // note that d(softplus(z))/dz = sigmoid(z)
  return (-sigmoid(-smoothness*(x-left))+sigmoid(-smoothness*(x-right)))/(right-left);
}
  
//!  Return M choose N, i.e., M! / ( N! (M-N)! )
  inline int n_choose(int M,int N)
    {
      int k=M-N;
      float res=1;
      int i;
      for (i=1;i<=k;i++) {
        res *= (i+N)/(float)i;
      }
      return (int)(res+0.499);
    }

  real safeflog(real a);
  real safeexp(real a);

  real log(real base, real a);
  real logtwo(real a);
  real safeflog(real base, real a);
	real safeflog2(real a);
 
  typedef real (*tRealFunc)(real);
  typedef real (*tRealReadFunc)(real,real);

//!  compute log(exp(log_a)+exp(log_b)) without losing too much precision
  real logadd(real log_a, real log_b);

//!  compute log(exp(log_a)-exp(log_b)) without losing too much precision
  real logsub(real log_a, real log_b);

//! return the dilogarithm function dilogarithm(x)
//!   = sum_{i=1}^{\infty} x^i/i^2 = int_{z=x}^0 log(1-z)/z dz
//! It is also useful because -dilogarithm(-exp(x)) is the primitive of 
//! the softplus function log(1+exp(x)).
real dilogarithm(real x);

inline real softplus_primitive(real x) {
  return -dilogarithm(-exp(x));
}

real hard_slope_integral(real left=0, real right=1, real a=0, real b=1);


// integral of the soft_slope function between a and b
real soft_slope_integral(real smoothness=1, real left=0, real right=1, real a=0, real b=1);

} // end of namespace PLearn


#endif
