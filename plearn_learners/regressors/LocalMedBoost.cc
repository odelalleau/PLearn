// -*- C++ -*-

// LocalMedBoost.cc
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
 * $Id: LocalMedBoost.cc, v 1.0 2004/07/19 10:00:00 Bengio/Kegl/Godbout              *
 * This file is part of the PLearn library.                                     *
 ******************************************************************************** */

#include "LocalMedBoost.h"
#include "RegressionTree.h"
#include "RegressionTreeRegisters.h"
#include "BaseRegressorWrapper.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(LocalMedBoost,
                        "Confidence-rated Regression by Localised Median Boosting",
                        "Robust regression by boosting the median\n"
                        "It implements the algorithm described in the paper: Robust Regression by Boosting the Median from professor Kegl.\n"
                        "It requires a base regressor that can separate a confidence function from the real output.\n"
                        "The base regressor must minimize a loss function in the form described in the paper.\n"
                        "It needs a loss_function_weight parameter used by the base regressor to computes its loss function.\n"
                        "Currently, a RegressionTree algorithm is implemented to serve as a base regressor.\n"
    );

LocalMedBoost::LocalMedBoost()     
    : robustness(0.1),
      adapt_robustness_factor(0.0),
      loss_function_weight(1.0),
      objective_function("l2"),
      regression_tree(1),
      max_nstages(1)
{
}

LocalMedBoost::~LocalMedBoost()
{
}

void LocalMedBoost::declareOptions(OptionList& ol)
{
    declareOption(ol, "robustness", &LocalMedBoost::robustness, OptionBase::buildoption,
                  "The robustness parameter of the boosting algorithm.\n");
    declareOption(ol, "adapt_robustness_factor", &LocalMedBoost::adapt_robustness_factor, OptionBase::buildoption,
                  "If not 0.0, robustness will be adapted at each stage with max(t)min(i) base_award + this constant.\n");
    declareOption(ol, "loss_function_weight", &LocalMedBoost::loss_function_weight, OptionBase::buildoption,
                  "The hyper parameter to balance the error and the confidence factor\n");  
    declareOption(ol, "objective_function", &LocalMedBoost::objective_function, OptionBase::buildoption,
                  "Indicates which of the base reward to use. default is l2 and the other posibility is l1.\n"
                  "Normally it should be consistent with the objective function optimised by the base regressor.\n"); 
    declareOption(ol, "regression_tree", &LocalMedBoost::regression_tree, OptionBase::buildoption,
                  "If set to 1, the tree_regressor_template is used instead of the base_regressor_template.\n"
                  "It permits to sort the train set only once for all boosting iterations.\n");   
    declareOption(ol, "max_nstages", &LocalMedBoost::max_nstages, OptionBase::buildoption,
                  "Maximum number of nstages in the hyper learner to size the vectors of base learners.\n"
                  "(If smaller than nstages, nstages is used)");
    declareOption(ol, "base_regressor_template", &LocalMedBoost::base_regressor_template, OptionBase::buildoption,
                  "The template for the base regressor to be boosted (used if the regression_tree option is set to 0).\n");   
    declareOption(ol, "tree_regressor_template", &LocalMedBoost::tree_regressor_template, OptionBase::buildoption,
                  "The template for a RegressionTree base regressor when the regression_tree option is set to 1.\n");  
    declareOption(ol, "tree_wrapper_template", &LocalMedBoost::tree_wrapper_template, OptionBase::buildoption,
                  "The template for a RegressionTree base regressor to be boosted thru a wrapper."
                  "This is useful when you want to used a different confidence function."
                  "The regression_tree option needs to be set to 2.\n");
 
    declareOption(ol, "end_stage", &LocalMedBoost::end_stage, OptionBase::learntoption,
                  "The last train stage after end of training\n");
    declareOption(ol, "bound", &LocalMedBoost::bound, OptionBase::learntoption,
                  "Cumulative bound computed after each boosting stage\n");
    declareOption(ol, "maxt_base_award", &LocalMedBoost::maxt_base_award, OptionBase::learntoption,
                  "max(t)min(i) base_award kept to adapt robustness at each stage.\n");
    declareOption(ol, "sorted_train_set", &LocalMedBoost::sorted_train_set, OptionBase::learntoption,
                  "A sorted train set when using a tree as a base regressor\n");
    declareOption(ol, "base_regressors", &LocalMedBoost::base_regressors, OptionBase::learntoption,
                  "The vector of base regressors built by the training at each boosting stage\n");
    declareOption(ol, "function_weights", &LocalMedBoost::function_weights, OptionBase::learntoption,
                  "The array of function weights built by the boosting algorithm\n");
    declareOption(ol, "loss_function", &LocalMedBoost::loss_function, OptionBase::learntoption,
                  "The array of loss_function values built by the boosting algorithm\n");
    declareOption(ol, "sample_weights", &LocalMedBoost::sample_weights, OptionBase::learntoption,
                  "The array to represent different distributions on the samples of the training set.\n");
    inherited::declareOptions(ol);
}

