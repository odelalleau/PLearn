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
   * $Id: SoftSlopeVariable.h,v 1.3 2003/12/05 22:41:41 yoshua Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#ifndef SoftSlopeVariable_INC
#define SoftSlopeVariable_INC

#include "NaryVariable.h"
#include "pl_math.h"
#include "Var_utils.h"

namespace PLearn <%
using namespace std;


// as smoothness-->infty this becomes the linear by part function that
// is 0 in [-infty,left], linear in [left,right], and 1 in [right,infty].
// For finite smoothness, it is a smoother function, always with value in the interval [0,1].
// It is always monotonically increasing wrt x (positive derivative in x).
class SoftSlopeVariable: public NaryVariable
{
protected:
    typedef NaryVariable inherited;
  //!  Default constructor for persistence
  SoftSlopeVariable() {}

public:
  SoftSlopeVariable(Variable* x, Variable* smoothness, Variable* left, Variable* right);
  PLEARN_DECLARE_OBJECT(SoftSlopeVariable);
  virtual void recomputeSize(int& l, int& w) const;
  
  
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
};

inline Var soft_slope(Var x, Var smoothness, Var left, Var right)
{ return new SoftSlopeVariable(x,smoothness,left,right); }

// derivative of soft_slope wrt x
inline Var d_soft_slope(Var x, Var smoothness, Var left, Var right)
{
  return (-sigmoid(-smoothness*(x-left))+sigmoid(-smoothness*(x-right)))/(right-left);
}

// limit of soft_slope when smoothness-->0
inline Var soft_slope_limit(Var x, Var smoothness, Var left, Var right)
{
  int l=0; 
  int w=0;
  if (x->length()>l) l=x->length();
  if (x->width()>w) w=x->width();
  if (smoothness->length()>l) l=smoothness->length();
  if (smoothness->width()>w) w=smoothness->width();
  if (left->length()>l) l=left->length();
  if (left->width()>w) w=left->width();
  if (right->length()>l) l=right->length();
  if (right->width()>w) w=right->width();
  if (x->length()>l && l!=x->length()) PLERROR("soft_slope_limit: Each argument should have the same size or size 1");
  if (x->width()>w && w!=x->width()) PLERROR("soft_slope_limit: Each argument should have the same size or size 1");
  if (smoothness->length()>l && l!=smoothness->length())  PLERROR("soft_slope_limit: Each argument should have the same size or size 1");
  if (smoothness->width()>w && w!=smoothness->width())  PLERROR("soft_slope_limit: Each argument should have the same size or size 1");
  if (left->length()>l && l!=left->length())  PLERROR("soft_slope_limit: Each argument should have the same size or size 1");
  if (left->width()>w && w!=left->width())  PLERROR("soft_slope_limit: Each argument should have the same size or size 1");
  if (right->length()>l && l!=right->length())  PLERROR("soft_slope_limit: Each argument should have the same size or size 1");
  if (right->width()>w && w!=right->width())  PLERROR("soft_slope_limit: Each argument should have the same size or size 1");
  Var res(l,w);
  res->value.fill(0.5);
  return res;
}

%> // end of namespace PLearn

#endif 
