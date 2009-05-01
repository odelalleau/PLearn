// -*- C++ -*-

// GaussianProcessRegressor.cc
//
// Copyright (C) 2006-2009 Nicolas Chapados 
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

/*! \file PLearn/plearn_learners/regressors/GaussianProcessRegressor.cc */

#define PL_LOG_MODULE_NAME "GaussianProcessRegressor"

// From PLearn
#include "GaussianProcessRegressor.h"
#include <plearn/base/stringutils.h>
#include <plearn/vmat/ExtendedVMatrix.h>
#include <plearn/math/pl_erf.h>
#include <plearn/var/GaussianProcessNLLVariable.h>
#include <plearn/var/ObjectOptionVariable.h>
#include <plearn/opt/Optimizer.h>
#include <plearn/io/pl_log.h>

#ifdef USE_BLAS_SPECIALISATIONS
#include <plearn/math/plapack.h>
#endif

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    GaussianProcessRegressor,
    "Implements Gaussian Process Regression (GPR) with an arbitrary kernel",
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
    "where the element (i,j) is equal to K(xi, xj), lambda is the VARIANCE of\n"
    "the observation noise (and can be interpreted as a weight decay\n"
    "coefficient), and y is the vector of training-set targets.\n"
    "\n"
    "The uncertainty in a prediction can be computed by calling\n"
    "computeConfidenceFromOutput.  Furthermore, if desired, this learner allows\n"
    "optimization of the kernel hyperparameters by direct optimization of the\n"
    "marginal likelihood w.r.t. the hyperparameters.  This mechanism relies on a\n"
    "user-provided Optimizer (see the 'optimizer' option) and does not rely on\n"
    "the PLearn HyperLearner system.\n"
    "\n"
    "GaussianProcessRegressor produces the following train costs:\n"
    "\n"
    "- \"nmll\" : the negative marginal log-likelihood on the training set.\n"
    "- \"mse\"  : the mean-squared error on the training set (by convention,\n"
    "           divided by two)\n"
    "\n"
    "and the following test costs:\n"
    "\n"
    "- \"nll\" : the negative log-likelihood of the test example under the\n"
    "          predictive distribution.  Available only if the option\n"
    "          'compute_confidence' is true.\n"
    "- \"mse\" : the squared error of the test example with respect to the\n"
    "          predictive mean (by convention, divided by two).\n"
    "\n"
    "The disadvantage of this learner is that its training time is O(N^3) in the\n"
    "number of training examples (due to the matrix inversion).  When saving the\n"
    "learner, the training set inputs must be saved, along with an additional\n"
    "matrix of length number-of-training-examples, and width number-of-targets.\n"
    "\n"
    "To alleviate the computational bottleneck of the exact method, the sparse\n"
    "approximation method of Projected Process is also available.  This method\n"
    "requires identifying M datapoints in the training set called the active\n"
    "set, although it makes use of all N training points for computing the\n"
    "likelihood.  The computational complexity of the approach is then O(NM^2).\n"
    "Note that in the current implementation, hyperparameter optimization is\n"
    "performed using ONLY the active set (called the \"Subset of Data\" method in\n"
    "the Rasmussen & Williams book).  Making use of the full set of datapoints\n"
    "is more computationally expensive and would require substantial updates to\n"
    "the PLearn Kernel class (to efficiently support asymmetric kernel-matrix\n"
    "gradient).  This may come later.\n"
    );

GaussianProcessRegressor::GaussianProcessRegressor() 
    : m_weight_decay(0.0),
      m_include_bias(true),
      m_compute_confidence(false),
      m_confidence_epsilon(1e-8),
      m_save_gram_matrix(false),
      m_solution_algorithm("exact")
{ }