void LocalMedBoost::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(robustness, copies);
    deepCopyField(adapt_robustness_factor, copies);
    deepCopyField(loss_function_weight, copies);
    deepCopyField(objective_function, copies);
    deepCopyField(regression_tree, copies);
    deepCopyField(max_nstages, copies);
    deepCopyField(base_regressor_template, copies);
    deepCopyField(tree_regressor_template, copies);
    deepCopyField(tree_wrapper_template, copies);
    deepCopyField(end_stage, copies);
    deepCopyField(bound, copies);
    deepCopyField(maxt_base_award, copies);
    deepCopyField(sorted_train_set, copies);
    deepCopyField(base_regressors, copies);
    deepCopyField(function_weights, copies);
    deepCopyField(loss_function, copies);
    deepCopyField(sample_weights, copies);
}

void LocalMedBoost::build()
{
    inherited::build();
    build_();
}

void LocalMedBoost::build_()
{
    if (train_set)
    {
        length = train_set->length();
        width = train_set->width();
        if (length < 2) PLERROR("LocalMedBoost: the training set must contain at least two samples, got %d", length);
        inputsize = train_set->inputsize();
        targetsize = train_set->targetsize();
        weightsize = train_set->weightsize();
        if (inputsize < 1) PLERROR("LocalMedBoost: expected  inputsize greater than 0, got %d", inputsize);
        if (targetsize != 1) PLERROR("LocalMedBoost: expected targetsize to be 1, got %d", targetsize);
        if (weightsize != 1) PLERROR("LocalMedBoost: expected weightsize to be 1, got %d", weightsize);
        sample_input.resize(inputsize);
        sample_target.resize(targetsize);
        sample_output.resize(4);
        sample_costs.resize(6);
        sample_weights.resize(length);
        base_rewards.resize(length);
        base_confidences.resize(length);
        base_awards.resize(length);
        exp_weighted_edges.resize(length);
        if (max_nstages < nstages) max_nstages = nstages;
    } 
}

