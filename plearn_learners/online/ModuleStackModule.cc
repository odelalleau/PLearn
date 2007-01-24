// -*- C++ -*-

// ModuleStackModule.cc
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

/*! \file ModuleStackModule.cc */



#include "ModuleStackModule.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    ModuleStackModule,
    "Wraps a stack of layered OnlineLearningModule into a single one",
    "The OnlineLearningModule's are disposed like superposed layers:\n"
    "outputs of module i are the inputs of module (i+1), the last layer is\n"
    "the output layer.\n"
    );

ModuleStackModule::ModuleStackModule() :
    n_modules(0)
{
}

void ModuleStackModule::declareOptions(OptionList& ol)
{
    // declareOption(ol, "", &ModuleStackModule::,
    //               OptionBase::buildoption,
    //               "");

    declareOption(ol, "modules", &ModuleStackModule::modules,
                  OptionBase::buildoption,
                  "The underlying modules");

    declareOption(ol, "n_modules", &ModuleStackModule::n_modules,
                  OptionBase::learntoption,
                  "The number of modules");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void ModuleStackModule::build_()
{
    // TODO: Do something with the random generator?

    n_modules = modules.length();

    if( n_modules > 0 )
    {
        values.resize( n_modules-1 );
        gradients.resize( n_modules-1 );
        diag_hessians.resize( n_modules-1 );

        input_size = modules[0]->input_size;
        output_size = modules[n_modules-1]->output_size;
    }
}

// ### Nothing to add here, simply calls build_
void ModuleStackModule::build()
{
    inherited::build();
    build_();
}


void ModuleStackModule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(modules, copies);
}

///////////
// fprop //
///////////
void ModuleStackModule::fprop(const Vec& input, Vec& output) const
{
    PLASSERT( n_modules > 0 );
    PLASSERT( input.size() == input_size );
    PLASSERT( output.size() == output_size );

    modules[0]->fprop( input, values[0] );
    for( int i=1 ; i<n_modules-1 ; i++ )
        modules[i]->fprop( values[i-1], values[i] );
    modules[n_modules-1]->fprop( values[n_modules-2], output );
}

/////////////////
// bpropUpdate //
/////////////////
void ModuleStackModule::bpropUpdate(const Vec& input, const Vec& output,
                                    Vec& input_gradient,
                                    const Vec& output_gradient)
{
    PLASSERT( n_modules > 0 );
    PLASSERT( input.size() == input_size );
    PLASSERT( output.size() == output_size );
    PLASSERT( output_gradient.size() == output_size );

    // bpropUpdate should be called just after the corresponding fprop,
    // so values should be up-to-date.
    modules[n_modules-1]->bpropUpdate( values[n_modules-2], output,
                                       gradients[n_modules-2], output_gradient );

    for( int i=n_modules-2 ; i>0 ; i-- )
        modules[i]->bpropUpdate( values[i-1], values[i],
                                 gradients[i-1], gradients[i] );

    modules[0]->bpropUpdate( input, values[0], input_gradient, gradients[0] );
}

void ModuleStackModule::bpropUpdate(const Vec& input, const Vec& output,
                                    const Vec& output_gradient)
{
    PLASSERT( n_modules > 0 );
    PLASSERT( input.size() == input_size );
    PLASSERT( output.size() == output_size );
    PLASSERT( output_gradient.size() == output_size );

    // bpropUpdate should be called just after the corresponding fprop,
    // so values should be up-to-date.
    modules[n_modules-1]->bpropUpdate( values[n_modules-2], output,
                                       gradients[n_modules-2],
                                       output_gradient );

    for( int i=n_modules-2 ; i>0 ; i-- )
        modules[i]->bpropUpdate( values[i-1], values[i],
                                 gradients[i-1], gradients[i] );

    modules[0]->bpropUpdate( input, values[0], gradients[0] );
}

//////////////////
// bbpropUpdate //
//////////////////
void ModuleStackModule::bbpropUpdate(const Vec& input, const Vec& output,
                                     Vec& input_gradient,
                                     const Vec& output_gradient,
                                     Vec& input_diag_hessian,
                                     const Vec& output_diag_hessian)
{
    PLASSERT( n_modules > 0 );
    PLASSERT( input.size() == input_size );
    PLASSERT( output.size() == output_size );
    PLASSERT( output_gradient.size() == output_size );
    PLASSERT( output_diag_hessian.size() == output_size );

    // bbpropUpdate should be called just after the corresponding fprop,
    // so values should be up-to-date.
    modules[n_modules-1]->bbpropUpdate( values[n_modules-2], output,
                                        gradients[n_modules-2], output_gradient,
                                        diag_hessians[n_modules-2],
                                        output_diag_hessian );

    for( int i=n_modules-2 ; i>0 ; i-- )
        modules[i]->bbpropUpdate( values[i-1], values[i],
                                  gradients[i-1], gradients[i],
                                  diag_hessians[i-1], diag_hessians[i] );

    modules[0]->bbpropUpdate( input, values[0], input_gradient, gradients[0],
                              input_diag_hessian, diag_hessians[0] );
}

void ModuleStackModule::bbpropUpdate(const Vec& input, const Vec& output,
                                     const Vec& output_gradient,
                                     const Vec& output_diag_hessian)
{
    PLASSERT( n_modules > 0 );
    PLASSERT( input.size() == input_size );
    PLASSERT( output.size() == output_size );
    PLASSERT( output_gradient.size() == output_size );
    PLASSERT( output_diag_hessian.size() == output_size );

    // bbpropUpdate should be called just after the corresponding fprop,
    // so values should be up-to-date.
    modules[n_modules-1]->bbpropUpdate( values[n_modules-2], output,
                                        gradients[n_modules-2], output_gradient,
                                        diag_hessians[n_modules-2],
                                        output_diag_hessian );

    for( int i=n_modules-2 ; i>0 ; i-- )
        modules[i]->bbpropUpdate( values[i-1], values[i],
                                  gradients[i-1], gradients[i],
                                  diag_hessians[i-1], diag_hessians[i] );

    modules[0]->bbpropUpdate( input, values[0],
                              gradients[0], diag_hessians[0] );
}

////////////
// forget //
////////////
void ModuleStackModule::forget()
{
    for( int i=0 ; i<n_modules ; i++ )
        modules[i]->forget();

    values.clear();
    gradients.clear();
    diag_hessians.clear();
}

//////////////
// finalize //
//////////////
void ModuleStackModule::finalize()
{
    for( int i=0 ; i<n_modules ; i++ )
        modules[i]->finalize();
}

//////////////////////
// bpropDoesNothing //
//////////////////////
bool ModuleStackModule::bpropDoesNothing()
{
    for( int i=0 ; i<n_modules ; i++ )
        if( !(modules[i]->bpropDoesNothing()) )
            return false;
    return true;
}

/////////////////////
// setLearningRate //
/////////////////////
void ModuleStackModule::setLearningRate(real dynamic_learning_rate)
{
    for( int i=0 ; i<n_modules ; i++ )
        modules[i]->setLearningRate( dynamic_learning_rate );
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
