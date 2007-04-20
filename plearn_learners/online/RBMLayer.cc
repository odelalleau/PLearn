// -*- C++ -*-

// RBMLayer.cc
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



#include "RBMLayer.h"
#include <plearn/math/TMat_maths.h>
#include <plearn/math/PRandom.h>
#include "RBMConnection.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_ABSTRACT_OBJECT(
    RBMLayer,
    "Virtual class for a layer of an RBM",
    "");

RBMLayer::RBMLayer( real the_learning_rate ) :
    learning_rate(the_learning_rate),
    momentum(0.),
    size(-1),
    expectation_is_up_to_date(false),
    expectations_are_up_to_date(false),
    pos_count(0),
    neg_count(0)
{
}

void RBMLayer::reset()
{
    activation.clear();
    sample.clear();
    expectation.clear();
    expectation_is_up_to_date = false;
    expectations_are_up_to_date = false;
}

void RBMLayer::clearStats()
{
    bias_pos_stats.clear();
    bias_neg_stats.clear();
    pos_count = 0;
    neg_count = 0;
}

void RBMLayer::forget()
{
    bias.clear();
    reset();
    clearStats();
}

void RBMLayer::declareOptions(OptionList& ol)
{
    declareOption(ol, "units_types", &RBMLayer::units_types,
                  OptionBase::nosave,
                  "Obsolete option.");

    declareOption(ol, "size", &RBMLayer::size,
                  OptionBase::buildoption,
                  "Number of units.");

    declareOption(ol, "learning_rate", &RBMLayer::learning_rate,
                  OptionBase::buildoption,
                  "Learning rate.");

    declareOption(ol, "momentum", &RBMLayer::momentum,
                  OptionBase::buildoption,
                  "Momentum.");

    declareOption(ol, "bias", &RBMLayer::bias,
                  OptionBase::learntoption,
                  "Biases of the units.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);

    redeclareOption(ol, "input_size", &RBMLayer::input_size,
                    OptionBase::learntoption,
                    "input_size = size");

    redeclareOption(ol, "output_size", &RBMLayer::output_size,
                    OptionBase::learntoption,
                    "output_size = size");
}

void RBMLayer::build_()
{
    if( size <= 0 )
        return;

    input_size = size;
    output_size = size;

    activation.resize( size );
    sample.resize( size );
    expectation.resize( size );
    expectation_is_up_to_date = false;
    expectations_are_up_to_date = false;

    bias.resize( size );
    bias_pos_stats.resize( size );
    bias_neg_stats.resize( size );
}

void RBMLayer::build()
{
    inherited::build();
    build_();
}


void RBMLayer::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(bias,           copies);
    deepCopyField(activation,     copies);
    deepCopyField(activations,    copies);
    deepCopyField(sample,         copies);
    deepCopyField(samples,        copies);
    deepCopyField(expectation,    copies);
    deepCopyField(expectations,   copies);
    deepCopyField(bias_pos_stats, copies);
    deepCopyField(bias_neg_stats, copies);
    deepCopyField(bias_inc,       copies);
}


void RBMLayer::setLearningRate( real the_learning_rate )
{
    learning_rate = the_learning_rate;
}

void RBMLayer::setMomentum( real the_momentum )
{
    momentum = the_momentum;
}


void RBMLayer::getUnitActivation( int i, PP<RBMConnection> rbmc, int offset )
{
    Vec act = activation.subVec(i,1);
    rbmc->computeProduct( i+offset, 1, act );
    act[0] += bias[i];
    expectation_is_up_to_date = false;
    expectations_are_up_to_date = false;
}

///////////////////////
// getAllActivations //
///////////////////////
void RBMLayer::getAllActivations( PP<RBMConnection> rbmc, int offset,
                                  bool minibatch)
{
    if (minibatch) {
        rbmc->computeProducts( offset, size, activations );
        activations += bias;
    } else {
        rbmc->computeProduct( offset, size, activation );
        activation += bias;
    }
    expectation_is_up_to_date = false;
    expectations_are_up_to_date = false;
}


/////////////////////
// getExpectations //
/////////////////////
Mat& RBMLayer::getExpectations() {
    return this->expectations;
}

///////////
// fprop //
///////////
void RBMLayer::fprop( const Vec& input, Vec& output ) const
{
    // Note: inefficient.

    // Yes it's ugly, blame the const plague
    RBMLayer* This = const_cast<RBMLayer*>(this);

    PLASSERT( input.size() == This->input_size );
    output.resize( This->output_size );

    This->activation << input;
    This->activation += bias;
    This->expectation_is_up_to_date = false;
    This->expectations_are_up_to_date = false;

    output << This->expectation;
}

