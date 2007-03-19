// -*- C++ -*-

// GaussianProcessNLLVariable.cc
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

/*! \file GaussianProcessNLLVariable.cc */

#define PL_LOG_MODULE_NAME "GaussianProcessNLLVariable"

// From PLearn
#include <plearn/io/pl_log.h>
#include <plearn/math/TMat_maths.h>
#include <plearn/io/MatIO.h>
#include "GaussianProcessNLLVariable.h"

#ifdef USE_BLAS_SPECIALISATIONS
#include <plearn/math/plapack.h>
#endif

namespace PLearn {
using namespace std;

/** GaussianProcessNLLVariable **/

PLEARN_IMPLEMENT_OBJECT(
    GaussianProcessNLLVariable,
    "Compute the Negative-Log-Marginal-Likelihood for Gaussian Process Regression",
    "This Variable computes the negative-log-marginal likelihood function\n"
    "associated with Gaussian Process Regression (see GaussianProcessRegressor).\n"
    "It is primarily used to carry out hyperparameter optimization by conjugate\n"
    "gradient descent.\n"
    "\n"
    "To compute both the fprop and bprop (gradient of marginal NLL w.r.t. each\n"
    "hyperparameter), it requires the specification of the Kernel object used,\n"
    "the VMatrix of inputs, the VMatrix of targets, and the variables that wrap\n"
    "the hyperparameter options within the Kernel object structure (presumably\n"
    "ObjectOptionVariable, or similar).  These variables must be scalar\n"
    "variables.  To get something like Automatic Relevance Determination, you\n"
    "should specify separately each Variable (in the PLearn sense) that\n"
    "corresponds to a given input hyperparameter.\n"
    );

GaussianProcessNLLVariable::GaussianProcessNLLVariable()
    : m_save_gram_matrix(0),
      m_kernel(0),
      m_noise(0),
      m_allow_bprop(true)
{ }


GaussianProcessNLLVariable::GaussianProcessNLLVariable(
    Kernel* kernel, real noise, Mat inputs, Mat targets,
    const TVec<string>& hyperparam_names, const VarArray& hyperparam_vars,
    bool allow_bprop, bool save_gram_matrix, PPath expdir)
    : inherited(hyperparam_vars, 1, 1),
      m_save_gram_matrix(save_gram_matrix),
      m_expdir(expdir),
      m_kernel(kernel),
      m_noise(noise),
      m_inputs(inputs),
      m_targets(targets),
      m_hyperparam_names(hyperparam_names),
      m_hyperparam_vars(hyperparam_vars),
      m_allow_bprop(allow_bprop)
{
    build();
}


void GaussianProcessNLLVariable::recomputeSize(int& l, int& w) const
{
    // This is always the case for this variable
    l = 1;
    w = 1;
}


// ### Nothing to add here, simply calls build_
void GaussianProcessNLLVariable::build()
{
    inherited::build();
    build_();
}

void GaussianProcessNLLVariable::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(m_kernel,          copies);
    deepCopyField(m_inputs,          copies);
    deepCopyField(m_targets,         copies);
    deepCopyField(m_hyperparam_names,copies);
    deepCopyField(m_hyperparam_vars, copies);
    deepCopyField(m_gram,            copies);
    deepCopyField(m_gram_derivative, copies);
    deepCopyField(m_cholesky_gram,   copies);
    deepCopyField(m_alpha_t,         copies);
    deepCopyField(m_alpha_buf,       copies);
    deepCopyField(m_inverse_gram,    copies);
    deepCopyField(m_cholesky_gram,   copies);
}

