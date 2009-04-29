// -*- C++ -*-

// RBMLayer.h
//
// Copyright (C) 2006 Dan Popovici
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

// Authors: Dan Popovici

/*! \file PLearn/plearn_learners/online/RBMLayer.h */


#ifndef RBMLayer_INC
#define RBMLayer_INC

#include <plearn/base/Object.h>
#include "OnlineLearningModule.h"

namespace PLearn {
using namespace std;

// forward declarations
class RBMConnection;

/**
 * Virtual class for a layer in an RBM.
 *
 */
class RBMLayer: public OnlineLearningModule
{
    typedef OnlineLearningModule inherited;

public:
    //#####  Public Build Options  ############################################

    //! Learning rate
    real learning_rate;

    //! Momentum
    real momentum;

    //! Number of units
    int size;

    //! Obsolete option, still here for script compatibility
    string units_types;

    //! Type of decay applied to the biases
    string bias_decay_type;

    //! Bias decay parameter
    real bias_decay_parameter;

    //! background gibbs chain options
    //! each element of this vector is a number of updates after which
    //! the moving average coefficient is incremented (by incrementing
    //! its inverse sigmoid by gibbs_ma_increment). After the last
    //! increase has been made, the moving average coefficient stays constant.
    Vec gibbs_ma_schedule;
    real gibbs_ma_increment;
    real gibbs_initial_ma_coefficient;

    //#####  Learnt Options  ##################################################

    // stores the bias of the unit
    Vec bias;

    //! used for Gibbs chain methods only
    real gibbs_ma_coefficient;

    //#####  Not Options  #####################################################

    //! Size of batches when using mini-batch
    int batch_size;

    //! activation value: \f$ \sum Wx + b \f$
    Vec activation;
    Mat activations; // for mini-batch operations

    //! Contains a sample of the random variable in this layer
    Vec sample;
    Mat samples;

    //! Contains the expected value of the random variable in this layer
    Vec expectation;

    //! flags that expectation was computed based on most recently computed
    //! value of activation
    bool expectation_is_up_to_date;

    //! Indicate whether expectations were computed based on most recently
    //! computed values of activations.
    bool expectations_are_up_to_date;

    static const int INFINITE_CONFIGURATIONS = 0x7fffffff;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    RBMLayer( real the_learning_rate=0. );

    // Your other public member functions go here

    //! Sets the learning rate
    virtual void setLearningRate( real the_learning_rate );

    //! Sets the momentum
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

    //! Accessor to the 'expectations' matrix.
    const Mat& getExpectations();

    //! Uses "rbmc" to compute the activation of unit "i" of this layer.
    //! This activation is computed by the "i+offset"-th unit of "rbmc"
    virtual void getUnitActivation( int i, PP<RBMConnection> rbmc,
                                    int offset=0 );

    //! Uses "rbmc" to obtain the activations of all units in this layer.
    //! Unit 0 of this layer corresponds to unit "offset" of "rbmc".
    virtual void getAllActivations( PP<RBMConnection> rbmc, int offset = 0,
                                    bool minibatch = false);

    //change the flag of expectation_is_up_to_date to false
    virtual void expectation_is_not_up_to_date();

    //! generate a sample, and update the sample field
    virtual void generateSample() = 0 ;

    //! Generate a mini-batch set of samples.
    virtual void generateSamples() = 0;

    //! Compute expectation.
    virtual void computeExpectation() = 0 ;

    //! Compute expectations (mini-batch).
    virtual void computeExpectations() = 0 ;

    //! Adds the bias to input, consider this as the activation, then compute
    //! the expectation
    virtual void fprop(const Vec& input, Vec& output) const;
    virtual void fprop(const Mat& inputs, Mat& outputs);

    //! computes the expectation given the conditional input
    //! and the given bias
    virtual void fprop( const Vec& input, const Vec& rbm_bias,
                        Vec& output ) const;

    //! back-propagates the output gradient to the input,
    //! and update the bias (and possibly the quadratic term)
    virtual void bpropUpdate(const Vec& input, const Vec& output,
                             Vec& input_gradient,
                             const Vec& output_gradient,
                             bool accumulate=false) = 0 ;

    //! Back-propagate the output gradient to the input, and update parameters.
    virtual void bpropUpdate(const Mat& inputs, const Mat& outputs,
                             Mat& input_gradients,
                             const Mat& output_gradients,
                             bool accumulate=false) = 0;

