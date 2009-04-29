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

/*! \file PLearn/plearn_learners/online/RBMLayer.cc */



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
    bias_decay_type("none"),
    bias_decay_parameter(0),
    gibbs_ma_increment(0.1),
    gibbs_initial_ma_coefficient(0.1),
    batch_size(0),
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
    bias_inc.clear();
    expectation_is_up_to_date = false;
    expectations_are_up_to_date = false;
}

void RBMLayer::clearStats()
{
    bias_pos_stats.clear();
    bias_neg_stats.clear();
    pos_count = 0;
    neg_count = 0;
    gibbs_ma_coefficient = gibbs_initial_ma_coefficient;
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

    declareOption(ol, "bias_decay_type", &RBMLayer::bias_decay_type,
                  OptionBase::buildoption,
                  "Bias decay type:\n"
                  " - none: no decay applied\n"
                  " - negative: pushes the biases towards -\\infty\n"
                  " - l2: applies an l2 penalty");

    declareOption(ol, "bias_decay_parameter", &RBMLayer::bias_decay_parameter,
                  OptionBase::buildoption,
                  "Bias decay parameter.");

    declareOption(ol, "gibbs_ma_schedule", &RBMLayer::gibbs_ma_schedule,
                  OptionBase::buildoption,
                  "Each element of this vector is a number of updates after which\n"
                  "the moving average coefficient is incremented (by incrementing\n"
                  "its inverse sigmoid by gibbs_ma_increment). After the last\n"
                  "increase has been made, the moving average coefficient stays constant.\n");

    declareOption(ol, "gibbs_ma_increment", &RBMLayer::gibbs_ma_increment,
                  OptionBase::buildoption,
                  "The increment in the inverse sigmoid of the moving average coefficient\n"
                  "to apply after the number of updates reaches an element of the gibbs_ma_schedule.\n");

    declareOption(ol, "gibbs_initial_ma_coefficient", &RBMLayer::gibbs_initial_ma_coefficient,
                  OptionBase::buildoption,
                  "Initial moving average coefficient for the negative phase statistics in the Gibbs chain.\n");

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

void RBMLayer::declareMethods(RemoteMethodMap& rmm)
{
    // Make sure that inherited methods are declared
    rmm.inherited(inherited::_getRemoteMethodMap_());

    declareMethod(rmm, "setAllBias", &RBMLayer::setAllBias,
                  (BodyDoc("Set the biases values"),
                   ArgDoc ("bias", "the vector of biases")));

    declareMethod(rmm, "generateSample", &RBMLayer::generateSample,
                  (BodyDoc("Generate a sample, and update the sample field")));
    declareMethod(rmm, "getAllActivations", &RBMLayer::getAllActivations,
                  (BodyDoc("Uses 'rbmc' to obtain the activations of all units in this layer. \n"
                           "Unit 0 of this layer corresponds to unit 'offset' of 'rbmc'."),
                   ArgDoc("PP<RBMConnection> rbmc", "RBM Connection"),
                   ArgDoc("int offset", "Offset"),
                   ArgDoc("bool minibatch", "Use minibatch")));
    declareMethod(rmm, "computeExpectation", &RBMLayer::computeExpectation,
                  (BodyDoc("Compute expectation.")));
}

////////////
// build_ //
////////////
void RBMLayer::build_()
{
    if( size <= 0 )
        return;

    input_size = size;
    output_size = size;

    activation.resize( size );
    activations.resize( 0, size );
    sample.resize( size );
    samples.resize( 0, size );
    expectation.resize( size );
    expectations.resize( 0, size );
    expectation_is_up_to_date = false;
    expectations_are_up_to_date = false;

    bias.resize( size );
    bias_pos_stats.resize( size );
    bias_neg_stats.resize( size );
}

///////////
// build //
///////////
void RBMLayer::build()
{
    inherited::build();
    build_();
}


/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void RBMLayer::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(gibbs_ma_schedule,    copies);
    deepCopyField(bias,                 copies);
    deepCopyField(activation,           copies);
    deepCopyField(activations,          copies);
    deepCopyField(sample,               copies);
    deepCopyField(samples,              copies);
    deepCopyField(expectation,          copies);
    deepCopyField(bias_pos_stats,       copies);
    deepCopyField(bias_neg_stats,       copies);
    deepCopyField(bias_inc,             copies);
    deepCopyField(ones,                 copies);
    deepCopyField(expectations,         copies);
    deepCopyField(tmp,                  copies);
}


