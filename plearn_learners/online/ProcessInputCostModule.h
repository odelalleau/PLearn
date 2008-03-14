// -*- C++ -*-

// ProcessInputCostModule.h
//
// Copyright (C) 2007 Pascal Lamblin
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

/*! \file ProcessInputCostModule.h */


#ifndef ProcessInputCostModule_INC
#define ProcessInputCostModule_INC

#include <plearn_learners/online/CostModule.h>

namespace PLearn {

/**
 * Processes the input through an embedded OnlineLearningModule.
 * This Module embeds an OnlineLearningModule, processing_module, and a
 * CostModule, cost_module. The input goes through processing_module,
 * the output of which is used as input by the CostModule.
 * If you want the input to go through several processing steps, you can use a
 * ModuleStackModule as processing_module.
 *
 * @todo: code ModuleStackModule
 */
class ProcessInputCostModule : public CostModule
{
    typedef CostModule inherited;

public:
    //#####  Public Build Options  ############################################

    //! Module that processes the input
    PP<OnlineLearningModule> processing_module;

    //! CostModule that outputs this cost
    PP<CostModule> cost_module;

    //! Size of processing_module's output
    int processed_size;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    ProcessInputCostModule();

    //! Given the input and the target, compute only the first cost
    //! (of which we will compute the gradient)
    virtual void fprop(const Vec& input, const Vec& target, real& cost) const;

    //! Minibatch version
    virtual void fprop(const Mat& inputs, const Mat& targets, Vec& costs );

    //! Given the input and the target, compute a vector of costs
    //! (possibly resize it appropriately)
    virtual void fprop(const Vec& input, const Vec& target, Vec& cost) const;

    //! Minibatch version
    virtual void fprop(const Mat& inputs, const Mat& targets, Mat& costs )
        const;

    //! Adapt based on the cost, and compute input gradient to backpropagate.
    virtual void bpropUpdate(const Vec& input, const Vec& target, real cost,
                             Vec& input_gradient, bool accumulate=false);

    //! Minibatch version
    virtual void bpropUpdate(const Mat& inputs, const Mat& targets,
                             const Vec& costs, Mat& input_gradients,
                             bool accumulate=false);

    /* Optional
       N.B. A DEFAULT IMPLEMENTATION IS PROVIDED IN THE SUPER-CLASS, WHICH
       JUST CALLS
            bpropUpdate(input, target, cost, input_gradient)
       AND IGNORES INPUT GRADIENT.
    //! Adapt based on the the cost.
    virtual void bpropUpdate(const Vec& input, const Vec& target,
                             real cost);
    */

    //! Similar to bpropUpdate, but adapt based also on the estimation
    //! of the diagonal of the Hessian matrix, and propagates this back.
    //! If these methods are defined, you can use them INSTEAD of
    //! bpropUpdate(...)
    virtual void bbpropUpdate(const Vec& input, const Vec& target, real cost,
                              Vec& input_gradient, Vec& input_diag_hessian,
                              bool accumulate=false);

    /* Optional
       N.B. A DEFAULT IMPLEMENTATION IS PROVIDED IN THE SUPER-CLASS,
       WHICH JUST CALLS
            bbpropUpdate(input, target, cost, input_gradient, in_hess)
       AND IGNORES INPUT HESSIAN AND INPUT GRADIENT.
    //! This version does not obtain the input gradient and diag_hessian.
    virtual void bbpropUpdate(const Vec& input, const Vec& target,
                              real cost);
    */

    //! Reset the parameters to the state they would be BEFORE starting
    //! training.  Note that this method is necessarily called from
    //! build().
    virtual void forget();


    //! Perform some processing after training, or after a series of
    //! fprop/bpropUpdate calls to prepare the model for truly out-of-sample
    //! operation.
    virtual void finalize();

    //! In case bpropUpdate does not do anything, make it known
    virtual bool bpropDoesNothing();

    //! If this class has a learning rate (or something close to it), set it
    virtual void setLearningRate(real dynamic_learning_rate);

    //! Indicates the name of the computed costs
    virtual TVec<string> name();


    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(ProcessInputCostModule);

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

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
    //#####  Not Options  #####################################################
    mutable Vec processed_value;
    mutable Mat processed_values;
    mutable Vec processed_gradient;
    mutable Mat processed_gradients;
    mutable Vec processed_diag_hessian;
    mutable Mat processed_diag_hessians;
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(ProcessInputCostModule);

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
