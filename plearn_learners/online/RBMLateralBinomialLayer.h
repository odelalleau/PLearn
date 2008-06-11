// -*- C++ -*-

// RBMLateralBinomialLayer.h
//
// Copyright (C) 2006 Hugo Larochelle
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

// Authors: Hugo Larochelle

/*! \file RBMLateralBinomialLayer.h */


#ifndef RBMLateralBinomialLayer_INC
#define RBMLateralBinomialLayer_INC

#include "RBMLayer.h"

namespace PLearn {
using namespace std;

/**
 * Layer in an RBM formed with binomial units, with lateral connections
 *
 */
class RBMLateralBinomialLayer: public RBMLayer
{
    typedef RBMLayer inherited;

public:
    //#####  Public Build Options  ############################################

    //! Number of passes through the lateral connections
    int n_lateral_connections_passes;

    //! Dampening factor
    //! ( expectation_t = (1-df) * currrent mean field + df * expectation_{t-1})
    real dampening_factor;

    //! Mean-field precision threshold that, once reached, stops the mean-field
    //! expectation approximation computation. Used only in computeExpectation().
    //! Precision is computed as:
    //!   dist(last_mean_field, current_mean_field) / size
    real mean_field_precision_threshold;

    //! Length of the topographic map
    int topographic_length;

    //! Width of the topographic map
    int topographic_width;

    //! Vertical radius of the topographic local weight patches
    int topographic_patch_vradius;

    //! Horizontal radius of the topographic local weight patches
    int topographic_patch_hradius;

    //! Initial value for the topographic_lateral_weights
    real topographic_lateral_weights_init_value;

    //! Indication that the topographic_lateral_weights should
    //! be fixed at their initial value.
    bool do_not_learn_topographic_lateral_weights;

    //! Lateral connections
    Mat lateral_weights;

    //! Local topographic lateral connections
    TVec< Vec > topographic_lateral_weights;

    //! Accumulates positive contribution to the gradient of lateral weights
    Mat lateral_weights_pos_stats;

    //! Accumulates negative contribution to the gradient of lateral weights
    Mat lateral_weights_neg_stats;

    //! Indication that a parametric predictor of the mean-field
    //! approximation of the hidden layer conditional distribution.
    bool use_parametric_mean_field;

    //! Output weights of the mean field predictor
    Mat mean_field_output_weights;

    //! Output bias of the mean field predictor
    Vec mean_field_output_bias;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    RBMLateralBinomialLayer( real the_learning_rate=0. );

    //! resets activations, sample and expectation fields
    virtual void reset();

    //! resets the statistics and counts
    virtual void clearStats();

    //! forgets everything
    virtual void forget();

    //! generate a sample, and update the sample field
    virtual void generateSample() ;

    //! Inherited.
    virtual void generateSamples();

    //! Compute expectation.
    virtual void computeExpectation() ;

    //! Compute mini-batch expectations.
    virtual void computeExpectations();

    //! forward propagation
    virtual void fprop( const Vec& input, Vec& output ) const;

    //! Batch forward propagation
    virtual void fprop( const Mat& inputs, Mat& outputs );

    //! forward propagation with provided bias
    virtual void fprop( const Vec& input, const Vec& rbm_bias,
                        Vec& output ) const;

    //! back-propagates the output gradient to the input
    virtual void bpropUpdate(const Vec& input, const Vec& output,
                             Vec& input_gradient, const Vec& output_gradient,
                             bool accumulate=false);

    //! back-propagates the output gradient to the input and the bias
    virtual void bpropUpdate(const Vec& input, const Vec& rbm_bias,
                             const Vec& output,
                             Vec& input_gradient, Vec& rbm_bias_gradient,
                             const Vec& output_gradient) ;

    //! Back-propagate the output gradient to the input, and update parameters.
    virtual void bpropUpdate(const Mat& inputs, const Mat& outputs,
                             Mat& input_gradients,
                             const Mat& output_gradients,
                             bool accumulate = false);

    //! Computes the negative log-likelihood of target given the
    //! internal activations of the layer
    virtual real fpropNLL(const Vec& target);
    virtual void fpropNLL(const Mat& targets, const Mat& costs_column);

