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
   * $Id: pl_math.cc,v 1.4 2003/11/28 21:55:24 yoshua Exp $
   * This file is part of the PLearn library.
   ******************************************************* */


/*! \file PLearnLibrary/PLearnCore/pl_io.cc */

#include "pl_math.h"

namespace PLearn <%
using namespace std;


# ifdef BIGENDIAN
_plearn_nan_type plearn_nan = { 0x7f, 0xc0, 0, 0 };
# endif
# ifdef LITTLEENDIAN
_plearn_nan_type plearn_nan = { 0, 0, 0xc0, 0x7f };
# endif

  float tanhtable[TANHTABLESIZE];

  PLMathInitializer::PLMathInitializer()
  {
    //! Fill the tanh table
    real scaling = MAXTANHX/(TANHTABLESIZE-1);
    for(int i=0; i<TANHTABLESIZE; i++)
      tanhtable[i] = (float) tanh(i*scaling);
  }
  
  PLMathInitializer::~PLMathInitializer() 
  {}
  
  PLMathInitializer pl_math_initializer;

real safeflog(real a)
{
  if (a < 0.0)
    PLERROR("safeflog: negative argument (%f)", a);
  if (a < 1e-25)
    return -57.5;
  else return (real)log((double)a);
}

real safeexp(real a)
{
#if USEDOUBLE
  if (a < -300) return 0;
  if (a > 300) return 1e38;
#else
  if (a < -87) return 0;
  if (a > 43) return 5e18;
#endif
  return exp(a);
}

real log(real base, real a)
{
  return log(a) / log(base);
}

real logtwo(real a)
{
  return log(a) / LOG_2;
}

real safeflog(real base, real a)
{
	return safeflog(a) / safeflog(base);
}

real safeflog2(real a)
{
	return safeflog(a) / LOG_2;
}

// compute log(exp(log_a)+exp(log_b)) without losing too much precision
real logadd(real log_a, real log_b)
{
  if (log_a < log_b)
    { // swap them
      real tmp = log_a;
      log_a = log_b;
      log_b = tmp;
    }
  real negative_absolute_difference = log_b - log_a;
  if (negative_absolute_difference < MINUS_LOG_THRESHOLD)
    return log_a;
  return log_a + log1p(exp(negative_absolute_difference));
}

real square_f(real x)
 { return x*x; }

// compute log(exp(log_a)-exp(log_b)) without losing too much precision
real  logsub(real log_a, real log_b)
{
  if (log_a < log_b)
    PLERROR("log_sub: log_a (%f) should be greater than log_b (%f)", log_a, log_b);
 
  real negative_absolute_difference = log_b - log_a;
 
  if (FEQUAL(log_a, log_b))
    return -FLT_MAX;
  else if (negative_absolute_difference < MINUS_LOG_THRESHOLD)
    return log_a;
  else
    return log_a + log1p(-exp(negative_absolute_difference));
}

real small_dilogarithm(real x)
{
  real somme = x;
  real prod = x;
  int i=2;
  for (;i<=999;i++)
  {
    real coef = (i-1.0)/i;
    prod *= x*coef*coef;
    somme += prod;
    if (fabs(prod/somme)<1e-16) break; // tolerance
  }
  if (i==1000) PLWARNING("dilogarithm: insufficient precision");
  return somme;
}

real positive_dilogarithm(real x)
{
  if (x<0.5)
    return small_dilogarithm(x);
  else if (x<1.0)
    return Pi*Pi/6.0 - small_dilogarithm(1.0-x) - log(x)*log(1-x);
  else if (x==1.0)
    return Pi*Pi/6.0;
  else if (x<=1.01)
  {
    real delta=x-1.0;
    real log_delta=log(delta);
    return Pi*Pi/6.0 + delta*(1-log_delta+delta*
                              ((2*log_delta-1)/4 + delta*
                               ((1-3*log_delta)/9 + delta*
                                ((4*log_delta-1)/16 + delta*
                                 ((1-5*log_delta)/25 + delta*
                                  ((6*log_delta-1)/36 + delta*
                                   ((1-7*log_delta)/49 + delta*
                                    (8*log_delta-1)/64)))))));
  }
  else if (x<=2.0)
  {
    real logx = log(x);
    return Pi*Pi/6.0 + small_dilogarithm(1.0-1.0/x) - logx*(0.5*logx+log(1-1/x));
  } else 
  {
    real logx = log(x);
    return Pi*Pi/3.0 - small_dilogarithm(1.0/x) - 0.5*logx*logx;
  }
}

real dilogarithm(real x)
{
  if (x<0)
    return -positive_dilogarithm(-x) + 0.5*positive_dilogarithm(x*x);
  else 
    if (x==0) return 0;
  else
    return positive_dilogarithm(x);
}

%> // end of namespace PLearn
