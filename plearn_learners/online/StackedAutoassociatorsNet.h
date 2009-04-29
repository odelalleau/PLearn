// -*- C++ -*-

// StackedAutoassociatorsNet.h
//
// Copyright (C) 2007 Hugo Larochelle
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

/*! \file StackedAutoassociatorsNet.h */


#ifndef StackedAutoassociatorsNet_INC
#define StackedAutoassociatorsNet_INC

#include <plearn_learners/generic/PLearner.h>
#include <plearn_learners/online/OnlineLearningModule.h>
#include <plearn_learners/online/CostModule.h>
#include <plearn_learners/online/NLLCostModule.h>
#include <plearn_learners/online/RBMClassificationModule.h>
#include <plearn_learners/online/RBMLayer.h>
#include <plearn_learners/online/RBMMixedLayer.h>
#include <plearn_learners/online/RBMConnection.h>
#include <plearn/misc/PTimer.h>

namespace PLearn {

/**
 * Neural net, trained layer-wise in a greedy fashion using autoassociators.
 * It is highly inspired by the DeepBeliefNet class, and can use use the
 * same RBMLayer and RBMConnection components.
 */
class StackedAutoassociatorsNet : public PLearner
{
    typedef PLearner inherited;

public:
    //#####  Public Build Options  ############################################

    //! The learning rate used during the autoassociator gradient descent
    //! training. It is also used for the partial costs.
    real greedy_learning_rate;

    //! The decrease constant of the learning rate used during the
    //! autoassociator gradient descent training. When a hidden layer has
    //! finished its training, the learning rate is reset to it's initial
    //! value.
    real greedy_decrease_ct;

    //! The learning rate used during the fine tuning gradient descent
    real fine_tuning_learning_rate;

    //! The decrease constant of the learning rate used during fine tuning
    //! gradient descent
    real fine_tuning_decrease_ct;

    //! L1 penalty weight on the hidden layers, to encourage sparsity during
    //! the greedy unsupervised phases
    real l1_neuron_decay;

    //! Value around which the L1 penalty should be centered, i.e.
    //!    L1(h) = | h - l1_neuron_decay_center |
    //! where h is the value of the neurons.
    real l1_neuron_decay_center;

    //! Training batch size (1=stochastic learning, 0=full batch learning)
    int batch_size;

    //! Number of examples to use during each phase of greedy pre-training.
    //! The number of fine-tunig steps is defined by nstages.
    TVec<int> training_schedule;

    //! Whether to do things by stages, including fine-tuning, or on-line
    bool online;

    //! The layers of units in the network
    TVec< PP<RBMLayer> > layers;

    //! The weights of the connections between the layers
    TVec< PP<RBMConnection> > connections;

    //! The weights of the reconstruction connections between the layers
    TVec< PP<RBMConnection> > reconstruction_connections;

    //! Optional weights to capture correlation and anti-correlation
    //! in the hidden layers. They must have the same input and
    //! output sizes, compatible with their corresponding layers.
    TVec< PP<RBMConnection> > correlation_connections;

    //! Optional weights from each inputs to all other inputs'
    //! reconstruction, which can capture simple (linear or log-linear)
    //! correlations between inputs.
    mutable TVec< PP<RBMConnection> > direct_connections;

    //! Module that takes as input the output of the last layer
    //! (layers[n_layers-1), and feeds its output to final_cost
    //! which defines the fine-tuning criteria.
    PP<OnlineLearningModule> final_module;

    //! The cost function to be applied on top of the neural network
    //! (i.e. at the output of final_module). Its gradients will be
    //! backpropagated to final_module and then backpropagated to
    //! the layers.
    PP<CostModule> final_cost;

    //! Corresponding additional supervised cost function to be applied on
    //! top of each hidden layer during the autoassociator training stages.
    //! The gradient for these costs are not backpropagated to previous layers.
    TVec< PP<CostModule> > partial_costs;

    //! Relative weights of the partial costs. If not defined,
    //! weights of 1 will be assumed for all partial costs.
    Vec partial_costs_weights;

    //! Optional target connections during greedy training.
    //! They connect the target with the hidden layer from which
    //! the autoassociator's cost (including partial cost) is computed
    //! (only during training).
    //! Currently works only if target is a class index.
    TVec< PP<RBMConnection> > greedy_target_connections;

