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
   * $Id: IsAboveThresholdVariable.h,v 1.4 2003/12/16 17:44:52 plearner Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#ifndef IsAboveThresholdVariable_INC
#define IsAboveThresholdVariable_INC

#include "UnaryVariable.h"

namespace PLearn <%
using namespace std;

  
//!  Does elementwise newx_i = (x_i>=threshold ?truevalue :falsevalue);
//        OR          newx_i = (x_i>threshold ?truevalue :falsevalue);
// according to the 'strict' field.
class IsAboveThresholdVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  IsAboveThresholdVariable() : threshold(), truevalue(), falsevalue() {}

protected:
  real threshold, truevalue, falsevalue;
public:
  bool strict;
  IsAboveThresholdVariable(Variable* input, real the_threshold, real the_truevalue, real the_falsevalue, bool strict=false);
  PLEARN_DECLARE_OBJECT(IsAboveThresholdVariable);
  virtual void recomputeSize(int& l, int& w) const;
  
  
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
  virtual void rfprop();
};


inline Var isAboveThreshold(Var v, real threshold=0, real truevalue=1, real falsevalue=0, bool strict=false)
{ return new IsAboveThresholdVariable(v,threshold,truevalue,falsevalue,strict); }

inline Var operator>=(Var v, real threshold)
{ return new IsAboveThresholdVariable(v, threshold, 1, 0); }

inline Var operator<=(Var v, real threshold)
{ return new IsAboveThresholdVariable(v, threshold, 0, 1); }

%> // end of namespace PLearn

#endif 
