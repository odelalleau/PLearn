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
   * $Id: LocalizedFeaturesLayerVariable.h,v 1.4 2005/05/24 16:24:40 tihocan Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#ifndef LocalizedFeaturesLayerVariable_INC
#define LocalizedFeaturesLayerVariable_INC

#include "NaryVariable.h"

namespace PLearn {
using namespace std;


//! Single layer of a neural network with local connectivity
//! upon a set of localized features, i.e. each feature is
//! associated with a location in some low-dimensional space
//! and each hidden unit takes input only from a subset of
//! features that are in some local region in that space.
class LocalizedFeaturesLayerVariable: public NaryVariable
{
  typedef NaryVariable inherited;

  //! INTERNAL LEARNED PARAMETERS

  //! INTERNAL COMPUTATION
  int n_features; // = nb inputs
  int n_subsets; // = feature_subsets.length()
  int n_connections; // = n_hidden_per_subset * sum_i feature_subsets[i].length()

public:

  //! OPTIONS
  bool backprop_to_inputs;
  VMat feature_locations; // n_features x n_dim
  TVec<TVec<int> > feature_subsets; // n_subsets x (nb of features in i-th subset)
  int n_hidden_per_subset;
  bool knn_subsets;
  int n_neighbors_per_subset;
  bool gridding_subsets;
  bool center_on_feature_locations;
  long seed;

  //!  Default constructor for persistence
  LocalizedFeaturesLayerVariable();

  PLEARN_DECLARE_OBJECT(LocalizedFeaturesLayerVariable);

  virtual void build();

  virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

  virtual void recomputeSize(int& l, int& w) const;
  virtual void fprop();
  virtual void bprop();

protected:

  static void declareOptions(OptionList &ol);

  virtual void computeSubsets();

private:

  void build_();

};

DECLARE_OBJECT_PTR(LocalizedFeaturesLayerVariable);

} // end of namespace PLearn

#endif 
