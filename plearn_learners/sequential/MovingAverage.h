// -*- C++ -*-

// MovingAverage.h
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



#ifndef MOVING_AVERAGE
#define MOVING_AVERAGE

#include "SequentialLearner.h"

namespace PLearn <%
using namespace std;

//! This SequentialLearner only takes the n previous target to predict the
//! next one.  This is a memoryless learner!

class MovingAverage: public SequentialLearner
{
  public:

    int window_length; // the length of the window on which to calculate the average
    Array<string> cost_funcs; // a list of cost functions (only MSE for now)

  private:
    //! This does the actual building
    void build_();

  protected:
    //! Declare this class' options
    static void declareOptions(OptionList& ol);

  public:

    //! Constructor
    MovingAverage();

    //! simply calls inherited::build() then build_()
    virtual void build();
    
    virtual void train();
 
    virtual void test(VMat testset, PP<VecStatsCollector> test_stats,
        VMat testoutputs=0, VMat testcosts=0) const;

    virtual void computeCostsFromOutputs(const Vec& inputs, const Vec& outputs,
        const Vec& targets, Vec& costs) const;

    virtual TVec<string> getTrainCostNames() const;
    virtual TVec<string> getTestCostNames() const;

    virtual void forget();

    //!  Does the necessary operations to transform a shallow copy (this)
    //!  into a deep copy by deep-copying all the members that need to be.
    typedef SequentialLearner inherited;
    PLEARN_DECLARE_OBJECT(MovingAverage);
    //virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);
};

//! Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(MovingAverage);

%> // end of namespace PLearn

#endif