    //! Indication that, at test time, all costs for all
    //! layers (up to the currently trained layer) should be computed.
    bool compute_all_test_costs;

    //! Indication that the autoassociators are also trained to
    //! reconstruct their hidden layers (inspired from CD1 in an RBM)
    bool reconstruct_hidden;

    //! Type of noise that corrupts the autoassociators input
    string noise_type;

    //! Method used to fill the double_input vector when using missing_data
    //| noise type.
    string missing_data_method;

    //! Weight given to a corrupted (or missing) data when
    //! backpropagating the gradient of reconstruction cost.
    real corrupted_data_weight;

    //! Weight given to a data (not corrupted or not missing) when
    //! backpropagating the gradient of reconstruction cost.
    real data_weight;

    //! Random fraction of the autoassociators' input components that
    //! masked, i.e. unsused to reconstruct the input.
    real fraction_of_masked_inputs;

    //! Probability of masking each input component. Either this option
    //! or fraction_of_masked_inputs should be > 0.
    real probability_of_masked_inputs;

    //! Probability of masking the target, 
    //! when using greedy_target_connections.
    real probability_of_masked_target;

    //! training set mean of that component
    bool mask_with_mean;

    //! Indication that inputs should be masked with 
    //! 0 or 1 according to prop_salt_noise. 
    bool mask_with_pepper_salt;

    //! Probability that we mask the input by 1 instead of 0.
    real prob_salt_noise;

    //! Standard deviation of Gaussian noise
    real gaussian_std;

    //! Parameter \f$ \tau \f$ for corrupted input sampling:
    //! \f$ \tilde{x}_k ~ B((x_k - 0.5) \tau + 0.5) \f$
    real binary_sampling_noise_parameter;

    //! Number of samples to use for unsupervised fine-tuning
    int unsupervised_nstages;

    //! The learning rate used during the unsupervised fine tuning gradient
    //! descent
    real unsupervised_fine_tuning_learning_rate;

    //! The decrease constant of the learning rate used during
    //! unsupervised fine tuning gradient descent
    real unsupervised_fine_tuning_decrease_ct;

    //! Indicates how many layers will be corrupted during
    //! gready layer-wise learning (starting with input layer)
    int nb_corrupted_layer;

    //! Indication that only the input layer should be masked
    //! during greedy layer-wise learning
    bool mask_input_layer_only;

    //! Indication that only the input layer should be masked
    //! during unsupervised fine-tuning
    bool mask_input_layer_only_in_unsupervised_fine_tuning;

    //! The number of samples to use to compute training stats.
    //! -1 (default) means the number of training samples.
    int train_stats_window;


    //#####  Public Learnt Options  ###########################################

    //! Number of layers
    int n_layers;

    //! Number of samples visited so far during unsupervised fine-tuning
    int unsupervised_stage;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    StackedAutoassociatorsNet();

    //#####  PLearner Member Functions  #######################################

    //! Returns the size of this learner's output, (which typically
    //! may depend on its inputsize(), targetsize() and set options).
    virtual int outputsize() const;

    //! (Re-)initializes the PLearner in its fresh state (that state may depend
    //! on the 'seed' option) and sets 'stage' back to 0 (this is the stage of
    //! a fresh learner!).
    virtual void forget();

    //! The role of the train method is to bring the learner up to
    //! stage==nstages, updating the train_stats collector with training costs
    //! measured on-line in the process.
    virtual void train();

    //! Computes the output from the input.
    virtual void computeOutput(const Vec& input, Vec& output) const;

    //! Computes the output from the input.
    virtual void computeOutputs(const Mat& input, Mat& output) const;

    //! Computes the costs from already computed output.
    virtual void computeCostsFromOutputs(const Vec& input, const Vec& output,
                                         const Vec& target, Vec& costs) const;

    virtual void computeOutputsAndCosts(const Mat& input, const Mat& target,
                                        Mat& output, Mat& costs) const;

    //! Returns the names of the costs computed by computeCostsFromOutpus (and
    //! thus the test method).
    virtual TVec<std::string> getTestCostNames() const;

    //! Returns the names of the objective costs that the train method computes
    //! and  for which it updates the VecStatsCollector train_stats.
    virtual TVec<std::string> getTrainCostNames() const;


