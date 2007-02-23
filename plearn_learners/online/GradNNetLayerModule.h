// -*- C++ -*-

// GradNNetLayerModule.h
//
// Copyright (C) 2005 Pascal Lamblin
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
   * $Id: GradNNetLayerModule.h,v 1.3 2006/01/08 00:14:53 lamblinp Exp $
   ******************************************************* */

// Authors: Pascal Lamblin

/*! \file GradNNetLayerModule.h */


#ifndef GradNNetLayerModule_INC
#define GradNNetLayerModule_INC

#include <plearn/base/Object.h>
#include <plearn/math/TMat_maths.h>
#include "OnlineLearningModule.h"

namespace PLearn {

/**
 * Affine transformation module, with stochastic gradient descent updates.
 *
 * Neural Network layer, using stochastic gradient to update neuron weights,
 *      Output = weights * Input + bias
 * Weights and bias are updated by online gradient descent, with learning
 * rate possibly decreasing in 1/(1 + n_updates_done * decrease_constant).
 * An L1 and L2 regularization penalty can be added to push weights to 0.
 * Weights can be initialized to 0, to a given initial matrix, or randomly
 * from a uniform distribution.
 *
 */
class GradNNetLayerModule : public OnlineLearningModule
{
    typedef OnlineLearningModule inherited;

public:
    //#####  Public Build Options  ############################################

    //! Starting learning-rate, by which we multiply the gradient step
    real start_learning_rate;

    //! learning_rate = start_learning_rate / (1 + decrease_constant*t),
    //! where t is the number of updates since the beginning
    real decrease_constant;

    //! Optional initial weights of the neurons (one row per neuron).
    Mat init_weights;

    //! Optional initial bias of the neurons.
    Vec init_bias;

    //! If init_weights is not provided, the weights are initialized randomly
    //! from a uniform in [-r,r], with r = init_weights_random_scale/input_size
    real init_weights_random_scale;

    //! Optional (default=0) factor of L1 regularization term
    real L1_penalty_factor;

    //! Optional (default=0) factor of L2 regularization term
    real L2_penalty_factor;

    //! The weights, one neuron per line
    Mat weights;

    //! The bias
    Vec bias;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    GradNNetLayerModule();

    // Your other public member functions go here

    virtual void fprop(const Vec& input, Vec& output) const;

    virtual void bpropUpdate(const Vec& input, const Vec& output,
                             const Vec& output_gradient);

    virtual void bpropUpdate(const Vec& input, const Vec& output,
                             Vec& input_gradient, const Vec& output_gradient,
                             bool accumulate=false);

    virtual void bbpropUpdate(const Vec& input, const Vec& output,
                              const Vec& output_gradient,
                              const Vec& output_diag_hessian);

    /* Bad implementation. Let the parent call PLERROR.
    virtual void bbpropUpdate(const Vec& input, const Vec& output,
                              Vec& input_gradient,
                              const Vec& output_gradient,
                              Vec& input_diag_hessian,
                              const Vec& output_diag_hessian);
    */

    virtual void forget();

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(GradNNetLayerModule);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //#####  Protected Options  ###############################################

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

    // The rest of the private stuff goes here
    real learning_rate;
    int step_number;
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(GradNNetLayerModule);

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