void LocalMedBoost::train()
{
    if (!train_set) PLERROR("LocalMedBoost: the learner has not been properly built");
    if (stage == 0)
    {
        base_regressors.resize(max_nstages);
        tree_regressors.resize(max_nstages);
        tree_wrappers.resize(max_nstages);
        function_weights.resize(max_nstages);
        loss_function.resize(max_nstages);
        initializeSampleWeight();
        initializeLineSearch();
        bound = 1.0;
        if (regression_tree > 0)
            sorted_train_set = new RegressionTreeRegisters(train_set,
                                                           report_progress,
                                                           verbosity);
    }
    PP<ProgressBar> pb;
    if (report_progress) pb = new ProgressBar("LocalMedBoost: train stages: ", nstages);
    for (; stage < nstages; stage++)
    {
        verbose("LocalMedBoost: The base regressor is being trained at stage: " + tostring(stage), 4);
        if (regression_tree > 0)
        {
            if (regression_tree == 1)
            {
                tree_regressors[stage] = ::PLearn::deepCopy(tree_regressor_template);
                tree_regressors[stage]->setTrainingSet(VMat(sorted_train_set));
                base_regressors[stage] = tree_regressors[stage];
            }
            else
            {
                tree_wrappers[stage] = ::PLearn::deepCopy(tree_wrapper_template);
                tree_wrappers[stage]->setSortedTrainSet(sorted_train_set);
                base_regressors[stage] = tree_wrappers[stage];
            }
        }
        else
        {
            base_regressors[stage] = ::PLearn::deepCopy(base_regressor_template);
        }
        base_regressors[stage]->setOption("loss_function_weight", tostring(loss_function_weight));
        base_regressors[stage]->setTrainingSet(train_set, true);
        base_regressors[stage]->setTrainStatsCollector(new VecStatsCollector);
        base_regressors[stage]->train();
        end_stage = stage + 1;
        computeBaseAwards();
        if (capacity_too_large)
        {
            verbose("LocalMedBoost: capacity too large, each base awards smaller than robustness: " + tostring(robustness), 2);
        }
        if (capacity_too_small)
        {
            verbose("LocalMedBoost: capacity too small, edge: " + tostring(edge), 2);
        }
        function_weights[stage] = findArgminFunctionWeight();
        computeLossBound();
        verbose("LocalMedBoost: stage: " + tostring(stage) + " alpha: " + tostring(function_weights[stage]) + " robustness: " + tostring(robustness), 3);
        if (function_weights[stage] <= 0.0) break;
        recomputeSampleWeight();
        if (report_progress) pb->update(stage);
    }
    if (report_progress)
    {
        pb = new ProgressBar("LocalMedBoost : computing the statistics: ", train_set->length());
    } 
    train_stats->forget();
    min_margin = 1E15;
    for (each_train_sample_index = 0; each_train_sample_index < train_set->length(); each_train_sample_index++)
    {  
        train_set->getExample(each_train_sample_index, sample_input, sample_target, sample_weight);
        computeOutput(sample_input, sample_output);
        computeCostsFromOutputs(sample_input, sample_output, sample_target, sample_costs); 
        train_stats->update(sample_costs);
        if (sample_costs[5] < min_margin) min_margin = sample_costs[5];
        if (report_progress) pb->update(each_train_sample_index);
    }
    train_stats->finalize();
    verbose("LocalMedBoost: we are done, thank you!", 3);
}

void LocalMedBoost::computeBaseAwards()
{ 
    edge = 0.0;
    capacity_too_large = true;
    capacity_too_small = true;
    real mini_base_award = INT_MAX;
    int sample_costs_index;
    if (objective_function == "l1") sample_costs_index=3;
    else sample_costs_index=2;

    for (each_train_sample_index = 0; each_train_sample_index < length; each_train_sample_index++)
    {
        train_set->getExample(each_train_sample_index, sample_input, sample_target, sample_weight);
        base_regressors[stage]->computeOutputAndCosts(sample_input, sample_target, sample_output, sample_costs);
        base_rewards[each_train_sample_index] = sample_costs[sample_costs_index];

        base_confidences[each_train_sample_index] = sample_costs[1];
        base_awards[each_train_sample_index] = base_rewards[each_train_sample_index] * base_confidences[each_train_sample_index];
        if (base_awards[each_train_sample_index] < mini_base_award) mini_base_award = base_awards[each_train_sample_index];
        edge += sample_weight * base_awards[each_train_sample_index];
        if (base_awards[each_train_sample_index] < robustness) capacity_too_large = false;
    }
    if (stage == 0) maxt_base_award = mini_base_award;
    if (mini_base_award > maxt_base_award) maxt_base_award = mini_base_award;
    if (adapt_robustness_factor > 0.0)
    {
        robustness = maxt_base_award + adapt_robustness_factor;
        capacity_too_large = false;
    }
    if (edge >= robustness)
    {
        capacity_too_small = false;
    }
}

void LocalMedBoost::computeLossBound()
{ 
    loss_function[stage] = computeFunctionWeightFormula(function_weights[stage]);
    bound *= loss_function[stage];
}

void LocalMedBoost::initializeLineSearch()
{
    bracketing_learning_rate = 1.618034;
    bracketing_zero = 1.0e-10;
    interpolation_learning_rate = 0.381966;
    interpolation_precision = 1.0e-5;
    max_learning_rate = 100.0;
    bracket_a_start = 0.0;
    bracket_b_start = 1.0;
}

