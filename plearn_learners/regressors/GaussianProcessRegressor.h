// -*- C++ -*-

// GaussianProcessRegressor.h
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

/*! \file GaussianProcessRegressor.h */


#ifndef GaussianProcessRegressor_INC
#define GaussianProcessRegressor_INC

// From PLearn
#include <plearn/ker/Kernel.h>
#include <plearn_learners/generic/PLearner.h>

namespace PLearn {

class GaussianProcessNLLVariable;            //!< Forward-declare
class Optimizer;

/**
 *  Implements Gaussian Process Regression (GPR) with an arbitrary kernel
 *
 *  Given a kernel K(x,y) = phi(x)'phi(y), where phi(x) is the projection of a
 *  vector x into feature space, this class implements a version of Gaussian
 *  Process Regression, giving the prediction at x as
 *
 *      f(x) = k(x)'(M + lambda I)^-1 y,
 *
 *  where x is the test vector where to estimate the response, k(x) is the
 *  vector of kernel evaluations between the test vector and the elements of
 *  the training set, namely
 *
 *      k(x) = (K(x,x1), K(x,x2), ..., K(x,xN))',
 *
 *  M is the Gram Matrix on the elements of the training set, i.e. the matrix
 *  where the element (i,j) is equal to K(xi, xj), lambda is the VARIANCE of
 *  the observation noise (and can be interpreted as a weight decay
 *  coefficient), and y is the vector of training-set targets.
 *
 *  The uncertainty in a prediction can be computed by calling
 *  computeConfidenceFromOutput.  Furthermore, if desired, this learner allows
 *  optimization of the kernel hyperparameters by direct optimization of the
 *  marginal likelihood w.r.t. the hyperparameters.  This mechanism relies on a
 *  user-provided Optimizer (see the 'optimizer' option) and does not rely on
 *  the PLearn HyperLearner system.
 *
 *  GaussianProcessRegressor produces the following train costs:
 *
 *  - "nmll" : the negative marginal log-likelihood on the training set.
 *  - "mse"  : the mean-squared error on the training set (by convention,
 *             divided by two)
 *
 *  and the following test costs:
 *
 *  - "nll" : the negative log-likelihood of the test example under the
 *            predictive distribution.  Available only if the option
 *            'compute_confidence' is true.
 *  - "mse" : the squared error of the test example with respect to the
 *            predictive mean (by convention, divided by two).
 *
 *  The disadvantage of this learner is that its training time is O(N^3) in the
 *  number of training examples (due to the matrix inversion).  When saving the
 *  learner, the training set inputs must be saved, along with an additional
 *  matrix of length number-of-training-examples, and width number-of-targets.
 */
class GaussianProcessRegressor : public PLearner
{
    typedef PLearner inherited;

public:
    //#####  Public Build Options  ############################################

    /// Kernel to use for the computation.  This must be a similarity kernel
    /// (i.e. closer vectors give higher kernel evaluations).
    Ker m_kernel;

    /**
     *  Weight-decay coefficient (default = 0).  This is the lambda parameter
     *  in the class-help explanations, and corresponds to the variance of the
     *  observation noise in the gaussian process generative model.
     */
    real m_weight_decay;

    /**
     *  Whether to include a bias term in the regression (true by default).
     *  The effect of this option is NOT to prepend a column of 1 to the inputs
     *  (which has often no effect for GP regression), but to estimate a
     *  separate mean of the targets, perform the GP regression on the
     *  zero-mean targets, and add it back when computing the outputs.
     */
    bool m_include_bias;
    
    /**
     *  Whether to perform the additional train-time computations required
     *  to compute confidence intervals.  This includes computing a separate
     *  inverse of the Gram matrix.  Specification of this option is necessary
     *  for calling both computeConfidenceFromOutput and computeOutputCovMat.
     */
    bool m_compute_confidence;

    /**
     *  Small regularization to be added post-hoc to the computed output
     *  covariance matrix and confidence intervals; this is mostly used as a
     *  disaster prevention device, to avoid negative predictive variance
     */
    real m_confidence_epsilon;
    
    /**
     *  List of hyperparameters to optimize.  They must be specified in the
     *  form "option-name":initial-value, where 'option-name' is the name of an
     *  option to set within the Kernel object (the array-index form
     *  'option[i]' is supported), and 'initial-value' is the
     *  (PLearn-serialization string representation) for starting point for the
     *  optimization.  Currently, the hyperparameters are constrained to be
     *  scalars.
     */
    TVec< pair<string,string> > m_hyperparameters;

    /**
     *  If the kernel support automatic relevance determination (ARD; e.g.
     *  SquaredExponentialARDKernel), the list of hyperparameters corresponding
     *  to each input can be created automatically by giving an option prefix
     *  and an initial value.  The ARD options are created to have the form
     *
     *     'prefix[0]', 'prefix[1]', 'prefix[N-1]'
     *
     *  where N is the number of inputs.  This option is useful when the
     *  dataset inputsize is not (easily) known ahead of time. 
     */
    pair<string,string> m_ARD_hyperprefix_initval;

