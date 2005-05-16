// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 2005 Yoshua Bengio

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
   * $Id: FNetLayerVariable.h,v 1.1 2005/05/16 16:05:16 yoshua Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#ifndef ProductVariable_INC
#define ProductVariable_INC

#include "NaryVariable.h"

namespace PLearn {
using namespace std;


//! Single layer of a neural network, with acceleration tricks
//!
//! The two inputs are (1) the input of the layer (n_inputs x minibatch_size) and (2) the weights
//! and biases (layer_size x (n_inputs + 1)).
class FNetLayerVariable: public NaryVariable
{
  typedef NaryVariable inherited;

  //! INTERNAL LEARNED PARAMETERS
  Mat mu; // [hidden unit, input element] centering normalization
  Mat invs; // 1/sqrt(mu2 - mu*mu) for normalization
  real gradient_threshold; // threshold for |dC/da[k,i]| to 'activate' gradient propagation through unit i at example k

  //! INTERNAL COMPUTATION
  Mat mu2; // to compute invs, the inverse of the standard deviation
  real avg_act_gradient; // mov. avg of |dC/da[i,k]|
  bool no_bprop_has_been_done; // have we ever done a bprop before?
  TVec<Mat> u; // normalized input [example index in minibatch](hidden unit, input index)
  Mat inh; // inhibitory signal on each hidden uniti (minibatch_size x n_hidden)
  Mat cum_inh; 

  // syntactic sugar
  int n_inputs;
  int n_hidden;
  int minibatch_size;

public:

  //! OPTIONS
  bool inhibit_next_units; 
  bool normalize_inputs;
  bool backprop_to_inputs;
  real exp_moving_average_coefficient;
  real average_error_fraction_to_threshold;

  //!  Default constructor for persistence
  FNetLayerVariable();

  FNetLayerVariable(Var inputs, Var weights,
                    Var biases, Var inhibition_weights,
                    bool _inhibit_next_units=true,
                    bool _normalize_inputs=true,
                    bool _backprop_to_inputs=false,
                    real _exp_moving_average_coefficient=0.001,
                    real _average_error_fraction_to_threshold=0.5);

  PLEARN_DECLARE_OBJECT(FNetLayerVariable);

  static void declareOptions(OptionList &ol);
  virtual void build();

  virtual void recomputeSize(int& l, int& w) const;
  virtual void fprop();
  virtual void bprop();

protected:
  void build_();
};

} // end of namespace PLearn

#endif 
