// -*- C++ -*-

// StackedModulesModule.cc
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

/*! \file StackedModulesModule.cc */



#include "StackedModulesModule.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    StackedModulesModule,
    "Wraps a stack of OnlineLearningModule, which are layers",
    "The OnlineLearningModule's are disposed like superposed layers:\n"
    "outputs of module i are the inputs of module (i+1), the last layer is\n"
    "the output layer.\n");

StackedModulesModule::StackedModulesModule() :
    last_layer_is_cost( false ),
    target_size( 0 ),
    nmodules( 0 )
{
}

void StackedModulesModule::declareOptions(OptionList& ol)
{
    /*
    declareOption(ol, "", &StackedModulesModule::,
                  OptionBase::buildoption,
                  "");
     */
    declareOption(ol, "modules", &StackedModulesModule::modules,
                  OptionBase::buildoption,
                  "Underlying layers of the Module");

    declareOption(ol, "last_layer_is_cost",
                  &StackedModulesModule::last_layer_is_cost,
                  OptionBase::buildoption,
                  "Indicates if the last layer is a cost layer (taking input"
                  " and target\n"
                  "as input, and outputing the cost we will minimize),"
                  " allowing this\n"
                  "module to behave the same way.\n");

    declareOption(ol, "target_size", &StackedModulesModule::target_size,
                  OptionBase::buildoption,
                  "If last_layer_is_cost, the size of the target");

    declareOption(ol, "nmodules", &StackedModulesModule::nmodules,
                  OptionBase::learntoption,
                  "Number of module layers");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void StackedModulesModule::build_()
{
    // initialize random generator from seed
    if( !random_gen )
        random_gen = new PRandom();
    else
        random_gen->manual_seed( random_gen->seed_ );

    // get some options
    nmodules = modules.length();
    if( nmodules == 0 )
        return;

    if( last_layer_is_cost && target_size <= 0 )
        PLERROR("StackedModulesModule::build_() - Please provide a target_size"
                "  > 0\n"
                "(is '%d').\n", target_size );
    if( !last_layer_is_cost )
        target_size = 0;

    assert( modules[0]->input_size >= 0 );
    input_size = modules[0]->input_size + target_size;

    int& last_module_output_size = modules[nmodules-1]->output_size;
    if( last_layer_is_cost )
        last_module_output_size = 1;

    output_size = last_module_output_size;

    // build the modules
    buildLayers();

}

void StackedModulesModule::buildLayers()
{
    // first values will be "input" values
    int size = input_size - target_size;
    values.resize( nmodules+1 );
    values[0].resize( size );
    gradients.resize( nmodules+1 );
    gradients[0].resize( size );
    // TODO: use it only if we actually use bbprop?
    diag_hessians.resize( nmodules+1 );
    diag_hessians[0].resize( size );

    for( int i=0 ; i<nmodules ; i++ )
    {
        modules[i]->estimate_simpler_diag_hessian =
            estimate_simpler_diag_hessian;
        modules[i]->random_gen = random_gen;
        modules[i]->build();

        size = modules[i]->output_size;
        values[i+1].resize( size );
        gradients[i+1].resize( size );
        diag_hessians[i+1].resize( size );
    }

    // stores the input of the last module, and the target if there is one
    cost_layer_input = values[nmodules-1];
    if( last_layer_is_cost )
        cost_layer_input.resize( cost_layer_input.size() + target_size );
}

void StackedModulesModule::build()
{
    inherited::build();
    build_();
}


void StackedModulesModule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(modules, copies);
    deepCopyField(values, copies);
    deepCopyField(cost_layer_input, copies);
    deepCopyField(gradients, copies);
    deepCopyField(diag_hessians, copies);
}

