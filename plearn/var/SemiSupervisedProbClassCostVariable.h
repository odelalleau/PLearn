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
   * $Id: SemiSupervisedProbClassCostVariable.h,v 1.3 2004/02/20 21:11:52 chrish42 Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#ifndef SemiSupervisedProbClassCostVariable_INC
#define SemiSupervisedProbClassCostVariable_INC

#include "NaryVariable.h"

namespace PLearn {
using namespace std;


/*!  
//! This is a cost function for semi-supervised training of a probabilistic classifier.
//! It takes three arguments: (prob,target,prior).
//! The prob should be positive and summing to 1 (e.g. the output of a softmax).
//! The target is either an integer (which indexes the output vector) or a missing value.
//! The prior is a vector of probabilities (they don't have to be normalized) which
//! act as a prior when the target is not provided.
//! There are two types of cases: when the target is given (supervised case)
//! and when the target is missing (unsupervised case).
//! The cost value is computed as follows:
//!
//!   If target is not missing:
//!     cost = - log(prob[target])
//!   Else
//!     cost = - (1/flatten_factor) log sum_i (prior[i] * prob[i])^flatten_factor
//!
//! The only option of this class is the hyper-parameter "flatten_factor"
//! If prior is an empty Var, then prior[i]=1 is assumed.
//!
//! N.B.
//!   - prior can be understood as the supervised target for supervised examples
//!     (then the above cost becomes the classical max. likelihood, = log prob_{target})
//!   - for unsupervised examples prior embodies the information available on the target;
//!     reasonable choices are:
//!       - all 1's (recommanded)
//!       - prior relative frequencies (conditional, if some other information can be used, heuristic) 
//!       - output of the model trained on supervised examples only (?? not clear from theory)
//!     In the latter two cases it might be desirable to modulate the "strength" of the
//!     prior probabilities by raising them to some power. Note that prior[i] do not
//!     have to be normalized but they should be non-negative.
//!   - when flatten_factor=1 the gradient on unsupervised examples (prior=const) is 0
//!   - increasing "flatten_factor" increases the confidence given to the model current prediction on unsupervised examples
//!   - this cost may be very sensitive to INITIALIZATION:
//!        - it would be wise to start from purely supervised training (i.e. a=1, or skip the unsupervised examples)
//!        - and gradually increase it (<=5, say) e.g. a=1, then a=1.1, then a=1.5, then a=2.
//!
//! N.B2: THE IMPLEMENTATION OF THIS CLASS CHEATS AND DOES NOT COMPUTE THE GRADIENTS
//! WITH RESPECT TO THE target AND prior INPUTS, AS SUCH GRADIENTS SHOULD NOT BE USED.
//!
*/
class SemiSupervisedProbClassCostVariable: public NaryVariable
{
protected:
  //!  Default constructor for persistence
  SemiSupervisedProbClassCostVariable() {}

  //! the option
  real flatten_factor;

  //! temporaries for avoiding re-computations in the bprop
  Vec raised_prob;
  real sum_raised_prob;

public:
  SemiSupervisedProbClassCostVariable(Var prob_, Var target_, Var prior_, real flattenfactor=1.0);
  PLEARN_DECLARE_OBJECT(SemiSupervisedProbClassCostVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
  virtual void rfprop();
  Var& prob() { return varray[0]; }
  Var& target() { return varray[1]; }
  Var& prior() { return varray[2]; }
};


} // end of namespace PLearn

#endif 
