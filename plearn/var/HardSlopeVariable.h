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
   * $Id: HardSlopeVariable.h,v 1.3 2004/02/20 21:11:50 chrish42 Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#ifndef HardSlopeVariable_INC
#define HardSlopeVariable_INC

#include "NaryVariable.h"
#include "Var_operators.h"
#include "Var_all.h"
//#include "pl_math.h"
//#include "Var_utils.h"

namespace PLearn {
using namespace std;


// as smoothness-->infty this becomes the linear by part function that
// is 0 in [-infty,left], linear in [left,right], and 1 in [right,infty].
// For finite smoothness, it is a smoother function, always with value in the interval [0,1].
// It is always monotonically increasing wrt x (positive derivative in x).
class HardSlopeVariable: public NaryVariable
{
protected:
    typedef NaryVariable inherited;
  //!  Default constructor for persistence
  HardSlopeVariable() {}

public:
  HardSlopeVariable(Variable* x, Variable* smoothness, Variable* left, Variable* right);
  PLEARN_DECLARE_OBJECT(HardSlopeVariable);
  virtual void recomputeSize(int& l, int& w) const;
  
  
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
};

inline Var hard_slope(Var x, Var left, Var right)
{ return new HardSlopeVariable(x,left,right); }

// derivative of hard_slope wrt x
inline Var d_hard_slope(Var x, Var left, Var right)
{
  return ifThenElse((x>=left)*(x<=right),invertElements(right-left),var(0.0));
}

} // end of namespace PLearn

#endif 