//! given the input, compute the output (possibly resize it  appropriately)
void StackedModulesModule::fprop(const Vec& input, Vec& output) const
{
    assert( input.size() == input_size );
    assert( modules[0]->input_size + target_size == input_size );
    int last_input_size = values[nmodules-1].size();

    values[0] << input.subVec(0, input_size - target_size );

    for( int i=0 ; i<nmodules-1 ; i++ )
        modules[i]->fprop( values[i], values[i+1] );

    if( last_layer_is_cost )
    {
        cost_layer_input.subVec( last_input_size, target_size )
            << input.subVec( input_size - target_size, target_size );
    }

    modules[nmodules-1]->fprop( cost_layer_input, values[nmodules] );
    output.resize( output_size );
    output << values[ nmodules ];
}

//! this version allows to obtain the input gradient as well
//! N.B. THE DEFAULT IMPLEMENTATION IN SUPER-CLASS JUST RAISES A PLERROR.
void StackedModulesModule::bpropUpdate(const Vec& input, const Vec& output,
                                       Vec& input_gradient,
                                       const Vec& output_gradient)
{
    // If last_layer_is_cost, the gradient wrt it is 1
    if( last_layer_is_cost )
        gradients[nmodules][0] = 1;
    else
        gradients[nmodules] << output_gradient;

    // values should have the values given by fprop(), so
    // values[nmodules] should already be equal to output
    modules[nmodules-1]->bpropUpdate( cost_layer_input, values[nmodules],
                                      gradients[nmodules-1],
                                      gradients[nmodules] );

    for( int i=nmodules-2 ; i>=0 ; i-- )
        modules[i]->bpropUpdate( values[i], values[i+1],
                                 gradients[i], gradients[i+1] );

    input_gradient = gradients[0].copy();
}

//! reset the parameters to the state they would be BEFORE starting training.
//! Note that this method is necessarily called from build().
void StackedModulesModule::forget()
{
    random_gen->manual_seed( random_gen->seed_ );

    // reset inputs
    values[0].clear();
    gradients[0].clear();
    diag_hessians[0].clear();

    // reset modules and outputs
    for( int i=0 ; i<nmodules ; i++ )
    {
        modules[i]->forget();
        values[i+1].clear();
        gradients[i+1].clear();
        diag_hessians[i+1].clear();
    }
}

/* THIS METHOD IS OPTIONAL
//! reset the parameters to the state they would be BEFORE starting training.
//! Note that this method is necessarily called from build().
//! THE DEFAULT IMPLEMENTATION PROVIDED IN THE SUPER-CLASS DOES NOT DO
//! ANYTHING.
void StackedModulesModule::finalize()
{
}
*/

//! Similar to bpropUpdate, but adapt based also on the estimation
//! of the diagonal of the Hessian matrix, and propagates this
//! back. If these methods are defined, you can use them INSTEAD of
//! bpropUpdate(...)
//! N.B. A DEFAULT IMPLEMENTATION IS PROVIDED IN THE SUPER-CLASS, WHICH
//! RAISES A PLERROR.
void StackedModulesModule::bbpropUpdate(const Vec& input, const Vec& output,
                                        Vec& input_gradient,
                                        const Vec& output_gradient,
                                        Vec& input_diag_hessian,
                                        const Vec& output_diag_hessian)
{
    // If last_layer_is_cost, the gradient wrt it is 1 and hessian is 0
    if( last_layer_is_cost )
    {
        gradients[nmodules][0] = 1;
        diag_hessians[nmodules][0] = 1;
    }
    else
    {
        gradients[nmodules] << output_gradient;
        diag_hessians[nmodules] << output_diag_hessian;
    }

    // values should have the values given by fprop(), so
    // values[nmodules] should already be equal to output
    for( int i=nmodules-1 ; i>=0 ; i-- )
        modules[i]->bbpropUpdate( values[i], values[i+1],
                                  gradients[i], gradients[i+1],
                                  diag_hessians[i], diag_hessians[i+1] );

    input_gradient = gradients[0].copy();
    input_diag_hessian = diag_hessians[0].copy();
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
