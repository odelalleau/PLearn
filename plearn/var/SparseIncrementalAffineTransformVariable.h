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
   * $Id: SparseIncrementalAffineTransformVariable.h 1442 2004-04-27 15:58:16Z morinf $
   * This file is part of the PLearn library.
   ******************************************************* */

#ifndef SparseIncrementalAffineTransformVariable_INC
#define SparseIncrementalAffineTransformVariable_INC

#include "BinaryVariable.h"
#include <plearn/math/StatsCollector.h>

namespace PLearn {
using namespace std;


//! Affine transformation of a vector variable, with weights that are sparse and incrementally added
//! Should work for both column and row vectors: result vector will be of same kind (row or col)
//! First row of transformation matrix contains bias b, following rows contain linear-transformation T
//! Will compute b + x.T. 
//! In order to make sure that T is sparse, SparseIncrementalAffineTransformVariable only considers 
//! a subset of the entries in T,
//! and only bprops to those entries, ignoring the others. The number of entries of T
//! seen is incrementally increased, by selecting unseen entries with the highest average incoming
//! gradient since the last addition of entries. When a new weight is added, it is set
//! to start_grad_prop proportion of the average incoming gradient to that weight.
class SparseIncrementalAffineTransformVariable: public BinaryVariable
{
  typedef BinaryVariable inherited;

private:
  Mat temp_grad;

protected:
  //TVec<TVec<int> > positions;
  Vec<StatsCollector> sc_input;
  Vec<StatsCollector> sc_grad;
  TMat<StatsCollector> sc_input_grad;
  Mat positions;
  Mat sums;
  Vec input_average;
  int n_grad_samples;
  bool has_seen_input;
  int n_weights;

public:

  int add_n_weights;
  real start_grad_prop;
  real running_average_prop;

  //!  Default constructor for persistence
  SparseIncrementalAffineTransformVariable(): add_n_weights(0),  start_grad_prop(1){}
  SparseIncrementalAffineTransformVariable(Variable* vec, Variable* transformation, real the_running_average_prop, real the_start_grad_prop);

  PLEARN_DECLARE_OBJECT(SparseIncrementalAffineTransformVariable);

  virtual void build();

  static void declareOptions(OptionList & ol);

  virtual void recomputeSize(int& l, int& w) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
  virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);
  void reset();

protected:
  void build_();
};

DECLARE_OBJECT_PTR(SparseIncrementalAffineTransformVariable);

//! first row of transformation is the bias.
inline Var sparse_incremental_affine_transform(Var vec, Var transformation, real the_running_average_prop=0.01, real the_start_grad_prop=1)
{ 
  return new SparseIncrementalAffineTransformVariable(vec,transformation,the_running_average_prop,the_start_grad_prop);
}

} // end of namespace PLearn

#endif 
