// -*- C++ -*-

// KNNRegressor.h
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
   * $Id$ 
   ******************************************************* */

// Authors: Nicolas Chapados

/*! \file KNNRegressor.h */


#ifndef KNNRegressor_INC
#define KNNRegressor_INC

#include <plearn_learners/generic/PLearner.h>
#include <plearn_learners/nearest_neighbors/GenericNearestNeighbors.h>

namespace PLearn {

/**
 * This class provides a simple multivariate regressor based upon an
 * enclosed K-nearest-neighbors finder (derived from
 * GenericNearestNeighbors; specified with the 'knn' option).
 *
 * The class contains several options to determine the number of neighbors
 * to use (K).  This number always overrides the option 'num_neighbors'
 * that may have been specified in the GenericNearestNeighbors utility
 * object.  Basically, the generic formula for the number of neighbors is
 *
 *     K = max(kmin, kmult*(n^kpow)),
 *
 * where 'kmin', 'kmult', and 'kpow' are options, and 'n' is the number of
 * examples in the training set.
 *
 * The cost output from this class is:
 *
 * - 'mse', the mean-squared error, i.e. given an output o and target t,
 *       mse(o,t) = \sum_i (o[i]-t[i])^2,
 *
 * If the option 'use_knn_costs_as_weights' is true (by default), it is
 * assumed that the costs coming from the 'knn' object are kernel
 * evaluations for each nearest neighbor.  These are used as weights to
 * determine the final class probabilities.  (NOTE: it is important to use
 * a kernel that computes a SIMILARITY MEASURE, and not a DISTANCE MEASURE;
 * the default EpanechnikovKernel has the proper behavior.)  If the option
 * is false, an equal weighting is used (equivalent to square window).  In
 * addition, a different weighting kernel may be specified with the
 * 'kernel' option.
 *
 * A local weighted regression model may be trained at each test point
 * by specifying a 'local_model'.  For instance, to perform local linear
 * regression, you may use a LinearRegressor for this purpose.
 */
class KNNRegressor: public PLearner
{
  typedef PLearner inherited;

protected:
  //! Internal use: temporary buffer for knn output
  mutable Vec knn_output;

  //! Internal use: temporary buffer for knn costs
  mutable Vec knn_costs;
  
public:
  //#####  Public Build Options  ############################################

  //! The K-nearest-neighbors finder to use (default is an
  //! ExhaustiveNearestNeighbors with an EpanechnikovKernel, lambda=1)
  PP<GenericNearestNeighbors> knn;

  //! Minimum number of neighbors to use (default=5)
  int kmin;

  //! Multiplicative factor on n^kpow to determine number of neighbors to
  //! use (default=0)
  real kmult;

  //! Power of the number of training examples to determine number of
  //! neighbors (default=0.5)
  real kpow;

  //! Whether to weigh each of the K neighbors by the kernel evaluations,
  //! obtained from the costs coming out of the 'knn' object (default=true)
  bool use_knn_costs_as_weights;

  //! Disregard the 'use_knn_costs_as_weights' option, and use this kernel
  //! to weight the observations.  If this object is not specified
  //! (default), and the 'use_knn_costs_as_weights' is false, the
  //! rectangular kernel is used.
  Ker kernel;

  //! Train a local regression model from the K neighbors, weighted by
  //! the kernel evaluations.  This is carried out at each test point.
  PP<PLearner> local_model;

public:
  //#####  Object Methods  ##################################################

  //! Default constructor.
  KNNRegressor();

  //! Simply calls inherited::build() then build_().
  virtual void build();

  //! Transforms a shallow copy into a deep copy.
  virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

  // Declares other standard object methods.
  // If your class is not instantiatable (it has pure virtual methods)
  // you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT.
  PLEARN_DECLARE_OBJECT(KNNRegressor);

public:
  //#####  PLearner Methods  ####################################################

  //! Overridden to call knn->setTrainingSet
  virtual void setTrainingSet(VMat training_set, bool call_forget=true);

  //! Returns the size of this learner's output, (which typically
  //! may depend on its inputsize(), targetsize() and set options).
  virtual int outputsize() const;

  //! Forwarded to knn
  virtual void forget();
    
  //! Forwarded to knn
  virtual void train();

  //! Computes the output from the input.
  virtual void computeOutput(const Vec& input, Vec& output) const;

  //! Computes the costs from already computed output. 
  virtual void computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                       const Vec& target, Vec& costs) const;

  //! Delegate to local model if one is specified; not implemented
  //! otherwise (although one could easily return the standard error
  //! of the mean, weighted by the kernel measure; -- to do).
  virtual
  bool computeConfidenceFromOutput(const Vec& input, const Vec& output,
                                   real probability,
                                   TVec< pair<real,real> >& intervals) const;
  
  //! Returns the names of the costs computed by computeCostsFromOutpus
  //! (and thus the test method).
  virtual TVec<std::string> getTestCostNames() const;

  //! Returns the names of the objective costs that the train method computes and 
  //! for which it updates the VecStatsCollector train_stats.
  virtual TVec<std::string> getTrainCostNames() const;

private: 
  //! This does the actual building. 
  void build_();

protected: 
  //! Declares this class' options.
  static void declareOptions(OptionList& ol);
};

// Declares a few other classes and functions related to this class.
DECLARE_OBJECT_PTR(KNNRegressor);
  
} // end of namespace PLearn

#endif