    void greedyStep(const Vec& input, const Vec& target, int index,
                    Vec train_costs);
    void greedyStep(const Mat& inputs, const Mat& targets, int index,
                    Mat& train_costs);

    void unsupervisedFineTuningStep(const Vec& input, const Vec& target,
                                    Vec& train_costs);
    void unsupervisedFineTuningStep(const Mat& inputs, const Mat& targets,
                                    Mat& train_costs);

    void fineTuningStep(const Vec& input, const Vec& target,
                        Vec& train_costs);
    void fineTuningStep(const Mat& inputs, const Mat& targets,
                        Mat& train_costs);

    void onlineStep(const Vec& input, const Vec& target,
                    Vec& train_costs);
    void onlineStep(const Mat& inputs, const Mat& targets,
                    Mat& train_costs);


    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS
    PLEARN_DECLARE_OBJECT(StackedAutoassociatorsNet);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:

    //! Actual size of a mini-batch (size of the training set if batch_size==1)
    int minibatch_size;

    //#####  Not Options  #####################################################

    //! Stores the activations of the input and hidden layers
    //! (at the input of the layers)
    mutable TVec<Vec> activations;
    mutable TVec<Mat> activations_m;

    //! Stores the expectations of the input and hidden layers
    //! (at the output of the layers)
    mutable TVec<Vec> expectations;
    mutable TVec<Mat> expectations_m;
    //! In case of missing_data: expectations doubled before corruption 
    //! or before propagation to the next layer.
    mutable TVec< Vec > doubled_expectations;

    //! Stores the gradient of the cost wrt the activations of
    //! the input and hidden layers
    //! (at the input of the layers)
    mutable TVec<Vec> activation_gradients;
    mutable TVec<Mat> activation_gradients_m;

    //! Stores the gradient of the cost wrt the expectations of
    //! the input and hidden layers
    //! (at the output of the layers)
    mutable TVec<Vec> expectation_gradients;
    mutable TVec<Mat> expectation_gradients_m;
    //! Stores the gradients of the doubled version of expectations
    mutable TVec< Vec > doubled_expectation_gradients;

    //! Reconstruction activations
    mutable Vec reconstruction_activations;
    mutable Mat reconstruction_activations_m;

    //! Reconstruction activation gradients
    mutable Vec reconstruction_activation_gradients;
    mutable Mat reconstruction_activation_gradients_m;

    //! Reconstruction expectation gradients
    mutable Vec reconstruction_expectation_gradients;
    mutable Mat reconstruction_expectation_gradients_m;

    //! Unsupervised fine-tuning reconstruction activations
    TVec< Vec > fine_tuning_reconstruction_activations;

    //! Unsupervised fine-tuning reconstruction expectations
    TVec< Vec > fine_tuning_reconstruction_expectations;

    //! Unsupervised fine-tuning reconstruction activations gradients
    TVec< Vec > fine_tuning_reconstruction_activation_gradients;

    //! Unsupervised fine-tuning reconstruction expectations gradients
    TVec< Vec > fine_tuning_reconstruction_expectation_gradients;

    //! Reconstruction activation gradients coming from hidden reconstruction
    mutable Vec reconstruction_activation_gradients_from_hid_rec;

    //! Reconstruction expectation gradients coming from hidden reconstruction
    mutable Vec reconstruction_expectation_gradients_from_hid_rec;

    //! Hidden reconstruction activations
    mutable Vec hidden_reconstruction_activations;

    //! Hidden reconstruction activation gradients
    mutable Vec hidden_reconstruction_activation_gradients;

    //! Activations before the correlation layer
    mutable TVec<Vec> correlation_activations;
    mutable TVec<Mat> correlation_activations_m;

    //! Expectations before the correlation layer
    mutable TVec<Vec> correlation_expectations;
    mutable TVec<Mat> correlation_expectations_m;

    //! Gradients of activations before the correlation layer
    mutable TVec<Vec> correlation_activation_gradients;
    mutable TVec<Mat> correlation_activation_gradients_m;

    //! Gradients of expectations before the correlation layer
    mutable TVec<Vec> correlation_expectation_gradients;
    mutable TVec<Mat> correlation_expectation_gradients_m;

