// -*- C++ -*-

// SequentialModelSelector.h
//
// Copyright (C) 2003 Rejean Ducharme, Yoshua Bengio
// Copyright (C) 2003 Pascal Vincent
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



#ifndef SEQUENTIAL_MODEL_SELECTOR
#define SEQUENTIAL_MODEL_SELECTOR

#include "SequentialLearner.h"

namespace PLearn {
using namespace std;

/*!
*/

class SequentialModelSelector: public SequentialLearner
{
private:
  // *********************
  // * private members   *
  // *********************

  //! This does the actual building
  void build_();

protected:

  // *********************
  // * protected options *
  // *********************
  
  // *********************
  // * protected members *
  // *********************
  
  //! See common_costs options for details. Row index: model. Column index: common_cost.
  TMat<int> common_cost_indices;
  TVec<int> best_model; // best model selected at time t
  Vec sequence_costs;  // the costs of each model on the training set
  

  // *********************
  // * protected methods *
  // *********************
  
  //! Declare this class' options
  static void declareOptions(OptionList& ol);
  
public:

  // *********************
  // * public options    *
  // *********************

  /*! 
    Does the model selector hass to save errors at each step. 
    Default: true.
  */
  bool stepwise_save;

  //! List of all the models.
  TVec< PP<SequentialLearner> > models;  

  //! If the user desires to provide a name for each model instead of model_i
  TVec< string > model_names;

  /*!
    The names of costs that are common to all models and that the user wishes the model
    selector to keep track of. The first one is considered to be the main cost, the one
    from which models will be compared to choose the best model.
   */
  TVec<string> common_costs;
  
  /*!
    From the common_costs list, the first cost is the one from which models will be compared 
    to choose the best model. But should the best model be chosen according to the
    
    max/min
      +/-   1: Mean
      +/-   2: Mean / Variance
      +/-   3: more to come.

    of the cost realizations. 
    Default: 1.
   */
  int comparison_type;

  /*!
    If positive, the comparison performed on the basis of common_cost[0] will be applyed only
    the comparison_window last elements of the cost sequence.
    Default: -1. (No window)
  */
  int comparison_window;

  // *********************
  // * public methods    *
  // *********************

  //! Constructor
  SequentialModelSelector();

  //! compute the cost of the given sequence of errors (based on the cost_type)
  real sequenceCost(const Vec& sequence_errors);
  
  //! Computes a paired t-test between common_cost[cc] series of models m1 and m2
  real paired_t_test(const int& m1, const int& m2, int cc=0) const;

  //! simply calls inherited::build() then build_()
  virtual void build();

  //! Redefines so that it ALSO calls the method on all the learners in the TVec models
  virtual void setExperimentDirectory(const string& _expdir);
  
  virtual void train();
 
  virtual void test(VMat testset, PP<VecStatsCollector> test_stats,
        VMat testoutputs=0, VMat testcosts=0) const;

  virtual void computeOutput(const Vec& input, Vec& output) const;
  
  virtual void computeCostsFromOutputs(const Vec& input, const Vec& output,
                                       const Vec& target, Vec& costs) const;
  
  virtual void computeOutputAndCosts(const Vec& input, const Vec& target,
                                     Vec& output, Vec& costs) const;
  
  virtual void computeCostsOnly(const Vec& input, const Vec& target, Vec& costs) const;

  virtual void matlabSave(const string& matlab_subdir);

  //! This should return the names of the costs computed by computeCostsFromOutputs
  virtual TVec<string> getTestCostNames() const;
  
  //! This should return the names of the objective costs that the train method
  //! computes and for which it updates the VecStatsCollector train_stats
  virtual TVec<string> getTrainCostNames() const;

  virtual void forget();
  
  //!  Does the necessary operations to transform a shallow copy (this)
  //!  into a deep copy by deep-copying all the members that need to be.
  typedef SequentialLearner inherited;
  PLEARN_DECLARE_OBJECT(SequentialModelSelector);
  virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);
};

//! Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(SequentialModelSelector);

} // end of namespace PLearn

#endif
