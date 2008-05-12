// -*- C++ -*-

// CostModule.h
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

/*! \file CostModule.h */


#ifndef CostModule_INC
#define CostModule_INC

#include <plearn_learners/online/OnlineLearningModule.h>

namespace PLearn {

/**
 * General class representing a cost function module.
 * It usually takes an input and a target, and outputs one cost.
 * It can also output more costs, in that case the first one will be the
 * objective function to be decreased.
 */
class CostModule : public OnlineLearningModule
{
    typedef OnlineLearningModule inherited;

public:
    //#####  Public Build Options  ############################################

    //! Size of the target
    int target_size;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    CostModule();

    // Your other public member functions go here

    //! given the input and target, compute the main output (cost)
    virtual void fprop(const Vec& input, const Vec& target, real& cost ) const;

    //! Mini-batch version.
    virtual void fprop(const Mat& inputs, const Mat& targets, Vec& costs );

    //! this version allows for several costs
    virtual void fprop(const Vec& input, const Vec& target, Vec& cost ) const;

    //! Mini-batch version with several costs..
    virtual void fprop(const Mat& inputs, const Mat& targets, Mat& costs)
        const;

    //! this version is provided for compatibility with the parent class
    virtual void fprop(const Vec& input_and_target, Vec& output) const;

    //! Adapt based on the cost gradient, and obtain the input gradient
    virtual void bpropUpdate(const Vec& input, const Vec& target, real cost,
                             Vec& input_gradient, bool accumulate=false);

    //! Adapt based on the mini-batch cost gradient, and obtain the mini-batch
    //! input gradient.
    virtual void bpropUpdate(const Mat& inputs, const Mat& targets,
            const Vec& costs, Mat& input_gradients, bool accumulate = false)
    {
        PLERROR("bpropUpdate on mini-batches not implemented in class %s",
                classname().c_str());
    }

    //! Without the input gradient
    virtual void bpropUpdate(const Vec& input, const Vec& target, real cost );

    virtual void bpropUpdate(const Mat& inputs, const Mat& targets,
            const Vec& costs);

    //! this version is provided for compatibility with the parent class.
    virtual void bpropUpdate(const Vec& input_and_target, const Vec& output,
                             Vec& input_and_target_gradient,
                             const Vec& output_gradient,
                             bool accumulate=false);


    // TODO Had to override to compile, this is weird.
    virtual void bpropUpdate(const Mat& input, const Mat& output,
                             Mat& input_gradient, const Mat& output_gradient,
                             bool accumulate=false) 
    {
        inherited::bpropUpdate(input, output, input_gradient, output_gradient,
                accumulate);
    }

    //! Similar to bpropUpdate, but adapt based also on the estimation
    //! of the diagonal of the Hessian matrix, and propagates this back.
    virtual void bbpropUpdate(const Vec& input, const Vec& target, real cost,
                              Vec& input_gradient, Vec& input_diag_hessian,
                              bool accumulate=false);

    //! Without the input gradient and diag_hessian
    virtual void bbpropUpdate(const Vec& input, const Vec& target, real cost );

    //! this version is provided for compatibility with the parent class.
    virtual void bbpropUpdate(const Vec& input_and_target, const Vec& output,
                              Vec& input_and_target_gradient,
                              const Vec& output_gradient,
                              Vec& input_and_target_diag_hessian,
                              const Vec& output_diag_hessian,
                              bool accumulate=false);
    virtual void forget();

    //! Indicates the name of the computed costs
    virtual TVec<string> costNames();

    //! Overridden so that the default ports being returned are "prediction",
    //! "target" and "cost".
    virtual const TVec<string>& getPorts();

    //! Overridden so that the default behavior returns proper widths for the
    //! 'prediction', 'target' and 'cost' ports.
    virtual const TMat<int>& getPortSizes();

    //! Overridden to try to use the standard mini-batch fprop when possible.
    virtual void fprop(const TVec<Mat*>& ports_value);

    //! Overridden to try to use the standard mini-batch bprop when possible.
    virtual void bpropAccUpdate(const TVec<Mat*>& ports_value,
                                const TVec<Mat*>& ports_gradient);

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(CostModule);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);


protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    static void declareOptions(OptionList& ol);

protected:
    mutable Vec tmp_costs;
    mutable Vec tmp_input_and_target;
    mutable Vec tmp_input_and_target_gradient;
    mutable Vec tmp_input_and_target_diag_hessian;
    Mat tmp_costs_mat;
    Mat tmp_input_gradients;
    Vec store_costs; //!< Used to store costs temporarily.

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    void build_();

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(CostModule);

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
