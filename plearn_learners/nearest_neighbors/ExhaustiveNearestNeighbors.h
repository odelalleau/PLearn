// -*- C++ -*-

// ExhaustiveNearestNeighbors.h
//
// Copyright (C) 2004 Nicolas Chapados
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
   * $Id: ExhaustiveNearestNeighbors.h,v 1.1 2004/12/20 15:46:50 chapados Exp $ 
   ******************************************************* */

// Authors: Nicolas Chapados


#ifndef ExhaustiveNearestNeighbors_INC
#define ExhaustiveNearestNeighbors_INC

#include <plearn_learners/generic/GenericNearestNeighbors.h>
#include <plearn/kernel/Kernel.h>

namespace PLearn {

/**
 * This class provides the basic implementation of the classical O(N^2)
 * nearest-neighbors algorithm.  For each test point, it performs an
 * exhaustive search in the training set to find the K (specified by the
 * inherited 'num_neighbors' option) closest examples according to a
 * user-specified Kernel.
 *
 * The output costs are simply the kernel values for each found training
 * point.  The costs are named 'ker0', 'ker1', ..., 'kerK-1'.
 *
 * The training set is SAVED with this learner, under the option name
 * 'training_mat'. Otherwise, one would NOT be able to reload the learner
 * and carry out test operations!
 */ 
class ExhaustiveNearestNeighbors: public GenericNearestNeighbors
{
  typedef GenericNearestNeighbors inherited;

protected:
  //! Matrixified version of the training set.  Saved.
  Mat training_mat;
  
public:
  //#####  Public Build Options  ############################################

  //! Kernel that must be used to evaluate distances.  Default is a
  //! DistanceKernel with n=2, which gives an Euclidian distance.
  Ker kernel;

public:
  //#####  Object Methods  ##################################################
  
  //! Default constructor.
  ExhaustiveNearestNeighbors();

  //! Simply calls inherited::build() then build_().
  virtual void build();

  //! Transforms a shallow copy into a deep copy.
  virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

  // Declares other standard object methods.
  // If your class is not instantiatable (it has pure virtual methods)
  // you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT.
  PLEARN_DECLARE_OBJECT(ExhaustiveNearestNeighbors);


public:
  //#####  PLearner Methods  ################################################

  //! Returns the size of this learner's output, (which typically
  //! may depend on its inputsize(), targetsize() and set options).
  virtual int outputsize() const;

  //! (Re-)initializes the PLearner in its fresh state (that state may
  //! depend on the 'seed' option)
  //! And sets 'stage' back to 0 (this is the stage of a fresh learner!).
  virtual void forget();
    
  //! The role of the train method is to bring the learner up to stage==nstages,
  //! updating the train_stats collector with training costs measured on-line in the process.
  virtual void train();

  //! Compute the output and cost from the input
  virtual void computeOutputAndCosts(const Vec& input, const Vec& target,
                                     Vec& output, Vec& costs) const;
  
  //! Computes the output from the input.
  virtual void computeOutput(const Vec& input, Vec& output) const;

  //! Computes the costs from already computed output. 
  virtual void computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                       const Vec& target, Vec& costs) const;
  
  //! Returns the names of the costs computed by computeCostsFromOutpus
  //! (and thus the test method).
  virtual TVec<std::string> getTestCostNames() const;

  //! Returns the names of the objective costs that the train method
  //! computes and for which it updates the VecStatsCollector train_stats.
  virtual TVec<std::string> getTrainCostNames() const;

private: 
  //! This does the actual building. 
  void build_();

protected: 
  //! Declares this class' options.
  static void declareOptions(OptionList& ol);
};

// Declares a few other classes and functions related to this class.
DECLARE_OBJECT_PTR(ExhaustiveNearestNeighbors);
  
} // end of namespace PLearn

#endif
