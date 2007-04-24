// -*- C++ -*-

// CostModule.cc
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

/*! \file CostModule.cc */



#include "CostModule.h"
#include <plearn/math/TMat_maths.h>


namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    CostModule,
    "General class representing a cost function module",
    "It usually takes an input and a target, and outputs one cost.\n"
    "It can also output more costs, in that case the first one will be the\n"
    "objective function to be decreased.\n");

CostModule::CostModule() :
    target_size(-1)
{
}

void CostModule::declareOptions(OptionList& ol)
{
    declareOption(ol, "target_size", &CostModule::target_size,
                  OptionBase::buildoption,
                  "Size of the target vectors.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);

    redeclareOption(ol, "output_size", &CostModule::output_size,
                    OptionBase::buildoption,
                    "Number of costs (outputs).");
}

void CostModule::build_()
{
}

// ### Nothing to add here, simply calls build_
void CostModule::build()
{
    inherited::build();
    build_();
}


/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void CostModule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(tmp_costs,                            copies);
    deepCopyField(tmp_input_and_target,                 copies);
    deepCopyField(tmp_input_and_target_gradient,        copies);
    deepCopyField(tmp_input_and_target_diag_hessian,    copies);
    deepCopyField(tmp_costs_mat,                        copies);
    deepCopyField(tmp_input_gradients,                  copies);
}


///////////
// fprop //
///////////
void CostModule::fprop(const Vec& input, const Vec& target, Vec& cost) const
{
    PLERROR("CostModule::fprop(const Vec& input, const Vec& target, Vec& cost)"
            "\n"
            "is not implemented. You have to implement it in your class.\n");
}

void CostModule::fprop(const Mat& inputs, const Mat& targets, Mat& costs) const
{
    //PLWARNING("CostModule::fprop - Not implemented for class %s",
    //       classname().c_str());
    // Default (possibly inefficient) implementation.
    costs.resize(inputs.length(), output_size);
    Vec input, target, cost;
    for (int i = 0; i < inputs.length(); i++) {
        input = inputs(i);
        target = targets(i);
        cost = costs(i);
        this->fprop(input, target, cost);
    }
}

void CostModule::fprop(const Vec& input, const Vec& target, real& cost) const
{
    // Keep only the first cost.
    fprop( input, target, tmp_costs );
    cost = tmp_costs[0];
}

void CostModule::fprop(const Mat& inputs, const Mat& targets, Vec& costs)
{
    //PLWARNING("In CostModule::fprop - Using default (possibly inefficient) "
    //        "implementation for class %s", classname().c_str());
    // Keep only the first cost.
    tmp_costs_mat.resize(inputs.length(), output_size);
    fprop(inputs, targets, tmp_costs_mat);
    costs << tmp_costs_mat.column(0);
}

//! for compatibility with OnlineLearningModule interface
void CostModule::fprop(const Vec& input_and_target, Vec& output) const
{
    PLASSERT( input_and_target.size() == input_size + target_size );
    fprop( input_and_target.subVec( 0, input_size ),
           input_and_target.subVec( input_size, target_size ),
           output );
}


/////////////////
// bpropUpdate //
/////////////////
void CostModule::bpropUpdate(const Vec& input, const Vec& target, real cost,
                             Vec& input_gradient, bool accumulate)
{
    // default version, calling the bpropUpdate with inherited prototype
    tmp_input_and_target.resize( input_size + target_size );
    tmp_input_and_target.subVec( 0, input_size ) << input;
    tmp_input_and_target.subVec( input_size, target_size ) << target;
    tmp_input_and_target_gradient.resize( input_size + target_size );
    tmp_costs.resize(1);
    tmp_costs[0] = cost;
    static const Vec one(1,1);

    bpropUpdate( tmp_input_and_target, tmp_costs,
                 tmp_input_and_target_gradient, one );

    if( accumulate )
    {
        PLASSERT_MSG( input_gradient.size() == input_size,
                      "Cannot resize input_gradient AND accumulate into it" );
        input_gradient += tmp_input_and_target_gradient.subVec( 0, input_size );
    }
    else
    {
        input_gradient.resize( input_size );
        input_gradient << tmp_input_and_target_gradient.subVec( 0, input_size );
    }
}

void CostModule::bpropUpdate(const Vec& input, const Vec& target, real cost)
{
    bpropUpdate( input, target, cost, tmp_input_gradient );
}

void CostModule::bpropUpdate(const Mat& inputs, const Mat& targets,
        const Vec& costs)
{
    PLWARNING("In CostModule::bpropUpdate - Using default (possibly "
        "inefficient) version for class %s", classname().c_str());
    bpropUpdate( inputs, targets, costs, tmp_input_gradients );
}

void CostModule::bpropUpdate(const Vec& input_and_target, const Vec& output,
                             Vec& input_and_target_gradient,
                             const Vec& output_gradient,
                             bool accumulate)
{
    inherited::bpropUpdate( input_and_target, output,
                            input_and_target_gradient, output_gradient,
                            accumulate );
}


void CostModule::bbpropUpdate(const Vec& input, const Vec& target, real cost,
                              Vec& input_gradient, Vec& input_diag_hessian,
                              bool accumulate)
{
    // default version, calling the bpropUpdate with inherited prototype
    tmp_input_and_target.resize( input_size + target_size );
    tmp_input_and_target.subVec( 0, input_size ) << input;
    tmp_input_and_target.subVec( input_size, target_size ) << target;
    tmp_input_and_target_gradient.resize( input_size + target_size );
    tmp_input_and_target_diag_hessian.resize( input_size + target_size );
    tmp_costs.resize(1);
    tmp_costs[0] = cost;
    static const Vec one(1,1);
    static const Vec zero(1);

    bbpropUpdate( tmp_input_and_target, tmp_costs,
                  tmp_input_and_target_gradient, one,
                  tmp_input_and_target_diag_hessian, zero,
                  accumulate );

    if( accumulate )
    {
        PLASSERT_MSG( input_gradient.size() == input_size,
                      "Cannot resize input_gradient AND accumulate into it" );
        PLASSERT_MSG( input_diag_hessian.size() == input_size,
                      "Cannot resize input_diag_hessian AND accumulate into it"
                    );

        input_gradient += tmp_input_and_target_gradient.subVec( 0, input_size );
        input_diag_hessian +=
            tmp_input_and_target_diag_hessian.subVec( 0, input_size );
    }
    else
    {
        input_gradient.resize( input_size );
        input_diag_hessian.resize( input_size );
        input_gradient << tmp_input_and_target_gradient.subVec( 0, input_size );
        input_diag_hessian <<
            tmp_input_and_target_diag_hessian.subVec( 0, input_size );
    }
}

void CostModule::bbpropUpdate(const Vec& input, const Vec& target, real cost)
{
    bbpropUpdate( input, target, cost,
                  tmp_input_gradient, tmp_input_diag_hessian );
}

void CostModule::bbpropUpdate(const Vec& input_and_target, const Vec& output,
                              Vec& input_and_target_gradient,
                              const Vec& output_gradient,
                              Vec& input_and_target_diag_hessian,
                              const Vec& output_diag_hessian,
                              bool accumulate)
{
    inherited::bbpropUpdate( input_and_target, output,
                             input_and_target_gradient,
                             output_gradient,
                             input_and_target_diag_hessian,
                             output_diag_hessian,
                             accumulate );
}

void CostModule::forget()
{
}

TVec<string> CostModule::name()
{
    return TVec<string>();
}

} // end of namespace PLearn


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
