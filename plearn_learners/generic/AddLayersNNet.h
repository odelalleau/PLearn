// -*- C++ -*-

// AddLayersNNet.h
//
// Copyright (C) 2004 Olivier Delalleau 
// 
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
   * $Id: AddLayersNNet.h,v 1.7 2004/10/12 17:32:55 tihocan Exp $ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file AddLayersNNet.h */


#ifndef AddLayersNNet_INC
#define AddLayersNNet_INC

#include "NNet.h"

namespace PLearn {
using namespace std;

class AddLayersNNet: public NNet {

private:

  typedef NNet inherited;
  
protected:

  // *********************
  // * protected options *
  // *********************

  // Fields below are not options.

  //! This vector contains the 'real' parts' sizes (there is not '-1').
  //! It is filled at build time from 'parts_size' and the training set inputsize.
  TVec<int> real_parts_size;

  //! Contains the added hidden layers.
  VarArray hidden_layers;

  //! Contains the weights for the hidden layers added.
  VarArray hidden_weights;

public:

  // ************************
  // * public build options *
  // ************************

  TVec<int> add_hidden;
  string added_hidden_transfer_func;
  TVec<int> parts_size;

  // ****************
  // * Constructors *
  // ****************

  //! Default constructor.
  AddLayersNNet();

  // ********************
  // * PLearner methods *
  // ********************

private: 

  //! This does the actual building. 
  void build_();

protected: 
  
  //! Declares this class' options.
  static void declareOptions(OptionList& ol);

public:

  // ************************
  // **** Object methods ****
  // ************************

  //! Simply calls inherited::build() then build_().
  virtual void build();

  //! Transforms a shallow copy into a deep copy.
  virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

  // Declares other standard object methods.
  PLEARN_DECLARE_OBJECT(AddLayersNNet);

  // **************************
  // **** PLearner methods ****
  // **************************

  //! Return the current activations of a given added hidden layer.
  //! This is not a copy, so they should not be modified.
  Vec getHiddenUnitsActivation(int layer);

  //! Return the hidden weights for a given hidden layer.
  //! This is not a copy, so they should not be modified.
  Mat getHiddenWeights(int layer);

  //! Return the weights going out of a given added hidden layer.
  //! This is not a copy, so they should not be modified.
  Mat getOutputHiddenWeights(int layer);

protected:

  //! Overridden to account for the added layers.
  virtual void initializeParams(bool set_seed = true);
  virtual void buildPenalties(const Var& hidden_layer);

};

// Declares a few other classes and functions related to this class.
DECLARE_OBJECT_PTR(AddLayersNNet);
  
} // end of namespace PLearn

#endif
