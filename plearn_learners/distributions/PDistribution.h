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
   * $Id: PDistribution.h,v 1.12 2004/05/26 18:39:42 tihocan Exp $ 
   ******************************************************* */

/*! \file PDistribution.h */
#ifndef PDistribution_INC
#define PDistribution_INC

#include "PLearner.h"

namespace PLearn {
using namespace std;

class PDistribution: public PLearner
{

private:

  typedef PLearner inherited;

protected:

  // *********************
  // * protected options *
  // *********************

  TVec<int> cond_sort;
  int n_input;
  int n_margin;
  int n_target;

  // Fields below are not options.

  //! A boolean indicating whether the input, target and margin part are
  //! already sorted nicely, so we actually don't have to swap indices
  //! when given a new vector.
  bool already_sorted;

  //! A vector indicating which indices need to be swapped when we have modified
  //! the conditional flags, in order to still have input, target, margin in this
  //! order. It is made of pairs (j,k) indicating that the new j-th variable
  //! must be the old k-th variable.
  TVec<int> cond_swap;

  //! The step when plotting the curve (upper case outputs_def).
  real delta_curve;

  //! A boolean indicating whether the distribution is only a full joint
  //! distribution (no conditional or marginalized variables). Its value is
  //! deduced from the conditional flags.
  bool full_joint_distribution;

  mutable Vec target_part;      //!< Used to store the y part in p(y|x).

  //! A boolean indicating whether the input part has changed since last time,
  //! and thus if setInput() needs to be called.
  mutable bool need_set_input;

public:

  // ************************
  // * public build options *
  // ************************

  TVec<int> conditional_flags;

  mutable Vec input_part;

  // Interval of values for computing histrogram output (upper case outputs_def).
  real lower_bound, upper_bound; 

  // TODO
  // number of (histogram) curve points if outputs_def is upper case.
  int n_curve_points;

  /* TODO
  //! A string where the characters have the following meaning:
  //! 'l'->log_density, 'd' -> density, 'c' -> cdf, 's' -> survival_fn
  //! (subclasses may define more uses, such as: 'e' -> expectation, 'v' -> variance)
  //! Upper case produces the whole curve in output.
  */
  string outputs_def;

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
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

  // Declares other standard object methods
  PLEARN_DECLARE_OBJECT(PDistribution);

  // **************************
  // **** PLearner methods ****
  // **************************

  //! returned value depends on outputs_def
  virtual int outputsize() const;

  //! (Re-)initializes the PLearner in its fresh state (that state may depend on the 'seed' option)
  //! And sets 'stage' back to 0   (this is the stage of a fresh learner!)
  virtual void forget();

  //! The role of the train method is to bring the learner up to stage==nstages,
  //! updating the train_stats collector with training costs measured on-line in the process.
  virtual void train();

  //! Produces outputs according to what is specified in outputs_def
  virtual void computeOutput(const Vec& input, Vec& output) const;

  //! Computes negative log likelihood (NLL) 
  //! assuming log-likelihood is first output.  
  virtual void computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                       const Vec& target, Vec& costs) const;

  // *******************************
  // **** PDistribution methods ****
  // *******************************

  //! If the full joint distribution was already the one computed, return
  //! false and old_flags is of length 0.
  //! Otherwise, return true, conditional_flags is emptied (in order to compute
  //! the full joint distribution), with a backup in old_flags.
  bool ensureFullJointDistribution(TVec<int>& old_flags);

  //! Resize input_part and target_part according to n_input and n_target.
  void resizeParts();

  //! Set the conditional flags.
  void setConditionalFlags(TVec<int> flags);

  //! Set the value for the input part of a conditional probability.
  //! This needs to be implemented in subclasses if there is something
  //! special to do (like precomputing some stuff).
  virtual void setInput(const Vec& input) const;

  //! Overridden so that some stuff is updated according to the conditional
  //! flags when the training set is set.
  virtual void setTrainingSet(VMat training_set, bool call_forget=true);

  //! Sort a vector or a matrix according to the conditional flags currently
  //! defined. The indices are sorted as follows: input, target, margin.
  //! The vector (or matrix) is assumed to be sorted according to the
  //! previously defined flags.
  void sortFromFlags(Vec& v);
  void sortFromFlags(Mat& m, bool sort_columns = true, bool sort_rows = false);

  //! Split an input into the part corresponding to the 'real' input, and the
  //! target. Note that the 'margin' part is lost, since we don't need it.
  //! Return true iff the input part has changed since last time (this is false
  //! only it is absent from input, which can be seen from its length).
  bool splitCond(const Vec& input, Vec& input_part, Vec& target_part) const;
  
  //! This method updates the internal data given a new sorting of the variables
  //! defined by the conditional flags. The default version does nothing: it
  //! should be implemented in each conditional subclass.
  virtual void updateFromConditionalSorting();

  //! Returns [ "NLL" ]
  virtual TVec<string> getTestCostNames() const;

  //! Returns [ "NLL" ]
  virtual TVec<string> getTrainCostNames() const;

  //! return log of probability density log(p(x))
  virtual real log_density(const Vec& x) const;

  //! return probability density p(x)
  //! [ default version returns exp(log_density(x)) ]
  virtual real density(const Vec& x) const;
  
  //! return survival function: P(X>x)
  virtual real survival_fn(const Vec& x) const;

  //! return cdf: P(X<x)
  virtual real cdf(const Vec& x) const;

  //! return E[X] 
  virtual void expectation(Vec& mu) const;

  //! return Var[X]
  virtual void variance(Mat& cov) const;

  //! Resets the random number generator used by generate using the given seed
  virtual void resetGenerator(long g_seed) const;
  
  //! return a pseudo-random sample generated from the distribution.
  virtual void generate(Vec& x) const;

  //! X must be a N x inputsize() matrix. that will be filled
  //! This will call generate N times to fill the N rows of the matrix. 
  void generateN(const Mat& X) const;

};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(PDistribution);
  
} // end of namespace PLearn

#endif
