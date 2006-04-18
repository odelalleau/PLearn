// -*- C++ -*-

// RBMMixedLayer.cc
//
// Copyright (C) 2006 Pascal Lamblin & Dan Popovici
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

// Authors: Pascal Lamblin & Dan Popovici

/*! \file RBMPLayer.cc */



#include "RBMMixedLayer.h"
#include <plearn/math/TMat_maths.h>
#include "RBMParameters.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    RBMMixedLayer,
    "Layer in an RBM, concatenating other sub-layers",
    "");

RBMMixedLayer::RBMMixedLayer()
{
}

RBMMixedLayer::RBMMixedLayer( TVec< PP<RBMLayer> > the_sub_layers ) :
    sub_layers( the_sub_layers )
{
    build();
}

//! Uses "rbmp" to obtain the activations of unit "i" of this layer.
//! This activation vector is computed by the "i+offset"-th unit of "rbmp"
void RBMMixedLayer::getUnitActivations( int i, PP<RBMParameters> rbmp,
                                        int offset )
{
    int j = layer_of_unit[i];
    PP<RBMLayer> layer = sub_layers[i];
    int sub_index = i - init_positions[j];
    int total_offset = offset + init_positions[j];

    layer->getUnitActivations( sub_index, rbmp, total_offset );
    expectation_is_up_to_date = false;
}

//! Uses "rbmp" to obtain the activations of all units in this layer.
//! Unit 0 of this layer corresponds to unit "offset" of "rbmp".
void RBMMixedLayer::getAllActivations( PP<RBMParameters> rbmp, int offset )
{
    for( int i=0 ; i<n_layers ; i++ )
    {
        int total_offset = offset + init_positions[i];
        sub_layers[i]->getAllActivations( rbmp, total_offset );
    }
    expectation_is_up_to_date = false;
}

void RBMMixedLayer::generateSample()
{
    for( int i=0 ; i<n_layers ; i++ )
        sub_layers[i]->generateSample();
}

void RBMMixedLayer::computeExpectation()
{
    if( expectation_is_up_to_date )
        return;

    for( int i=0 ; i<n_layers ; i++ )
        sub_layers[i]->computeExpectation();

    expectation_is_up_to_date = true;
}


void RBMMixedLayer::declareOptions(OptionList& ol)
{
    declareOption(ol, "sub_layers", &RBMMixedLayer::sub_layers,
                  OptionBase::buildoption,
                  "The concatenated RBMLayers composing this layer.");

    declareOption(ol, "init_positions", &RBMMixedLayer::init_positions,
                  OptionBase::learntoption,
                  " Initial index of the sub_layers.");

    declareOption(ol, "layer_of_unit", &RBMMixedLayer::layer_of_unit,
                  OptionBase::learntoption,
                  "layer_of_unit[i] is the index of sub_layer containing unit"
                  " i.");

    declareOption(ol, "n_layers", &RBMMixedLayer::n_layers,
                  OptionBase::learntoption,
                  "Number of sub-layers.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void RBMMixedLayer::build_()
{
    units_types = "";
    size = 0;
    activations.resize( 0 );
    sample.resize( 0 );
    expectation.resize( 0 );
    expectation_is_up_to_date = false;
    layer_of_unit.resize( 0 );

    n_layers = sub_layers.size();
    init_positions.resize( n_layers );

    for( int i=0 ; i<n_layers ; i++ )
    {
        init_positions[i] = size;

        PP<RBMLayer> cur_layer = sub_layers[i];
        units_types += cur_layer->units_types;
        size += cur_layer->size;

        activations = merge( activations, cur_layer->activations );
        sample = merge( sample, cur_layer->sample );
        expectation = merge( expectation, cur_layer->expectation );
        layer_of_unit.append( TVec<int>( cur_layer->size, i ) );
    }
}

void RBMMixedLayer::build()
{
    inherited::build();
    build_();
}


void RBMMixedLayer::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(sub_layers, copies);
    deepCopyField(init_positions, copies);
    deepCopyField(layer_of_unit, copies);
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