    //! Hidden layers for the correlation connections
    mutable TVec< PP<RBMLayer> > correlation_layers;

    //! Activations from the direct connections
    mutable Vec direct_activations;

    //! Sum of activations from the direct and reconstruction connections
    mutable Vec direct_and_reconstruction_activations;

    //! Gradient of sum of activations from the direct and reconstruction
    //! connections
    mutable Vec direct_and_reconstruction_activation_gradients;

    //! Position in the total cost vector of the different partial costs
    mutable TVec<int> partial_costs_positions;

    //! Cost value of partial_costs
    mutable Vec partial_cost_value;
    mutable Mat partial_cost_values;
    mutable Vec partial_cost_values_0;

    //! Input of the final_cost
    mutable Vec final_cost_input;
    mutable Mat final_cost_inputs;

    //! Cost value of final_cost
    mutable Vec final_cost_value;
    mutable Mat final_cost_values;
    mutable Vec final_cost_values_0;

    //! Stores the gradient of the cost at the input of final_cost
    mutable Vec final_cost_gradient;
    mutable Mat final_cost_gradients;

    //! Layers randomly masked, for unsupervised fine-tuning.
    TVec< Vec > corrupted_autoassociator_expectations;

    //! Stores the weight of each data used when
    //! backpropagating the gradient of reconstruction cost.
    //! The weight is either corrupted_data_weight or data_weight
    //! if the data has been corrupted or not, respectively.
    //! Used for example to put emphasis on corrupted/missing data
    //! during the reconstruction.
    mutable Vec reconstruction_weights;

    //! Layers random binary maske, for online learning.
    TVec< Vec > binary_masks;

    //! For when corrupt_input() with binary_mask parameter is called
    Vec tmp_mask;

    //! Indices of the expectation components
    TVec< TVec<int> > autoassociator_expectation_indices;

    //! Mean of layers on the training set for each layer
    TVec<Vec> expectation_means;

    //! Vectorial representation of the target
    Vec target_vec;
    Vec target_vec_gradient;
    //! For online case
    TVec< Vec > targets_vec;
    TVec< Vec > targets_vec_gradient;

    //! Stages of the different greedy phases
    TVec<int> greedy_stages;

    //! Currently trained layer (1 means the first hidden layer,
    //! n_layers means the output layer)
    int currently_trained_layer;

protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    static void declareOptions(OptionList& ol);

    //! Declare the methods that are remote-callable
    static void declareMethods(RemoteMethodMap& rmm);

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    void build_();

    void build_layers_and_connections();

    void build_classification_cost();

    void build_costs();

    void setLearningRate( real the_learning_rate );

    void corrupt_input(const Vec& input, Vec& corrupted_input, int layer);

    //! Useful in case that noise_type == "missing_data", 
    //! returns the input if it's not the case.
    void double_input(const Vec& input, Vec& doubled_input, bool double_grad=false) const;

    //! Useful in case that noise_type == "missing_data",
    //! returns the input if it's not the case.
    void divide_input(const Vec& input, Vec& divided_input) const ;

    //! Supposes the learner is already trained.
    //! Allows a codage-decodage ktime from a source image. Returns the 'fantasize' image. 
    //! You can choose how many layers to use (including raws layer) by defining the size of sample. 
    //! You can corrupt layers differently during the codage phase by defining maskNoiseFractOrProb 
    //! You can apply a binary sampling (1) or not (0) differently for each layer during the decode phase
    //! Lower element in sample and maskNoiseFractOrProb correspond to lower layer. 
    //! Example using 3 hidden layers of a learner: 
    //!     maskNoiseFractOrProb = [0.1,0.25,0]  // noise applied on raws layer
    //!                                          // and first hidden layer.
    //!     sample = [1,0,0] // sampling only before reconstruction of the
    //!                      // raws input.
    Vec fantasizeKTime(int KTime, const Vec& srcImg, const Vec& sample,
                        const Vec& maskNoiseFractOrProb);

    void corrupt_input(const Vec& input, Vec& corrupted_input, int layer, Vec& binary_mask);

    //! Global storage to save memory allocations.
    mutable Vec tmp_output;
    mutable Mat tmp_output_mat;

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(StackedAutoassociatorsNet);

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
