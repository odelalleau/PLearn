// -*- C++ -*-

// InferenceRBM.h
//
// Copyright (C) 2008 Pascal Lamblin
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

/*! \file InferenceRBM.h */


#ifndef InferenceRBM_INC
#define InferenceRBM_INC

#include <plearn/base/Object.h>
#include <plearn_learners/online/RBMLayer.h>
#include <plearn_learners/online/RBMMultinomialLayer.h>
#include <plearn_learners/online/RBMBinomialLayer.h>
#include <plearn_learners/online/RBMMixedLayer.h>
#include <plearn_learners/online/RBMMatrixConnection.h>
#include <plearn_learners/online/RBMMixedConnection.h>

namespace PLearn {

/**
 * RBM to be used when doing joint supervised learning by CD.
 * We have input, target and hidden layer. We can compute hidden given
 * (input, target), target given input, or hidden given input.
 *
 * @todo Write class to-do's here if there are any.
 */
class InferenceRBM : public Object
{
    typedef Object inherited;

public:
    //#####  Public Build Options  ############################################

    //! ### declare public option fields (such as build options) here
    //! Input layer (part of visible)
    PP<RBMLayer> input_layer;

    //! Target layer (other part of visible)
    PP<RBMMultinomialLayer> target_layer;

    //! Hidden
    PP<RBMBinomialLayer> hidden_layer;

    //! Connection between input and hidden
    PP<RBMMatrixConnection> input_to_hidden;

    //! Connection between target and hidden
    PP<RBMMatrixConnection> target_to_hidden;

    //! How to compute hidden and target expectation given input
    //! Possible values are:
    //!     - "exact": exact inference, O(target_size), default
    //!     - "gibbs": estimation by Gibbs sampling
    string exp_method;

    //! Number of Gibbs steps to use if exp_method=="gibbs"
    int n_gibbs_steps;

    //! Random numbers generator
    PP<PRandom> random_gen;

    //! Whether to use fast approximations in softplus computation
    bool use_fast_approximations;



public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    // ### Make sure the implementation in the .cc
    // ### initializes all fields to reasonable default values.
    InferenceRBM();

    // Your other public member functions go here

    void hiddenExpGivenVisible(const Mat& visible);
    void hiddenExpGivenInputTarget(const Mat& input, const TVec<int>& target);
    void hiddenExpGivenInput(const Mat& input);
    void targetExpGivenInput(const Mat& input);

    Mat getHiddenExpGivenVisible(const Mat& visible);
    Mat getHiddenExpGivenInputTarget(const Mat& input, const TVec<int>& target);
    Mat getHiddenExpGivenInput(const Mat& input);
    Mat getTargetExpGivenInput(const Mat& input);

    void supCDStep(const Mat& visible);
    void unsupCDStep(const Mat& input);

    virtual void setLearningRate(real the_learning_rate);

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(InferenceRBM);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    // (PLEASE IMPLEMENT IN .cc)
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //#####  Protected Options  ###############################################

    // ### Declare protected option fields (such as learned parameters) here
    //! Visible layer (concatenation of input and target)
    PP<RBMMixedLayer> visible_layer;

    PP<RBMMixedConnection> visible_to_hidden;

    //! Size of input layer
    int input_size;

    //! Size of target layer
    int target_size;

    //! Size of visible layer
    int visible_size;

    //! Size of hidden layer
    int hidden_size;

protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    static void declareOptions(OptionList& ol);

    //! Declares the class methods.
    static void declareMethods(RemoteMethodMap& rmm);

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    void build_();

    void build_rbms();

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
    mutable Mat v0;
    mutable Mat h0;
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(InferenceRBM);

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