    //! Computes the gradient of the negative log-likelihood of target
    //! with respect to the layer's bias, given the internal activations.
    //! Will also update the lateral weight connections according
    //! to their gradient. Assumes computeExpectation(s) or
    //! fpropNLL was called before.
    virtual void bpropNLL(const Vec& target, real nll, Vec& bias_gradient);
    virtual void bpropNLL(const Mat& targets, const Mat& costs_column,
                          Mat& bias_gradients);

    //! Accumulates positive phase statistics
    virtual void accumulatePosStats( const Vec& pos_values );
    virtual void accumulatePosStats( const Mat& ps_values);

    //! Accumulates negative phase statistics
    virtual void accumulateNegStats( const Vec& neg_values );
    virtual void accumulateNegStats( const Mat& neg_values );

    //! Update bias and lateral connections parameters
    //! according to accumulated statistics
    virtual void update();

    //! Updates ONLY the bias parameters according to the given gradient
    virtual void update( const Vec& grad );

    //! Update bias and lateral connections
    //! parameters according to one pair of vectors
    virtual void update( const Vec& pos_values, const Vec& neg_values );

    //! Update bias and lateral connections
    //! parameters according to one pair of matrices.
    virtual void update( const Mat& pos_values, const Mat& neg_values );

    // neg_stats <-- gibbs_chain_statistics_forgetting_factor * neg_stats
    //              +(1-gibbs_chain_statistics_forgetting_factor)
    //               * gibbs_neg_values
    // delta w = -lrate * ( pos_values
    //                  - ( background_gibbs_update_ratio*neg_stats
    //                     +(1-background_gibbs_update_ratio)
    //                      * cd_neg_values ) )
    virtual void updateCDandGibbs( const Mat& pos_values,
                                   const Mat& cd_neg_values,
                                   const Mat& gibbs_neg_values,
                                   real background_gibbs_update_ratio );

    // neg_stats <-- gibbs_chain_statistics_forgetting_factor * neg_stats
    //              +(1-gibbs_chain_statistics_forgetting_factor)
    //               * \sum_i gibbs_neg_values_i / minibatch_size
    // delta bias = -lrate * \sum_i (pos_values_i - neg_stats) / minibatch_size
    virtual void updateGibbs( const Mat& pos_values,
                              const Mat& gibbs_neg_values );

    //! compute -bias' unit_values
    virtual real energy(const Vec& unit_values) const;

    //! This function is not implemented for this class (returns an error)
    virtual real freeEnergyContribution(const Vec& unit_activations) const;

    virtual int getConfigurationCount();

    virtual void getConfiguration(int conf_index, Vec& output);

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(RBMLateralBinomialLayer);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //#####  Not Options  #####################################################

    mutable Vec dampening_expectation;
    mutable Mat dampening_expectations;

    mutable Vec mean_field_input;
    mutable Vec pre_sigmoid_mean_field_output;

    mutable TVec<Vec> temp_output;
    mutable TVec<Mat> temp_outputs;

    mutable Vec current_temp_output, previous_temp_output;
    mutable Mat current_temp_outputs, previous_temp_outputs;

    mutable Vec bias_plus_input;
    mutable Mat bias_plus_inputs;

    Vec temp_input_gradient;
    Vec temp_mean_field_gradient;
    Vec temp_mean_field_gradient2;

    Mat lateral_weights_gradient;
    Mat lateral_weights_inc;

    TVec< Vec > topographic_lateral_weights_gradient;

protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    static void declareOptions(OptionList& ol);

    //! Computes mat[i][j] += 0.5 * (v1[i] * v2[j] +  v1[j] * v2[i])
    void externalSymetricProductAcc(const Mat& mat, const Vec& v1,
                                    const Vec& v2);

    void productTopoLateralWeights( const Vec& result, const Vec& input ) const;

    void productTopoLateralWeightsGradients( const Vec& input, const Vec& input_gradient,
                                             const Vec& result_gradient,
                                             const TVec< Vec >& weights_gradient );

    void updateTopoLateralWeightsCD( const Vec& pos_values, const Vec& neg_values );

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    void build_();

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(RBMLateralBinomialLayer);

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
