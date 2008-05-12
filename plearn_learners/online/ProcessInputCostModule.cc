// -*- C++ -*-

// ProcessInputCostModule.cc
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

/*! \file ProcessInputCostModule.cc */



#include "ProcessInputCostModule.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    ProcessInputCostModule,
    "Processes the input through an embedded OnlineLearningModule",
    "This Module embeds an OnlineLearningModule, processing_module, and a\n"
    "CostModule, cost_module. The input goes through processing_module,\n"
    "the output of which is used as input by the CostModule.\n"
    "If you want the input to go through several processing steps, you can\n"
    "use a ModuleStackModule as processing_module.\n"
    );

ProcessInputCostModule::ProcessInputCostModule() :
    processed_size( -1 )
{
}

void ProcessInputCostModule::declareOptions(OptionList& ol)
{
    // declareOption(ol, "myoption", &ProcessInputCostModule::myoption,
    //               OptionBase::buildoption,
    //               "Help text describing this option");

    declareOption(ol, "processing_module",
                  &ProcessInputCostModule::processing_module,
                  OptionBase::buildoption,
                  "Module that processes the input");

    declareOption(ol, "cost_module",
                  &ProcessInputCostModule::cost_module,
                  OptionBase::buildoption,
                  "Module that outputs the cost");

    declareOption(ol, "processed_size",
                  &ProcessInputCostModule::processed_size,
                  OptionBase::learntoption,
                  "Size of processing_module's output");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void ProcessInputCostModule::build_()
{
    if( processing_module )
    {
        input_size = processing_module->input_size;
        processed_size = processing_module->output_size;
        // If we have a random_gen and processing_module does not, share it
        if( random_gen && !(processing_module->random_gen) )
        {
            processing_module->random_gen = random_gen;
            processing_module->forget();
        }
    }

    if( cost_module )
    {
        output_size = cost_module->output_size;
        target_size = cost_module->target_size;
        // If we have a random_gen and cost_module does not, share it
        if( random_gen && !(cost_module->random_gen) )
        {
            cost_module->random_gen = random_gen;
            cost_module->forget();
        }
    }

    if( processing_module && cost_module )
        PLASSERT( processed_size == cost_module->input_size );
}

void ProcessInputCostModule::build()
{
    inherited::build();
    build_();
}


void ProcessInputCostModule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(processing_module, copies);
    deepCopyField(cost_module, copies);
    deepCopyField(processed_value, copies);
    deepCopyField(processed_values, copies);
    deepCopyField(processed_gradient, copies);
    deepCopyField(processed_gradients, copies);
    deepCopyField(processed_diag_hessian, copies);
    deepCopyField(processed_diag_hessians, copies);
}

///////////
// fprop //
///////////
void ProcessInputCostModule::fprop(const Vec& input, const Vec& target,
                                   real& cost) const
{
    PLASSERT( processing_module );
    PLASSERT( cost_module );
    PLASSERT( input.size() == input_size );
    PLASSERT( target.size() == target_size );

    processing_module->fprop( input, processed_value );
    cost_module->fprop( processed_value, target, cost );
}

void ProcessInputCostModule::fprop(const Mat& inputs, const Mat& targets,
                                   Vec& costs )
{
    PLASSERT( processing_module );
    PLASSERT( cost_module );
    PLASSERT( inputs.width() == input_size );
    PLASSERT( targets.width() == target_size );
    PLASSERT( inputs.length() == targets.length() );

    processing_module->fprop( inputs, processed_values );
    cost_module->fprop( processed_values, targets, costs );
}

void ProcessInputCostModule::fprop(const Vec& input, const Vec& target,
                                   Vec& cost) const
{
    PLASSERT( processing_module );
    PLASSERT( cost_module );
    PLASSERT( input.size() == input_size );
    PLASSERT( target.size() == target_size );

    processing_module->fprop( input, processed_value );
    cost_module->fprop( processed_value, target, cost );
}

void ProcessInputCostModule::fprop(const Mat& inputs, const Mat& targets,
                                   Mat& costs ) const
{
    PLASSERT( processing_module );
    PLASSERT( cost_module );
    PLASSERT( inputs.width() == input_size );
    PLASSERT( targets.width() == target_size );
    PLASSERT( inputs.length() == targets.length() );

    processing_module->fprop( inputs, processed_values );
    cost_module->fprop( processed_values, targets, costs );
}



