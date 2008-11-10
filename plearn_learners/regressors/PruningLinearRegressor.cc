// -*- C++ -*-

// PruningLinearRegressor.cc
//
// Copyright (C) 2008 Rejean Ducharme
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

// Authors: Rejean Ducharme

/*! \file PruningLinearRegressor.cc */

#include "PruningLinearRegressor.h"
#include <plearn/math/plapack.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    PruningLinearRegressor,
    "Same as LinearRegressor, but adding the pruning of the regression coefficients",
    "This class permits to reduce the degree of freedom of a LinearRegressor by\n"
    "pruning some regression coefficients.  Several pruning methods are supported:\n"
    "  - minimum t-ratio: keep only the coefficients for which the t-ratio exceeds a threshold\n"
    "  - absolute max: keep only a maximum number of coefficient (with best t-ratios)\n"
    "  - relative max: keep only a maximum fraction of coefficient (with best t-ratios)\n"
    );

PruningLinearRegressor::PruningLinearRegressor()
    : pruning_method("max_number"),
      min_t_ratio(0.05),
      max_number(50),
      max_fraction(0.5)
{ }

void PruningLinearRegressor::declareOptions(OptionList& ol)
{
    //#####  Build Options  ####################################################

    declareOption(ol, "pruning_method", &PruningLinearRegressor::pruning_method,
                  OptionBase::buildoption,
                  "The pruning method:\n"
                  " - \"max_number\"    = keep only the weights with the k-best t-ratio\n"
                  " - \"max_fraction\"  = same as \"max_number\", but using a fraction rather than a hard threshold\n"
                  " - \"min_t_ratio\"   = keep only the weights with t-ratio > min_t_ratio");

    declareOption(ol, "min_t_ratio", &PruningLinearRegressor::min_t_ratio,
                  OptionBase::buildoption,
                  "Minimum t-ratio for not pruning a coefficient");

    declareOption(ol, "max_number", &PruningLinearRegressor::max_number,
                  OptionBase::buildoption,
                  "Maximum number of coefficients (the default)");

    declareOption(ol, "max_fraction", &PruningLinearRegressor::max_fraction,
                  OptionBase::buildoption,
                  "Maximum fraction (in [0,1]) of coefficients");

    //#####  Learnt Options  ###################################################

    declareOption(ol, "t_ratio", &PruningLinearRegressor::t_ratio,
                  OptionBase::learntoption,
                  "t-ratio statistics for the estimator b (regression coefficients)\n"
                  "Saved as a learned option to allow computing statistical significance\n"
                  "of the coefficients when the model is reloaded and used in test mode.");

    declareOption(ol, "input_indices", &PruningLinearRegressor::input_indices,
                  OptionBase::learntoption,
                  "Indices of inputs kept for regression");

    inherited::declareOptions(ol);
}

void PruningLinearRegressor::build_()
{
    if (pruning_method == "max_number")
    {
        if (max_number < 1)
            PLERROR("\"max_number\" should be strictly positive");
    }
    else if (pruning_method == "max_fraction")
    {
        if (max_fraction <= 0.0  ||  max_fraction >= 1.0)
            PLERROR("\"max_fraction\" should be in range ]0,1[");
    }
    else if (pruning_method == "min_t_ratio")
    {
        if (min_t_ratio <= 0.0)
            PLERROR("\"min_t_ratio\" should be strictly positive");
    }
    else
        PLERROR("Pruning method \"%s\" not supported", pruning_method.c_str());
}

void PruningLinearRegressor::build()
{
    inherited::build();
    build_();
}

void PruningLinearRegressor::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(t_ratio, copies);
    deepCopyField(input_indices, copies);
}

void PruningLinearRegressor::forget()
{
    inherited::forget();
    t_ratio.resize(0);
}

void PruningLinearRegressor::setTrainingSet(VMat training_set, bool call_forget)
{
    inherited::setTrainingSet(training_set, call_forget);
    if (targetsize() > 1)
        PLERROR("PruningLinearRegressor works only with single target problems");
}

void PruningLinearRegressor::train()
{
    // train with all coefficients
    inherited::train();

    // find the dataset indices corresponding to coefficients not pruned
    newDatasetIndices();

    // Add target and (possibly) weight indices
    TVec<int> all_indices = input_indices.copy();
    all_indices.append(inputsize());        // target index = inputsize
    if (weightsize())
        all_indices.append(inputsize()+1);  // weight index = target index + 1

    // Build new training set
    VMat new_trainset = train_set.columns(all_indices);
    int new_inputsize = input_indices.length();
    new_trainset->defineSizes(new_inputsize, 1, weightsize());

    // Train with this new training set
    setTrainingSet(new_trainset);
    inherited::train();
}

void PruningLinearRegressor::computeOutput(const Vec& input, Vec& output) const
{
    Vec actual_input(input_indices.length());
    selectElements(input, input_indices, actual_input);
    inherited::computeOutput(actual_input, output);
}

void PruningLinearRegressor::computeTRatio()
{
    // We wish to compute the t-ratio of the estimator coefficients.
    // For that purpose, we use the following formula:
    //
    //    t = |b| / sigma_b
    //
    // where b is the coefficients vector and sigma_b is the stderr matrix
    // of the estimator of b.  The latter is computed as:
    //
    //     sigma_b = s^2 * inverse(X'X)
    //
    // where s^2 is the residual variance (estimated using the
    // LinearRegressor::computeResidualsVariance method)
    // and X is the matrix of regressors..

    const int ninputs = weights.length();
    Mat sigma_b(ninputs, ninputs);
    t_ratio.resize(ninputs);

    // We compute the estimator
    PLASSERT(resid_variance.length() == 1);
    PLASSERT(weights.width() == 1);
    real residual_variance = resid_variance[0];
 
    Mat XtX_copy = XtX.copy();  // matInvert overwrite the input matrix
    Mat XtX_inverse(XtX.length(), XtX.width());
    matInvert(XtX_copy, XtX_inverse);
    for (int i=0; i<ninputs; i++)
    {
        real sigma_b = sqrt(residual_variance*XtX_inverse(i,i));
        t_ratio[i] = abs(weights(i,0)) / sigma_b;
    }
}

void PruningLinearRegressor::newDatasetIndices()
{
    // Compute first the t-ratios
    computeTRatio();

    // Sort all t-ratios
    int nb_weights = weights.length();
    Vec t_ratio_sort = t_ratio.copy();
    sortElements(t_ratio_sort, true);

    // Find the t-ratio threshold
    real t_ratio_threshold = 0.0;
    if (pruning_method == "max_number")
    {
        int keep_n_weights = min(max_number, nb_weights);
        if (include_bias)
            ++keep_n_weights;
        t_ratio_threshold = t_ratio_sort[keep_n_weights-1];
    }
    else if (pruning_method == "max_fraction")
    {
        int keep_n_weights = max_fraction*nb_weights;
        t_ratio_threshold = t_ratio_sort[keep_n_weights-1];
    }
    else if (pruning_method == "min_t_ratio")
    {
        t_ratio_threshold = min_t_ratio;
    }

    // Find kept (not pruned) coefficient indices
    input_indices.resize(0);
    int offset = include_bias ? 1 : 0;
    for (int i=0; i<nb_weights; i++)
    {
        if (t_ratio[i] >= t_ratio_threshold)
        {
            int data_index = i - offset;
            if (data_index >= 0)  // = -1 for bias
                input_indices.append(data_index);
        }
    }
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
