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
   * $Id: OneHotSquaredLoss.h,v 1.3 2003/12/16 17:44:52 plearner Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#ifndef OneHotSquaredLoss_INC
#define OneHotSquaredLoss_INC

#include "BinaryVariable.h"
// For inline function that can return either a OneHotSquaredLoss or a MatrixOneHotSquaredLoss
#include "MatrixOneHotSquaredLoss.h"
namespace PLearn <%
using namespace std;



/*! * Efficient and numerically stable loss functions * */

//! Computes sum(square_i(netout[i]-(i==classnum ?hotval :coldval)) 
//! This is used typically in a classification setting where netout is a Var 
//! of network outputs, and classnum is the target class number.
class OneHotSquaredLoss: public BinaryVariable
{
protected:
  typedef BinaryVariable inherited;
  //!  Default constructor for persistence
  OneHotSquaredLoss() {}

  real coldval_, hotval_;

public:
  OneHotSquaredLoss(Variable* netout, Variable* classnum, real coldval=0., real hotval=1.);
  PLEARN_DECLARE_OBJECT(OneHotSquaredLoss);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
  virtual void rfprop();
};


inline Var onehot_squared_loss(Var network_output, Var classnum, real coldval=0., real hotval=1.)
{ 
  if (network_output->isVec())  
  return new OneHotSquaredLoss(network_output, classnum, coldval, hotval);
  else return new MatrixOneHotSquaredLoss(network_output, classnum, coldval, hotval);
}

%> // end of namespace PLearn

#endif 
