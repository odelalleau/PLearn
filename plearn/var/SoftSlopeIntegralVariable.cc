// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2001-2002 Nicolas Chapados, Ichiro Takeuchi, Jean-Sebastien Senecal
// Copyright (C) 2002 Xiangdong Wang, Christian Dorion

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
   * $Id: SoftSlopeIntegralVariable.cc,v 1.2 2004/01/05 01:29:06 yoshua Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "SoftSlopeIntegralVariable.h"
#include "Var_utils.h"

namespace PLearn <%
using namespace std;


/** SoftSlopeIntegralVariable **/

SoftSlopeIntegralVariable::  SoftSlopeIntegralVariable(Variable* smoothness, Variable* left, Variable* right, real a_, real b_)
  :NaryVariable(VarArray(smoothness) & Var(left) & Var(right), 
                smoothness->length()<left->length()?left->length():smoothness->length(), 
                smoothness->width()<left->width()?left->width():smoothness->width()),
   a(a_), b(b_)
{}


PLEARN_IMPLEMENT_OBJECT(SoftSlopeIntegralVariable, 
                        "This Var computes the integral of the soft_slope function in an interval.", 
                        "Compute the integral of soft_slope(x,s,l,r) over x from a to b\n");

void SoftSlopeIntegralVariable::recomputeSize(int& l, int& w) const
{ 
  l=0; 
  w=0;
  for (int i=0;i<3;i++)
  {
    if (varray[i]->length()>l) l=varray[i]->length();
    if (varray[i]->width()>w) w=varray[i]->width();
  }
  for (int i=0;i<3;i++)
  {
    if (varray[i]->length()!=l || varray[i]->width()!=w)
    {
      if (varray[i]->length()!=1 || varray[i]->width()!=1)
        PLERROR("Each argument of SoftSlopeIntegralVariable should either have the same length/width as the others or length 1");
    }
  }
}


void SoftSlopeIntegralVariable::fprop()
{
  int n=nelems();
  int n1=varray[0]->nelems();
  int n2=varray[1]->nelems();
  int n3=varray[2]->nelems();
  real* smoothness = varray[0]->valuedata;
  real* left = varray[1]->valuedata;
  real* right = varray[2]->valuedata;

  if (n1==n && n2==n && n3==n)
    for(int i=0; i<n; i++)
      valuedata[i] = soft_slope_integral(smoothness[i], left[i], right[i],a,b);
  else if (n1==1 && n2==n && n3==n)
    for(int i=0; i<n; i++)
      valuedata[i] = soft_slope_integral(*smoothness, left[i], right[i],a,b);
  else
  {
    int m1= n1==1?0:1;
    int m2= n2==1?0:1;
    int m3= n3==1?0:1;
    for(int i=0; i<n; i++,smoothness+=m1,left+=m2,right+=m3)
      valuedata[i] = soft_slope_integral(*smoothness, *left, *right, a, b);
  }
}


void SoftSlopeIntegralVariable::bprop()
{
  int n=nelems();
  int n1=varray[0]->nelems();
  int n2=varray[1]->nelems();
  int n3=varray[2]->nelems();
  int m1= n1==1?0:1;
  int m2= n2==1?0:1;
  int m3= n3==1?0:1;
  real* smoothness = varray[0]->valuedata;
  real* left = varray[1]->valuedata;
  real* right = varray[2]->valuedata;
  real* dsmoothness = varray[0]->gradientdata;
  real* dleft = varray[1]->gradientdata;
  real* dright = varray[2]->gradientdata;
  for(int i=0; i<n; i++,smoothness+=m1,left+=m2,right+=m3,dsmoothness+=m1,dleft+=m2,dright+=m3)
  {
    if (*smoothness == 0) continue;
    // soft_slope_integral =
    // (b - a) + (softplus_primitive(-smoothness*(b-right)) - softplus_primitive(-smoothness*(b-left))
    //    -softplus_primitive(-smoothness*(a-right)) + softplus_primitive(-smoothness*(a-left)))/
    //     (smoothness*smoothness*(right-left));
    // hence 
    //
    //  d/dsmoothness = (softplus(-smoothness*(b-right))*(right-b) + softplus(-smoothness*(b-left))*(b-left)
    //                 + softplus(-smoothness*(a-right))*(a-right) + softplus(-smoothness*(a-left))*(left-a))/
    //     (smoothness*smoothness*(right-left))
    //     - 2 * (soft_slope_integral - b + a) / smoothness
    //n
    //  d/dleft = (-softplus(-smoothness*(b-left)) + softplus(-smoothness*(a-left)))/
    //     (smoothness*(right-left))
    //     + (soft_slope_integral - b + a)/ (right-left)
    //
    //  d/dright =  (softplus(-smoothness*(b-right)) - softplus(-smoothness*(a-right)))/(smoothness*(right-left))
    //         - (soft_slope_integral - b + a)/ (right-left)
    //
    real inv_smoothness = 1.0 / *smoothness;
    real br = (b-*right);
    real bl = (b-*left);
    real ar = (a-*right);
    real al = (a-*left);
    real t1 = softplus(- *smoothness*br);
    real t2 = softplus(- *smoothness*bl);
    real t3 = softplus(- *smoothness*ar);
    real t4 = softplus(- *smoothness*al);
    real inv_delta=1.0/(*right-*left);
    real ssiab = valuedata[i]-b+a;
    *dsmoothness += gradientdata[i] * 
      ((-t1*br + t2*bl + t3*ar - t4*al)*inv_smoothness*inv_delta - 2*ssiab)*inv_smoothness;
    *dleft += gradientdata[i] * ((-t2 + t4)*inv_smoothness + ssiab) * inv_delta;
    *dright += gradientdata[i] * ((t1 - t3)*inv_smoothness - ssiab) * inv_delta;
  }
}

void SoftSlopeIntegralVariable::symbolicBprop()
{
  PLERROR("SoftSlopeIntegralVariable::symbolicBprop() not implemented");
}



%> // end of namespace PLearn