void GaussianProcessNLLVariable::declareOptions(OptionList& ol)
{
    declareOption(
        ol, "save_gram_matrix", &GaussianProcessNLLVariable::m_save_gram_matrix,
        OptionBase::buildoption,
        "If true, the Gram matrix is saved before undergoing Cholesky\n"
        "decomposition; useful for debugging if the matrix is quasi-singular.");
    
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void GaussianProcessNLLVariable::build_()
{
    PLASSERT( m_kernel && m_inputs.isNotNull() && m_targets.isNotNull() );
}


//#####  alpha  ###############################################################

const Mat& GaussianProcessNLLVariable::alpha() const
{
    m_alpha_buf.resize(m_alpha_t.width(), m_alpha_t.length());
    transpose(m_alpha_t, m_alpha_buf);
    return m_alpha_buf;
}
    

//#####  fprop  ###############################################################

// ### computes value from varray values
void GaussianProcessNLLVariable::fprop()
{
    // logVarray(m_hyperparam_vars, "FProp current hyperparameters:", true);

    // Ensure that the current hyperparameter variable values are propagated
    // into kernel options
    m_hyperparam_vars.fprop();
    
    fbpropFragments(m_kernel, m_noise, m_inputs, m_targets, m_allow_bprop,
                    m_save_gram_matrix, m_expdir,
                    m_gram, m_cholesky_gram, m_alpha_t, m_inverse_gram,
                    m_cholesky_tmp, m_rhs_tmp);

    // Assuming y is a column vector...  For multivariate targets, we
    // separately dot each column of the targets with corresponding columns of
    // alpha, and add as many of the other two terms as there are variables
    //
    //     0.5 * y'*alpha + sum(log(diag(L))) + 0.5*n*log(2*pi)
    //
    // Don't forget that alpha_t is transposed
    const int n = m_alpha_t.width();
    const int m = m_alpha_t.length();

    real logdet_log2pi = 0;
    for (int i=0 ; i<n ; ++i)
        logdet_log2pi += pl_log(m_cholesky_gram(i,i));
    logdet_log2pi += 0.5 * n * pl_log(2*M_PI);
    
    real nll = 0;
    for (int i=0 ; i<m ; ++i)
        nll += 0.5*dot(m_targets.column(i), m_alpha_t.row(i)) + logdet_log2pi;
    value[0] = nll;
}


//#####  bprop  ###############################################################

// ### computes varray gradients from gradient
void GaussianProcessNLLVariable::bprop()
{
    PLASSERT_MSG( m_allow_bprop,
                  "GaussianProcessNLLVariable must be constructed with the option "
                  "'will_bprop'=True in order to call bprop" );
    PLASSERT( m_hyperparam_names.size() == m_hyperparam_vars.size() );
    PLASSERT( m_alpha_t.width() == m_inverse_gram.width() );
    PLASSERT( m_inverse_gram.width() == m_inverse_gram.length() );
    PLASSERT( m_kernel );
    
    // Loop over the hyperparameters in order to compute the derivative of the
    // gram matrix once for each hyperparameter.  Then loop over the target
    // variables to accumulate the gradient.  For each target, we must compute
    //
    //    trace((K^-1 - alpha*alpha') * dK/dtheta_j)
    //
    // Since both the first term inside the trace and the derivative of the
    // gram matrix are symmetric square matrices, the trace is efficiently
    // computed as the sum of the elementwise product of those matrices.
    //
    // Don't forget that m_alpha_t is transposed.
    for (int j=0, m=m_hyperparam_names.size() ; j<m ; ++j) {
        real dnll_dj = 0;
        m_kernel->computeGramMatrixDerivative(m_gram_derivative,
                                              m_hyperparam_names[j]);
        for (int i=0, n=m_alpha_t.length() ; i<n ; ++i) {
            real* curalpha = m_alpha_t[i];
            real  cur_trace = 0.0;

            // Sum over all rows and columns of matrix
            real* curalpha_row = curalpha;
            for (int row=0, nrows=m_inverse_gram.length()
                     ; row<nrows ; ++row, ++curalpha_row)
            {
                real* p_inverse_gram     = m_inverse_gram[row];
                real* p_gram_derivative  = m_gram_derivative[row];
                real  curalpha_row_value = *curalpha_row;
                real* curalpha_col       = curalpha;
                real  row_trace          = 0.0;

                for (int col=0 ; col <= row ; ++col, ++curalpha_col)
                {
                    if (col == row)
                        row_trace *= 2.;
                    
                    row_trace +=
                        (*p_inverse_gram++ - curalpha_row_value * *curalpha_col)
                        * *p_gram_derivative++;

                    // curtrace +=
                    //     (m_inverse_gram(row,col) - curalpha(row,0)*curalpha(col,0))
                    //     * m_gram_derivative(row,col);
                }
                cur_trace += row_trace;
            }

            dnll_dj += cur_trace / 2.0;
        }
        m_hyperparam_vars[j]->gradient[0] += dnll_dj * gradient[0];
    }
}


//#####  fbpropFragments (NO LAPACK)  #########################################

#ifndef USE_BLAS_SPECIALISATIONS
void GaussianProcessNLLVariable::fbpropFragments(
    Kernel* kernel, real noise, const Mat& inputs, const Mat& targets,
    bool compute_inverse, bool save_gram_matrix, const PPath& expdir,
    Mat& gram, Mat& L, Mat& alpha_t, Mat& inv,
    Vec& tmp_chol, Mat& tmp_rhs)
{
    PLASSERT( kernel );
    PLASSERT( inputs.length() == targets.length() );
    const int trainlength = inputs.length();
    const int targetsize  = targets.width();
    
    // The RHS matrix (when solving the linear system Gram*Params=RHS) is made
    // up of two parts: the regression targets themselves, and the identity
    // matrix if we requested them (for confidence intervals).  After solving
    // the linear system, set the gram-inverse appropriately.
    int rhs_width = targetsize + (compute_inverse? trainlength : 0);
    tmp_rhs.resize(trainlength, rhs_width);
    tmp_rhs.subMatColumns(0, targetsize) << targets;
    if (compute_inverse) {
        Mat rhs_identity = tmp_rhs.subMatColumns(targetsize, trainlength);
        identityMatrix(rhs_identity);
    }

    // Compute Gram Matrix and add weight decay to diagonal
    kernel->setDataForKernelMatrix(inputs);
    gram.resize(trainlength, trainlength);
    kernel->computeGramMatrix(gram);
    addToDiagonal(gram, noise);

    // Save the Gram matrix if requested
    if (save_gram_matrix) {
        static int counter = 1;
        string filename = expdir / ("gram_matrix_" +
                                    tostring(counter++) + ".pmat");
        savePMat(filename, gram);
    }

    // Dump a fragment of the Gram Matrix to the debug log
    DBG_MODULE_LOG << "Gram fragment: "
                   << gram(0,0) << ' '
                   << gram(1,0) << ' '
                   << gram(1,1) << endl;

    // Compute Cholesky decomposition and solve the linear system
    alpha_t.resize(trainlength, rhs_width);
    L.resize(trainlength, trainlength);
    tmp_chol.resize(trainlength);
    solveLinearSystemByCholesky(gram, tmp_rhs, alpha_t, &L, &tmp_chol);

    // Must return transpose here since the code has been modified to work with
    // a transposed alpha, to better interface with lapack (much faster in the
    // latter case to avoid superfluous transposes).
    if (compute_inverse) {
        inv     = alpha_t.subMatColumns(targetsize, trainlength);
        alpha_t = transpose(alpha_t.subMatColumns(0, targetsize));
    }
    else
        alpha_t = transpose(alpha_t);
}
#endif

//#####  fbpropFragments (LAPACK)  ############################################

#ifdef USE_BLAS_SPECIALISATIONS
void GaussianProcessNLLVariable::fbpropFragments(
    Kernel* kernel, real noise, const Mat& inputs, const Mat& targets,
    bool compute_inverse, bool save_gram_matrix, const PPath& expdir,
    Mat& gram, Mat& L, Mat& alpha_t, Mat& inv,
    Vec& tmp_chol, Mat& tmp_rhs)
{
    PLASSERT( kernel );
    PLASSERT( inputs.length() == targets.length() );
    const int trainlength = inputs.length();
    const int targetsize  = targets.width();
    
    // The RHS matrix (when solving the linear system Gram*Params=RHS) is made
    // up of two parts: the regression targets themselves, and the identity
    // matrix if we requested them (for confidence intervals).  After solving
    // the linear system, set the gram-inverse appropriately.  To interface
    // nicely with LAPACK, we store this in a transposed format.
    int rhs_width = targetsize + (compute_inverse? trainlength : 0);
    tmp_rhs.resize(rhs_width, trainlength);
    Mat targets_submat = tmp_rhs.subMatRows(0, targetsize);
    transpose(targets, targets_submat);
    if (compute_inverse) {
        Mat rhs_identity = tmp_rhs.subMatRows(targetsize, trainlength);
        identityMatrix(rhs_identity);
    }

    // Compute Gram Matrix and add weight decay to diagonal
    kernel->setDataForKernelMatrix(inputs);
    gram.resize(trainlength, trainlength);
    kernel->computeGramMatrix(gram);
    addToDiagonal(gram, noise);

    // Save the Gram matrix if requested
    if (save_gram_matrix) {
        static int counter = 1;
        string filename = expdir / ("gram_matrix_" +
                                    tostring(counter++) + ".pmat");
        savePMat(filename, gram);
    }

    // Dump a fragment of the Gram Matrix to the debug log
    DBG_MODULE_LOG << "Gram fragment: "
                   << gram(0,0) << ' '
                   << gram(1,0) << ' '
                   << gram(1,1) << endl;

    // Compute Cholesky decomposition and solve the linear system.  LAPACK
    // solves in-place, but luckily we don't need either the Gram and RHS
    // matrices after solving.  Note that for now we don't bother to create an
    // appropriately transposed RHS (will come later).
    lapackCholeskyDecompositionInPlace(gram);
    lapackCholeskySolveInPlace(gram, tmp_rhs, true /* column-major */);
    alpha_t = tmp_rhs;                         // LAPACK solves in-place
    L       = gram;                            // LAPACK solves in-place
    
    if (compute_inverse) {
        inv     = alpha_t.subMatRows(targetsize, trainlength);
        alpha_t = alpha_t.subMatRows(0, targetsize);
    }
}
#endif


//#####  logVarray  ###########################################################

void GaussianProcessNLLVariable::logVarray(const VarArray& varr,
                                           const string& title, bool debug)
{
    string entry = title + '\n';
    for (int i=0, n=varr.size() ; i<n ; ++i) {
        entry += right(varr[i]->getName(), 35) + ": " + tostring(varr[i]->value[0]);
        if (i < n-1)
            entry += '\n';
    }
    if (debug) {
        DBG_MODULE_LOG << entry << endl;
    }
    else {
        MODULE_LOG << entry << endl;
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
