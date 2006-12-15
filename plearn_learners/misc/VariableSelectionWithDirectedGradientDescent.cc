// -*- C++ -*-

// VariableSelectionWithDirectedGradientDescent.cc
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


/* **************************************************************************************************************    
 * $Id: VariableSelectionWithDirectedGradientDescent.cc, v 1.0 2005/01/15 10:00:00 Bengio/Kegl/Godbout        *
 * This file is part of the PLearn library.                                                                   *
 ************************************************************************************************************** */

#include "VariableSelectionWithDirectedGradientDescent.h"
#include <plearn/base/tostring.h>

namespace PLearn {
using namespace std;

VariableSelectionWithDirectedGradientDescent::VariableSelectionWithDirectedGradientDescent()
    : learning_rate(1e-2)
{
}

PLEARN_IMPLEMENT_OBJECT(VariableSelectionWithDirectedGradientDescent,
                        "Variable selection algorithm", 
                        "Variable selection algorithm using a linear density estimator and\n"
                        "directed gradient descent to identify most relevant variables.\n"
                        "\n"
                        "There are 4 options to set:\n"
                        "   learning_rate, the gradient step to be used by the descent algorithm,\n"
                        "   nstages, the number of epoch to be performed by the algorithm,\n"
                        "   verbosity, the level of information you want to get while in progress,\n"
                        "   report_progress, whether a progress bar should inform you of the progress.\n"
                        "\n"
                        "If both verbosity > 1 and report_progress is not zero, it works but it is ugly.\n"
                        "\n"
                        "The selected variables are returned in the selected_variables vector in\n"
                        "the order of their selection. The vector is a learnt option of the algorithm.\n"
                        "\n"
                        "The target should be binary, with values 0 and 1. It can be multi-dimensional,\n"
                        "in which case a different predictor is learned for each target, with all\n"
                        "predictors sharing the same set of variables. Note that the cost is currently\n"
                        "only computed for the first target.\n"
    );

void VariableSelectionWithDirectedGradientDescent::declareOptions(OptionList& ol)
{ 
    declareOption(ol, "learning_rate", &VariableSelectionWithDirectedGradientDescent::learning_rate, OptionBase::buildoption,
                  "The learning rate of the gradient descent algorithm.\n");
    declareOption(ol, "input_weights", &VariableSelectionWithDirectedGradientDescent::input_weights, OptionBase::learntoption,
                  "The lerant weights of the linear probability estimator.\n");
    declareOption(ol, "weights_selected", &VariableSelectionWithDirectedGradientDescent::weights_selected, OptionBase::learntoption,
                  "The vector that identifies the non-zero weights.\n");
    declareOption(ol, "selected_variables", &VariableSelectionWithDirectedGradientDescent::selected_variables, OptionBase::learntoption,
                  "The vector with the selected variables in the order of their selection.\n");
    inherited::declareOptions(ol);
}

void VariableSelectionWithDirectedGradientDescent::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(input_weights, copies);
    deepCopyField(weights_selected, copies);
    deepCopyField(selected_variables, copies);
    deepCopyField(sample_input, copies);
    deepCopyField(sample_target, copies);
    deepCopyField(sample_weight, copies);
    deepCopyField(sample_output, copies);
    deepCopyField(sample_cost, copies);
    deepCopyField(train_criterion, copies);
    deepCopyField(weights_gradient, copies);
    deepCopyField(sum_of_abs_gradient, copies);
}

void VariableSelectionWithDirectedGradientDescent::build()
{
    inherited::build();
    build_();
}

void VariableSelectionWithDirectedGradientDescent::build_()
{
}

