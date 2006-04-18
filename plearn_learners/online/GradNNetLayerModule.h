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
 * This class
 *
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

    Mat init_weights;

    real init_weights_random_scale;

    real L1_penalty_factor, L2_penalty_factor; //! weight decays

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    GradNNetLayerModule();

    // Your other public member functions go here

    virtual void fprop(const Vec& input, Vec& output) const;

    virtual void bpropUpdate(const Vec& input, const Vec& output,
                             const Vec& output_gradient);

    virtual void bpropUpdate(const Vec& input, const Vec& output,
                             Vec& input_gradient, const Vec& output_gradient);

    virtual void bbpropUpdate(const Vec& input, const Vec& output,
                              const Vec& output_gradient,
                              const Vec& output_diag_hessian);

    virtual void bbpropUpdate(const Vec& input, const Vec& output,
                              Vec& input_gradient,
                              const Vec& output_gradient,
                              Vec& input_diag_hessian,
                              const Vec& output_diag_hessian);

    virtual void forget();

    virtual Mat getWeights() const;
    virtual Vec getWeights(int i) const;
    virtual void setWeights(const Mat& the_weights);
    virtual void setWeights(int i, const Vec& the_weights);

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(GradNNetLayerModule);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    // (PLEASE IMPLEMENT IN .cc)
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //#####  Protected Options  ###############################################
    Mat weights; // one neuron per line, bias first

protected:
    //#####  Protected Member Functions  ######################################
    virtual void resetWeights();

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
