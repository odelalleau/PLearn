// -*- C++ -*-

// SequentialLearner.h
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



#ifndef SEQUENTIAL_LEARNER
#define SEQUENTIAL_LEARNER

#include "PLearner.h"

namespace PLearn <%
using namespace std;

/*!
   We use the following conventions throughout this module:
  
     - The last VMat received by the train(...) method is in the range [0,last_train_t-1]
     - The last VMat received by the test(....) method is in the range [0,last_test_t-1]
     - All the VMat's in a SequentialLearner have all their informations at column t
       available at time t.  Hence, for a dataset of length T, there is only T-horizon
       effective examples, with the (input, target) pairs starting at (0, horizon)
       and ending at (T-1-horizon, T-1)
*/

class SequentialLearner: public PLearner
{
  protected:
 
    int last_train_t; // last value of train_set.length() for which training was actually done (<= last_call_train_t)
  int last_call_train_t; // last value of train_set.length() for which train() was called (may not have done anything)
    mutable int last_test_t; // last value of test_set.length() for which testing was actually done

  public:

    int max_seq_len; // max length of the VMat that train can contain = max de t ci-haut
    int max_train_len; // max nb of (input,target) pairs actually used for training
    int train_step; // how often we have to re-train a model, (default = 1 = after every time step)
    int horizon; // by how much to offset the target columns wrt the input columns (default = 1)
    int outputsize_;

    // these two fields are used by other classes such as SequentialModelSelector
    // and SequentialValidation and they are filled when the method test is called 
    Mat predictions; // each element indexed by (time_index, output_index), there are (max_seq_len,outputsize) elements.
    // initial values may be 'missing value' 
    Mat errors; // each element indexed by (time_index, cost_index), there are (max_seq_len,nCosts) elements.
  
  //! This field is mainly used in financial learner such as FuturesTrader to get AND modify portofolios' positions during test
  mutable Mat state;

  private:
    //! This does the actual building
    void build_();

  protected:
    //! Declare this class' options
    static void declareOptions(OptionList& ol);

  public:

    //! Constructor
    SequentialLearner();

    //! simply calls inherited::build() then build_()
    virtual void build();

    //! Default behaviour: return train_set->targetsize()
    virtual int outputsize() const;
  
/*!       *** SUBCLASS WRITING: ***
      Does the actual training. Subclasses must implement this method.
      The method should upon entry, call setTrainingSet(training_set);
      The method should:
        - do nothing if we already called it with this value of train.length
          or a value between [last_train_t-train_step+1,last_train_t]
        - if not, train and update the value of last_train_t
        - in either case, update the value of last_call_train_t
*/
    virtual void train() =0;
 
/*!       *** SUBCLASS WRITING: ***
      The method should:
        - call computeOutputAndCosts on the test set
        - save the outputs and the costs in the  predictions & errors
          matrices, beginning at position last_call_train_t
*/
    virtual void test(VMat testset, PP<VecStatsCollector> test_stats,
        VMat testoutputs=0, VMat testcosts=0) const =0;

    virtual void computeOutputAndCosts(const Vec& input, const Vec& target,
        Vec& output, Vec& costs) const;

    virtual void computeCostsOnly(const Vec& input, const Vec& target,
        Vec& costs) const;

    virtual void computeOutput(const Vec& input, Vec& output) const;

    virtual void computeCostsFromOutputs(const Vec& input, const Vec& output,
        const Vec& target, Vec& costs) const;
  
    virtual void forget();

    //!  Does the necessary operations to transform a shallow copy (this)
    //!  into a deep copy by deep-copying all the members that need to be.
    PLEARN_DECLARE_ABSTRACT_OBJECT(SequentialLearner);
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);
};

//! Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(SequentialLearner);

%> // end of namespace PLearn

#endif