///////////
// train //
///////////
void VariableSelectionWithDirectedGradientDescent::train()
{
    if (!train_set)
        PLERROR("VariableSelectionWithDirectedGradientDescent: the algorithm has not been properly built");
    if (stage == 0) {
        // Initialize stuff before training.
        length = train_set->length();
        width = train_set->width();
        if (length < 1)
            PLERROR("VariableSelectionWithDirectedGradientDescent: the training set must contain at least one sample, got %d", length);
        inputsize = train_set->inputsize();
        targetsize = train_set->targetsize();
        weightsize = train_set->weightsize();
        if (inputsize < 1)
            PLERROR("VariableSelectionWithDirectedGradientDescent: expected  inputsize greater than 0, got %d", inputsize);
        if (targetsize <= 0)
            PLERROR("In VariableSelectionWithDirectedGradientDescent::train - The targetsize must be >= 1", targetsize);
        if (weightsize != 0)
            PLERROR("VariableSelectionWithDirectedGradientDescent: expected weightsize to be 1, got %d", weightsize_);
        input_weights.resize(targetsize, inputsize + 1);
        weights_selected.resize(inputsize + 1);
        weights_gradient.resize(targetsize, inputsize + 1);
        sample_input.resize(inputsize);
        sample_target.resize(1);
        sample_output.resize(1);
        sample_cost.resize(1);
        train_criterion.resize(targetsize);
        sum_of_abs_gradient.resize(inputsize);
    }

    input_weights.fill(0);
    weights_selected.fill(false);
    if (report_progress)
    {
        pb = new ProgressBar("VariableSelectionWithDirectedGradientDescent : train stages: ", nstages);
    }
/*
  We loop through the data for the specified maximum number of stages.
*/
    for (; stage < nstages; stage++)
    {
        weights_gradient.fill(0);
/*
  We compute the train criterion for this stage and compute the weight gradient.
*/
        train_criterion.fill(0);
        for (int i = 0; i < targetsize; i++) {
            for (row = 0; row < length; row++)
            {
                real target = train_set(row, inputsize + i);
                if (is_missing(target))
                    continue;
                n7_value = input_weights(i, inputsize);
                for (col = 0; col < inputsize; col++)
                {
                    n7_value += input_weights(i, col) * train_set(row, col);
                }
#ifdef BOUNDCHECK
                if (!fast_exact_is_equal(target, 0.0) &&
                    !fast_exact_is_equal(target, 1.0))
                    PLERROR("In VariableSelectionWithDirectedGradientDescent::train - The target should be 0 or 1");
#endif
                if (fast_exact_is_equal(target, 0)) target = -1; // We work with -1 and 1 instead.
                n8_value = target * n7_value;
                n9_value = 1.0 / (1.0 + exp(-n8_value));
                n10_value = -pl_log(n9_value);
                train_criterion[i] += n10_value;
                n10_gradient = 1.0;
                n9_gradient = n10_gradient * (-1.0 / n9_value);
                n8_gradient = n9_gradient * n9_value * 1.0 / (1.0 + exp(n8_value));
                n7_gradient = n8_gradient * target;
                for (col = 0; col < inputsize; col++)
                {
                    weights_gradient(i, col) += n7_gradient * train_set(row, col);
                }
                weights_gradient(i, inputsize) += n7_gradient;     
            }
        }
/*
  We perform this stage weight update according to the directed gradient descent algorithm.
*/
        sum_of_abs_gradient.fill(0);
        for (int i = 0; i < targetsize; i++) {
            // Bias update.
            input_weights(i, inputsize) -= learning_rate * weights_gradient(i, inputsize);
            // Compute sum of |gradient|.
            for (int j = 0; j < inputsize; j++)
                sum_of_abs_gradient[j] += fabs(weights_gradient(i,j));
        }
        weights_gradient_max = 0.0;
        for (col = 0; col < inputsize; col++)
        {
            if (sum_of_abs_gradient[col] > weights_gradient_max)
            {
                weights_gradient_max = sum_of_abs_gradient[col];
                weights_gradient_max_col = col;
            }
        }
        if (!weights_selected[weights_gradient_max_col])
        {
            selected_variables.append(weights_gradient_max_col);
            verbose("VariableSelectionWithDirectedGradientDescent: variable " + tostring(weights_gradient_max_col)
                    + " was added.", 2);
        }
        weights_selected[weights_gradient_max_col] = true;
        // Weights update.
        for (int i = 0; i < targetsize; i++)
            for (col = 1; col < inputsize; col++)
                input_weights(i, col) -= learning_rate * weights_gradient(i, col) * real(weights_selected[col]);
        verbose("VariableSelectionWithDirectedGradientDescent: After " + tostring(stage) + " stages, the train criterion is: "
                + tostring(train_criterion), 3);
        if (report_progress) pb->update(stage);
    }
    if (report_progress)
    {
        pb = new ProgressBar("VariableSelectionWithDirectedGradientDescent : computing the training statistics: ", length);
    }
    train_stats->forget();
    for (row = 0; row < length; row++)
    {   
        train_set->getExample(row, sample_input, sample_target, sample_weight);
        for (int i = 0; i < sample_target.length(); i++)
            if (fast_exact_is_equal(sample_target[i], 0)) sample_target[i] = -1; // We work with -1 and 1.
        computeOutput(sample_input, sample_output);
        computeCostsFromOutputs(sample_input, sample_output, sample_target, sample_cost);
        train_stats->update(sample_cost);
        if (report_progress) pb->update(row);
    }
    train_stats->finalize();
    verbose("VariableSelectionWithDirectedGradientDescent: After " + tostring(stage) + " stages, average error is: "
            + tostring(train_stats->getMean()), 1);
}

