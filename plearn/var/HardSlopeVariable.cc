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
   * $Id: HardSlopeVariable.cc,v 1.3 2004/04/27 16:02:26 morinf Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "HardSlopeVariable.h"
#include "Var_utils.h"

namespace PLearn {
using namespace std;


/** HardSlopeVariable **/

PLEARN_IMPLEMENT_OBJECT(HardSlopeVariable, 
                        "This Var computes the hard_slope function", 
                        "The hard_slope function is linear by parts function:\n"
                        "0 in [-infty,left], linear in [left,right], and 1 in [right,infty], and continuous.\n"
                        "If the arguments are vectors than the operation is performed element by element on all of them.\n");

HardSlopeVariable::  HardSlopeVariable(Variable* x, Variable* left, Variable* right)
  : inherited(VarArray(x,left) & Var(right), 
              x->length()<left->length()?left->length():x->length(), 
              x->width()<left->width()?left->width():x->width()) 
{}


void HardSlopeVariable::recomputeSize(int& l, int& w) const
{ 
    l = w = 0;
    if (varray.size() >= 3) {
        for (int i = 0;i < 3; i++) {
            if (varray[i]->length()>l)
                l = varray[i]->length();
            if (varray[i]->width() > w)
                w = varray[i]->width();
        }
        for (int i = 0;i < 3; i++) {
            if (varray[i]->length() != l || varray[i]->width() != w) {
                if (varray[i]->length() != 1 || varray[i]->width() != 1)
                    PLERROR("Each argument of HardSlopeVariable should either have the same length/width as the others or length 1");
            }
        }
    }
}


void HardSlopeVariable::fprop()
{
  int n=nelems();
  int n1=varray[0]->nelems();
  int n2=varray[1]->nelems();
  int n3=varray[2]->nelems();
  real* x = varray[0]->valuedata;
  real* left = varray[1]->valuedata;
  real* right = varray[2]->valuedata;

  if (n1==n && n2==n && n3==n)
    for(int i=0; i<n; i++)
      valuedata[i] = hard_slope(x[i], left[i], right[i]);
  else if (n1==1 && n2==n && n3==n)
    for(int i=0; i<n; i++)
      valuedata[i] = hard_slope(*x, left[i], right[i]);
  else
  {
    int m1= n1==1?0:1;
    int m2= n2==1?0:1;
    int m3= n3==1?0:1;
    for(int i=0; i<n; i++,x+=m1,left+=m2,right+=m3)
      valuedata[i] = hard_slope(*x, *left, *right);
  }
}


void HardSlopeVariable::bprop()
{
  int n=nelems();
  int n1=varray[0]->nelems();
  int n2=varray[1]->nelems();
  int n3=varray[2]->nelems();
  int m1= n1==1?0:1;
  int m2= n2==1?0:1;
  int m3= n3==1?0:1;
  real* x = varray[0]->valuedata;
  real* left = varray[1]->valuedata;
  real* right = varray[2]->valuedata;
  real* dx = varray[0]->gradientdata;
  real* dleft = varray[1]->gradientdata;
  real* dright = varray[2]->gradientdata;
  for(int i=0; i<n; i++,x+=m1,left+=m2,right+=m3,dx+=m1,dleft+=m2,dright+=m3)
  {
    real tleft = *x - *left;
    real tright = *x - *right;
    if (tright<=0 && tleft>=0)
    {
      real inv_delta=1.0/(*right - *left);
      real dll = tright*inv_delta;
      real drr = -tleft*inv_delta;
      real dxx = inv_delta;
      *dx += gradientdata[i] * dxx;
      *dleft += gradientdata[i] * dll;
      *dright += gradientdata[i] * drr;
    }
  }
}

void HardSlopeVariable::symbolicBprop()
{
  PLERROR("HardSlopeVariable::symbolicBprop() not implemented");
}



} // end of namespace PLearn


