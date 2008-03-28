// -*- C++ -*-

// BaseRegressorWrapper.h
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
 * $Id: BaseRegressorWrapper.h, v 1.0 2004/07/19 10:00:00 Bengio/Kegl/Godbout   *
 * This file is part of the PLearn library.                                     *
 ******************************************************************************** */

/*! \file PLearnLibrary/PLearnAlgo/BaseRegressorWrapper.h */

#ifndef BaseRegressorWrapper_INC
#define BaseRegressorWrapper_INC

#include <plearn_learners/generic/PLearner.h>

namespace PLearn {
using namespace std;
class RegressionTree;
class BaseRegressorConfidence;
class RegressionTreeRegisters;

class BaseRegressorWrapper: public PLearner
{
    typedef PLearner inherited;
  
private:

/*
  Build options: they have to be set before training
*/

    real loss_function_weight;                                // the hyper parameter to balance the error and the confidence factor
    int  mean_imputation;                                     // if set to 1, the algorithm will perform mean imputation based on the training set
    int regression_tree;                                      // indicator to use the tree_regressor_template is set to 1, and the base_regressor_template otherwise
    int use_confidence_function;                              // indicator to use the confidence_function_template to build a confidence estimate
    int use_base_regressor_confidence;                        // indicator to use the confidence computed as the second output of the base regressor
    PP<PLearner> base_regressor_template;                     // template for a generic regressor as the base learner to be boosted 
    PP<RegressionTree> tree_regressor_template;               // template for a tree regressor to be boosted as the base regressor
    PP<BaseRegressorConfidence> confidence_function_template; // template for the confidence function to be learnt from the train set 
    PP<RegressionTreeRegisters> sorted_train_set;             // a sorted train set when using a regression tree
  
/*
  Learnt options: they are sized and initialized if need be, at stage 0
*/


    PP<PLearner> base_regressor;                              // base regressors built at each boosting stage 
    PP<RegressionTree> tree_regressor;                        // base regressors built at each boosting stage 
    PP<BaseRegressorConfidence> confidence_function;          // confidence function learnt from the train set
    VMat base_regressor_train_set;                            // a train set with the chosen missing value strategies applied for the base regressor
    TVec<real> variable_means;                                // the vector of variable means when doing mean imputation
 
/*
  Work fields: they are sized and initialized if need be, at buid time
*/ 
 
   
  
public:
    BaseRegressorWrapper();
    virtual              ~BaseRegressorWrapper();
    
    PLEARN_DECLARE_OBJECT(BaseRegressorWrapper);

    static  void         declareOptions(OptionList& ol);
    virtual void         makeDeepCopyFromShallowCopy(CopiesMap &copies);
    virtual void         build();
    virtual void         train();
    virtual void         forget();
    virtual int          outputsize() const;
    virtual TVec<string> getTrainCostNames() const;
    virtual TVec<string> getTestCostNames() const;
    virtual void         computeOutput(const Vec& input, Vec& output) const;
    virtual void         computeOutputAndCosts(const Vec& input, const Vec& target, Vec& output, Vec& costs) const;
    virtual void         computeCostsFromOutputs(const Vec& input, const Vec& output, const Vec& target, Vec& costs) const;
    void         setSortedTrainSet(PP<RegressionTreeRegisters> the_sorted_train_set);
  
private:
    void         build_();
    void         verbose(string msg, int level);
};

DECLARE_OBJECT_PTR(BaseRegressorWrapper);

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
