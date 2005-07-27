// -*- C++ -*-

// PDistribution.h
//
// Copyright (C) 2003  Pascal Vincent 
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

/*! \file PDistribution.h */
#ifndef PDistribution_INC
#define PDistribution_INC

#include <plearn_learners/generic/PLearner.h>
#include <plearn/math/PRandom.h>

namespace PLearn {
using namespace std;

//! Note that many methods are declared as 'const' because of the 'const'
//! plague, but are actually not true 'const' methods.
//! This is also why almost everything is mutable.
class PDistribution: public PLearner
{

private:

  typedef PLearner inherited;

  //! Global storage to save memory allocations.
  mutable Vec store_expect, store_result;
  mutable Mat store_cov;

protected:

  //! Random number generator.
  PP<PRandom> random;

  // *********************
  // * protected options *
  // *********************

  mutable TVec<int> cond_sort;
  mutable int n_input;
  mutable int n_margin;
  mutable int n_target;

  // Fields below are not options.

  //! A boolean indicating whether the input, target and margin part are
  //! already sorted nicely, so we actually don't have to swap indices
  //! when given a new vector.
  mutable bool already_sorted;

  //! A vector indicating which indices need to be swapped when we have modified
  //! the conditional flags, in order to still have input, target, margin in this
  //! order. It is made of pairs (j,k) indicating that the new j-th variable
  //! must be the old k-th variable.
  mutable TVec<int> cond_swap;

  //! The step when plotting the curve (upper case outputs_def).
  real delta_curve;

  //! A boolean indicating whether the distribution is only a full joint
  //! distribution (no conditional or marginalized variables). Its value is
  //! deduced from the conditional flags.
  mutable bool full_joint_distribution;

  //! A boolean indicating whether the input part has changed since last time,
  //! and thus if setInput() needs to be called.
  mutable bool need_set_input;

  mutable Vec input_part;       //!< Used to store the x part in p(y|x).
  mutable Vec target_part;      //!< Used to store the y part in p(y|x).

public:

  // ************************
  // * public build options *
  // ************************

  mutable TVec<int> conditional_flags;
  real lower_bound, upper_bound; 
  int n_curve_points;
  string outputs_def;
  Vec provide_input;

  // ****************
  // * Constructors *
  // ****************

  //! Default constructor.
  PDistribution();

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

  //! simply calls inherited::build() then build_() 
  virtual void build();

  //! Transforms a shallow copy into a deep copy
  virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

  // Declares other standard object methods
  PLEARN_DECLARE_OBJECT(PDistribution);

  // **************************
  // **** PLearner methods ****
  // **************************

  //! Returned value depends on outputs_def.
  virtual int outputsize() const;

  //! (Re-)initializes the PLearner in its fresh state (that state may depend on the 'seed' option)
  //! And sets 'stage' back to 0   (this is the stage of a fresh learner!)
  virtual void forget();

  //! The role of the train method is to bring the learner up to stage==nstages,
  //! updating the train_stats collector with training costs measured on-line in the process.
  virtual void train();

  //! Produce outputs according to what is specified in outputs_def.
  virtual void computeOutput(const Vec& input, Vec& output) const;

  //! Computes negative log likelihood (NLL) 
  //! assuming log-likelihood is first output.  
  virtual void computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                       const Vec& target, Vec& costs) const;

  // *******************************
  // **** PDistribution methods ****
  // *******************************

private:

  //! Set the conditional flags, but does not call updateFromConditionalSorting().
  //! This method is called at build time so that flags information is available
  //! to subclasses during their build. The updateFromConditionalSorting() method
  //! should then be called when the subclass' build ends (this will be done in
  //! finishConditionalBuild()).
  void setConditionalFlagsWithoutUpdate(const TVec<int>& flags) const;

protected:

  //! If the full joint distribution was already the one computed, return
  //! false and old_flags is of length 0.
  //! Otherwise, return true, conditional_flags is emptied (in order to compute
  //! the full joint distribution), with a backup in old_flags.
  bool ensureFullJointDistribution(TVec<int>& old_flags);

  //! Finish the build of a conditional distribution. This method should be called
  //! at the end of the build_() method in a subclass.
  //! It will call updateFromConditionalSorting() and setInput() (if necessary).
  void finishConditionalBuild();

  //! Resize input_part and target_part according to n_input and n_target.
  void resizeParts() const;

  //! Sort a vector or a matrix according to the conditional flags currently
  //! defined. The indices are sorted as follows: input, target, margin.
  //! The vector (or matrix) is assumed to be sorted according to the
  //! previously defined flags.
  void sortFromFlags(Vec& v) const;
  void sortFromFlags(Mat& m, bool sort_columns = true, bool sort_rows = false) const;

  //! Split an input into the part corresponding to the 'real' input (in
  //! 'input_part'), and the target (in 'target_part').
  //! Note that the 'margin' part is lost, since we don't need it.
  //! Return true iff the input part has changed since last time (this is false
  //! only it is absent from input, which can be seen from its length).
  bool splitCond(const Vec& input) const;

  //! Called in computeOutput when an unknown character is found.
  virtual void unknownOutput(char def, const Vec& input, Vec& output, int& k) const;
  
  //! This method updates the internal data given a new sorting of the variables
  //! defined by the conditional flags. The default version does nothing: it
  //! should be implemented in each conditional subclass.
  virtual void updateFromConditionalSorting() const;

public:

  //! Set the conditional flags.
  void setConditionalFlags(const TVec<int>& flags) const;

  //! Set the value for the input part of a conditional probability.
  //! This needs to be implemented in subclasses if there is something
  //! special to do (like precomputing some stuff).
  virtual void setInput(const Vec& input) const;

  //! Overridden so that some stuff is updated according to the conditional
  //! flags when the training set is set.
  virtual void setTrainingSet(VMat training_set, bool call_forget=true);

  //! Return [ "NLL" ].
  virtual TVec<string> getTestCostNames() const;

  //! Return [ ].
  virtual TVec<string> getTrainCostNames() const;

  //! Return log of probability density log(p(y | x)).
  virtual real log_density(const Vec& y) const;

  //! Return probability density p(y | x)
  //! (default version returns exp(log_density(y))).
  virtual real density(const Vec& y) const;
  
  //! Return survival function: P(Y>y | x).
  virtual real survival_fn(const Vec& y) const;

  //! Return cdf: P(Y<y | x).
  virtual real cdf(const Vec& y) const;

  //! Return E[Y | x].
  virtual void expectation(Vec& mu) const;

  //! Return Var[Y | x].
  virtual void variance(Mat& cov) const;

  //! Reset the random number generator used by generate() using the given seed.
  //! Default behavior is to call random.manual_seed(g_seed) and to save the
  //! given seed.
  //! This method is called in build().
  virtual void resetGenerator(long g_seed) const;
  
  //! Return a pseudo-random sample generated from the distribution.
  virtual void generate(Vec& y) const;

  //! X must be a N x inputsize() matrix. that will be filled.
  //! This will call generate N times to fill the N rows of the matrix. 
  void generateN(const Mat& Y) const;

};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(PDistribution);
  
} // end of namespace PLearn

#endif