real LocalMedBoost::findArgminFunctionWeight()
{
    p_step = bracketing_learning_rate;
    p_lim = max_learning_rate;
    p_tin = bracketing_zero;
    x_a = bracket_a_start;
    x_b = bracket_b_start;
    f_a = computeFunctionWeightFormula(x_a);
    f_b = computeFunctionWeightFormula(x_b);
    x_lim = 0.0;
    if (f_b > f_a)
    {
        t_sav = x_a; x_a = x_b; x_b = t_sav;
        t_sav = f_a; f_a = f_b; f_b = t_sav;
    }
    x_c = x_b + p_step * (x_b - x_a);
    f_c = computeFunctionWeightFormula(x_c);
    while (f_b > f_c)
    {
        t_r = (x_b - x_a) * (f_b - f_c);
        t_q = (x_b - x_c) * (f_b - f_a);
        t_sav = t_q - t_r;
        if (t_sav < 0.0)
        {
            t_sav *= -1.0;
            if (t_sav < p_tin)
            {
                t_sav = p_tin;
            }
            t_sav *= -1.0;
        }
        else
        {
            if (t_sav < p_tin)
            {
                t_sav = p_tin;
            }      
        }
        x_u = (x_b - ((x_b - x_c) * t_q) - ((x_b - x_a) * t_r)) / (2 * t_sav);
        x_lim = x_b + p_lim * (x_c - x_b);
        if(((x_b -x_u) * (x_u - x_c)) > 0.0)
        {
            f_u = computeFunctionWeightFormula(x_u);
            if (f_u < f_c)
            {
                x_a = x_b;
                x_b = x_u;
                f_a = f_b;
                f_b = f_u;
                break;
            }
            else
            {
                if (f_u > f_b)
                {
                    x_c = x_u;
                    f_c = f_u;
                    break;
                }	
            }
            x_u = x_c + p_step * (x_c - x_b);
            f_u = computeFunctionWeightFormula(x_u);
        }
        else
        {
            if (((x_c -x_u) * (x_u - x_lim)) > 0.0)
            {
                f_u = computeFunctionWeightFormula(x_u);
                if (f_u < f_c)
                {
                    x_b = x_c; x_c = x_u;
                    x_u = x_c + p_step * (x_c - x_b);
                    f_b = f_c; f_c = f_u;
                    f_u = computeFunctionWeightFormula(x_u);  
                }
            }
            else
            {
                if (((x_u -x_lim) * (x_lim - x_c)) >= 0.0)
                {
                    x_u = x_lim;
                    f_u = computeFunctionWeightFormula(x_u);
                }
                else
                {
                    x_u = x_c + p_step * (x_c - x_b);
                    f_u = computeFunctionWeightFormula(x_u);
                }
            }
        }
        x_a = x_b; x_b = x_c; x_c = x_u;
        f_a = f_b; f_b = f_c; f_c = f_u;    
    }
    p_step = interpolation_learning_rate;
    p_to1 = interpolation_precision;
    x_d = x_e = 0.0;
    x_v = x_w = x_x = x_b;
    f_v = f_w = f_x = f_b;  
    if (x_a < x_c)
    {
        x_b = x_c;
    }
    else
    {
        x_b = x_a;
        x_a = x_c;    
    }
    for (iter = 1; iter <= 100; iter++)
    {
        x_xmed = 0.5 * (x_a + x_b);
        p_tol1 = p_to1 * fabs(x_x) + p_tin;
        p_tol2 = 2.0 * p_tol1;
        if (fabs(x_x - x_xmed) <= (p_tol2 - 0.5 * (x_b - x_a)))
        {
            break;
        }
        if (fabs(x_e) > p_tol1)
        {
            t_r = (x_x - x_w) * (f_x - f_v);
            t_q = (x_x - x_v) * (f_x - f_w);
            t_p = (x_x - x_v) * t_q - (x_x - x_w) * t_r;
            t_q = 2.0 * (t_q - t_r);
            if (t_q > 0.0)
            {
                t_p = -t_p;
            }
            t_q = fabs(t_q);
            t_sav= x_e;
            x_e = x_d;
            if (fabs(t_p) >= fabs(0.5 * t_q * t_sav) || 
                t_p <= t_q * (x_a - x_x) ||
                t_p >= t_q * (x_b - x_x))
            {
                if (x_x >= x_xmed)
                {
                    x_d = p_step * (x_a - x_x);
                }
                else
                {
                    x_d = p_step * (x_b - x_x);
                }
            }
            else
            {
                x_d = t_p / t_q;
                x_u = x_x + x_d;
                if (x_u - x_a < p_tol2 || x_b - x_u < p_tol2)
                {
                    x_d = p_tol1;
                    if (x_xmed - x_x < 0.0)
                    {
                        x_d = -x_d;
                    }
                }
            }
        }
        else
        {
            if (x_x >= x_xmed)
            {
                x_d = p_step * (x_a - x_x);
            }
            else
            {
                x_d = p_step * (x_b - x_x);
            }      
        }
        if (fabs(x_d) >= p_tol1)
        {
            x_u = x_x + x_d;
        }
        else
        {
            if (x_d < 0.0)
            {
                x_u = x_x - p_tol1;
            }
            else
            {
                x_u = x_x + p_tol1;
            }
        }
        f_u = computeFunctionWeightFormula(x_u);
        if (f_u <= f_x)
        {
            if (x_u >= x_x)
            {
                x_a = x_x;
            }
            else
            {
                x_b = x_x;
            }
            x_v = x_w; x_w = x_x; x_x = x_u;
            f_v = f_w; f_w = f_x; f_x = f_u;
        }
        else
        {
            if (x_u < x_x)
            {
                x_a = x_u;
            }
            else
            {
                x_b = x_u;
            }
            if (f_u <= f_w || x_w == x_x)
            {
                x_v = x_w; x_w = x_u;
                f_v = f_w; f_w = f_u;        
            }
            else
            {
                if (f_u <= f_v || x_v == x_x || x_v == x_w)
                {
                    x_v = x_u;
                    f_v = f_u; 
                }
            }
        } 
    }
    return x_x;
}