void GaussianProcessRegressor::declareOptions(OptionList& ol)
{
    //#####  Build Options  ###################################################
    
    declareOption(
        ol, "kernel", &GaussianProcessRegressor::m_kernel,
        OptionBase::buildoption,
        "Kernel to use for the computation.  This must be a similarity kernel\n"
        "(i.e. closer vectors give higher kernel evaluations).");

    declareOption(
        ol, "weight_decay", &GaussianProcessRegressor::m_weight_decay,
        OptionBase::buildoption,
        "Weight decay coefficient (default = 0)");

    declareOption(
        ol, "include_bias", &GaussianProcessRegressor::m_include_bias,
        OptionBase::buildoption,
        "Whether to include a bias term in the regression (true by default)\n"
        "The effect of this option is NOT to prepend a column of 1 to the inputs\n"
        "(which has often no effect for GP regression), but to estimate a\n"
        "separate mean of the targets, perform the GP regression on the\n"
        "zero-mean targets, and add it back when computing the outputs.\n");

    declareOption(
        ol, "compute_confidence", &GaussianProcessRegressor::m_compute_confidence,
        OptionBase::buildoption,
        "Whether to perform the additional train-time computations required\n"
        "to compute confidence intervals.  This includes computing a separate\n"
        "inverse of the Gram matrix.  Specification of this option is necessary\n"
        "for calling both computeConfidenceFromOutput and computeOutputCovMat.\n");

    declareOption(
        ol, "confidence_epsilon", &GaussianProcessRegressor::m_confidence_epsilon,
        OptionBase::buildoption,
        "Small regularization to be added post-hoc to the computed output\n"
        "covariance matrix and confidence intervals; this is mostly used as a\n"
        "disaster prevention device, to avoid negative predictive variance\n");
    
    declareOption(
        ol, "hyperparameters", &GaussianProcessRegressor::m_hyperparameters,
        OptionBase::buildoption,
        "List of hyperparameters to optimize.  They must be specified in the\n"
        "form \"option-name\":initial-value, where 'option-name' is the name\n"
        "of an option to set within the Kernel object (the array-index form\n"
        "'option[i]' is supported), and 'initial-value' is the\n"
        "(PLearn-serialization string representation) for starting point for the\n"
        "optimization.  Currently, the hyperparameters are constrained to be\n"
        "scalars.\n");

    declareOption(
        ol, "ARD_hyperprefix_initval",
        &GaussianProcessRegressor::m_ARD_hyperprefix_initval,
        OptionBase::buildoption,
        "If the kernel support automatic relevance determination (ARD; e.g.\n"
        "SquaredExponentialARDKernel), the list of hyperparameters corresponding\n"
        "to each input can be created automatically by giving an option prefix\n"
        "and an initial value.  The ARD options are created to have the form\n"
        "\n"
        "   'prefix[0]', 'prefix[1]', 'prefix[N-1]'\n"
        "\n"
        "where N is the number of inputs.  This option is useful when the\n"
        "dataset inputsize is not (easily) known ahead of time. \n");
    
    declareOption(
        ol, "optimizer", &GaussianProcessRegressor::m_optimizer,
        OptionBase::buildoption,
        "Specification of the optimizer to use for train-time hyperparameter\n"
        "optimization.  A ConjGradientOptimizer should be an adequate choice.\n");

    declareOption(
        ol, "save_gram_matrix", &GaussianProcessRegressor::m_save_gram_matrix,
        OptionBase::buildoption,
        "If true, the Gram matrix is saved before undergoing Cholesky each\n"
        "decomposition; useful for debugging if the matrix is quasi-singular.\n"
        "It is saved in the current expdir under the names 'gram_matrix_N.pmat'\n"
        "where N is an increasing counter.\n");

    declareOption(
        ol, "solution_algorithm", &GaussianProcessRegressor::m_solution_algorithm,
        OptionBase::buildoption,
        "Solution algorithm used for the regression.  If \"exact\", use the exact\n"
        "Gaussian process solution (requires O(N^3) computation).  If\n"
        "\"projected-process\", use the PP approximation, which requires O(MN^2)\n"
        "computation, where M is given by the size of the active training\n"
        "examples specified by the \"active-set\" option.  Default=\"exact\".\n");

    declareOption(
        ol, "active_set_indices", &GaussianProcessRegressor::m_active_set_indices,
        OptionBase::buildoption,
        "If a sparse approximation algorithm is used (e.g. projected process),\n"
        "this specifies the indices of the training-set examples which should be\n"
        "considered to be part of the active set.  Note that these indices must\n"
        "be SORTED IN INCREASING ORDER and should not contain duplicates.\n");

    
    //#####  Learnt Options  ##################################################

    declareOption(
        ol, "alpha", &GaussianProcessRegressor::m_alpha,
        OptionBase::learntoption,
        "Matrix of learned parameters, determined from the equation\n"
        "\n"
        "  (K + lambda I)^-1 y\n"
        "\n"
        "(don't forget that y can be a matrix for multivariate output problems)\n"
        "\n"
        "In the case of the projected-process approximation, this contains\n"
        "the result of the equiation\n"
        "\n"
        "  (lambda K_mm + K_mn K_nm)^-1 K_mn y\n");

    declareOption(
        ol, "gram_inverse", &GaussianProcessRegressor::m_gram_inverse,
        OptionBase::learntoption,
        "Inverse of the Gram matrix, used to compute confidence intervals (must\n"
        "be saved since the confidence intervals are obtained from the equation\n"
        "\n"
        "  sigma^2 = k(x,x) - k(x)'(K + lambda I)^-1 k(x)\n"
        "\n"
        "An adjustment similar to 'alpha' is made for the projected-process\n"
        "approximation.\n");

    declareOption(
        ol, "subgram_inverse", &GaussianProcessRegressor::m_subgram_inverse,
        OptionBase::learntoption,
        "Inverse of the sub-Gram matrix, i.e. K_mm^-1.  Used only with the\n"
        "projected-process approximation.\n");
    
    declareOption(
        ol, "target_mean", &GaussianProcessRegressor::m_target_mean,
        OptionBase::learntoption,
        "Mean of the targets, if the option 'include_bias' is true");
    
    declareOption(
        ol, "training_inputs", &GaussianProcessRegressor::m_training_inputs,
        OptionBase::learntoption,
        "Saved version of the training set, which must be kept along for\n"
        "carrying out kernel evaluations with the test point.  If using the\n"
        "projected-process approximation, only the inputs in the active set are\n"
        "saved.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void GaussianProcessRegressor::build_()
{
    if (! m_kernel)
        PLERROR("GaussianProcessRegressor::build_: 'kernel' option must be specified");

    if (! m_kernel->is_symmetric)
        PLERROR("GaussianProcessRegressor::build_: the kernel (%s) must be symmetric",
                m_kernel->classname().c_str());
    
    // If we are reloading the model, set the training inputs into the kernel
    if (m_training_inputs.size() > 0)
        m_kernel->setDataForKernelMatrix(m_training_inputs);

    // If we specified hyperparameters without an optimizer, complain.
    // (It is mildly legal to specify an optimizer without hyperparameters;
    // this does nothing).
    if (m_hyperparameters.size() > 0 && ! m_optimizer)
        PLERROR("GaussianProcessRegressor::build_: 'hyperparameters' are specified "
                "but no 'optimizer'; an optimizer is required in order to carry out "
                "hyperparameter optimization");

    if (m_confidence_epsilon < 0)
        PLERROR("GaussianProcessRegressor::build_: 'confidence_epsilon' must be non-negative");

    // Cache solution algorithm in quick form
    if (m_solution_algorithm == "exact")
        m_algorithm_enum = AlgoExact;
    else if (m_solution_algorithm == "projected-process")
        m_algorithm_enum = AlgoProjectedProcess;
    else
        PLERROR("GaussianProcessRegressor::build_: the option solution_algorithm=='%s' "
                "is not supported.  Value must be in {'exact', 'projected-process'}",
                m_solution_algorithm.c_str());
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

    deepCopyField(m_kernel,                     copies);
    deepCopyField(m_hyperparameters,            copies);
    deepCopyField(m_optimizer,                  copies);
    deepCopyField(m_active_set_indices,         copies);
    deepCopyField(m_alpha,                      copies);
    deepCopyField(m_gram_inverse,               copies);
    deepCopyField(m_subgram_inverse,            copies);
    deepCopyField(m_target_mean,                copies);
    deepCopyField(m_training_inputs,            copies);
    deepCopyField(m_kernel_evaluations,         copies);
    deepCopyField(m_gram_inverse_product,       copies);
    deepCopyField(m_intervals,                  copies);
    deepCopyField(m_gram_traintest_inputs,      copies);
    deepCopyField(m_gram_inv_traintest_product, copies);
    deepCopyField(m_sigma_reductor,             copies);
}


//#####  setTrainingSet  ######################################################

void GaussianProcessRegressor::setTrainingSet(VMat training_set, bool call_forget)
{
    PLASSERT( training_set );
    int inputsize = training_set->inputsize() ;
    if (inputsize < 0)
        PLERROR("GaussianProcessRegressor::setTrainingSet: the training set inputsize "
                "must be specified (current value = %d)", inputsize);

    // Convert to a real matrix in order to make saving it saner
    m_training_inputs = training_set.subMatColumns(0, inputsize).toMat();
    inherited::setTrainingSet(training_set, call_forget);
}


//#####  outputsize  ##########################################################

int GaussianProcessRegressor::outputsize() const
{
    return targetsize();
}


//#####  forget  ##############################################################

void GaussianProcessRegressor::forget()
{
    inherited::forget();
    if (m_optimizer)
        m_optimizer->reset();
    m_alpha.resize(0,0);
    m_target_mean.resize(0);
    m_gram_inverse.resize(0,0);
    stage = 0;
}
    

//#####  train  ###############################################################

void GaussianProcessRegressor::train()
{
    // This generic PLearner method does a number of standard stuff useful for
    // (almost) any learner, and return 'false' if no training should take
    // place. See PLearner.h for more details.
    if (!initTrain())
        return;

    // If we use the projected process approximation, make sure that the
    // active-set indices are specified and that they are sorted in increasing
    // order
    if (m_algorithm_enum == AlgoProjectedProcess) {
        if (m_active_set_indices.size() == 0)
            PLERROR("GaussianProcessRegressor::train: with the projected-process "
                    "approximation, the active_set_indices option must be specified.");
        int last_index = -1;
        for (int i=0, n=m_active_set_indices.size() ; i<n ; ++i) {
            int cur_index = m_active_set_indices[i];
            if (cur_index <= last_index)
                PLERROR("GaussianProcessRegressor::train: the option active_set_indices "
                        "must be sorted and should not contain duplicates; at index %d, "
                        "encounted value %d whereas previous value was %d.",
                        i, cur_index, last_index);
            last_index = cur_index;
        }
    }
    
    PLASSERT( m_kernel );
    if (! train_set || ! m_training_inputs)
        PLERROR("GaussianProcessRegressor::train: the training set must be specified");
    int trainlength = train_set->length();
    int activelength= ( m_algorithm_enum == AlgoProjectedProcess?
                        m_active_set_indices.size() : trainlength );
    int inputsize   = train_set->inputsize() ;
    int targetsize  = train_set->targetsize();
    int weightsize  = train_set->weightsize();
    if (inputsize  < 0 || targetsize < 0 || weightsize < 0)
        PLERROR("GaussianProcessRegressor::train: inconsistent inputsize/targetsize/weightsize "
                "(%d/%d/%d) in training set", inputsize, targetsize, weightsize);
    if (weightsize > 0)
        PLERROR("GaussianProcessRegressor::train: observations weights are not currently supported");

    // Subtract the mean if we require it
    Mat targets(trainlength, targetsize);
    train_set.subMatColumns(inputsize, targetsize)->getMat(0,0,targets);
    if (m_include_bias) {
        m_target_mean.resize(targets.width());
        columnMean(targets, m_target_mean);
        targets -= m_target_mean;
    }

    // Determine the subset of training inputs and targets to use depending on
    // the training algorithm
    Mat sub_training_inputs;
    Mat sub_training_targets;
    if (m_algorithm_enum == AlgoExact) {
        sub_training_inputs = m_training_inputs;
        sub_training_targets= targets;
    }
    else if (m_algorithm_enum == AlgoProjectedProcess) {
        sub_training_inputs .resize(activelength, inputsize);
        sub_training_targets.resize(activelength, targetsize);
        selectRows(m_training_inputs, m_active_set_indices, sub_training_inputs);
        selectRows(targets,           m_active_set_indices, sub_training_targets);
    }
    
    // Optimize hyperparameters
    VarArray hyperparam_vars;
    PP<GaussianProcessNLLVariable> nll =
        hyperOptimize(sub_training_inputs, sub_training_targets, hyperparam_vars);
    PLASSERT( nll );
    
    // Compute parameters.  Be careful to also propagate through the
    // hyperparameter variables to ensure the latest values are correctly set
    // into their respective kernels.
    hyperparam_vars.fprop();
    nll->fprop();
    if (m_algorithm_enum == AlgoExact) {
        m_alpha = nll->alpha();
        m_gram_inverse = nll->gramInverse();
    }
    else if (m_algorithm_enum == AlgoProjectedProcess) {
        trainProjectedProcess(m_training_inputs, sub_training_inputs, targets);

        // Full training set no longer required from now on
        m_training_inputs = sub_training_inputs;
        m_kernel->setDataForKernelMatrix(m_training_inputs);
    }

    if (getTrainStatsCollector()) {
        // Compute train statistics by running a test over the training set.
        // This works uniformly for all solution algorithms, albeit with some
        // performance hit.
        PP<VecStatsCollector> test_stats = new VecStatsCollector;
        test(getTrainingSet(), test_stats);
    
        // And accumulate some statistics.  Note: the NLL corresponds to the
        // subset-of-data version if the projected-process approximation is
        // used.  It is the exact NLL if the exact algorithm is used.
        Vec costs(3);
        costs.subVec(0,2) << test_stats->getMean();
        costs[2] = nll->value[0];
        getTrainStatsCollector()->update(costs);
    }
    MODULE_LOG << "Train marginal NLL (subset-of-data): " << nll->value[0] << endl;
}


//#####  computeOutput  #######################################################

void GaussianProcessRegressor::computeOutput(const Vec& input, Vec& output) const
{
    PLASSERT( m_kernel && m_alpha.isNotNull() && m_training_inputs.size() > 0 );
    PLASSERT( m_alpha.width()  == output.size() );
    PLASSERT( m_alpha.length() == m_training_inputs.length() );
    PLASSERT( input.size()     == m_training_inputs.width()  );

    m_kernel_evaluations.resize(m_alpha.length());
    computeOutputAux(input, output, m_kernel_evaluations);
}


void GaussianProcessRegressor::computeOutputAux(
    const Vec& input, Vec& output, Vec& kernel_evaluations) const
{
    if (input.hasMissing()) {
        output.fill(MISSING_VALUE);
        kernel_evaluations.fill(MISSING_VALUE);
        return;
    }
    
    m_kernel->evaluate_all_i_x(input, kernel_evaluations);

    // Finally compute k(x,x_i) * (K + \lambda I)^-1 y.
    // This expression does not change depending on whether we are using
    // the exact algorithm or the projected-process approximation.
    product(Mat(1, output.size(), output),
            Mat(1, kernel_evaluations.size(), kernel_evaluations),
            m_alpha);

    if (m_include_bias)
        output += m_target_mean;
}


//#####  computeCostsFromOutputs  #############################################

void GaussianProcessRegressor::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                                       const Vec& target, Vec& costs) const
{
    costs.resize(2);

    // NLL cost is the NLL of the target under the predictive distribution
    // (centered at predictive mean, with variance obtainable from the
    // confidence bounds).  HOWEVER, to obain it, we have to be able to compute
    // the confidence bounds.  If impossible, simply set missing-value for the
    // NLL cost.
    if (m_compute_confidence) {
#ifdef BOUNDCHECK
        static const float PROBABILITY = pl_erf(1. / (2*sqrt(2.0)));  // 0.5 stddev
        bool confavail = computeConfidenceFromOutput(input, output, PROBABILITY,
                                                     m_intervals);
#endif
        PLASSERT( confavail && m_intervals.size() == output.size() &&
                  output.size() == target.size() );
        static const real LN_2PI_OVER_2 = pl_log(2*M_PI) / 2.0;
        real nll = 0;
        for (int i=0, n=output.size() ; i<n ; ++i) {
            real sigma = m_intervals[i].second - m_intervals[i].first;
            sigma = max(sigma, real(1e-15));        // Very minor regularization
            real diff = target[i] - output[i];
            nll += diff*diff / (2.*sigma*sigma) + pl_log(sigma) + LN_2PI_OVER_2;
        }
        costs[0] = nll;
    }
    else
        costs[0] = MISSING_VALUE;
    
    real squared_loss = 0.5*powdistance(output,target);
    costs[1] = squared_loss;
}     


//#####  computeConfidenceFromOutput  #########################################

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

    // BIG assumption: assume that computeOutput has just been called and that
    // m_kernel_evaluations contains the right stuff.
    PLASSERT( m_kernel && m_gram_inverse.isNotNull() );
    real base_sigma_sq = m_kernel(input, input);
    m_gram_inverse_product.resize(m_kernel_evaluations.size());

    real sigma;
    if (m_algorithm_enum == AlgoExact) {
        product(m_gram_inverse_product, m_gram_inverse, m_kernel_evaluations);
        real sigma_reductor = dot(m_gram_inverse_product, m_kernel_evaluations);
        sigma = sqrt(max(real(0.),
                         base_sigma_sq - sigma_reductor + m_confidence_epsilon));
    }
    else if (m_algorithm_enum == AlgoProjectedProcess) {
        // From R&W eq. (8.27).
        product(m_gram_inverse_product, m_subgram_inverse, m_kernel_evaluations);
        productScaleAcc(m_gram_inverse_product, m_gram_inverse, m_kernel_evaluations,
                        -1.0, 1.0);
        real sigma_reductor = dot(m_gram_inverse_product, m_kernel_evaluations);
        sigma = sqrt(max(real(0.),
                         base_sigma_sq - sigma_reductor + m_confidence_epsilon));
    }

    // two-tailed
    const real multiplier = gauss_01_quantile((1+probability)/2);
    real half_width = multiplier * sigma;
    intervals.resize(output.size());
    for (int i=0, n=output.size() ; i<n ; ++i)
        intervals[i] = std::make_pair(output[i] - half_width,
                                      output[i] + half_width);
    return true;
}


