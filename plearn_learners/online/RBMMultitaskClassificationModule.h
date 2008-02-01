// -*- C++ -*-

// RBMMultitaskClassificationModule.h
//
// Copyright (C) 2006 Pascal Lamblin
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

// Authors: Pascal Lamblin

/*! \file RBMMultitaskClassificationModule.h */


#ifndef RBMMultitaskClassificationModule_INC
#define RBMMultitaskClassificationModule_INC

#include <plearn_learners/online/OnlineLearningModule.h>
#include "RBMConnection.h"
#include "RBMMatrixConnection.h"
#include "RBMMixedConnection.h"
#include "RBMLayer.h"
#include "RBMBinomialLayer.h"
#include "RBMMultinomialLayer.h"

namespace PLearn {

/**
 * Computes a mean-field approximate of p(y|x), with y a binary vector.
 * This module contains an RBMConnection, an RBMBinomialLayer (hidden),
 * an RBMMatrixConnection (transposed) and an RBMBinomialLayer (target).
 * The two RBMConnections are combined in joint_connection.
 */
class RBMMultitaskClassificationModule : public OnlineLearningModule
{
    typedef OnlineLearningModule inherited;

public:
    //#####  Public Build Options  ############################################
    //! Connection between the previous layer, and last_layer
    PP<RBMConnection> previous_to_last;

    //! Top-level layer (the one in the middle if we unfold)
    PP<RBMBinomialLayer> last_layer;

    //! Connection between last_layer and target_layer
    PP<RBMMatrixConnection> last_to_target;

    //! Layer containing the one-hot vector containing the target (or its
    //! prediction)
    PP<RBMBinomialLayer> target_layer;

    //! Number of mean-field iterations
    int n_mean_field_iterations;

    //! Indication that fprop should output the value of the activation
    //! before the squashing function and the application of the bias, 
    //! instead of the mean-field approximation.
    bool fprop_outputs_activation;

    //#####  Public Learnt Options  ###########################################
    //! Connection grouping previous_to_last and last_to_target
    PP<RBMMixedConnection> joint_connection;

    //! Size of last_layer
    int last_size;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    // ### Make sure the implementation in the .cc
    // ### initializes all fields to reasonable default values.
    RBMMultitaskClassificationModule();

    // Your other public member functions go here

    //! given the input, compute the output (possibly resize it appropriately)
    virtual void fprop(const Vec& input, Vec& output) const;

    //! Adapt based on the output gradient: this method should only
    //! be called just after a corresponding fprop; it should be
    //! called with the same arguments as fprop for the first two arguments
    //! (and output should not have been modified since then).
    //! Since sub-classes are supposed to learn ONLINE, the object
    //! is 'ready-to-be-used' just after any bpropUpdate.
    //! N.B. A DEFAULT IMPLEMENTATION IS PROVIDED IN THE SUPER-CLASS, WHICH
    //! JUST CALLS
    //!     bpropUpdate(input, output, input_gradient, output_gradient)
    //! AND IGNORES INPUT GRADIENT.
    // virtual void bpropUpdate(const Vec& input, const Vec& output,
    //                          const Vec& output_gradient);

    //! this version allows to obtain the input gradient as well
    virtual void bpropUpdate(const Vec& input, const Vec& output,
                             Vec& input_gradient,
                             const Vec& output_gradient,
                             bool accumulate=false);

    //! Similar to bpropUpdate, but adapt based also on the estimation
    //! of the diagonal of the Hessian matrix, and propagates this
    //! back. If these methods are defined, you can use them INSTEAD of
    //! bpropUpdate(...)
    //! N.B. A DEFAULT IMPLEMENTATION IS PROVIDED IN THE SUPER-CLASS,
    //! WHICH JUST CALLS
    //!     bbpropUpdate(input, output, input_gradient, output_gradient,
    //!                  out_hess, in_hess)
    //! AND IGNORES INPUT HESSIAN AND INPUT GRADIENT.
    // virtual void bbpropUpdate(const Vec& input, const Vec& output,
    //                           const Vec& output_gradient,
    //                           const Vec& output_diag_hessian);

    //! this version allows to obtain the input gradient and diag_hessian
    //! N.B. A DEFAULT IMPLEMENTATION IS PROVIDED IN THE SUPER-CLASS, WHICH
    //! RAISES A PLERROR.
    // virtual void bbpropUpdate(const Vec& input, const Vec& output,
    //                           Vec& input_gradient,
    //                           const Vec& output_gradient,
    //                           Vec& input_diag_hessian,
    //                           const Vec& output_diag_hessian
    //                           bool accumulate=false);

    //! reset the parameters to the state they would be BEFORE starting
    //! training.  Note that this method is necessarily called from
    //! build().
    virtual void forget();

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT
    PLEARN_DECLARE_OBJECT(RBMMultitaskClassificationModule);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //#####  Not Options  #####################################################

    mutable TVec< Vec > mean_field_activations_target;
    mutable TVec< Vec > mean_field_approximations_target;
    mutable TVec< Vec > mean_field_activations_hidden;
    mutable TVec< Vec > mean_field_approximations_hidden;

protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    static void declareOptions(OptionList& ol);

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    void build_();

private:
    //#####  Private Data Members  ############################################

    //! Stores the gradient of the weights between the target and the hidden layer
    mutable Mat last_to_target_gradient;

    //! Mean gradient propagation
    mutable Vec mean_field_activations_gradient_target;
    mutable Vec mean_field_approximations_gradient_target;
    mutable Vec mean_field_activations_gradient_hidden;
    mutable Vec mean_field_approximations_gradient_hidden;
    
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(RBMMultitaskClassificationModule);

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
