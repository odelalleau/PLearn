// -*- C++ -*-
// PvGradNNet.h
//
// Copyright (C) 2007 PA M
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

// Authors: PA M

/*! \file PvGradNNet.h */


#ifndef PvGradNNet_INC
#define PvGradNNet_INC

#include <plearn_learners/generic/EXPERIMENTAL/mNNet.h>

namespace PLearn {

/**
 * Multi-layer neural network based on matrix-matrix multiplications.
 */
class PvGradNNet : public mNNet
{
    typedef mNNet inherited;

public:
    //#####  Public Build Options  ############################################

    //! Initial size of steps in parameter space
    real pv_initial_stepsize;

    //! Bounds for the step sizes
    real pv_min_stepsize, pv_max_stepsize;

    //! Coefficients by which to multiply the step sizes
    real pv_acceleration, pv_deceleration;

    //! PV's gradient minimum number of samples to estimate confidence
    int pv_min_samples;

    //! Minimum required confidence (probability of being positive or negative)
    //! for taking a step.
    real pv_required_confidence;

    int pv_strategy;

    //! If this is set to true, then we will randomly choose the step sign for
    // each parameter based on the estimated probability of it being positive
    // or negative.
    bool pv_random_sample_step;

public:
    //#####  Public Not Build Options  ########################################

public:
    //#####  Public Member Functions  #########################################

    PvGradNNet();

    //#####  PLearner Member Functions  #######################################

    //! (Re-)initializes the PLearner in its fresh state (that state may depend
    //! on the 'seed' option) and sets 'stage' back to 0 (this is the stage of
    //! a fresh learner!).
    virtual void forget();

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS
    PLEARN_DECLARE_OBJECT(PvGradNNet);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //#####  Protected Options  ###############################################
    // ### Declare protected option fields (such as learned parameters) here

protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    static void declareOptions(OptionList& ol);

    virtual void bpropUpdateNet(const int t);

    void pvGrad(); 
    void discountGrad();
    void globalSyncGrad();
    void neuronSyncGrad();
    
private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    void build_();

private:
    //#####  Private Data Members  ############################################

    //! Holds the number of samples gathered for each weight
    TVec<int> pv_all_nsamples;
    TVec< TMat<int> > pv_layer_nsamples;
    //! Sum of collected gradients 
    Vec pv_all_sum;
    TVec<Mat> pv_layer_sum;
    //! Sum of squares of collected gradients 
    Vec pv_all_sumsquare;
    TVec<Mat> pv_layer_sumsquare;

    //! Temporary add-on. Allows an undetermined signed value (zero).
    TVec<int> pv_all_stepsigns;
    TVec< TMat<int> > pv_layer_stepsigns;

    //! The step size (absolute value) to be taken for each parameter.
    Vec pv_all_stepsizes;
    TVec<Mat> pv_layer_stepsizes;

    //! accumulated statistics of gradients on each parameter.
    //PP<VecStatsCollector> pv_gradstats;


};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(PvGradNNet);

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