    /**
     *  Specification of the optimizer to use for train-time hyperparameter
     *  optimization.  A ConjGradientOptimizer should be an adequate choice.
     */
    PP<Optimizer> m_optimizer;

    /**
     *  If true, the Gram matrix is saved before undergoing Cholesky each
     *  decomposition; useful for debugging if the matrix is quasi-singular.
     *  It is saved in the current expdir under the names 'gram_matrix_N.pmat'
     *  where N is an increasing counter.
     */
    bool m_save_gram_matrix;


public:
    //#####  Public Member Functions  #########################################

    /// Default constructor
    GaussianProcessRegressor();


    //#####  PLearner Member Functions  #######################################

    /// Isolate the training inputs and create and ExtendedVMatrix (to include
    /// a bias) if required
    virtual void setTrainingSet(VMat training_set, bool call_forget=true);

    /// Returns the size of this learner's output, (which typically
    /// may depend on its inputsize(), targetsize() and set options).
    virtual int outputsize() const;

    /// (Re-)initializes the PLearner in its fresh state (that state may depend
    /// on the 'seed' option) and sets 'stage' back to 0 (this is the stage of a
    /// fresh learner!).
    virtual void forget();
    
    /// The role of the train method is to bring the learner up to stage==nstages,
    /// updating the train_stats collector with training costs measured on-line
    /// in the process.
    virtual void train();

    /// Computes the output from the input.
    virtual void computeOutput(const Vec& input, Vec& output) const;

    /// Computes the costs from already computed output. 
    virtual void computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                         const Vec& target, Vec& costs) const;

    /// Compute the confidence intervals based on the GP output variance
    virtual
    bool computeConfidenceFromOutput(const Vec& input, const Vec& output,
                                     real probability,
                                     TVec< pair<real,real> >& intervals) const;

    /// Compute the posterior mean and covariance matrix of a set of inputs.
    /// Note that if any of the inputs contains a missing value (NaN), then
    /// the whole covariance matrix is NaN (in the current implementation).
    virtual void computeOutputCovMat(const Mat& inputs, Mat& outputs,
                                     TVec<Mat>& covariance_matrices) const;
    
    /// Returns the names of the costs computed by computeCostsFromOutputs (and
    /// thus the test method). 
    virtual TVec<std::string> getTestCostNames() const;

    /// Returns the names of the objective costs that the train method computes and 
    /// for which it updates the VecStatsCollector train_stats.
    virtual TVec<std::string> getTrainCostNames() const;


    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(GaussianProcessRegressor);

    // Simply calls inherited::build() then build_() 
    virtual void build();

    /// Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    /// Declares the class options.
    static void declareOptions(OptionList& ol);

    /// Utility internal function for computeOutput, which accepts the
    /// destination for kernel evaluations in argument, and performs no error
    /// checking nor vector resizes
    void computeOutputAux(const Vec& input, Vec& output,
                          Vec& kernel_evaluations) const;
    
    /// Optimize the hyperparameters if any.  Return a Variable on which
    /// train() carries out a final fprop for obtaining the final trained
    /// learner parameters.
    PP<GaussianProcessNLLVariable> hyperOptimize(
        const Mat& inputs, const Mat& targets, VarArray& hyperparam_vars);

protected:
    //#####  Protected Options  ###############################################

    /**
     *  Matrix of learned parameters, determined from the equation
     *
     *    (M + lambda I)^-1 y
     *
     *  (don't forget that y can be a matrix for multivariate output problems)
     */
    Mat m_alpha;

    /**
     *  Inverse of the Gram matrix, used to compute confidence intervals (must
     *  be saved since the confidence intervals are obtained from the equation
     *
     *    sigma^2 = k(x,x) - k(x)'(M + lambda I)^-1 k(x)
     */
    Mat m_gram_inverse;

    /// Mean of the targets, if the option 'include_bias' is true
    Vec m_target_mean;
    
    /// Saved version of the training set inputs, which must be kept along for
    /// carrying out kernel evaluations with the test point
    Mat m_training_inputs;

    /// Buffer for kernel evaluations at test time
    mutable Vec m_kernel_evaluations;

    /// Buffer for the product of the gram inverse with kernel evaluations
    mutable Vec m_gram_inverse_product;

    //! Buffer to hold confidence intervals when computing costs from outputs
    mutable TVec< pair<real,real> > m_intervals;

    //! Buffer to hold the Gram matrix of train inputs with test inputs.
    //! Element i,j contains K(test(i), train(j)).
    mutable Mat m_gram_traintest_inputs;

    //! Buffer to hold the product of the gram inverse with gram_traintest_inputs
    mutable Mat m_gram_inv_traintest_product;

    //! Buffer to hold the sigma reductor for m_gram_inverse_product
    mutable Mat m_sigma_reductor;
    
private: 
    /// This does the actual building. 
    void build_();
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(GaussianProcessRegressor);
  
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
