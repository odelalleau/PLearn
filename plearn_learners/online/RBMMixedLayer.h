// -*- C++ -*-

// RBMMixedLayer.h
//
// Copyright (C) 2006 Dan Popovici & Pascal Lamblin
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

// Authors: Dan Popovici & Pascal Lamblin

/*! \file RBMMixedLayer.h */


#ifndef RBMMixedLayer_INC
#define RBMMixedLayer_INC

#include "RBMLayer.h"

namespace PLearn {
using namespace std;

/**
 * Layer in an RBM formed with the concatenation of other layers
 *
 * @todo: yes
 */
class RBMMixedLayer: public RBMLayer
{
    typedef RBMLayer inherited;

public:
    //#####  Public Build Options  ############################################

    //! The concatenated RBMLayers composing this layer.
    TVec< PP<RBMLayer> > sub_layers;


public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    RBMMixedLayer();

    //! Constructor from the sub_layers
    RBMMixedLayer( TVec< PP<RBMLayer> > the_sub_layers );

    //! Sets the learning rate, also in the sub_layers
    virtual void setLearningRate( real the_learning_rate );

    //! Sets the momentum, also in the sub_layers
    virtual void setMomentum( real the_momentum );

    //! Sets batch_size and resize activations, expectations, and samples
    virtual void setBatchSize( int the_batch_size );

    //! Copy the given expectation in the 'expectation' vector.
    virtual void setExpectation(const Vec& the_expectation);

    //! Make the 'expectation' vector point to the given data vector (so no
    //! copy is performed).
    virtual void setExpectationByRef(const Vec& the_expectation);

    //! Copy the given expectations in the 'expectations' matrix.
    virtual void setExpectations(const Mat& the_expectations);

    //! Make the 'expectations' matrix point to the given data matrix (so no
    //! copy is performed).
    virtual void setExpectationsByRef(const Mat& the_expectations);

    // Your other public member functions go here
    //! Uses "rbmc" to compute the activation of unit "i" of this layer.
    //! This activation is computed by the "i+offset"-th unit of "rbmc"
    virtual void getUnitActivation( int i, PP<RBMConnection> rbmc,
                                    int offset=0 );

    //! Uses "rbmc" to obtain the activations of all units in this layer.
    //! Unit 0 of this layer corresponds to unit "offset" of "rbmc".
    virtual void getAllActivations( PP<RBMConnection> rbmc, int offset=0,
                                    bool minibatch = false );

    //! compute a sample, and update the sample field
    virtual void generateSample();

    //! generate activations.length() samples
    virtual void generateSamples();

    //! compute the expectation
    virtual void computeExpectation();

    //! compute the expectations according to activations
    virtual void computeExpectations();

    //! forward propagation
    virtual void fprop( const Vec& input, Vec& output ) const;

    //! Batch forward propagation
    virtual void fprop( const Mat& inputs, Mat& outputs ) const;

    //! forward propagation with provided bias
    virtual void fprop( const Vec& input, const Vec& rbm_bias,
                        Vec& output ) const;

    //! back-propagates the output gradient to the input
    virtual void bpropUpdate(const Vec& input, const Vec& output,
                             Vec& input_gradient, const Vec& output_gradient,
                             bool accumulate=false);

    //! Back-propagate the output gradient to the input, and update parameters.
    virtual void bpropUpdate(const Mat& inputs, const Mat& outputs,
                             Mat& input_gradients,
                             const Mat& output_gradients,
                             bool accumulate = false);

    //! back-propagates the output gradient to the input and the bias
    virtual void bpropUpdate(const Vec& input, const Vec& rbm_bias,
                             const Vec& output,
                             Vec& input_gradient, Vec& rbm_bias_gradient,
                             const Vec& output_gradient) ;

    //! Computes the negative log-likelihood of target given the
    //! internal activations of the layer
    virtual real fpropNLL(const Vec& target);

    //! Batch fpropNLL
    virtual void fpropNLL(const Mat& targets, const Mat& costs_column);

    //! Computes the gradient of the negative log-likelihood of target
    //! with respect to the layer's bias, given the internal activations
    virtual void bpropNLL(const Vec& target, real nll, Vec& bias_gradient);
    virtual void bpropNLL(const Mat& targets, const Mat& costs_column,
                          Mat& bias_gradients);

    //! Accumulates positive phase statistics
    virtual void accumulatePosStats( const Vec& pos_values );

    //! Accumulates negative phase statistics
    virtual void accumulateNegStats( const Vec& neg_values );

    //! Update parameters according to accumulated statistics
    virtual void update();

    //! Update parameters according to one pair of vectors
    virtual void update( const Vec& pos_values, const Vec& neg_values );

    //! Update parameters according to several pairs of vectors
    virtual void update( const Mat& pos_values, const Mat& neg_values );

    //! resets activations, sample and expectation fields
    virtual void reset();

    //! resets the statistics and counts
    virtual void clearStats();

    //! forgets everything
    virtual void forget();

    //! Compute -bias' unit_values
    virtual real energy(const Vec& unit_values) const;

    virtual int getConfigurationCount();

    virtual void getConfiguration(int conf_index, Vec& output);

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(RBMMixedLayer);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:

    //! Initial index of the sub_layers.
    TVec<int> init_positions;

    //! layer_of_unit[i] is the index of sub_layer containing unit i
    TVec<int> layer_of_unit;

    //! Number of sub-layers.
    int n_layers;

    //#####  Not Options  #####################################################

protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    static void declareOptions(OptionList& ol);

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    void build_();

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
    mutable Vec nlls;
    mutable Mat mat_nlls;
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(RBMMixedLayer);

} // end of namespace PLearn

#endif


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
