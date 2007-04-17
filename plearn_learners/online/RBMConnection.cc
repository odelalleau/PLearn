// -*- C++ -*-

// RBMConnection.cc
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

/*! \file RBMConnection.cc */



#include "RBMConnection.h"
#include <plearn/math/TMat_maths.h>
#include <plearn/base/stringutils.h>
//#include "RBMLayer.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_ABSTRACT_OBJECT(
    RBMConnection,
    "Virtual class for the linear transformation between two layers of an RBM",
    "");

///////////////////
// RBMConnection //
///////////////////
RBMConnection::RBMConnection( real the_learning_rate ) :
    learning_rate(the_learning_rate),
    momentum(0.),
    down_size(-1),
    up_size(-1),
    going_up(true),
    pos_count(0),
    neg_count(0)
{
}

////////////////////
// declareOptions //
////////////////////
void RBMConnection::declareOptions(OptionList& ol)
{
    declareOption(ol, "down_size", &RBMConnection::down_size,
                  OptionBase::buildoption,
                  "Number of units in down layer.");

    declareOption(ol, "up_size", &RBMConnection::up_size,
                  OptionBase::buildoption,
                  "Number of units in up layer.");

    declareOption(ol, "learning_rate", &RBMConnection::learning_rate,
                  OptionBase::buildoption,
                  "The learning rate, used both in update() and bpropUpdate() "
                  "methods\n");

    declareOption(ol, "momentum", &RBMConnection::momentum,
                  OptionBase::buildoption,
                  "The momentum, used both in update() and bpropUpdate() "
                  "methods\n");

    declareOption(ol, "initialization_method",
                  &RBMConnection::initialization_method,
                  OptionBase::buildoption,
                  "The method used to initialize the weights:\n"
                  "  - \"uniform_linear\" = a uniform law in [-1/d, 1/d]\n"
                  "  - \"uniform_sqrt\"   = a uniform law in [-1/sqrt(d),"
                  " 1/sqrt(d)]\n"
                  "  - \"zero\"           = all weights are set to 0,\n"
                  "where d = max( up_layer_size, down_layer_size ).\n");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);

    redeclareOption(ol, "input_size", &RBMConnection::input_size,
                    OptionBase::learntoption,
                    "Equals to down_size");

    redeclareOption(ol, "output_size", &RBMConnection::output_size,
                    OptionBase::learntoption,
                    "Equals to up_size");
}

////////////
// build_ //
////////////
void RBMConnection::build_()
{
    string im = lowerstring( initialization_method );
    if( im == "" || im == "uniform_sqrt" )
        initialization_method = "uniform_sqrt";
    else if( im == "uniform_linear" )
        initialization_method = im;
    else if( im == "zero" )
        initialization_method = im;
    else
        PLERROR( "RBMConnection::build_ - initialization_method\n"
                 "\"%s\" unknown.\n", initialization_method.c_str() );

    // for the "OnlineLearningModule" interface
    if( down_size < 0 )
        down_size = input_size;
    else
        input_size = down_size;

    if( up_size < 0 )
        up_size = output_size;
    else
        output_size = up_size;
}

///////////
// build //
///////////
void RBMConnection::build()
{
    inherited::build();
    build_();
}


/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void RBMConnection::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(input_vec, copies);
}

/////////////////////
// setLearningRate //
/////////////////////
void RBMConnection::setLearningRate( real the_learning_rate )
{
    learning_rate = the_learning_rate;
}

/////////////////
// setMomentum //
/////////////////
void RBMConnection::setMomentum( real the_momentum )
{
    momentum = the_momentum;
}

//////////////////
// setAsUpInput //
//////////////////
void RBMConnection::setAsUpInput( const Vec& input ) const
{
    PLASSERT( input.size() == up_size );
    input_vec = input;
    going_up = false;
}

////////////////////
// setAsDownInput //
////////////////////
void RBMConnection::setAsDownInput( const Vec& input ) const
{
    PLASSERT( input.size() == down_size );
    input_vec = input;
    going_up = true;
}

////////////
// update //
////////////
void RBMConnection::update( const Vec& pos_down_values,
                            const Vec& pos_up_values,
                            const Vec& neg_down_values,
                            const Vec& neg_up_values)
{
    // Not-so-efficient implementation
    accumulatePosStats( pos_down_values, pos_up_values );
    accumulateNegStats( neg_down_values, neg_up_values );
    update();
}

///////////
// fprop //
///////////
void RBMConnection::fprop(const Vec& input, Vec& output) const
{
    // propagates the activations.
    setAsDownInput( input );
    output.resize( output_size );
    computeProduct( 0, output_size, output );
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
