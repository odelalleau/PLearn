// -*- C++ -*-

// GaussianProcessRegressor.cc
//
// Copyright (C) 2006 Nicolas Chapados 
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
   * $Id: .pyskeleton_header 544 2003-09-01 00:05:31Z plearner $ 
   ******************************************************* */

// Authors: Nicolas Chapados

/*! \file GaussianProcessRegressor.cc */

// From PLearn
#include <plearn/vmat/ExtendedVMatrix.h>
#include <plearn/math/pl_erf.h>

#include "GaussianProcessRegressor.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    GaussianProcessRegressor,
    "Kernelized version of linear ridge regression.",
    "Given a kernel K(x,y) = phi(x)'phi(y), where phi(x) is the projection of a\n"
    "vector x into feature space, this class implements a version of the ridge\n"
    "estimator, giving the prediction at x as\n"
    "\n"
    "    f(x) = k(x)'(M + lambda I)^-1 y,\n"
    "\n"
    "where x is the test vector where to estimate the response, k(x) is the\n"
    "vector of kernel evaluations between the test vector and the elements of\n"
    "the training set, namely\n"
    "\n"
    "    k(x) = (K(x,x1), K(x,x2), ..., K(x,xN))',\n"
    "\n"
    "M is the Gram Matrix on the elements of the training set, i.e. the matrix\n"
    "where the element (i,j) is equal to K(xi, xj), lamdba is the weight decay\n"
    "coefficient, and y is the vector of training-set targets.\n"
    "\n"
    "The disadvantage of this learner is that its training time is O(N^3) in the\n"
    "number of training examples (due to the matrix inversion).  When saving the\n"
    "learner, the training set must be saved, along with an additional vector of\n"
    "the length of the training set.\n");


GaussianProcessRegressor::GaussianProcessRegressor() 
    : m_weight_decay(0.0),
      m_include_bias(true),
      m_compute_confidence(false)
{ }


void GaussianProcessRegressor::declareOptions(OptionList& ol)
{
    declareOption(ol, "kernel", &GaussianProcessRegressor::m_kernel,
                  OptionBase::buildoption,
                  "Kernel to use for the computation.  This must be a similarity kernel\n"
                  "(i.e. closer vectors give higher kernel evaluations).");

    declareOption(ol, "weight_decay", &GaussianProcessRegressor::m_weight_decay,
                  OptionBase::buildoption,
                  "Weight decay coefficient (default = 0)");

    declareOption(ol, "include_bias", &GaussianProcessRegressor::m_include_bias,
                  OptionBase::buildoption,
                  "Whether to include a bias term in the regression (true by default)");

    declareOption(ol, "compute_confidence", &GaussianProcessRegressor::m_compute_confidence,
                  OptionBase::buildoption,
                  "Whether to perform the additional train-time computations required\n"
                  "to compute confidence intervals.  This includes computing a separate\n"
                  "inverse of the Gram matrix\n");
    
    declareOption(ol, "params", &GaussianProcessRegressor::m_params,
                  OptionBase::learntoption,
                  "Vector of learned parameters, determined from the equation\n"
                  "    (M + lambda I)^-1 y");

    declareOption(ol, "gram_inverse", &GaussianProcessRegressor::m_gram_inverse,
                  OptionBase::learntoption,
                  "Inverse of the Gram matrix, used to compute confidence intervals (must\n"
                  "be saved since the confidence intervals are obtained from the equation\n"
                  "\n"
                  "  sigma^2 = k(x,x) - k(x)'(M + lambda I)^-1 k(x)\n");
    
    declareOption(ol, "training_inputs", &GaussianProcessRegressor::m_training_inputs,
                  OptionBase::learntoption,
                  "Saved version of the training set, which must be kept along for\n"
                  "carrying out kernel evaluations with the test point");
    
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void GaussianProcessRegressor::build_()
{
    if (! m_kernel)
        PLERROR("GaussianProcessRegressor::build_: 'kernel' option must be specified");

    // If we are reloading the model, set the training inputs into the kernel
    if (m_training_inputs)
        m_kernel->setDataForKernelMatrix(m_training_inputs);
}

// ### Nothing to add here, simply calls build_
void GaussianProcessRegressor::build()
{
    inherited::build();
    build_();
}


void GaussianProcessRegressor::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(m_kernel,               copies);
    deepCopyField(m_params,               copies);
    deepCopyField(m_gram_inverse,         copies);
    deepCopyField(m_training_inputs,      copies);
    deepCopyField(m_kernel_evaluations,   copies);
    deepCopyField(m_gram_inverse_product, copies);
    deepCopyField(m_extended_input,       copies);
    deepCopyField(m_actual_input,         copies);
}


void GaussianProcessRegressor::setTrainingSet(VMat training_set, bool call_forget)
{
    PLASSERT( training_set );
    int inputsize = training_set->inputsize() ;
    if (inputsize < 0)
        PLERROR("GaussianProcessRegressor::setTrainingSet: the training set inputsize "
                "must be specified (current value = %d)", inputsize);
    m_training_inputs = training_set.subMatColumns(0, inputsize);
    if (m_include_bias)                          // prepend a first column of ones
        m_training_inputs = new ExtendedVMatrix(m_training_inputs,0,0,1,0,1.0);

    // Convert to a real matrix in order to make saving it saner
    m_training_inputs = m_training_inputs.toMat();

    inherited::setTrainingSet(training_set, call_forget);
}


