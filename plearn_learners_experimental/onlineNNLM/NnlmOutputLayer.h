// -*- C++ -*-

// NnlmOutputLayer.h
//
// Copyright (C) 2006 Pierre-Antoine Manzagol
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

// Authors: Pierre-Antoine Manzagol

/*! \file NnlmOutputLayer.h */


#ifndef NnlmOutputLayer_INC
#define NnlmOutputLayer_INC

#include <plearn/base/Object.h>
#include <plearn/math/TMat_maths.h>
#include <plearn_learners/online/OnlineLearningModule.h>

namespace PLearn {

/**
 * Implements the output layer for the Neural Network Language Model. 
 *
 * The previous layer produces the context's (semantic, etc) representation, 'r'.
 * This layer holds the model for p(r|w), where w is a word.
 *
 * @todo 
 * @deprecated 
 * 
 */
class NnlmOutputLayer : public OnlineLearningModule
{
    typedef OnlineLearningModule inherited;

public:
    //#####  Public Build Options  ############################################

    //! - NNLM related -
    int vocabulary_size;
    int word_representation_size;
    int context_size;

    //! discounts the gaussians' old parameters in the computation of the new
    //! ones
    real start_gaussian_learning_discount_rate;
    real gaussian_learning_decrease_constant;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    NnlmOutputLayer();

    //! Sets w ( fprop computes p(r,w) )
    void setCurrentWord(int the_current_word);

    //! Sets the context (encoded in wordtags) - NOT USED
    void setContext(const Vec& the_current_context);


    //! Computes p(r,w), where r the real distributed context representation 
    //! in [0,1] and w the word at the considered position
    //! given the input, compute the output (possibly resize it  appropriately)
    virtual void fprop(const Vec& input, Vec& output) const;

    //! Adapt based on the output gradient: this method should only
    //! be called just after a corresponding fprop; it should be
    //! called with the same arguments as fprop for the first two arguments
    //! (and output should not have been modified since then).
    //! Since sub-classes are supposed to learn ONLINE, the object
    //! is 'ready-to-be-used' just after any bpropUpdate.
    virtual void bpropUpdate(const Vec& input, const Vec& output,
                             const Vec& output_gradient);

    //! NOT IMPLEMENTED - GRADIENT COMPUTED IN NnlmOnlineLearner
    //! And I'm not sure why... TODO find out
    //! this version allows to obtain the input gradient as well
    //! N.B. THE DEFAULT IMPLEMENTATION IN SUPER-CLASS JUST RAISES A PLERROR.
    // virtual void bpropUpdate(const Vec& input, const Vec& output,
    //                          Vec& input_gradient,
    //                          const Vec& output_gradient);

    //! reset the parameters to the state they would be BEFORE starting
    //! training.  Note that this method is necessarily called from
    //! build().
    virtual void forget();

    //! optionally perform some processing after training, or after a
    //! series of fprop/bpropUpdate calls to prepare the model for truly
    //! out-of-sample operation.  THE DEFAULT IMPLEMENTATION PROVIDED IN
    //! THE SUPER-CLASS DOES NOT DO ANYTHING.
    // virtual void finalize();


    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT
    PLEARN_DECLARE_OBJECT(NnlmOutputLayer);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);


protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    static void declareOptions(OptionList& ol);

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    void build_();
    void resetParameters();

private:
    //#####  Private Data Members  ############################################


public:
    //#####  Public NOT Options  ##############################################

    //! keeps track of updates
    int step_number;

    //! We use a mixture with a uniform to prevent negligeable probabilities
    //! which cause gradient explosions. Learned as mean of p(g|r)
    //! (probability that gaussian is responsible for observation, given r)
    //! uniform mixture coefficient
    real umc;

    //! The gaussian parameters
    //! No longer computed as in these comments
    Mat mu;       // mu(i) -> moyenne empirique des x quand y=i
    Mat sigma2;   // sigma2(i) -> variance empirique des x quand y=i

    //! unigramme absolu
    Vec pi;       // pi[i] -> moyenne empirique de y==i

    //! Variables intermédiaires de compte
    Mat sumX;     // sumX(i) -> sum_t x_t 1_{y==i}
    Mat sumX2;    // sumX2(i) -> sum_t x_t^2 1_{y==i}
    TVec<int> sumI;     // sumI(i) -> sum_t 1_{y==i}
    int s_sumI;  // sum_t 1

    //#####  Don't need to be saved  ##########################################
    //! the current word -> we use its parameters to compute output
    int current_word;
    Vec context;

    //! temporary variables
    //! TODO clean this up
    mutable real r;
    mutable real g_exponent;
    mutable real det_g_covariance;
    mutable real log_g_normalization;

    mutable real log_p_rg_i;
    mutable real log_p_r_i;
    mutable real log_p_ri;

    mutable real log_p_g_r;
    mutable real sum_log_p_g_r;

    //! The original way of computing the mus and sigmas (ex. mu memorize \sum r
    //! and then divide) had the effect learning slowed down with time.
    //! We use this discount rate now.
    //! TODO validate computation of mus and sigmas
    //! gaussian_learning_discount_rate
    real gldr;

};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(NnlmOutputLayer);

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