/////////////////////
// setLearningRate //
/////////////////////
void RBMLayer::setLearningRate( real the_learning_rate )
{
    learning_rate = the_learning_rate;
}

/////////////////
// setMomentum //
/////////////////
void RBMLayer::setMomentum( real the_momentum )
{
    momentum = the_momentum;
}

//////////////////
// setBatchSize //
//////////////////
void RBMLayer::setBatchSize( int the_batch_size )
{
    batch_size = the_batch_size;
    PLASSERT( activations.width() == size );
    activations.resize( batch_size, size );
    PLASSERT( expectations.width() == size );
    expectations.resize( batch_size, size );
    PLASSERT( samples.width() == size );
    samples.resize( batch_size, size );
}


///////////////////////
// getUnitActivation //
///////////////////////
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
        setBatchSize(activations.length());
    } else {
        rbmc->computeProduct( offset, size, activation );
        activation += bias;
    }
    expectation_is_up_to_date = false;
    expectations_are_up_to_date = false;
}

void RBMLayer::expectation_is_not_up_to_date()
{
    expectation_is_up_to_date = false;
}


/////////////////////
// getExpectations //
/////////////////////
const Mat& RBMLayer::getExpectations() {
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

    This->computeExpectation();

    output << This->expectation;
}

void RBMLayer::fprop(const Mat& inputs, Mat& outputs)
{
    // Note: inefficient.
    PLASSERT( inputs.width() == input_size );
    int mbatch_size = inputs.length();
    outputs.resize(mbatch_size, output_size);

    setBatchSize(mbatch_size);
    activations << inputs;
    for (int k = 0; k < mbatch_size; k++)
        activations(k) += bias;

    expectations_are_up_to_date = false;
    computeExpectations();
    outputs << expectations;
}

void RBMLayer::fprop( const Vec& input, const Vec& rbm_bias,
                      Vec& output ) const
{
    PLERROR("In RBMLayer::fprop(): not implemented in subclass %s",
            this->classname().c_str());
}

void RBMLayer::bpropUpdate(const Vec& input, const Vec& rbm_bias,
                           const Vec& output,
                           Vec& input_gradient, Vec& rbm_bias_gradient,
                           const Vec& output_gradient)
{
    PLERROR("In RBMLayer::bpropUpdate(): not implemented in subclass %s",
            this->classname().c_str());
}

real RBMLayer::fpropNLL(const Vec& target)
{
    PLERROR("In RBMLayer::fpropNLL(): not implemented in subclass %s",
            this->classname().c_str());
    return REAL_MAX;
}

real RBMLayer::fpropNLL(const Vec& target, const Vec& cost_weights)
{
    PLERROR("weighted version of RBMLayer::fpropNLL not implemented in subclass %s",
            this->classname().c_str());
    return REAL_MAX;
}


void RBMLayer::fpropNLL(const Mat& targets, const Mat& costs_column)
{
    PLWARNING("batch version of RBMLayer::fpropNLL may not be optimized in subclass %s",
              this->classname().c_str());
    PLASSERT( targets.width() == input_size );
    PLASSERT( targets.length() == batch_size );
    PLASSERT( costs_column.width() == 1 );
    PLASSERT( costs_column.length() == batch_size );

    Mat tmp;
    tmp.resize(1,input_size);
    Vec target;
    target.resize(input_size);

    computeExpectations();
    expectation_is_up_to_date = false;
    for (int k=0;k<batch_size;k++) // loop over minibatch
    {
        selectRows(expectations, TVec<int>(1, k), tmp );
        expectation << tmp;
        selectRows( activations, TVec<int>(1, k), tmp );
        activation << tmp;
        selectRows( targets, TVec<int>(1, k), tmp );
        target << tmp;
        costs_column(k,0) = fpropNLL( target );
    }
}

