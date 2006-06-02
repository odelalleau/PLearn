// -*- C++ -*-

// RBMParameters.cc
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

/*! \file RBMParameters.cc */



#include "RBMParameters.h"
#include <plearn/math/TMat_maths.h>
#include <plearn/base/stringutils.h>
//#include "RBMLayer.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_ABSTRACT_OBJECT(
    RBMParameters,
    "Virtual class for the parameters between two layers of an RBM",
    "");

RBMParameters::RBMParameters( real the_learning_rate ) :
    learning_rate(the_learning_rate),
    going_up(true),
    pos_count(0),
    neg_count(0)
{
}

RBMParameters::RBMParameters( string down_types, string up_types,
                              the_learning_rate ) :
    up_units_types(up_types),
    down_units_types(down_types),
    learning_rate(the_learning_rate),
    going_up(true),
    pos_count(0),
    neg_count(0)
{
    // We're not sure inherited::build() has been called
    build();
}

void RBMParameters::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    declareOption(ol, "up_units_types", &RBMParameters::up_units_types,
                  OptionBase::buildoption,
                  "Each character of this string describes the type of an"
                  " up unit:\n"
                  "  - 'l' if the energy function of this unit is linear\n"
                  "    (binomial or multinomial unit),\n"
                  "  - 'q' if it is quadratic (for a gaussian unit).\n");

    declareOption(ol, "down_units_types", &RBMParameters::down_units_types,
                  OptionBase::buildoption,
                  "Same meaning as 'up_units_types', but with down units");

    declareOption(ol, "learning_rate", &RBMParameters::learning_rate,
                  OptionBase::buildoption,
                  "The learning rate, used both in update() and bpropUpdate() "
                  "methods\n");

    declareOption(ol, "initialization_method",
                  &RBMParameters::initialization_method,
                  OptionBase::buildoption,
                  "The method used to initialize the weights:\n"
                  "  - \"uniform_linear\" = a uniform law in [-1/d, 1/d]\n"
                  "  - \"uniform_sqrt\"   = a uniform law in [-1/sqrt(d),"
                  " 1/sqrt(d)]\n"
                  "  - \"zero\"           = all weights are set to 0,\n"
                  "where d = max( up_layer_size, down_layer_size ).\n");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void RBMParameters::build_()
{
    up_layer_size = up_units_types.size();
    down_layer_size = down_units_types.size();
    if( up_layer_size == 0 || down_layer_size == 0 )
        return;

    string im = lowerstring( initialization_method );
    if( im == "" || im == "uniform_sqrt" )
        initialization_method = "uniform_sqrt";
    else if( im == "uniform_linear" )
        initialization_method = im;
    else if( im == "zero" )
        initialization_method = im;
    else
        PLERROR( "RBMParameters::build_ - initialization_method\n"
                 "\"%s\" unknown.\n", initialization_method.c_str() );

    input_size = down_layer_size;
    output_size = up_layer_size;
}

void RBMParameters::build()
{
    inherited::build();
    build_();
}


void RBMParameters::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.

    deepCopyField(input_vec, copies);
}

void RBMParameters::setAsUpInput( const Vec& input ) const
{
    assert( input.size() == up_layer_size );
    input_vec = input;
    going_up = false;
}

void RBMParameters::setAsDownInput( const Vec& input ) const
{
    assert( input.size() == down_layer_size );
    input_vec = input;
    going_up = true;
}

//! given the input, compute the output (possibly resize it  appropriately)
void RBMParameters::fprop(const Vec& input, Vec& output) const
{
    // propagates the activations.
    setAsDownInput( input );
    output.resize( output_size );
    computeUnitActivations( 0, output_size, output );
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
