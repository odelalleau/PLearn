// -*- C++ -*-

// NatGradEstimator.h
//
// Copyright (C) 2007 yoshua Bengio
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

// Authors: yoshua Bengio

/*! \file NatGradEstimator.h */


#ifndef NatGradEstimator_INC
#define NatGradEstimator_INC

#include <plearn/base/Object.h>
#include <plearn/math/TMat_impl.h>

namespace PLearn {

/**
 * Class used for converting a sequence of n-dimensional gradients g_t
 * into covariance-corrected update directions v_t, approximating
 *     v_t = inv(C_t) g_t,
 * with C_t = gamma C_{t-1} + g_t g_t'.
 * 
 * There is a main method, the operator(), which takes a g_t and fills v_t.
 * The process can be initialized by init(). 
 */
class NatGradEstimator : public Object
{
    typedef Object inherited;

public:
    //#####  Public Build Options  ############################################

    //! ### declare public option fields (such as build options) here

    //! mini-batch size for covariance eigen-decomposition
    int cov_minibatch_size;

    //! regularization coefficient of covariance matrix (initial values on diagonal)
    real lambda;

    //! number of eigenvectors-eigenvalues that is preserved of the covariance matrix
    int n_eigen;

    //! learning rate of the inversion iterations
    real alpha;

    //! forgetting factor in moving average estimator of covariance
    real gamma;

    //! number of iterations for approaching the solution of inv(C) v_t = g_t
    int inversion_n_iterations;

    //! number of input dimensions (size of g_t or v_t)
    int n_dim;

    //! wether to use the u0 and its correction for initialization the inversion iteration
    bool use_double_init;

    //! verbosity level, track improvement, spectrum, etgc.
    int verbosity;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    // ### Make sure the implementation in the .cc
    // ### initializes all fields to reasonable default values.
    NatGradEstimator();

    // Your other public member functions go here

    //! initialize the object to start collecting covariance statistics from fresh
    void init();

    //! main method of this class: reads from the gradient "g" field
    //! and writes into the "v" field an estimator of inv(cov) g.
    //! The argument is an index over examples, which is used to
    //! know when cycling through a minibatch. The statistics on
    //! the covariance are updated.
    void operator()(int t, const Vec& g, Vec v);

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT
    PLEARN_DECLARE_OBJECT(NatGradEstimator);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    // (PLEASE IMPLEMENT IN .cc)
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //#####  Protected Options  ###############################################

    // ### Declare protected option fields (such as learned parameters) here

    //! k principal eigenvectors of the estimated covariance matrix
    Mat Ut; // dimension = n_eigen x n_dim

    //! k principal eigenvalues of the estimated covariance matrix
    Vec D; // subvector of E

    //! eigenvalues of the Gram matrix
    Vec E;

    //! eigenvalue attributed to the non-principal eigenvectors
    real sigma;

    //! gradient vectors collected during the minibatch, in each row of Gt
    Mat Gt; // dimension = cov_minibatch_size x n_dim

    //! previous value of t
    int previous_t;

    //! initial v's for the gradients in this minibatch
    Mat initial_v;
    //! vg[i] = initial_v[i] . Gt[i]
    Vec vg;

protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    // (PLEASE IMPLEMENT IN .cc)
    static void declareOptions(OptionList& ol);

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    // (PLEASE IMPLEMENT IN .cc)
    void build_();

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here

    //! temporary buffer
    Vec tmp_v;
    //! Gram matrix, of dimension (k + minibatch_size, k + minibatch_size)
    //! and its sub-matrices
    Mat M, M11, M12, M21, M22;
    //! k+1 eigenvectors of the Gram matrix (in the rows)
    Mat Vt;
    Mat Vkt; //! sub-matrix of Vt with first n_eigen elements of each eigen-vector
    Mat Vbt; //! sub-matrix of Vt with last cov_minibatch_size elements of each eigen-vector
    //! temp for new value of Ut
    Mat newUt;
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(NatGradEstimator);

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