void RBMLayer::bpropNLL(const Vec& target, real nll, Vec& bias_gradient)
{
    PLERROR("In RBMLayer::bpropNLL(): not implemented in subclass %s",
            this->classname().c_str());
}

void RBMLayer::bpropNLL(const Mat& targets,  const Mat& costs_column,
                        Mat& bias_gradients)
{
    PLERROR("In RBMLayer::bpropNLL(): not implemented in subclass %s",
            this->classname().c_str());
}

////////////////////////
// accumulatePosStats //
////////////////////////
void RBMLayer::accumulatePosStats( const Vec& pos_values )
{
    bias_pos_stats += pos_values;
    pos_count++;
}
void RBMLayer::accumulatePosStats( const Mat& pos_values )
{
    for (int i=0;i<pos_values.length();i++)
        bias_pos_stats += pos_values(i);
    pos_count+=pos_values.length();
}

////////////////////////
// accumulateNegStats //
////////////////////////
void RBMLayer::accumulateNegStats( const Vec& neg_values )
{
    bias_neg_stats += neg_values;
    neg_count++;
}
void RBMLayer::accumulateNegStats( const Mat& neg_values )
{
    for (int i=0;i<neg_values.length();i++)
        bias_neg_stats += neg_values(i);
    neg_count+=neg_values.length();
}

////////////
// update //
////////////
void RBMLayer::update()
{
    // bias += learning_rate * (bias_pos_stats/pos_count
    //                          - bias_neg_stats/neg_count)
    real pos_factor = learning_rate / pos_count;
    real neg_factor = -learning_rate / neg_count;

    real* b = bias.data();
    real* bps = bias_pos_stats.data();
    real* bns = bias_neg_stats.data();

    if( fast_is_equal( momentum, 0.) )
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
        //              + learning_rate * (bias_pos_stats/pos_count
        //                                  - bias_neg_stats/neg_count)
        // bias += bias_inc
        real* binc = bias_inc.data();
        for( int i=0 ; i<size ; i++ )
        {
            binc[i] = momentum*binc[i] + pos_factor*bps[i] + neg_factor*bns[i];
            b[i] += binc[i];
        }
    }

    applyBiasDecay();

    clearStats();
}

void RBMLayer::update( const Vec& grad )
{
    real* b = bias.data();
    real* gb = grad.data();
    real* binc = momentum==0?0:bias_inc.data();

    for( int i=0 ; i<size ; i++ )
    {
        if( fast_is_equal( momentum, 0.) )
        {
            // update the bias: bias -= learning_rate * input_gradient
            b[i] -= learning_rate * gb[i];
        }
        else
        {
            // The update rule becomes:
            // bias_inc = momentum * bias_inc - learning_rate * input_gradient
            // bias += bias_inc
            binc[i] = momentum * binc[i] - learning_rate * gb[i];
            b[i] += binc[i];
        }
    }

    applyBiasDecay();
}

void RBMLayer::update( const Mat& grad )
{
    int batch_size = grad.length();
    real* b = bias.data();
    real* binc = momentum==0?0:bias_inc.data();
    real avg_lr = learning_rate / (real)batch_size;

    for( int isample=0; isample<batch_size; isample++)
        for( int i=0 ; i<size ; i++ )
        {
            if( fast_is_equal( momentum, 0.) )
            {
                // update the bias: bias -= learning_rate * input_gradient
                b[i] -= avg_lr * grad(isample,i);
            }
            else
            {
                // The update rule becomes:
                // bias_inc = momentum * bias_inc - learning_rate * input_gradient
                // bias += bias_inc
                binc[i] = momentum * binc[i] - avg_lr * grad(isample,i);
                b[i] += binc[i];
            }
        }
}


