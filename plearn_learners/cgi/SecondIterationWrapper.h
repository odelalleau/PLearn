// -*- C++ -*-

// SecondIterationWrapper.h
// Copyright (c) 1998-2002 Pascal Vincent
// Copyright (C) 1999-2002 Yoshua Bengio and University of Montreal
// Copyright (c) 2002 Jean-Sebastien Senecal, Xavier Saint-Mleux, Rejean Ducharme
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


/* ********************************************************************************    
 * $Id: SecondIterationWrapper.h, v 1.0 2004/07/19 10:00:00 Bengio/Kegl/Godbout   *
 * This file is part of the PLearn library.                                     *
 ******************************************************************************** */

/*! \file SecondIterationWrapper.h */

#ifndef SecondIterationWrapper_INC
#define SecondIterationWrapper_INC

#include <plearn_learners/generic/PLearner.h>
#include <plearn/base/stringutils.h>

namespace PLearn {
using namespace std;

class SecondIterationWrapper: public PLearner
{
    typedef PLearner inherited;
  
private:

/*
  Build options: they have to be set before training
*/

    int class_prediction;
    VMat ref_train;
    VMat ref_test;
    VMat ref_sales;
    VMat train_dataset;
    VMat test_dataset;
    PP<PLearner> base_regressor_template;                     // template for a generic regressor as the base learner to be boosted 
  
/*
  Learnt options: they are sized and initialized if need be, at stage 0
*/


    PP<PLearner> base_regressor;                              // base regressors built at each boosting stage 
 
/*
  Work fields: they are sized and initialized if need be, at buid time
*/ 
    int margin;
    int loan;
    int sales;
    int tclass;
    Mat search_table;
    
    Vec sample_input;
    Vec sample_target;
    Vec sample_output;
    Vec sample_costs;

  
public:
    SecondIterationWrapper();
    virtual              ~SecondIterationWrapper();
    
    PLEARN_DECLARE_OBJECT(SecondIterationWrapper);

    static  void         declareOptions(OptionList& ol);
    virtual void         makeDeepCopyFromShallowCopy(CopiesMap &copies);
    virtual void         build();
    virtual void         train();
    virtual int          outputsize() const;
    virtual TVec<string> getTrainCostNames() const;
    virtual TVec<string> getTestCostNames() const;
    virtual void         computeOutput(const Vec& input, Vec& output) const;
    virtual void         computeOutputAndCosts(const Vec& input, const Vec& target, Vec& output, Vec& costs) const;
    virtual void         computeCostsFromOutputs(const Vec& input, const Vec& output, const Vec& target, Vec& costs) const;
  
private:
    void         build_();
    void         computeClassStatistics();
    void         computeSalesStatistics();
    real         deGaussianize(real prediction);
};

DECLARE_OBJECT_PTR(SecondIterationWrapper);

} // end of namespace PLearn

#endif


/*
  Local Variables:
  mode:c++
  c-basic-offset:4
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:79
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=79 :
