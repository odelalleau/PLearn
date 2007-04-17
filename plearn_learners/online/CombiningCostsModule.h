// -*- C++ -*-

// CombiningCostsModule.h
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

/*! \file CombiningCostsModule.h */


#ifndef CombiningCostsModule_INC
#define CombiningCostsModule_INC

#include <plearn_learners/online/CostModule.h>

namespace PLearn {

/**
 * Combine several CostModules with the same input and target.
 * It is possible to assign a weight on each of the sub_modules, so the
 * back-propagated gradient will be a weighted sum of the modules' gradients.
 * The first output is the weighted sum of the cost, the following ones are the
 * original costs.
 */
class CombiningCostsModule : public CostModule
{
    typedef CostModule inherited;

public:
    //#####  Public Build Options  ############################################

    //! Vector containing the different sub_costs
    TVec< PP<CostModule> > sub_costs;

    //! The weights associated to each of the sub_costs
    Vec cost_weights;

    //! Number of sub-costs
    int n_sub_costs;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    CombiningCostsModule();

    // Your other public member functions go here

    //! given the input and target, compute the cost
    virtual void fprop(const Vec& input, const Vec& target, Vec& cost) const;

    //! Adapt based on the output gradient: this method should only
    //! be called just after a corresponding fprop.
    virtual void bpropUpdate(const Vec& input, const Vec& target, real cost,
                             Vec& input_gradient, bool accumulate=false);

    //! Calls this method on the sub_costs
    virtual void bpropUpdate(const Vec& input, const Vec& target, real cost);

    //! Similar to bpropUpdate, but adapt based also on the estimation
    //! of the diagonal of the Hessian matrix, and propagates this back.
    virtual void bbpropUpdate(const Vec& input, const Vec& target, real cost,
                              Vec& input_gradient, Vec& input_diag_hessian,
                              bool accumulate=false);

    //! Calls this method on the sub_costs
    virtual void bbpropUpdate(const Vec& input, const Vec& target, real cost);

    //! Overridden to do nothing (no warning message in particular).
    virtual void setLearningRate(real dynamic_learning_rate) {}

    //! reset the parameters to the state they would be BEFORE starting
    //! training.  Note that this method is necessarily called from
    //! build().
    virtual void forget();

    //! optionally perform some processing after training, or after a
    //! series of fprop/bpropUpdate calls to prepare the model for truly
    //! out-of-sample operation.
    virtual void finalize();

    //! in case bpropUpdate does not do anything, make it known
    virtual bool bpropDoesNothing();

    //! Indicates the name of the computed costs
    virtual TVec<string> name();

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(CombiningCostsModule);

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

    //! Stores the output values of the sub_costs
    mutable Vec sub_costs_values;

    //! Stores intermediate values of the input gradient
    mutable Vec partial_gradient;

    //! Stores intermediate values of the input diagonal of Hessian
    mutable Vec partial_diag_hessian;
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(CombiningCostsModule);

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