void RBMLayer::update( const Vec& pos_values, const Vec& neg_values)
{
    // bias += learning_rate * (pos_values - neg_values)
    real* b = bias.data();
    real* pv = pos_values.data();
    real* nv = neg_values.data();

    if( fast_is_equal( momentum, 0.) )
    {
        for( int i=0 ; i<size ; i++ )
            b[i] += learning_rate * ( pv[i] - nv[i] );
    }
    else
    {
        bias_inc.resize( size );
        real* binc = bias_inc.data();
        for( int i=0 ; i<size ; i++ )
        {
            binc[i] = momentum*binc[i] + learning_rate*( pv[i] - nv[i] );
            b[i] += binc[i];
        }
    }

    applyBiasDecay();

}

void RBMLayer::update( const Mat& pos_values, const Mat& neg_values)
{
    // bias += learning_rate * (pos_values - neg_values)

    int n = pos_values.length();
    PLASSERT( neg_values.length() == n );
    if (ones.length() < n) {
        ones.resize(n);
        ones.fill(1);
    } else if (ones.length() > n)
        // No need to fill with ones since we are only shrinking the vector.
        ones.resize(n);


    // We take the average gradient over the mini-batch.
    real avg_lr = learning_rate / n;

    if( fast_is_equal( momentum, 0.) )
    {
        transposeProductScaleAcc(bias, pos_values, ones,  avg_lr, real(1));
        transposeProductScaleAcc(bias, neg_values, ones, -avg_lr, real(1));
    }
    else
    {
        PLERROR("RBMLayer::update - Not implemented yet with momentum");
        /*
        bias_inc.resize( size );
        real* binc = bias_inc.data();
        for( int i=0 ; i<size ; i++ )
        {
            binc[i] = momentum*binc[i] + learning_rate*( pv[i] - nv[i] );
            b[i] += binc[i];
        }
        */
    }

    applyBiasDecay();

}

//////////////////////
// updateCDandGibbs //
//////////////////////
void RBMLayer::updateCDandGibbs( const Mat& pos_values,
                                 const Mat& cd_neg_values,
                                 const Mat& gibbs_neg_values,
                                 real background_gibbs_update_ratio )
{
    PLASSERT(pos_values.width()==size);
    PLASSERT(cd_neg_values.width()==size);
    PLASSERT(gibbs_neg_values.width()==size);
    int minibatch_size=gibbs_neg_values.length();
    PLASSERT(pos_values.length()==minibatch_size);
    PLASSERT(cd_neg_values.length()==minibatch_size);
    real normalize_factor=1.0/minibatch_size;

    // neg_stats <-- gibbs_chain_statistics_forgetting_factor * neg_stats
    //              +(1-gibbs_chain_statistics_forgetting_factor)
    //               * sumoverrows(gibbs_neg_values)
    tmp.resize(size);
    columnSum(gibbs_neg_values,tmp);
    if (neg_count==0)
        multiply(tmp,normalize_factor,bias_neg_stats);
    else
        multiplyScaledAdd(tmp,gibbs_ma_coefficient,
                          normalize_factor*(1-gibbs_ma_coefficient),
                          bias_neg_stats);
    neg_count++;

    // delta w = lrate * ( sumoverrows(pos_values)
    //                   - ( background_gibbs_update_ratio*neg_stats
    //                      +(1-background_gibbs_update_ratio)
    //                       * sumoverrows(cd_neg_values) ) )
    columnSum(pos_values,tmp);
    multiplyAcc(bias, tmp, learning_rate*normalize_factor);
    multiplyAcc(bias, bias_neg_stats,
                -learning_rate*background_gibbs_update_ratio);
    columnSum(cd_neg_values, tmp);
    multiplyAcc(bias, tmp,
                -learning_rate*(1-background_gibbs_update_ratio)*normalize_factor);

    applyBiasDecay();

}

