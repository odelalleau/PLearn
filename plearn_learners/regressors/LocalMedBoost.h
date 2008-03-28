// -*- C++ -*-

// LocalMedBoost.h
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
 * $Id: LocalMedBoost.h, v 1.0 2004/07/19 10:00:00 Bengio/Kegl/Godbout          *
 * This file is part of the PLearn library.                                     *
 ******************************************************************************** */

#ifndef LocalMedBoost_INC
#define LocalMedBoost_INC

#include <plearn_learners/generic/PLearner.h>

namespace PLearn {
using namespace std;
class RegressionTree;
class BaseRegressorWrapper;
class RegressionTreeRegisters;

class LocalMedBoost: public PLearner
{
    typedef PLearner inherited;
private:

/*
  Build options: they have to be set before training
*/

    real robustness;                                   // robustness parameter of the boosting algorithm
    real adapt_robustness_factor;                      // if not 0.0, robustness will be adapted at each stage with max(t)min(i) base_award + this constant
    real loss_function_weight;                         // hyper-parameter to balance the error and the confidence factor
    string objective_function;                         // indicates to used l2 or flaten_l2 as base regressor objective function
    int regression_tree;                               // indicator to use the tree_regressor_template if set to 1, and the base_regressor_template otherwise
    int max_nstages;                                   // maximum number of nstages in the hyper learner to size the vectors of base learners
    PP<PLearner> base_regressor_template;              // template for a generic regressor as the base learner to be boosted
    PP<RegressionTree> tree_regressor_template;        // template for a tree regressor to be boosted as the base regressor
    PP<BaseRegressorWrapper> tree_wrapper_template;    // template for a tree regressor to be boosted thru a wrapper for a different confidence function
  
/*
  Learnt options: they are sized and initialized if need be, at stage 0
*/

    int end_stage;                                      // last train stage after end of training
    real bound;                                         // cumulative bound computed after each boosting stage
    real maxt_base_award;                               // max(t)min(i) base_award to adapt robustness at each stage
    PP<RegressionTreeRegisters> sorted_train_set;       // a sorted train set when using a tree as a base regressor
    TVec< PP<PLearner> > base_regressors;               // base regressors built at each boosting stage 
    TVec< PP<RegressionTree> > tree_regressors;         // tree regressors built at each boosting stage
    TVec< PP<BaseRegressorWrapper> > tree_wrappers;     // tree regressors built at each boosting stage
    TVec<real> function_weights;                        // array of function weights built by the boosting algorithm 
    TVec<real> loss_function;                           // array of the loss function
    TVec<real> sample_weights;                          // array to represent different distributions on the samples of the training set
 
/*
  Work fields: they are sized and initialized if need be, at buid time
*/  

    int each_train_sample_index;                   // index to go thru the train set  
    int length;                                    // number of samples in train set
    int width;                                     // number of columns in train set
    int inputsize;                                 // input size of train set
    int targetsize;                                // output size of train set
    int weightsize;                                // weightsize size of train set
    bool capacity_too_large;                       // early stop of the algorithm because capacity is too large
    bool capacity_too_small;                       // early stop of the algorithm because capacity is too small
    real edge;                                     // computed sum of the weighted base rewards
    real min_margin;                               // minimum margin for all samples
    Vec sample_input;                              // vector to hold the sample input vector
    Vec sample_target;                             // vector to hold the sample target
    real sample_weight;                            // real to hold the sample weight
    Vec sample_output;                             // vector to compute output from the base regressor and from boosting
    Vec sample_costs;                              // vector to compute cost from boosting for one sample
    TVec<real> base_rewards;                       // vector to compute the base reward of all samples after each boosting stage
    TVec<real> base_confidences;                   // vector to keep the base confidence of all samples after each booosting stage
    TVec<real> base_awards;                        // vector to comput the base awad of all samples after each boosting stage
    TVec<real> exp_weighted_edges;                 // vector to precompute the exp of the weighted edges for all samples after each boosting stages
    real sum_exp_weighted_edges;                   // to sum the exp of the weighted edges for all samples after each boosting stages

/*
  Work fields for the line search: they are sized and initialized if need be, at buid time
*/      
    
    real bracketing_learning_rate;              // various parameter of the line search initialised at the onset
    real bracketing_zero;                       // more of the same
    real interpolation_learning_rate;           // more of the same
    real interpolation_precision;               // more of the same
    real max_learning_rate;                     // more of the same
    real bracket_a_start;                       // more of the same
    real bracket_b_start;                       // more of the same
    int  iter;                                  // iteration counter of the line search
    real x_a, x_b, x_c, x_d, x_e;               // absissa value of various points thru the interpolation of the search
    real x_u, x_v, x_w, x_x;                    // more of the same
    real x_xmed, x_lim;                         // more of the same
    real f_a, f_b, f_c;                         // corresponding evaluation of the function to minimize
    real f_u, f_v, f_w, f_x;                    // more of the same
    real t_p, t_q, t_r, t_sav;                  // intermediary calculation thru the interpolation
    real p_step, p_lim, p_tin;                  // various parameters set thru the interpolation
    real p_to1, p_tol1, p_tol2;                 // more of the same
  
public:
    LocalMedBoost();
    virtual              ~LocalMedBoost();
   
    PLEARN_DECLARE_OBJECT(LocalMedBoost);

    static  void         declareOptions(OptionList& ol);
    virtual void         makeDeepCopyFromShallowCopy(CopiesMap& copies);
    virtual void         build();
    virtual void         train();
    virtual void         forget();
    virtual int          outputsize() const;
    virtual TVec<string> getTrainCostNames() const;
    virtual TVec<string> getTestCostNames() const;
    virtual void         computeOutput(const Vec& input, Vec& output) const;
    virtual void         computeOutputAndCosts(const Vec& input, const Vec& target, Vec& output, Vec& costs) const;
    virtual void         computeCostsFromOutputs(const Vec& input, const Vec& output,
                                                 const Vec& target, Vec& costs) const;
private:
    void         build_();
    void         computeBaseAwards();
    void         computeLossBound();
    real         findArgminFunctionWeight();
    void         initializeLineSearch();
    real         computeFunctionWeightFormula(real alpha);
    void         recomputeSampleWeight();
    void         initializeSampleWeight();
    void         verbose(string the_msg, int the_level);
};

DECLARE_OBJECT_PTR(LocalMedBoost);

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
