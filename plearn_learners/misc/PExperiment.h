// -*- C++ -*-

// PExperiment.h
// 
// Copyright (C) 2002 Pascal Vincent, Frederic Morin
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
   * $Id: PExperiment.h,v 1.2 2003/05/07 05:39:19 plearner Exp $ 
   ******************************************************* */

/*! \file PExperiment.h */
#ifndef PExperiment_INC
#define PExperiment_INC

#include "Object.h"
#include "PLearner.h"
#include "VMat.h"
#include "Splitter.h"

namespace PLearn <%
using namespace std;

class PExperiment: public Object
{    
public:

  typedef Object inherited;

  // ************************
  // * public build options *
  // ************************
  
  // See declareOptions method in .cc for the role of these options.

  //! Path of this experiment's directory in which to save all experiment results (will be created if it does not already exist)
  string expdir;  
  PP<PLearner> learner;
  VMat dataset;
  PP<Splitter> splitter;
  bool save_models;
  bool save_initial_models;
  bool save_test_outputs;
  bool save_test_costs;

  // ****************
  // * Constructors *
  // ****************

  // Default constructor
  PExperiment();


  // ******************
  // * Object methods *
  // ******************

private: 
  //! This does the actual building. 
  // (Please implement in .cc)
  void build_();

protected: 
  //! Declares this class' options
  // (Please implement in .cc)
  static void declareOptions(OptionList& ol);

public:
  // simply calls inherited::build() then build_() 
  virtual void build();

  //! Provides a help message describing this class
  static string help();

  //! Declares name and deepCopy methods
  DECLARE_NAME_AND_DEEPCOPY(PExperiment);

  //! runs the experiment
  virtual void run();

};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(PExperiment);
  
%> // end of namespace PLearn

#endif