//#####  computeOutputCovMat  #################################################

void GaussianProcessRegressor::computeOutputCovMat(
    const Mat& inputs, Mat& outputs, TVec<Mat>& covariance_matrices) const
{
    PLASSERT( m_kernel && m_alpha.isNotNull() && m_training_inputs.size() > 0 );
    PLASSERT( m_alpha.width()  == outputsize() );
    PLASSERT( m_alpha.length() == m_training_inputs.length() );
    PLASSERT( inputs.width()   == m_training_inputs.width()  );
    PLASSERT( inputs.width()   == inputsize() );
    const int N = inputs.length();
    const int M = outputsize();
    const int T = m_training_inputs.length();
    outputs.resize(N, M);
    covariance_matrices.resize(M);

    // Preallocate space for the covariance matrix, and since all outputs share
    // the same matrix, copy it into the remaining elements of
    // covariance_matrices
    Mat& covmat = covariance_matrices[0];
    covmat.resize(N, N);
    for (int j=1 ; j<M ; ++j)
        covariance_matrices[j] = covmat;

    // Start by computing the matrix of kernel evaluations between the train
    // and test outputs, and compute the output
    m_gram_traintest_inputs.resize(N, T);
    bool has_missings = false;
    for (int i=0 ; i<N ; ++i) {
        Vec cur_traintest_kereval = m_gram_traintest_inputs(i);
        Vec cur_output = outputs(i);
        computeOutputAux(inputs(i), cur_output, cur_traintest_kereval);
        has_missings = has_missings || inputs(i).hasMissing();
    }

    // If any missings found in the inputs, don't bother with computing a
    // covariance matrix
    if (has_missings) {
        covmat.fill(MISSING_VALUE);
        return;
    }

    // Next compute the kernel evaluations between the test inputs; more or
    // less lifted from Kernel.cc ==> must see with Olivier how to better
    // factor this code
    Mat& K = covmat;

    PLASSERT( K.width() == N && K.length() == N );
    const int mod = K.mod();
    real Kij;
    real* Ki;
    real* Kji;
    for (int i=0 ; i<N ; ++i) {
        Ki  = K[i];
        Kji = &K[0][i];
        const Vec& cur_input_i = inputs(i);
        for (int j=0 ; j<=i ; ++j, Kji += mod) {
            Kij = m_kernel->evaluate(cur_input_i, inputs(j));
            *Ki++ = Kij;
            if (j<i)
                *Kji = Kij;    // Assume symmetry, checked at build
        }
    }

    // The predictive covariance matrix is for the exact cast(c.f. Rasmussen
    // and Williams):
    //
    //    cov(f*) = K(X*,X*) - K(X*,X) [K(X,X) + sigma*I]^-1 K(X,X*)
    //
    // where X are the training inputs, and X* are the test inputs.
    //
    // For the projected process case, it is:
    //
    //    cov(f*) = K(X*,X*) - K(X*,X_m) K_mm^-1 K(X*,X_m)
    //               + sigma^2 K(X*,X_m) (sigma^2 K_mm + K_mn K_nm)^-1 K(X*,X_m)
    //
    // Note that all sigma^2's have been absorbed into their respective
    // cached terms, and in particular in this context sigma^2 is emphatically
    // not equal to the weight decay.
    m_gram_inv_traintest_product.resize(T,N);
    m_sigma_reductor.resize(N,N);

    if (m_algorithm_enum == AlgoExact) {    
        productTranspose(m_gram_inv_traintest_product, m_gram_inverse,
                         m_gram_traintest_inputs);
        product(m_sigma_reductor, m_gram_traintest_inputs,
                m_gram_inv_traintest_product);
    }
    else if (m_algorithm_enum == AlgoProjectedProcess) {
        productTranspose(m_gram_inv_traintest_product, m_subgram_inverse,
                         m_gram_traintest_inputs);
        productTransposeScaleAcc(m_gram_inv_traintest_product, m_gram_inverse,
                                 m_gram_traintest_inputs, -1.0, 1.0);
        product(m_sigma_reductor, m_gram_traintest_inputs,
                m_gram_inv_traintest_product);
    }
    
    covmat -= m_sigma_reductor;

    // As a preventive measure, never output negative variance, even though
    // this does not garantee the non-negative-definiteness of the matrix
    for (int i=0 ; i<N ; ++i)
        covmat(i,i) = max(real(0.0), covmat(i,i) + m_confidence_epsilon);
}