    //! back-propagates the output gradient to the input and the bias
    virtual void bpropUpdate(const Vec& input, const Vec& rbm_bias,
                             const Vec& output,
                             Vec& input_gradient, Vec& rbm_bias_gradient,
                             const Vec& output_gradient);

    //! Computes the negative log-likelihood of target given the
    //! internal activations of the layer
    virtual real fpropNLL(const Vec& target);
    virtual void fpropNLL(const Mat& targets, const Mat& costs_column);
    virtual real fpropNLL(const Vec& target, const Vec& cost_weights);

    //! Computes the gradient of the negative log-likelihood of target
    //! with respect to the layer's bias, given the internal activations
    virtual void bpropNLL(const Vec& target, real nll, Vec& bias_gradient);
    virtual void bpropNLL(const Mat& targets, const Mat& costs_column,
                          Mat& bias_gradients);

    //! Accumulates positive phase statistics
    virtual void accumulatePosStats( const Vec& pos_values );
    virtual void accumulatePosStats( const Mat& ps_values);

    //! Accumulates negative phase statistics
    virtual void accumulateNegStats( const Vec& neg_values );
    virtual void accumulateNegStats( const Mat& neg_values );

    //! Update parameters according to accumulated statistics
    virtual void update();

    //! Updates parameters according to the given gradient
    virtual void update( const Vec& grad );
    virtual void update( const Mat& grad );

    //! Update parameters according to one pair of vectors
    virtual void update( const Vec& pos_values, const Vec& neg_values );

    //! Update parameters according to one pair of matrices.
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

    //! resets activations, sample and expectation fields
    virtual void reset();

    //! resets the statistics and counts
    virtual void clearStats();

    //! forgets everything
    virtual void forget();

    //! Set the internal bias values to rbm_bias
    virtual void setAllBias(const Vec& rbm_bias);

    //! Computes the contrastive divergence gradient with respect to the bias
    //! (or activations, which is equivalent).
    //! It should be noted that bpropCD does not call clearstats().
    virtual void bpropCD(Vec& bias_gradient);

    //! Computes the contrastive divergence gradient with respect to the bias
    //! (or activations, which is equivalent), given the positive and
    //! negative phase values.
    virtual void bpropCD(const Vec& pos_values, const Vec& neg_values,
                         Vec& bias_gradient);

    virtual real energy(const Vec& unit_values) const;

    //! Computes \f$ -log(\sum_{possible values of h} exp(h' unit_activations))\f$
    //! This quantity is used for computing the free energy of a sample x in
    //! the OTHER layer of an RBM, from which unit_activations was computed.
    virtual real freeEnergyContribution(const Vec& unit_activations) const;

    //! Computes gradient of the result of freeEnergyContribution
    //! \f$ -log(\sum_{possible values of h} exp(h' unit_activations))\f$
    //! with respect to unit_activations. Optionally, a gradient
    //! with respect to freeEnergyContribution can be given
    virtual void freeEnergyContributionGradient(const Vec& unit_activations,
                                                Vec& unit_activations_gradient,
                                                real output_gradient = 1,
                                                bool accumulate = false )
        const;

    //! Returns a number of different configurations the layer can be in.
    virtual int getConfigurationCount();

    //! Computes the conf_index configuration of the layer.
    virtual void getConfiguration(int conf_index, Vec& output);

    //! Applies the bias decay
    virtual void applyBiasDecay();

    //! Adds the bias decay to the bias gradients
    virtual void addBiasDecay(Vec& bias_gradient);

    //! Adds the bias decay to the bias gradients
    virtual void addBiasDecay(Mat& bias_gradient);

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_ABSTRACT_OBJECT(RBMLayer);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);


    //#####  Not Options  #####################################################
public:
    //! Accumulates positive contribution to the gradient of bias
    Vec bias_pos_stats;
    //! Accumulates negative contribution to the gradient of bias
    Vec bias_neg_stats;
protected:
    //! Stores the momentum of the gradient
    Vec bias_inc;

    //! A vector containing only ones, used to compute efficiently mini-batch
    //! updates.
    Vec ones;

    //! Count of positive examples
    int pos_count;
    //! Count of negative examples
    int neg_count;

    //! Expectations for mini-batch operations.
    //! It is protected to encourage the use of accessors.
    Mat expectations;

    Vec tmp;

protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    static void declareOptions(OptionList& ol);

    //! Declares the class Methods.
    static void declareMethods(RemoteMethodMap& rmm);

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    void build_();

private:
    //#####  Private Data Members  ############################################

};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(RBMLayer);

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