int GaussianProcessRegressor::outputsize() const
{
    return targetsize();
}

void GaussianProcessRegressor::forget()
{
    m_params.resize(0,0);
    m_gram_inverse.resize(0,0);
}
    
void GaussianProcessRegressor::train()
{
    PLASSERT( m_kernel );
    if (! train_set || ! m_training_inputs)
        PLERROR("GaussianProcessRegressor::train: the training set must be specified");
    int trainlength= train_set->length();
    int inputsize  = train_set->inputsize() ;
    int targetsize = train_set->targetsize();
    int weightsize = train_set->weightsize();
    if (inputsize  < 0 || targetsize < 0 || weightsize < 0)
        PLERROR("GaussianProcessRegressor::train: inconsistent inputsize/targetsize/weightsize "
                "(%d/%d/%d) in training set", inputsize, targetsize, weightsize);
    if (weightsize > 0)
        PLERROR("GaussianProcessRegressor::train: observations weights are not currently supported");

    // The RHS matrix (when solving the linear system Gram*Params=RHS) is made
    // up of two parts: the regression targets themselves, and the identity
    // matrix if we requested them (for confidence intervals).  After solving
    // the linear system, set the gram-inverse appropriately.
    int rhs_width = targetsize + (m_compute_confidence? trainlength : 0);
    Mat RHS(trainlength, rhs_width);
    Mat RHS_target = RHS.subMatColumns(0, targetsize);
    train_set.subMatColumns(inputsize, targetsize)->getMat(0,0,RHS_target);
    if (m_compute_confidence) {
        Mat RHS_identity = RHS.subMatColumns(targetsize, trainlength);
        identityMatrix(RHS_identity);
    }
    
    // Compute Gram Matrix and add weight decay to diagonal
    m_kernel->setDataForKernelMatrix(m_training_inputs);
    Mat gram_mat(m_training_inputs.length(), m_training_inputs.length());
    m_kernel->computeGramMatrix(gram_mat);
    addToDiagonal(gram_mat, m_weight_decay);

    // Compute parameters
    m_params.resize(RHS.length(), RHS.width());
    solveLinearSystemByCholesky(gram_mat, RHS, m_params);
    if (m_compute_confidence) {
        m_gram_inverse = m_params.subMatColumns(targetsize, trainlength);
        m_params = m_params.subMatColumns(0, targetsize);
    }
    
    // Compute train error if there is a train_stats_collector.  There is
    // probably an analytic formula, but ...
    if (getTrainStatsCollector())
        test(train_set, getTrainStatsCollector());

    stage = nstages;
}


void GaussianProcessRegressor::computeOutput(const Vec& input, Vec& output) const
{
    PLASSERT( m_kernel && m_params.isNotNull() && m_training_inputs );
    PLASSERT( output.size() == m_params.width() );
    PLASSERT( input.size() + m_include_bias == m_training_inputs.width() );

    if (m_include_bias) {
        m_extended_input.resize(m_training_inputs.width());
        m_extended_input.subVec(1,input.size()) << input;
        m_extended_input[0] = 1.0;
        m_actual_input = m_extended_input;
    }
    else
        m_actual_input = input;
    
    m_kernel_evaluations.resize(m_params.length());
    m_kernel->evaluate_all_x_i(m_actual_input, m_kernel_evaluations);

    // Finally compute k(x,x_i) * (M + \lambda I)^-1 y
    product(Mat(1, output.size(), output),
            Mat(1, m_kernel_evaluations.size(), m_kernel_evaluations),
            m_params);
}    


void GaussianProcessRegressor::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                                       const Vec& target, Vec& costs) const
{
    costs.resize(1);
    real squared_loss = powdistance(output,target);
    costs[0] = squared_loss;
}     


bool GaussianProcessRegressor::computeConfidenceFromOutput(
    const Vec& input, const Vec& output, real probability,
    TVec< pair<real,real> >& intervals) const
{
    if (! m_compute_confidence) {
        PLWARNING("GaussianProcessRegressor::computeConfidenceFromOutput: the option\n"
                  "'compute_confidence' must be true in order to compute valid\n"
                  "condidence intervals");
        return false;
    }

    // BIG-BIG assumption: assume that computeOutput has just been called and
    // that m_kernel_evaluations and m_actual_input point to the right stuff.
    PLASSERT( m_kernel && m_gram_inverse.isNotNull() );
    real base_sigma_sq = m_kernel(m_actual_input, m_actual_input);
    m_gram_inverse_product.resize(m_kernel_evaluations.size());
    product(m_gram_inverse_product, m_gram_inverse, m_kernel_evaluations);
    real sigma_reductor = dot(m_gram_inverse_product, m_kernel_evaluations);
    real sigma = sqrt(max(0., base_sigma_sq - sigma_reductor));

    // two-tailed
    const real multiplier = gauss_01_quantile((1+probability)/2);
    real half_width = multiplier * sigma;
    intervals.resize(output.size());
    for (int i=0, n=output.size() ; i<n ; ++i)
        intervals[i] = std::make_pair(output[i] - half_width,
                                      output[i] + half_width);
    return true;
}


TVec<string> GaussianProcessRegressor::getTestCostNames() const
{
    return TVec<string>(1, "mse");
}


TVec<string> GaussianProcessRegressor::getTrainCostNames() const
{
    return TVec<string>(1, "mse");
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