void VariableSelectionWithDirectedGradientDescent::verbose(string the_msg, int the_level)
{
    if (verbosity >= the_level)
        pout << the_msg << endl;
}

void VariableSelectionWithDirectedGradientDescent::forget()
{
    inputsize = -1; // For safety reasons.
    selected_variables.resize(0);
    stage = 0;
}

int VariableSelectionWithDirectedGradientDescent::outputsize() const
{
    return targetsize;
}

///////////////////////
// getTrainCostNames //
///////////////////////
TVec<string> VariableSelectionWithDirectedGradientDescent::getTrainCostNames() const
{
    TVec<string> return_msg(1);
    return_msg[0] = "negloglikelihood";
    return return_msg;
}

//////////////////////
// getTestCostNames //
//////////////////////
TVec<string> VariableSelectionWithDirectedGradientDescent::getTestCostNames() const
{ 
    return getTrainCostNames();
}

///////////////////
// computeOutput //
///////////////////
void VariableSelectionWithDirectedGradientDescent::computeOutput(const Vec& inputv, Vec& outputv) const
{
    outputv.resize(targetsize);
    for (int i = 0; i < targetsize; i++) {
        outputv[i] = input_weights(i, inputsize);
        for (int col = 0; col < inputsize; col++)
        {
            outputv[i] += input_weights(i, col) * inputv[col];
        }
    }
}

/////////////////////////////
// computeCostsFromOutputs //
/////////////////////////////
void VariableSelectionWithDirectedGradientDescent::computeCostsFromOutputs(const Vec& inputv, const Vec& outputv, 
                                                                           const Vec& targetv, Vec& costsv) const
{
    if (is_missing(outputv[0]))
    {
        costsv[0] = MISSING_VALUE;
        return;
        // ???  return MISSING_VALUE;
    }
    // Note that the "2 * target - 1" operation is only here to transform a 0/1
    // target into -1/1.
    costsv[0] = -pl_log(1.0 / (1.0 + exp(-(2.0 * targetv[0] - 1) * outputv[0])));;
}

////////////////////
// setTrainingSet //
////////////////////
void VariableSelectionWithDirectedGradientDescent::setTrainingSet(VMat training_set, bool call_forget) {
    targetsize = training_set->targetsize();
    inherited::setTrainingSet(training_set, call_forget);
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
