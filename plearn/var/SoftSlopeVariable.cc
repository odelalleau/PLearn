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
   * $Id: SoftSlopeVariable.cc,v 1.7 2004/04/16 17:37:55 yoshua Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "SoftSlopeVariable.h"
//#include "Var_utils.h"

namespace PLearn {
using namespace std;


/** SoftSlopeVariable **/

SoftSlopeVariable::  SoftSlopeVariable(Variable* x, Variable* smoothness, Variable* left, Variable* right, bool tabulated_)
  :NaryVariable(VarArray(x,smoothness) & Var(left) & Var(right), 
                x->length()<left->length()?left->length():x->length(), 
                x->width()<left->width()?left->width():x->width()), tabulated(tabulated_)
{}


PLEARN_IMPLEMENT_OBJECT(SoftSlopeVariable, 
                        "This Var computes the soft_slope function", 
                        "The soft_slope function is a soft version of linear by parts function.\n"
                        "(as smoothness goes to infty). More precisely it converges to a function that is\n"
                        "0 in [-infty,left], linear in [left,right], and 1 in [right,infty], and continuous\n"
                        "It is always monotonically increasing wrt x (positive derivative in x).\n"
                        "If the arguments are vectors than the operation is performed element by element on all of them.\n");

void SoftSlopeVariable::recomputeSize(int& l, int& w) const
{ 
  l=0; 
  w=0;
  for (int i=0;i<4;i++)
  {
    if (varray[i]->length()>l) l=varray[i]->length();
    if (varray[i]->width()>w) w=varray[i]->width();
  }
  for (int i=0;i<4;i++)
  {
    if (varray[i]->length()!=l || varray[i]->width()!=w)
    {
      if (varray[i]->length()!=1 || varray[i]->width()!=1)
        PLERROR("Each argument of SoftSlopeVariable should either have the same length/width as the others or length 1");
    }
  }
}


void SoftSlopeVariable::fprop()
{
  int n=nelems();
  int n1=varray[0]->nelems();
  int n2=varray[1]->nelems();
  int n3=varray[2]->nelems();
  int n4=varray[3]->nelems();
  real* x = varray[0]->valuedata;
  real* smoothness = varray[1]->valuedata;
  real* left = varray[2]->valuedata;
  real* right = varray[3]->valuedata;

  if (n1==n && n2==n && n3==n && n4==n)
    for(int i=0; i<n; i++)
      valuedata[i] = tabulated?tabulated_soft_slope(x[i], smoothness[i], left[i], right[i]):soft_slope(x[i], smoothness[i], left[i], right[i]);
  else if (n1==1 && n2==n && n3==n && n4==n)
    for(int i=0; i<n; i++)
      valuedata[i] = tabulated?tabulated_soft_slope(*x, smoothness[i], left[i], right[i]):soft_slope(*x, smoothness[i], left[i], right[i]);
  else
  {
    int m1= n1==1?0:1;
    int m2= n2==1?0:1;
    int m3= n3==1?0:1;
    int m4= n4==1?0:1;
    for(int i=0; i<n; i++,x+=m1,smoothness+=m2,left+=m3,right+=m4)
      valuedata[i] = tabulated?tabulated_soft_slope(*x, *smoothness, *left, *right):soft_slope(*x, *smoothness, *left, *right);
  }
}


void SoftSlopeVariable::bprop()
{
  int n=nelems();
  int n1=varray[0]->nelems();
  int n2=varray[1]->nelems();
  int n3=varray[2]->nelems();
  int n4=varray[3]->nelems();
  int m1= n1==1?0:1;
  int m2= n2==1?0:1;
  int m3= n3==1?0:1;
  int m4= n4==1?0:1;
  real* x = varray[0]->valuedata;
  real* smoothness = varray[1]->valuedata;
  real* left = varray[2]->valuedata;
  real* right = varray[3]->valuedata;
  real* dx = varray[0]->gradientdata;
  real* dsmoothness = varray[1]->gradientdata;
  real* dleft = varray[2]->gradientdata;
  real* dright = varray[3]->gradientdata;
  for(int i=0; i<n; i++,x+=m1,smoothness+=m2,left+=m3,right+=m4,dx+=m1,dsmoothness+=m2,dleft+=m3,dright+=m4)
  {
    if (*smoothness == 0) continue;
    real inv_smoothness = 1.0 / *smoothness;
    real t1 = sigmoid(- *smoothness*(*x-*left));
    real t2 = sigmoid(- *smoothness*(*x-*right));
    real inv_delta=1.0/(*right-*left);
    real rat = (tabulated?tabulated_soft_slope(*x, *smoothness, *left, *right):soft_slope(*x, *smoothness, *left, *right)) -1;
    real move = rat * inv_delta;
    real dss = (-t1*(*x-*left) + t2*(*x-*right))*inv_smoothness*inv_delta - rat * inv_smoothness;
    real dll = t1*inv_delta*inv_smoothness + move;
    real drr = -t2*inv_delta*inv_smoothness - move;
    real dxx = (-t1+t2)*inv_delta;
    *dx += gradientdata[i] * dxx;
    *dsmoothness += gradientdata[i] * dss;
    *dleft += gradientdata[i] * dll;
    *dright += gradientdata[i] * drr;
  }
}

void SoftSlopeVariable::symbolicBprop()
{
  PLERROR("SoftSlopeVariable::symbolicBprop() not implemented");
}



} // end of namespace PLearn