real LocalMedBoost::computeFunctionWeightFormula(real alpha)
{
    real return_value = 0.0;
    for (each_train_sample_index = 0; each_train_sample_index < length; each_train_sample_index++)
    {
        return_value += sample_weights[each_train_sample_index] * 
            exp(-1.0 * alpha * base_awards[each_train_sample_index]);
    }
    return_value *= safeexp(robustness * alpha);
    return return_value;
}

void LocalMedBoost::initializeSampleWeight()
{
    real init_weight = 1.0 / length;
    for (each_train_sample_index = 0; each_train_sample_index < length; each_train_sample_index++)
    {
        sample_weights[each_train_sample_index] = init_weight;
        train_set->put(each_train_sample_index, inputsize + targetsize, sample_weights[each_train_sample_index]);
    }
}

void LocalMedBoost::recomputeSampleWeight()
{
    sum_exp_weighted_edges = 0.0;
    for (each_train_sample_index = 0; each_train_sample_index < length; each_train_sample_index++)
    {
        exp_weighted_edges[each_train_sample_index] =  sample_weights[each_train_sample_index] *
            safeexp(-1.0 * function_weights[stage] * base_awards[each_train_sample_index]);
        sum_exp_weighted_edges += exp_weighted_edges[each_train_sample_index];
    }
    for (each_train_sample_index = 0; each_train_sample_index < length; each_train_sample_index++)
    {
        sample_weights[each_train_sample_index] = exp_weighted_edges[each_train_sample_index] / sum_exp_weighted_edges;
        train_set->put(each_train_sample_index, inputsize + targetsize,
                       sample_weights[each_train_sample_index]);
    }
}

void LocalMedBoost::verbose(string the_msg, int the_level)
{
    if (verbosity >= the_level)
        cout << the_msg << endl;
}
 

void LocalMedBoost::forget()
{
    stage = 0;
}

int LocalMedBoost::outputsize() const
{
    return 4;
}

TVec<string> LocalMedBoost::getTrainCostNames() const
{
    TVec<string> return_msg(6);
    return_msg[0] = "mse";
    return_msg[1] = "base_confidence";
    return_msg[2] = "l1";
    return_msg[3] = "rob_minus";
    return_msg[4] = "rob_plus";
    return_msg[5] = "min_rob";
    return return_msg;
}

TVec<string> LocalMedBoost::getTestCostNames() const
{ 
    return getTrainCostNames();
}

