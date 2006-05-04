// -*- C++ -*-

// NnlmWordRepresentationLayer.cc
//
// Copyright (C) 2006 Pierre-Antoine Manzagol
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

// Authors: Pierre-Antoine Manzagol

/*! \file NnlmWordRepresentationLayer.cc */



#include "NnlmWordRepresentationLayer.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    NnlmWordRepresentationLayer,
    "Implements the word representation layer for the online NNLM.",
    "MULTI-LINE \nHELP");

NnlmWordRepresentationLayer::NnlmWordRepresentationLayer() :
    OnlineLearningModule(),
    vocabulary_size( -1 ),
    word_representation_size( -1 ),
    context_size( -1 ),
    start_learning_rate( 0.001 ),
    decrease_constant( 0 ),
    step_number( 0 ),
    learning_rate( 0.0 )    
{
    // ### You may (or not) want to call build_() to finish building the object
    // ### (doing so assumes the parent classes' build_() have been called too
    // ### in the parent classes' constructors, something that you must ensure)
}

void NnlmWordRepresentationLayer::declareOptions(OptionList& ol)
{

    declareOption(ol, "vocabulary_size",
                  &NnlmWordRepresentationLayer::vocabulary_size,
                  OptionBase::buildoption,
                  "size of vocabulary used - defines the virtual input size");

    declareOption(ol, "word_representation_size",
                  &NnlmWordRepresentationLayer::word_representation_size,
                  OptionBase::buildoption,
                  "size of the real distributed word representation");

    declareOption(ol, "context_size",
                  &NnlmWordRepresentationLayer::context_size,
                  OptionBase::buildoption,
                  "size of word context");

    declareOption(ol, "start_learning_rate",
                  &NnlmWordRepresentationLayer::start_learning_rate,
                  OptionBase::buildoption,
                  "Learning-rate of stochastic gradient optimization");

    declareOption(ol, "decrease_constant",
                  &NnlmWordRepresentationLayer::decrease_constant,
                  OptionBase::buildoption,
                  "Decrease constant of stochastic gradient optimization");

    // * Learnt

    declareOption(ol, "step_number", &NnlmWordRepresentationLayer::step_number,
                  OptionBase::learntoption,
                  "The step number, incremented after each update.");

    declareOption(ol, "weights", &NnlmWordRepresentationLayer::weights,
                  OptionBase::learntoption,
                  "Input weights of the neurons (one row per neuron, no bias).");


    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);

}

void NnlmWordRepresentationLayer::build_()
{

    // *** Some variables are connected... 
    // for now we overwrite these
    input_size = context_size;
    output_size = context_size * word_representation_size;


    // *** A few sanity checks
    if( input_size <= 0 )
    {
        PLERROR("NnlmWordRepresentationLayer::build_: 'input_size' <= 0 (%i).\n"
                "You should set it to a positive integer.\n", input_size);
    }
    else if( word_representation_size * context_size != output_size )
    {
        PLERROR("NnlmWordRepresentationLayer::build_: 'output_size' inconsistent with\n"
                  " 'word_representation_size * input_size': %i != ( %i * %i)\n"
                  , output_size, word_representation_size, input_size);
    }
    else if( vocabulary_size <= 0 )
    {
        PLERROR("NnlmWordRepresentationLayer::build_: 'vocabulary_size' <= 0(%i).\n"
                  , vocabulary_size);
    }


    // *** Initialize weights if not loaded
    if( weights.size() == 0 )   {

        resetWeights();

        // TODO add an option for the seed
        if( !random_gen )   {
            random_generator = new PRandom( 1 );
        }

        real r = 1.0 / sqrt(input_size);
        random_gen->fill_random_uniform(weights,-r,r);

    }

}

// ### Nothing to add here, simply calls build_
void NnlmWordRepresentationLayer::build()
{
    inherited::build();
    build_();
}


void NnlmWordRepresentationLayer::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(weights, copies);

    // ### Remove this line when you have fully implemented this method.
    //PLERROR("NnlmWordRepresentationLayer::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

//! given the input, compute the output (possibly resize it  appropriately)
void NnlmWordRepresentationLayer::fprop(const Vec& input, Vec& output) const
{
}

/* THIS METHOD IS OPTIONAL
//! Adapt based on the output gradient: this method should only
//! be called just after a corresponding fprop; it should be
//! called with the same arguments as fprop for the first two arguments
//! (and output should not have been modified since then).
//! Since sub-classes are supposed to learn ONLINE, the object
//! is 'ready-to-be-used' just after any bpropUpdate.
//! N.B. A DEFAULT IMPLEMENTATION IS PROVIDED IN THE SUPER-CLASS, WHICH
//! JUST CALLS
//!     bpropUpdate(input, output, input_gradient, output_gradient)
//! AND IGNORES INPUT GRADIENT.
void NnlmWordRepresentationLayer::bpropUpdate(const Vec& input, const Vec& output,
                               const Vec& output_gradient)
{
}
*/

/* THIS METHOD IS OPTIONAL
//! this version allows to obtain the input gradient as well
//! N.B. THE DEFAULT IMPLEMENTATION IN SUPER-CLASS JUST RAISES A PLERROR.
void NnlmWordRepresentationLayer::bpropUpdate(const Vec& input, const Vec& output,
                               Vec& input_gradient,
                               const Vec& output_gradient)
{
}
*/

//! reset the parameters to the state they would be BEFORE starting training.
//! Note that this method is necessarily called from build().
void NnlmWordRepresentationLayer::forget()
{
}

/* THIS METHOD IS OPTIONAL
//! reset the parameters to the state they would be BEFORE starting training.
//! Note that this method is necessarily called from build().
//! THE DEFAULT IMPLEMENTATION PROVIDED IN THE SUPER-CLASS DOES NOT DO
//! ANYTHING.
void NnlmWordRepresentationLayer::finalize()
{
}
*/

/* THIS METHOD IS OPTIONAL
//! in case bpropUpdate does not do anything, make it known
//! THE DEFAULT IMPLEMENTATION PROVIDED IN THE SUPER-CLASS RETURNS false;
bool NnlmWordRepresentationLayer::bpropDoesNothing()
{
}
*/

/* THIS METHOD IS OPTIONAL
//! Similar to bpropUpdate, but adapt based also on the estimation
//! of the diagonal of the Hessian matrix, and propagates this
//! back. If these methods are defined, you can use them INSTEAD of
//! bpropUpdate(...)
//! N.B. A DEFAULT IMPLEMENTATION IS PROVIDED IN THE SUPER-CLASS, WHICH
//! JUST CALLS
//!     bbpropUpdate(input, output, input_gradient, output_gradient,
//!                  in_hess, out_hess)
//! AND IGNORES INPUT HESSIAN AND INPUT GRADIENT.
void NnlmWordRepresentationLayer::bbpropUpdate(const Vec& input, const Vec& output,
                                const Vec& output_gradient,
                                const Vec& output_diag_hessian)
{
}
*/

/* THIS METHOD IS OPTIONAL
//! Similar to bpropUpdate, but adapt based also on the estimation
//! of the diagonal of the Hessian matrix, and propagates this
//! back. If these methods are defined, you can use them INSTEAD of
//! bpropUpdate(...)
//! N.B. A DEFAULT IMPLEMENTATION IS PROVIDED IN THE SUPER-CLASS, WHICH
//! RAISES A PLERROR.
void NnlmWordRepresentationLayer::bbpropUpdate(const Vec& input, const Vec& output,
                                Vec& input_gradient,
                                const Vec& output_gradient,
                                Vec& input_diag_hessian,
                                const Vec& output_diag_hessian)
{
}
*/


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
