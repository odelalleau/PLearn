// -*- C++ -*-

// EmbeddedSequentialLearner.h
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



#ifndef EMBEDDED_SEQUENTIAL_LEARNER
#define EMBEDDED_SEQUENTIAL_LEARNER

#include "SequentialLearner.h"

namespace PLearn <%
using namespace std;

/*! This SequentialLearner simply embeddes a Learner that we wish
    to train sequentially.  Most of the methods simply forward the call
    to the underlying Learner.
*/

class EmbeddedSequentialLearner: public SequentialLearner
{
  public:

    typedef SequentialLearner inherited;

    PP<PLearner> learner;  // the underlying Learner

  private:
    //! This does the actual building
    void build_();

  protected:
    //! Declare this class' options
    static void declareOptions(OptionList& ol);

  public:

    //! Constructor
    EmbeddedSequentialLearner();

    //! simply calls inherited::build() then build_()
    virtual void build();
    
    //!  Does the actual training.
    virtual void train(VecStatsCollector& train_stats);
 
/*!       *** SUBCLASS WRITING: ***
      The method should:
        - call computeOutputAndCosts on the test set
        - save the outputs and the costs in the  predictions & errors
          matrices, beginning at position last_call_train_t
*/
    virtual void test(VMat testset, VecStatsCollector& test_stats,
        VMat testoutputs=0, VMat testcosts=0);

    //!  Does the necessary operations to transform a shallow copy (this)
    //!  into a deep copy by deep-copying all the members that need to be.
    DECLARE_NAME_AND_DEEPCOPY(EmbeddedSequentialLearner);
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    // PLearner methods to be passed to the underlying Learner

    virtual void forget();
 
    virtual void computeOutput(const VVec& input, Vec& output);
 
    virtual void computeCostsFromOutputs(const VVec& input, const Vec& output,
        const VVec& target, const VVec& weight, Vec& costs);
 
    virtual void computeOutputAndCosts(const VVec& input, VVec& target,
        const VVec& weight, Vec& output, Vec& costs);
 
    virtual void computeCostsOnly(const VVec& input, VVec& target, VVec& weight,
        Vec& costs);

 
    virtual TVec<string> getTestCostNames() const;
 
    virtual TVec<string> getTrainCostNames() const;


    // old Learner methods to be passed to the underlying Learner

/*
    virtual void computeCost(const Vec& input, const Vec& target,
        const Vec& output, const Vec& cost);

    virtual void computeCosts(const VMat& data, VMat costs);
 
    virtual void useAndCost(const Vec& input, const Vec& target,
        Vec output, Vec cost);

    virtual void useAndCostOnTestVec(const VMat& test_set, int i,
        const Vec& output, const Vec& cost);

    virtual void apply(const VMat& data, VMat outputs);

    virtual void applyAndComputeCosts(const VMat& data, VMat outputs, VMat costs);

    virtual void applyAndComputeCostsOnTestMat(const VMat& test_set,
        int i, const Mat& output_block, const Mat& cost_block);

    virtual void computeLeaveOneOutCosts(const VMat& data, VMat costs);

    virtual void computeLeaveOneOutCosts(const VMat& data, VMat costsmat,
        CostFunc costf);

    virtual Array<string> costNames() const;

    virtual Array<string> testResultsNames() const;

    virtual Array<string> trainObjectiveNames() const;
*/
};

//! Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(EmbeddedSequentialLearner);

%> // end of namespace PLearn

#endif