/////////////////
// updateGibbs //
/////////////////
void RBMLayer::updateGibbs( const Mat& pos_values,
                            const Mat& gibbs_neg_values)
{
    int minibatch_size = pos_values.length();
    PLASSERT(pos_values.width()==size);
    PLASSERT(gibbs_neg_values.width()==size);
    PLASSERT(minibatch_size==gibbs_neg_values.length());
    // neg_stats <-- gibbs_chain_statistics_forgetting_factor * neg_stats
    //              +(1-gibbs_chain_statistics_forgetting_factor)
    //               * meanoverrows(gibbs_neg_values)
    tmp.resize(size);
    real normalize_factor=1.0/minibatch_size;
    columnSum(gibbs_neg_values,tmp);
    if (neg_count==0)
        multiply(tmp, normalize_factor, bias_neg_stats);
    else // bias_neg_stats <-- tmp*(1-gibbs_chain_statistics_forgetting_factor)/minibatch_size
        //                    +gibbs_chain_statistics_forgetting_factor*bias_neg_stats
        multiplyScaledAdd(tmp,gibbs_ma_coefficient,
                          normalize_factor*(1-gibbs_ma_coefficient),
                          bias_neg_stats);
    neg_count++;

    bool increase_ma=false;
    for (int i=0;i<gibbs_ma_schedule.length();i++)
        if (gibbs_ma_schedule[i]==neg_count*minibatch_size)
        {
            increase_ma=true;
            break;
        }
    if (increase_ma)
        gibbs_ma_coefficient = sigmoid(gibbs_ma_increment + inverse_sigmoid(gibbs_ma_coefficient));


    // delta w = lrate * ( meanoverrows(pos_values) - neg_stats )
    columnSum(pos_values,tmp);
    multiplyAcc(bias, tmp, learning_rate*normalize_factor);
    multiplyAcc(bias, bias_neg_stats, -learning_rate);

    applyBiasDecay();

}

////////////////
// setAllBias //
////////////////
void RBMLayer::setAllBias(const Vec& rbm_bias)
{
    PLASSERT( rbm_bias.size() == size );
    bias << rbm_bias;
}

////////////////////
// setExpectation //
////////////////////
void RBMLayer::setExpectation(const Vec& the_expectation)
{
    expectation << the_expectation;
    expectation_is_up_to_date=true;
}

/////////////////////////
// setExpectationByRef //
/////////////////////////
void RBMLayer::setExpectationByRef(const Vec& the_expectation)
{
    expectation = the_expectation;
    expectation_is_up_to_date=true;
}

/////////////////////
// setExpectations //
/////////////////////
void RBMLayer::setExpectations(const Mat& the_expectations)
{
    batch_size = the_expectations.length();
    setBatchSize( batch_size );
    expectations << the_expectations;
    expectations_are_up_to_date=true;
}

//////////////////////////
// setExpectationsByRef //
//////////////////////////
void RBMLayer::setExpectationsByRef(const Mat& the_expectations)
{
    batch_size = the_expectations.length();
    setBatchSize( batch_size );
    expectations = the_expectations;
    expectations_are_up_to_date=true;
}

/////////////
// bpropCD //
/////////////
void RBMLayer::bpropCD(Vec& bias_gradient)
{
    // grad = -bias_pos_stats/pos_count + bias_neg_stats/neg_count

    real* bg = bias_gradient.data();
    real* bps = bias_pos_stats.data();
    real* bns = bias_neg_stats.data();

    for( int i=0 ; i<size ; i++ )
        bg[i] = -bps[i]/pos_count + bns[i]/neg_count;

    addBiasDecay(bias_gradient);

}

void RBMLayer::bpropCD(const Vec& pos_values, const Vec& neg_values,
                       Vec& bias_gradient)
{
    // grad = -bias_pos_stats/pos_count + bias_neg_stats/neg_count

    real* bg = bias_gradient.data();
    real* bps = pos_values.data();
    real* bns = neg_values.data();

    for( int i=0 ; i<size ; i++ )
        bg[i] = -bps[i] + bns[i];

    addBiasDecay(bias_gradient);

}

