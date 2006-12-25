// -*- C++ -*-

// GaussianProcessNLLVariable.h
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

// Authors: Nicolas Chapados

/*! \file GaussianProcessNLLVariable.h */


#ifndef GaussianProcessNLLVariable_INC
#define GaussianProcessNLLVariable_INC

#include <plearn/var/NaryVariable.h>
#include <plearn/ker/Kernel.h>

namespace PLearn {
using namespace std;

/*! * GaussianProcessNLLVariable * */

/**
 *  Compute the Negative-Log-Marginal-Likelihood for Gaussian Process Regression
 *
 *  This Variable computes the negative-log-marginal likelihood function
 *  associated with Gaussian Process Regression (see GaussianProcessRegressor).
 *  It is primarily used to carry out hyperparameter optimization by conjugate
 *  gradient descent.
 *
 *  To compute both the fprop and bprop (gradient of marginal NLL w.r.t. each
 *  hyperparameter), it requires the specification of the Kernel object used,
 *  the VMatrix of inputs, the VMatrix of targets, and the variables that wrap
 *  the hyperparameter options within the Kernel object structure (presumably
 *  ObjectOptionVariable, or similar).  These variables must be scalar
 *  variables.  To get something like Automatic Relevance Determination, you
 *  should specify separately each Variable (in the PLearn sense) that
 *  corresponds to a given input hyperparameter.
 */
class GaussianProcessNLLVariable : public NaryVariable
{
    typedef NaryVariable inherited;

public:
    //#####  Public Build Options  ############################################

    // (no options)
    
public:
    //#####  Public Member Functions  #########################################

    //! Default constructor, usually does nothing
    GaussianProcessNLLVariable();

    /**
     *  Constructor initializing from input variables.
     *
     *  @param kernel:  the kernel to use
     *  @param noise:   observation noise to add to the diagonal Gram matrix
     *  @param inputs:  matrix of training inputs
     *  @param targets: matrix of training targets (may be multivariate)
     *  @param hyperparam_names: names of kernel hyperparameters w.r.t. which
     *                  we should be backpropagating the NLL
     *  @param hyperparam_vars: PLearn Variables wrapping kernel hyperparameters
     *  @param allow_bprop: if true, assume we will be performing bprops on the
     *                  Variable; if not, only fprops are allowed.  BProps
     *                  involve computing a full inverse of the Gram matrix
     */
    GaussianProcessNLLVariable(Kernel* kernel, real noise,
                               Mat inputs, Mat targets,
                               const TVec<string>& hyperparam_names,
                               const VarArray& hyperparam_vars,
                               bool allow_bprop = true);

    
    //#####  PLearn::Variable methods #########################################

    virtual void recomputeSize(int& l, int& w) const;
    virtual void fprop();
    virtual void bprop();
    
    /**
     *  Compute the elements required for log-likelihood computation, fprop,
     *  and bprop.  Static since this is called by GaussianProcessRegressor.
     *
     *  @param[in]  kernel:   the kernel to use
     *  @param[in]  noise:    observation noise to add to the diagonal Gram matrix
     *  @param[in]  inputs:   matrix of training inputs
     *  @param[in]  targets:  matrix of training targets (may be multivariate)
     *  @param[in]  compute_inverse: whether to compute inverse of Gram matrix
     *  @param[out] gram:     The kernel (Gram) matrix
     *  @param[out] L:        Cholesky decomposition of the Gram matrix
     *  @param[out] alpha:    Solution to the linear system gram*alpha = targets
     *  @param[out] inv:      If required, the inverse Gram matrix
     *  @param[inout] tmpch:  Temporary storage for Cholesky decomposition
     *  @param[inout] tmprhs: Temporary storage for RHS
     */
    static void fbpropFragments(Kernel* kernel, real noise, const Mat& inputs,
                                const Mat& targets, bool compute_inverse,
                                Mat& gram, Mat& L, Mat& alpha, Mat& inv,
                                Vec& tmpch, Mat& tmprhs);

    //! Accessor to the last computed 'alpha' matrix in an fprop
    const Mat& alpha() const { return m_alpha; }

    //! Accessor to the last computed gram matrix inverse in an fprop
    const Mat& gramInverse() const { return m_inverse_gram; }

    //! Accessor to the training mse after an fprop
    real trainMSE() const { return m_mse; }
    

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(GaussianProcessNLLVariable);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //! Current kernel we should be using
    Kernel* m_kernel;

    //! Observation noise to be added to the diagonal of the Gram matrix
    real m_noise;
    
    //! Matrix of inputs
    Mat m_inputs;

    //! Matrix of regression targets
    Mat m_targets;

    //! Name of each hyperparameter contained in hyperparam_vars.  The name
    //! should be such that m_kernel->computeGramMatrixDerivative works.
    TVec<string> m_hyperparam_names;

    //! Variables standing for each hyperparameter, used to accumulate the
    //! gradient w.r.t. them.
    VarArray m_hyperparam_vars;

    //! Whether bprops are allowed
    bool m_allow_bprop;

    //! Holds the Gram matrix
    Mat m_gram;

    //! Holds the derivative of the Gram matrix with respect to an
    //! hyperparameter
    Mat m_gram_derivative;
    
    //! Holds the Cholesky decomposition of m_gram
    Mat m_cholesky_gram;

    //! Solution of the linear system gram*alpha = targets
    Mat m_alpha;

    //! Inverse of the Gram matrix
    Mat m_inverse_gram;
    
    //! Temporary storage for the Cholesky decomposition
    Vec m_cholesky_tmp;

    //! Temporary storage for holding the right-hand-side to be solved by Cholesky
    Mat m_rhs_tmp;

    //! Temporary holder of training-set MSE after an fprop
    real m_mse;
    
protected:
    //! Declares the class options.
    static void declareOptions(OptionList& ol);

private:
    //! This does the actual building.
    void build_();
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(GaussianProcessNLLVariable);

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