void RBMLayer::fprop( const Vec& input, const Vec& rbm_bias,
                      Vec& output ) const
{
    PLERROR("In RBMLayer::fprop(): not implemented");
}

void RBMLayer::bpropUpdate(const Vec& input, const Vec& rbm_bias,
                           const Vec& output,
                           Vec& input_gradient, Vec& rbm_bias_gradient,
                           const Vec& output_gradient)
{
    PLERROR("In RBMLayer::bpropUpdate(): not implemented");
}

real RBMLayer::fpropNLL(const Vec& target)
{
    PLERROR("In RBMLayer::fpropNLL(): not implemented");
    return REAL_MAX;
}

void RBMLayer::bpropNLL(const Vec& target, real nll, Vec& bias_gradient)
{
    PLERROR("In RBMLayer::bpropNLL(): not implemented");
}

////////////////////////
// accumulatePosStats //
////////////////////////
void RBMLayer::accumulatePosStats( const Vec& pos_values )
{
    bias_pos_stats += pos_values;
    pos_count++;
}

////////////////////////
// accumulateNegStats //
////////////////////////
void RBMLayer::accumulateNegStats( const Vec& neg_values )
{
    bias_neg_stats += neg_values;
    neg_count++;
}

////////////
// update //
////////////
void RBMLayer::update()
{
    // bias -= learning_rate * (bias_pos_stats/pos_count
    //                          - bias_neg_stats/neg_count)
    real pos_factor = -learning_rate / pos_count;
    real neg_factor = learning_rate / neg_count;

    real* b = bias.data();
    real* bps = bias_pos_stats.data();
    real* bns = bias_neg_stats.data();

    if( momentum == 0. )
    {
        // no need to use bias_inc
        for( int i=0 ; i<size ; i++ )
            b[i] += pos_factor * bps[i] + neg_factor * bns[i];
    }
    else
    {
        // ensure that bias_inc has the right size
        bias_inc.resize( size );

        // The update rule becomes:
        // bias_inc = momentum * bias_inc
        //              - learning_rate * (bias_pos_stats/pos_count
        //                                  - bias_neg_stats/neg_count)
        // bias += bias_inc
        real* binc = bias_inc.data();
        for( int i=0 ; i<size ; i++ )
        {
            binc[i] = momentum*binc[i] + pos_factor*bps[i] + neg_factor*bns[i];
            b[i] += binc[i];
        }
    }

    clearStats();
}

////////////
// update //
////////////
void RBMLayer::update( const Vec& pos_values, const Vec& neg_values)
{
    // bias -= learning_rate * (pos_values - neg_values)
    real* b = bias.data();
    real* pv = pos_values.data();
    real* nv = neg_values.data();

    if( momentum == 0. )
    {
        for( int i=0 ; i<size ; i++ )
            b[i] += learning_rate * ( nv[i] - pv[i] );
    }
    else
    {
        bias_inc.resize( size );
        real* binc = bias_inc.data();
        for( int i=0 ; i<size ; i++ )
        {
            binc[i] = momentum*binc[i] + learning_rate*( nv[i] - pv[i] );
            b[i] += binc[i];
        }
    }
}

////////////////
// setAllBias //
////////////////
void RBMLayer::setAllBias(const Vec& rbm_bias)
{
    PLASSERT( rbm_bias.size() == size );
    bias << rbm_bias;
}

/////////////////////
// setExpectations //
/////////////////////
void RBMLayer::setExpectations(const Mat& the_expectations)
{
    expectations.resize(the_expectations.length(), the_expectations.width());
    expectations << the_expectations;
}

/////////////
// bpropCD //
/////////////
void RBMLayer::bpropCD(Vec& bias_gradient)
{
    // grad = bias_pos_stats/pos_count - bias_neg_stats/neg_count

    real* bg = bias_gradient.data();
    real* bps = bias_pos_stats.data();
    real* bns = bias_neg_stats.data();

    for( int i=0 ; i<size ; i++ )
        bg[i] = bps[i]/pos_count - bns[i]/neg_count;
}

void RBMLayer::bpropCD(const Vec& pos_values, const Vec& neg_values,
                       Vec& bias_gradient)
{
    // grad = bias_pos_stats/pos_count - bias_neg_stats/neg_count

    real* bg = bias_gradient.data();
    real* bps = pos_values.data();
    real* bns = neg_values.data();

    for( int i=0 ; i<size ; i++ )
        bg[i] = bps[i] - bns[i];
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