//#####  get*CostNames  #######################################################

TVec<string> GaussianProcessRegressor::getTestCostNames() const
{
    TVec<string> c(2);
    c[0] = "nll";
    c[1] = "mse";
    return c;
}


TVec<string> GaussianProcessRegressor::getTrainCostNames() const
{
    TVec<string> c(3);
    c[0] = "nll";
    c[1] = "mse";
    c[2] = "marginal-nll";
    return c;
}


//#####  hyperOptimize  #######################################################

PP<GaussianProcessNLLVariable>
GaussianProcessRegressor::hyperOptimize(const Mat& inputs, const Mat& targets,
                                        VarArray& hyperparam_vars)
{
    // If there are no hyperparameters or optimizer, just create a simple
    // variable and return it right away.
    if (! m_optimizer || (m_hyperparameters.size() == 0 &&
                          m_ARD_hyperprefix_initval.first.empty()) )
    {
        return new GaussianProcessNLLVariable(
            m_kernel, m_weight_decay, inputs, targets,
            TVec<string>(), VarArray(), m_compute_confidence,
            m_save_gram_matrix, getExperimentDirectory());
    }

    // Otherwise create Vars that wrap each hyperparameter
    const int numhyper  = m_hyperparameters.size();
    const int numinputs = ( ! m_ARD_hyperprefix_initval.first.empty() ?
                            inputsize() : 0 );
    hyperparam_vars = VarArray(numhyper + numinputs);
    TVec<string> hyperparam_names(numhyper + numinputs);
    int i;
    for (i=0 ; i<numhyper ; ++i) {
        hyperparam_names[i] = m_hyperparameters[i].first;
        hyperparam_vars [i] = new ObjectOptionVariable(
            (Kernel*)m_kernel, m_hyperparameters[i].first, m_hyperparameters[i].second);
        hyperparam_vars[i]->setName(m_hyperparameters[i].first);
    }

    // If specified, create the Vars for automatic relevance determination
    string& ARD_name = m_ARD_hyperprefix_initval.first;
    string& ARD_init = m_ARD_hyperprefix_initval.second;
    if (! ARD_name.empty()) {
        // Small hack to ensure the ARD vector in the kernel has proper size
        Vec init(numinputs, lexical_cast<double>(ARD_init));
        m_kernel->changeOption(ARD_name, tostring(init, PStream::plearn_ascii));
        
        for (int j=0 ; j<numinputs ; ++j, ++i) {
            hyperparam_names[i] = ARD_name + '[' + tostring(j) + ']';
            hyperparam_vars [i] = new ObjectOptionVariable(
                (Kernel*)m_kernel, hyperparam_names[i], ARD_init);
            hyperparam_vars [i]->setName(hyperparam_names[i]);
        }
    }

    // Create the cost-function variable
    PP<GaussianProcessNLLVariable> nll = new GaussianProcessNLLVariable(
        m_kernel, m_weight_decay, inputs, targets, hyperparam_names,
        hyperparam_vars, true, m_save_gram_matrix, getExperimentDirectory());
    nll->setName("GaussianProcessNLLVariable");

    // Some logging about the initial values
    GaussianProcessNLLVariable::logVarray(hyperparam_vars,
                                          "Hyperparameter initial values:");
    
    // And optimize for nstages
    m_optimizer->setToOptimize(hyperparam_vars, (Variable*)nll);
    m_optimizer->build();
    PP<ProgressBar> pb(
        report_progress? new ProgressBar("Training GaussianProcessRegressor "
                                         "from stage " + tostring(stage) + " to stage " +
                                         tostring(nstages), nstages-stage)
        : 0);
    bool early_stopping = false;
    PP<VecStatsCollector> statscol = new VecStatsCollector;
    for (const int initial_stage = stage ; !early_stopping && stage < nstages
             ; ++stage)
    {
        if (pb)
            pb->update(stage - initial_stage);

        statscol->forget();
        early_stopping = m_optimizer->optimizeN(*statscol);
        statscol->finalize();
    }
    pb = 0;                                  // Finish progress bar right now

    // Some logging about the final values
    GaussianProcessNLLVariable::logVarray(hyperparam_vars,
                                          "Hyperparameter final values:");
    return nll;
}


