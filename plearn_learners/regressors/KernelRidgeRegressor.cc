// -*- C++ -*-

// KernelRidgeRegressor.cc
//
// Copyright (C) 2005 Nicolas Chapados 
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

/*! \file KernelRidgeRegressor.cc */

// From PLearn
#include <plearn/vmat/ExtendedVMatrix.h>

#include "KernelRidgeRegressor.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    KernelRidgeRegressor,
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


KernelRidgeRegressor::KernelRidgeRegressor() 
    : m_include_bias(true),
      m_weight_decay(0.0)
{ }


void KernelRidgeRegressor::declareOptions(OptionList& ol)
{
    declareOption(ol, "kernel", &KernelRidgeRegressor::m_kernel,
                  OptionBase::buildoption,
                  "Kernel to use for the computation.  This must be a similarity kernel\n"
                  "(i.e. closer vectors give higher kernel evaluations).");

    declareOption(ol, "weight_decay", &KernelRidgeRegressor::m_weight_decay,
                  OptionBase::buildoption,
                  "Weight decay coefficient (default = 0)");

    declareOption(ol, "include_bias", &KernelRidgeRegressor::m_include_bias,
                  OptionBase::buildoption,
                  "Whether to include a bias term in the regression (true by default)");

    declareOption(ol, "params", &KernelRidgeRegressor::m_params,
                  OptionBase::learntoption,
                  "Vector of learned parameters, determined from the equation\n"
                  "    (M + lambda I)^-1 y");

    declareOption(ol, "training_inputs", &KernelRidgeRegressor::m_training_inputs,
                  OptionBase::learntoption,
                  "Saved version of the training set, which must be kept along for\n"
                  "carrying out kernel evaluations with the test point");
    
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void KernelRidgeRegressor::build_()
{
    if (! m_kernel)
        PLERROR("KernelRidgeRegressor::build_: 'kernel' option must be specified");

    // If we are reloading the model, set the training inputs into the kernel
    if (m_training_inputs)
        m_kernel->setDataForKernelMatrix(m_training_inputs);
}

// ### Nothing to add here, simply calls build_
void KernelRidgeRegressor::build()
{
    inherited::build();
    build_();
}


void KernelRidgeRegressor::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(m_kernel,          copies);
    deepCopyField(m_params,          copies);
    deepCopyField(m_training_inputs, copies);
}


int KernelRidgeRegressor::outputsize() const
{
    return targetsize();
}

void KernelRidgeRegressor::forget()
{
    m_params.resize(0,0);
    m_training_inputs = 0;
}
    
void KernelRidgeRegressor::train()
{
    PLASSERT( m_kernel );
    if (! train_set)
        PLERROR("KernelRidgeRegressor::train: the training set must be specified");
    int inputsize  = train_set->inputsize() ;
    int targetsize = train_set->targetsize();
    int weightsize = train_set->weightsize();
    if (inputsize  < 0 || targetsize < 0 || weightsize < 0)
        PLERROR("KernelRidgeRegressor::train: inconsistent inputsize/targetsize/weightsize "
                "(%d/%d/%d) in training set", inputsize, targetsize, weightsize);
    if (weightsize > 0)
        PLERROR("KernelRidgeRegressor::train: observations weights are not currently supported");

    m_training_inputs = train_set.subMatColumns(0, inputsize).toMat();
    Mat targets       = train_set.subMatColumns(inputsize, targetsize).toMat();
    
    // Compute Gram Matrix and add weight decay to diagonal
    m_kernel->setDataForKernelMatrix(m_training_inputs);
    Mat gram_mat(m_training_inputs.length(), m_training_inputs.length());
    m_kernel->computeGramMatrix(gram_mat);
    addToDiagonal(gram_mat, m_weight_decay);

    // Compute parameters
    m_params.resize(targets.length(), targets.width());
    solveLinearSystemByCholesky(gram_mat, targets, m_params);

    // Compute train error if there is a train_stats_collector.  There is
    // probably an analytic formula, but ...
    if (getTrainStatsCollector())
        test(train_set, getTrainStatsCollector());
}


void KernelRidgeRegressor::computeOutput(const Vec& input, Vec& output) const
{
    PLASSERT( m_kernel && m_params.isNotNull() && m_training_inputs );

    PLASSERT( output.size() == m_params.width() );
    m_kernel_evaluations.resize(m_params.length());
    m_kernel->evaluate_all_x_i(input, m_kernel_evaluations);

    // Finally compute k(x,x_i) * (M + \lambda I)^-1 y
    product(Mat(1, output.size(), output),
            Mat(1, m_kernel_evaluations.size(), m_kernel_evaluations),
            m_params);
}    


void KernelRidgeRegressor::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                                   const Vec& target, Vec& costs) const
{
    costs.resize(1);
    real squared_loss = powdistance(output,target);
    costs[0] = squared_loss;
}     


TVec<string> KernelRidgeRegressor::getTestCostNames() const
{
    return TVec<string>(1, "mse");
}


TVec<string> KernelRidgeRegressor::getTrainCostNames() const
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
