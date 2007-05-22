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
    deepCopyField(store_costs,                          copies);
}

////////////////////
// bpropAccUpdate //
////////////////////
void CostModule::bpropAccUpdate(const TVec<Mat*>& ports_value,
                                const TVec<Mat*>& ports_gradient)
{
    if (ports_gradient.length() == 3) {
        Mat* pred_grad = ports_gradient[0];
        Mat* target_grad = ports_gradient[1];
        Mat* cost_grad = ports_gradient[2];
        if (!pred_grad && !target_grad && !cost_grad) {
            // No gradient is being asked or provided at all.
            return;
        }
        if (pred_grad && !target_grad && cost_grad &&
            pred_grad->isEmpty() && !cost_grad->isEmpty())
        {
            // We can probably use the standard mini-batch bpropUpdate.
            // PLASSERT( cost_grad->width() == 1 );
#ifdef BOUNDCHECK
            // The gradient on the cost must be one if we want to re-use
            // exactly the existing code.
            for (int i = 0; i < cost_grad->length(); i++) {
                for (int j = 0; j < cost_grad->width(); j++) {
                    PLASSERT( fast_exact_is_equal((*cost_grad)(i, j), 1) );
                }
            }
#endif
            Mat* cost_val = ports_value[2];
            PLASSERT( cost_val );
            Vec costs_vec;
            if (cost_val->mod() == 1) {
                // We can view the cost column matrix as a vector.
                costs_vec = cost_val->toVec();
            } else {
                // We need to make a copy of the cost.
                store_costs.resize(cost_val->length());
                store_costs << cost_val->column(0);
                costs_vec = store_costs;
            }
            Mat* pred_val = ports_value[0];
            Mat* target_val = ports_value[1];
            PLASSERT( pred_val && target_val );
            pred_grad->resize(pred_val->length(), pred_val->width());
            bpropUpdate(*pred_val, *target_val, costs_vec, *pred_grad, true);
            return;
        }
    }
    // Try to use the parent's default method.
    inherited::bpropAccUpdate(ports_value, ports_gradient);
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
    costs.resize(tmp_costs_mat.length());
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

void CostModule::fprop(const TVec<Mat*>& ports_value)
{
    PLASSERT( ports_value.length() == nPorts() );
    if (ports_value.length() == 3) {
        Mat* prediction = ports_value[0];
        Mat* target = ports_value[1];
        Mat* cost = ports_value[2];
        if (prediction && target && cost &&
            !prediction->isEmpty() && !target->isEmpty() && cost->isEmpty())
        {
            // Standard fprop: (prediction, target) -> cost
            fprop(*prediction, *target, *cost);
            return;
        }
    }
    // Default version does not work: try to re-use the parent's default fprop.
    inherited::fprop(ports_value);
}

////////////////////////
// getPortDescription //
////////////////////////
TVec<string> CostModule::getPortDescription(const string& port)
{
    if (port == "cost")
        return name();
    else
        return inherited::getPortDescription(port);
}

//////////////
// getPorts //
//////////////
const TVec<string>& CostModule::getPorts() {
    static TVec<string> default_ports;
    if (default_ports.isEmpty()) {
        default_ports.append("prediction");
        default_ports.append("target");
        default_ports.append("cost");
    }
    return default_ports;
}

//////////////////
// getPortSizes //
//////////////////
const TMat<int>& CostModule::getPortSizes() {
    int n_ports = nPorts();
    if (port_sizes.length() != n_ports) {
        port_sizes.resize(n_ports, 2);
        port_sizes.fill(-1);
        if (n_ports == 3) {
            PLASSERT( getPorts()[0] == "prediction" &&
                      getPorts()[1] == "target"     &&
                      getPorts()[2] == "cost" );
            port_sizes(0, 1) = input_size;
            port_sizes(1, 1) = target_size;
            port_sizes(2, 1) = output_size;
        }
    }
    return port_sizes;
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