//#####  trainProjectedProcess (LAPACK)  ######################################

void GaussianProcessRegressor::trainProjectedProcess(
    const Mat& all_training_inputs, const Mat& sub_training_inputs,
    const Mat& all_training_targets)
{
    PLASSERT( m_kernel );
    const int activelength= m_active_set_indices.length();
    const int trainlength = all_training_inputs.length();
    const int targetsize  = all_training_targets.width();
    
    // The RHS matrix (when solving the linear system Gram*Params=RHS) is made
    // up of two parts: the regression targets themselves, and the identity
    // matrix if we requested them (for confidence intervals).  After solving
    // the linear system, set the gram-inverse appropriately.  To interface
    // nicely with LAPACK, we store this in a transposed format.
    int rhs_width = targetsize + (m_compute_confidence? activelength : 0);
    Mat tmp_rhs(rhs_width, activelength);
    if (m_compute_confidence) {
        Mat rhs_identity = tmp_rhs.subMatRows(targetsize, activelength);
        identityMatrix(rhs_identity);
    }

    // We always need to solve K_mm^-1.  Prepare the RHS with the identity
    // matrix to be ready to solve with a Cholesky decomposition.
    m_subgram_inverse.resize(activelength, activelength);
    Mat gram_cholesky(activelength, activelength);
    identityMatrix(m_subgram_inverse);
    
    // Compute Gram Matrix and add weight decay to diagonal.  This is done in a
    // few steps: (1) K_mm (using the active-set only), (2) then separately
    // compute K_mn (active-set by all examples), (3) computing the covariance
    // matrix of K_mn to give an m x m matrix, (4) and finally add them up.
    // cf. R&W p. 179, eq. 8.26 :: (sigma_n^2 K_mm + K_mn K_nm)
    m_kernel->setDataForKernelMatrix(all_training_inputs);
    Mat gram(activelength, activelength);
    Mat asym_gram(activelength, trainlength);
    Vec self_cov(activelength);
    m_kernel->computeTestGramMatrix(sub_training_inputs, asym_gram, self_cov);
    // Note: asym_gram contains K_mn without any sampling noise.

    // DBG_MODULE_LOG << "Asym_gram =\n" << asym_gram << endl;
    
    // Obtain K_mm, also without self-noise.  Add some jitter as per
    // the Rasmussen & Williams code
    selectColumns(asym_gram, m_active_set_indices, gram);
    real jitter = m_weight_decay * trace(gram);
    addToDiagonal(gram, jitter);

    // DBG_MODULE_LOG << "Kmm =\n" << gram << endl;
    
    // Obtain an estimate of the EFFECTIVE sampling noise from the
    // difference between self_cov and the diagonal of gram
    Vec sigma_sq = self_cov - diag(gram);
    for (int i=0, n=sigma_sq.size() ; i<n ; ++i) // ensure does not get negative
        sigma_sq[i] = max(m_weight_decay, sigma_sq[i]);
    double sigma_sq_est = mean(sigma_sq);
    // DBG_MODULE_LOG << "Sigma^2 estimate = " << sigma_sq_est << endl;

    // Before clobbering K_mm, compute its inverse.
    gram_cholesky << gram;
    lapackCholeskyDecompositionInPlace(gram_cholesky);
    lapackCholeskySolveInPlace(gram_cholesky, m_subgram_inverse,
                               true /* column-major */);
    
    gram *= sigma_sq_est;                            // sigma_n^2 K_mm
    productTransposeAcc(gram, asym_gram, asym_gram); // Inner part of eq. 8.26

    // DBG_MODULE_LOG << "Gram =\n" << gram << endl;
    
    // Dump a fragment of the Gram Matrix to the debug log
    DBG_MODULE_LOG << "Projected-process Gram fragment: "
                   << gram(0,0) << ' '
                   << gram(1,0) << ' '
                   << gram(1,1) << endl;

    // The RHS should contain (K_mn*y)' = y'*K_mn'.  Compute it.
    Mat targets_submat = tmp_rhs.subMatRows(0, targetsize);
    transposeTransposeProduct(targets_submat, all_training_targets, asym_gram);
    // DBG_MODULE_LOG << "Projected RHS =\n" << targets_submat << endl;
    
    // Compute Cholesky decomposition and solve the linear system.  LAPACK
    // solves in-place, but luckily we don't need either the Gram and RHS
    // matrices after solving.
    lapackCholeskyDecompositionInPlace(gram);
    lapackCholeskySolveInPlace(gram, tmp_rhs, true /* column-major */);

    // Transpose final result.  LAPACK solved in-place for tmp_rhs.
    m_alpha.resize(tmp_rhs.width(), tmp_rhs.length());
    transpose(tmp_rhs, m_alpha);
    if (m_compute_confidence) {
        m_gram_inverse = m_alpha.subMatColumns(targetsize, activelength);
        m_alpha        = m_alpha.subMatColumns(0, targetsize);

        // Absorb sigma^2 into gram_inverse as per eq. 8.27 of R&W
        m_gram_inverse *= sigma_sq_est;
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