/////////////////
// bpropUpdate //
/////////////////
void ProcessInputCostModule::bpropUpdate(const Vec& input, const Vec& target,
                                         real cost, Vec& input_gradient,
                                         bool accumulate)
{
    PLASSERT( processing_module );
    PLASSERT( cost_module );
    PLASSERT( input.size() == input_size );
    PLASSERT( target.size() == target_size );

    if( accumulate )
    {
        PLASSERT_MSG( input_gradient.size() == input_size,
                      "Cannot resize input_gradient AND accumulate into it" );
    }

    cost_module->bpropUpdate( processed_value, target, cost,
                              processed_gradient );
    processing_module->bpropUpdate( input, processed_value,
                                    input_gradient, processed_gradient,
                                    accumulate );
}

void ProcessInputCostModule::bpropUpdate(const Mat& inputs, const Mat& targets,
                                         const Vec& costs, Mat& input_gradients,
                                         bool accumulate)
{
    PLASSERT( processing_module );
    PLASSERT( cost_module );
    PLASSERT( inputs.width() == input_size );
    PLASSERT( targets.width() == target_size );
    PLASSERT( inputs.length() == targets.length() );
    PLASSERT( inputs.length() == costs.size() );

    if( accumulate )
    {
        PLASSERT_MSG( input_gradients.width() == input_size
                      && input_gradients.length() == inputs.length(),
                      "Cannot resize input_gradient AND accumulate into it" );
    }

    cost_module->bpropUpdate( processed_values, targets, costs,
                              processed_gradients );
    processing_module->bpropUpdate( inputs, processed_values,
                                    input_gradients, processed_gradients,
                                    accumulate );
}


/////////////////
// bbpropUpdate //
/////////////////
void ProcessInputCostModule::bbpropUpdate(const Vec& input, const Vec& target,
                                          real cost, Vec& input_gradient,
                                          Vec& input_diag_hessian,
                                          bool accumulate)
{
    PLASSERT( processing_module );
    PLASSERT( cost_module );
    PLASSERT( input.size() == input_size );
    PLASSERT( target.size() == target_size );

    if( accumulate )
    {
        PLASSERT_MSG( input_gradient.size() == input_size,
                      "Cannot resize input_gradient AND accumulate into it" );
        PLASSERT_MSG( input_diag_hessian.size() == input_size,
                      "Cannot resize input_diag_hessian AND accumulate into it"
                    );
    }

    cost_module->bbpropUpdate( processed_value, target, cost,
                               processed_gradient, processed_diag_hessian );
    processing_module->bbpropUpdate( input, processed_value,
                                     input_gradient, processed_gradient,
                                     input_diag_hessian,
                                     processed_diag_hessian,
                                     accumulate );
}


////////////
// forget //
////////////
void ProcessInputCostModule::forget()
{
    PLASSERT( processing_module );
    PLASSERT( cost_module );

    processed_value.clear();
    processed_gradient.clear();
    processed_diag_hessian.clear();

    if( !random_gen )
    {
        PLWARNING("CombiningCostsModule: cannot forget() without random_gen");
        return;
    }

    // Ensures processing_module and cost_module can forget
    if( !(processing_module->random_gen) )
        processing_module->random_gen = random_gen;
    processing_module->forget();
    if( !(cost_module->random_gen) )
        cost_module->random_gen = random_gen;
    cost_module->forget();
}

//////////
// name //
//////////
TVec<string> ProcessInputCostModule::costNames()
{
    if (name == "" || name == classname())
        return cost_module->costNames();
    else
    {
        int n_costs = cost_module->costNames().length();
        TVec<string> cost_names(n_costs);
        for (int i=0; i<n_costs; i++)
            cost_names[i] = name + "." + cost_module->costNames()[i];

        return cost_names;
    }
}

//////////////
// finalize //
//////////////
void ProcessInputCostModule::finalize()
{
    processing_module->finalize();
    cost_module->finalize();
}

//////////////////////
// bpropDoesNothing //
//////////////////////
bool ProcessInputCostModule::bpropDoesNothing()
{
    return processing_module->bpropDoesNothing()
        && cost_module->bpropDoesNothing();
}

/////////////////////
// setLearningRate //
/////////////////////
void ProcessInputCostModule::setLearningRate(real dynamic_learning_rate)
{
    processing_module->setLearningRate( dynamic_learning_rate );
    cost_module->setLearningRate( dynamic_learning_rate );
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