real RBMLayer::energy(const Vec& unit_values) const
{
    PLERROR("RBMLayer::energy(Vec) not implemented in subclass %s\n",classname().c_str());
    return 0;
}

real RBMLayer::freeEnergyContribution(const Vec& unit_activations) const
{
    PLERROR("RBMLayer::freeEnergyContribution(Vec) not implemented in subclass %s\n",classname().c_str());
    return 0;
}

void RBMLayer::freeEnergyContributionGradient(const Vec& unit_activations,
                                              Vec& unit_activations_gradient,
                                              real output_gradient,
                                              bool accumulate ) const
{
    PLERROR("RBMLayer::freeEnergyContributionGradient(Vec, Vec) not implemented in subclass %s\n",classname().c_str());
}

int RBMLayer::getConfigurationCount()
{
    PLERROR("RBMLayer::getConfigurationCount() not implemented in subclass %s\n",classname().c_str());
    return 0;
}

void RBMLayer::getConfiguration(int conf_index, Vec& output)
{
    PLERROR("RBMLayer::getConfiguration(int, Vec) not implemented in subclass %s\n",classname().c_str());
}

void RBMLayer::addBiasDecay(Vec& bias_gradient)
{
    PLASSERT(bias_gradient.size()==size);

    real *bg = bias_gradient.data();
    real *b = bias.data();
    bias_decay_type = lowerstring(bias_decay_type);

    if (bias_decay_type=="none")
        {}
    else if (bias_decay_type=="negative")  // Pushes the biases towards -\infty
        for( int i=0 ; i<size ; i++ )
            bg[i] += learning_rate * bias_decay_parameter;
    else if (bias_decay_type=="l2")  // L2 penalty on the biases
        for (int i=0 ; i<size ; i++ )
            bg[i] += learning_rate * bias_decay_parameter * b[i];
    else
        PLERROR("RBMLayer::addBiasDecay(string) bias_decay_type %s is not in"
                " the list, in subclass %s\n",bias_decay_type.c_str(),classname().c_str());

}

void RBMLayer::addBiasDecay(Mat& bias_gradients)
{
    PLASSERT(bias_gradients.width()==size);
    if (bias_decay_type=="none")
        return;

    real avg_lr = learning_rate / bias_gradients.length();

    for(int b=0; b<bias_gradients.length(); b++)
    {
        real *bg = bias_gradients[b];
        real *b = bias.data();
        bias_decay_type = lowerstring(bias_decay_type);

        if (bias_decay_type=="negative")  // Pushes the biases towards -\infty
            for( int i=0 ; i<size ; i++ )
                bg[i] += avg_lr * bias_decay_parameter;
        else if (bias_decay_type=="l2")  // L2 penalty on the biases
            for (int i=0 ; i<size ; i++ )
                bg[i] += avg_lr * bias_decay_parameter * b[i];
        else
            PLERROR("RBMLayer::addBiasDecay(string) bias_decay_type %s is not in"
                    " the list, in subclass %s\n",bias_decay_type.c_str(),classname().c_str());
    }
}

void RBMLayer::applyBiasDecay()
{

    PLASSERT(bias.size()==size);

    real* b = bias.data();
    bias_decay_type = lowerstring(bias_decay_type);

    if (bias_decay_type=="none")
        {}
    else if (bias_decay_type=="negative") // Pushes the biases towards -\infty
        for( int i=0 ; i<size ; i++ )
            b[i] -= learning_rate * bias_decay_parameter;
    else if (bias_decay_type=="l2") // L2 penalty on the biases
        bias *= (1 - learning_rate * bias_decay_parameter);
    else
        PLERROR("RBMLayer::applyBiasDecay(string) bias_decay_type %s is not in"
                " the list, in subclass %s\n",bias_decay_type.c_str(),classname().c_str());

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