void LocalMedBoost::computeOutput(const Vec& inputv, Vec& outputv) const
{
    if (end_stage < 1)
        PLERROR("LocalMedBoost: No function has been built"); 
    TVec<real>  base_regressor_outputs;         // vector of base regressor outputs for a sample
    TVec<real>  base_regressor_confidences;     // vector of base regressor confidences for a sample
    Vec         base_regressor_outputv;         // vector of a base regressor computed prediction
    real        sum_alpha;
    real        sum_function_weights;           // sum of all regressor weighted confidences 
    real        norm_sum_function_weights;
    real        sum_fplus_weights;              // sum of the regressor weighted confidences for the f+ function
    real        sum_fminus_weights;
    real        zero_quantile;
    real        rob_quantile;
    real        output_rob_plus;
    real        output_rob_minus;
    real        output_rob_save;
    int         index_j;                        // index to go thru the base regressor's arrays
    int         index_t;                        // index to go thru the base regressor's arrays
    base_regressor_outputs.resize(end_stage);
    base_regressor_confidences.resize(end_stage);
    base_regressor_outputv.resize(2);
    sum_function_weights = 0.0;
    sum_alpha = 0.0;
    outputv[0] = -1E9;
    outputv[1] = 0.0;
    output_rob_plus = 1E9;
    output_rob_minus = -1E9;
    for (index_t = 0; index_t < end_stage; index_t++) 
    {
        base_regressors[index_t]->computeOutput(inputv, base_regressor_outputv);
        base_regressor_outputs[index_t] = base_regressor_outputv[0];
        base_regressor_confidences[index_t] = base_regressor_outputv[1];
        if (base_regressor_outputs[index_t] > outputv[0])
        {
            outputv[0] = base_regressor_outputs[index_t];
            outputv[1] = base_regressor_confidences[index_t];
        }
        sum_alpha += function_weights[index_t];
        sum_function_weights += function_weights[index_t] * base_regressor_confidences[index_t];
    }
    norm_sum_function_weights = sum_function_weights / sum_alpha;
    if (norm_sum_function_weights > 0.0) rob_quantile = 0.5 * (1.0 - (robustness / norm_sum_function_weights) * sum_function_weights);
    else rob_quantile = 0.0;
    zero_quantile = 0.5 * sum_function_weights;
    for (index_j = 0; index_j < end_stage; index_j++) 
    {
        sum_fplus_weights = 0.0;
        sum_fminus_weights = 0.0;
        for (index_t = 0; index_t < end_stage; index_t++)
        {
            if (base_regressor_outputs[index_j] < base_regressor_outputs[index_t])
            {
                sum_fplus_weights += function_weights[index_t] * base_regressor_confidences[index_t];
            }
            if (base_regressor_outputs[index_j] > base_regressor_outputs[index_t])
            {
                sum_fminus_weights += function_weights[index_t] * base_regressor_confidences[index_t];
            }
        }
        if (norm_sum_function_weights > 0.0 && sum_fplus_weights  < zero_quantile)
        {
            if (base_regressor_outputs[index_j] < outputv[0])
            {
                outputv[0] = base_regressor_outputs[index_j];
                outputv[1] = base_regressor_confidences[index_j];
            }
        }
        if (norm_sum_function_weights > 0.0 && sum_fplus_weights  < rob_quantile)
        {
            if (base_regressor_outputs[index_j] < output_rob_plus)
            {
                output_rob_plus = base_regressor_outputs[index_j];
            }
        }
        if (norm_sum_function_weights > 0.0 && sum_fminus_weights  < rob_quantile)
        {
            if (base_regressor_outputs[index_j] > output_rob_minus)
            {
                output_rob_minus = base_regressor_outputs[index_j];
            }
        }
    }
    if (output_rob_minus > output_rob_plus)
    {
        output_rob_save = output_rob_minus;
        output_rob_minus = output_rob_plus;
        output_rob_plus = output_rob_save;
    }
    outputv[2] = output_rob_minus;
    outputv[3] = output_rob_plus;
}

void LocalMedBoost::computeOutputAndCosts(const Vec& inputv, const Vec& targetv, Vec& outputv, Vec& costsv) const
{
    computeOutput(inputv, outputv);
    computeCostsFromOutputs(inputv, outputv, targetv, costsv);
}

void LocalMedBoost::computeCostsFromOutputs(const Vec& inputv, const Vec& outputv,
                                            const Vec& targetv, Vec& costsv) const
{
    costsv[0] = square_f(outputv[0] - targetv[0]);
    costsv[1] = outputv[1];
    if (abs(outputv[0] - targetv[0]) > loss_function_weight) costsv[2] = 1.0;
    else costsv[2] = 0.0;
    costsv[3] = outputv[3] - outputv[0];
    costsv[4] = outputv[0] - outputv[2];
    if (costsv[3] < costsv[4]) costsv[5] = costsv[3];
    else costsv[5] = costsv[4];
}

} // end of namespace PLearn


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
