// -*- C++ -*-

// StackedAutoassociatorsNet.cc
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

/*! \file StackedAutoassociatorsNet.cc */


#define PL_LOG_MODULE_NAME "StackedAutoassociatorsNet"

#include "StackedAutoassociatorsNet.h"
#include <plearn/io/pl_log.h>
#include <plearn/sys/Profiler.h>
#include <plearn/io/load_and_save.h>

#define minibatch_hack 0 // Do we force the minibatch setting? (debug hack)

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    StackedAutoassociatorsNet,
    "Neural net, trained layer-wise in a greedy fashion using autoassociators",
    "It is highly inspired by the DeepBeliefNet class, and can use the\n"
    "same RBMLayer and RBMConnection components.\n"
    );

StackedAutoassociatorsNet::StackedAutoassociatorsNet() :
    greedy_learning_rate( 0. ),
    greedy_decrease_ct( 0. ),
    fine_tuning_learning_rate( 0. ),
    fine_tuning_decrease_ct( 0. ),
    l1_neuron_decay( 0. ),
    l1_neuron_decay_center( 0 ),
    batch_size( 1 ),
    online( false ),
    compute_all_test_costs( false ),
    reconstruct_hidden( false ),
    noise_type( "masking_noise" ),
    missing_data_method( "binomial_complementary"),
    corrupted_data_weight( 1 ),
    data_weight( 1 ),
    fraction_of_masked_inputs( 0. ),
    probability_of_masked_inputs( 0. ),
    probability_of_masked_target( 0. ),
    mask_with_mean( false ),
    mask_with_pepper_salt( false ),
    pep_salt_zero_centered( 0. ),
    renoising( false ),
    noisy( 0 ),
    prob_salt_noise( 0.5 ),
    gaussian_std( 1. ),
    binary_sampling_noise_parameter( 1. ),
    unsupervised_nstages( 0 ),
    unsupervised_fine_tuning_learning_rate( 0. ),
    unsupervised_fine_tuning_decrease_ct( 0. ),
    nb_corrupted_layer( -1 ),
    mask_input_layer_only( false ),
    mask_input_layer_only_in_unsupervised_fine_tuning( false ),
    train_stats_window( -1 ),
    learnerExpdir(""),
    save_learner_before_fine_tuning( false ),
    keep_online_representations( false ),
    n_layers( 0 ),
    unsupervised_stage( 0 ),
    minibatch_size( 0 ),
    currently_trained_layer( 0 )
{
    // random_gen will be initialized in PLearner::build_()
    random_gen = new PRandom();
    nstages = 0;
    //To have faster test time by default. That don't change the result.
    if(test_minibatch_size==1)
        test_minibatch_size = 128;
}

void StackedAutoassociatorsNet::declareOptions(OptionList& ol)
{
    declareOption(ol, "greedy_learning_rate",
                  &StackedAutoassociatorsNet::greedy_learning_rate,
                  OptionBase::buildoption,
                  "The learning rate used during the autoassociator "
                  "gradient descent training");

    declareOption(ol, "greedy_decrease_ct",
                  &StackedAutoassociatorsNet::greedy_decrease_ct,
                  OptionBase::buildoption,
                  "The decrease constant of the learning rate used during "
                  "the autoassociator\n"
                  "gradient descent training. When a hidden layer has finished "
                  "its training,\n"
                  "the learning rate is reset to it's initial value.\n");

    declareOption(ol, "fine_tuning_learning_rate",
                  &StackedAutoassociatorsNet::fine_tuning_learning_rate,
                  OptionBase::buildoption,
                  "The learning rate used during the fine tuning gradient descent");

    declareOption(ol, "fine_tuning_decrease_ct",
                  &StackedAutoassociatorsNet::fine_tuning_decrease_ct,
                  OptionBase::buildoption,
                  "The decrease constant of the learning rate used during "
                  "fine tuning\n"
                  "gradient descent.\n");

    declareOption(ol, "l1_neuron_decay",
                  &StackedAutoassociatorsNet::l1_neuron_decay,
                  OptionBase::buildoption,
                  " L1 penalty weight on the hidden layers, to encourage "
                  "sparsity during\n"
                  "the greedy unsupervised phases.\n"
                  );

    declareOption(ol, "l1_neuron_decay_center",
                  &StackedAutoassociatorsNet::l1_neuron_decay_center,
                  OptionBase::buildoption,
                  "Value around which the L1 penalty should be centered, i.e.\n"
                  "   L1(h) = | h - l1_neuron_decay_center |\n"
                  "where h are the values of the neurons.\n");

    declareOption(ol, "training_schedule",
                  &StackedAutoassociatorsNet::training_schedule,
                  OptionBase::buildoption,
                  "Number of examples to use during each phase of greedy pre-training.\n"
                  "The number of fine-tunig steps is defined by nstages.\n");

    declareOption(ol, "layers", &StackedAutoassociatorsNet::layers,
                  OptionBase::buildoption,
                  "The layers of units in the network. The first element\n"
                  "of this vector should be the input layer and the\n"
                  "subsequent elements should be the hidden layers. The\n"
                  "output layer should not be included in layers.\n");

    declareOption(ol, "reconstruction_layers", &StackedAutoassociatorsNet::reconstruction_layers,
                  OptionBase::buildoption,
                  "The reconstruction layers in the network (if different than encodage layers.\n"
                  "The first element of this vector should be the layer for the input layer reconstruction and the\n"
                  "subsequent elements should be the layer for the reconstruction of hidden layers.\n");

    declareOption(ol, "connections", &StackedAutoassociatorsNet::connections,
                  OptionBase::buildoption,
                  "The weights of the connections between the layers");

    declareOption(ol, "reconstruction_connections",
                  &StackedAutoassociatorsNet::reconstruction_connections,
                  OptionBase::buildoption,
                  "The weights of the reconstruction connections between the "
                  "layers");

    declareOption(ol, "correlation_connections",
                  &StackedAutoassociatorsNet::correlation_connections,
                  OptionBase::buildoption,
                  "Optional weights to capture correlation and anti-correlation\n"
                  "in the hidden layers. They must have the same input and\n"
                  "output sizes, compatible with their corresponding layers.");

    declareOption(ol, "direct_connections",
                  &StackedAutoassociatorsNet::direct_connections,
                  OptionBase::buildoption,
                  "Optional weights from each inputs to all other inputs'\n"
                  "reconstruction, which can capture simple (linear or log-linear)\n"
                  "correlations between inputs.");

    declareOption(ol, "final_module", &StackedAutoassociatorsNet::final_module,
                  OptionBase::buildoption,
                  "Module that takes as input the output of the last layer\n"
                  "(layers[n_layers-1), and feeds its output to final_cost\n"
                  "which defines the fine-tuning criteria.\n"
                 );

    declareOption(ol, "final_cost", &StackedAutoassociatorsNet::final_cost,
                  OptionBase::buildoption,
                  "The cost function to be applied on top of the neural network\n"
                  "(i.e. at the output of final_module). Its gradients will be \n"
                  "backpropagated to final_module and then backpropagated to\n"
                  "the layers.\n"
                  );

    declareOption(ol, "partial_costs", &StackedAutoassociatorsNet::partial_costs,
                  OptionBase::buildoption,
                  "Corresponding additional supervised cost function to be "
                  "applied on \n"
                  "top of each hidden layer during the autoassociator "
                  "training stages. \n"
                  "The gradient for these costs are not backpropagated to "
                  "previous layers.\n"
        );

    declareOption(ol, "batch_size", &StackedAutoassociatorsNet::batch_size,
                  OptionBase::buildoption,
                  "Training batch size (1=stochastic learning, 0=full batch"
                  " learning)");

    declareOption(ol, "online", &StackedAutoassociatorsNet::online,
                  OptionBase::buildoption,
                  "If true then all unsupervised training stages (as well as\n"
                  "the fine-tuning stage) are done simultaneously.\n");

    declareOption(ol, "partial_costs_weights",
                  &StackedAutoassociatorsNet::partial_costs_weights,
                  OptionBase::buildoption,
                  "Relative weights of the partial costs. If not defined,\n"
                  "weights of 1 will be assumed for all partial costs.\n"
        );

    declareOption(ol, "greedy_target_connections",
                  &StackedAutoassociatorsNet::greedy_target_connections,
                  OptionBase::buildoption,
                  "Optional target connections during greedy training..\n"
                  "They connect the target with the hidden layer from which\n"
                  "the autoassociator's cost (including partial cost) is computed\n"
                  "(only during training).\n"
                  "Currently works only if target is a class index.\n"
        );

    declareOption(ol, "compute_all_test_costs",
                  &StackedAutoassociatorsNet::compute_all_test_costs,
                  OptionBase::buildoption,
                  "Indication that, at test time, all costs for all layers \n"
                  "(up to the currently trained layer) should be computed.\n"
        );

    declareOption(ol, "reconstruct_hidden",
                  &StackedAutoassociatorsNet::reconstruct_hidden,
                  OptionBase::buildoption,
                  "Indication that the autoassociators are also trained to\n"
                  "reconstruct their hidden layers (inspired from CD1 in an RBM).\n"
        );

    declareOption(ol, "noise_type",
                  &StackedAutoassociatorsNet::noise_type,
                  OptionBase::buildoption,
                  "Type of noise that corrupts the autoassociators input. "
                  "Choose among:\n"
                  " - \"missing_data\"\n"
                  " - \"masking_noise\"\n"
                  " - \"binary_sampling\"\n"
                  " - \"gaussian\"\n"
                  " - \"none\"\n"
        );

    declareOption(ol, "missing_data_method",
                  &StackedAutoassociatorsNet::missing_data_method,
                  OptionBase::buildoption,
                  "Method used to fill the double_input vector for missing_data noise type."
                  "Choose among:\n"
                  " - \"binomial_complementary\"\n"
                  " - \"one_if_missing\""
        );

    declareOption(ol, "corrupted_data_weight",
                  &StackedAutoassociatorsNet::corrupted_data_weight,
                  OptionBase::buildoption,
                  "Weight owned by a corrupted or missing data when"
                  "backpropagating the gradient of reconstruction cost.\n"
        );

    declareOption(ol, "data_weight",
                  &StackedAutoassociatorsNet::data_weight,
                  OptionBase::buildoption,
                  "Weight owned by a data not corrupted when"
                  "backpropagating the gradient of reconstruction cost.\n"
        );

    declareOption(ol, "fraction_of_masked_inputs",
                  &StackedAutoassociatorsNet::fraction_of_masked_inputs,
                  OptionBase::buildoption,
                  "Random fraction of the autoassociators' input components that\n"
                  "masked, i.e. unsused to reconstruct the input.\n"
        );

    declareOption(ol, "probability_of_masked_inputs",
                  &StackedAutoassociatorsNet::probability_of_masked_inputs,
                  OptionBase::buildoption,
                  "Probability of masking each input component. Either this "
                  "option.\n"
                  "or fraction_of_masked_inputs should be > 0.\n"
        );

    declareOption(ol, "probability_of_masked_target",
                  &StackedAutoassociatorsNet::probability_of_masked_target,
                  OptionBase::buildoption,
                  "Probability of masking the target, when using greedy_target_connections.\n"
        );

    declareOption(ol, "mask_with_mean",
                  &StackedAutoassociatorsNet::mask_with_mean,
                  OptionBase::buildoption,
                  "Indication that inputs should be masked with the "
                  "training set mean of that component.\n"
        );

    declareOption(ol, "mask_with_pepper_salt",
                  &StackedAutoassociatorsNet::mask_with_pepper_salt,
                  OptionBase::buildoption,
                  "Indication that inputs should be masked with "
                  "0 or 1 according to prob_salt_noise.\n"
        );

    declareOption(ol, "pep_salt_zero_centered",
                  &StackedAutoassociatorsNet::pep_salt_zero_centered,
                  OptionBase::buildoption,
                  " Indicate if the mask is zero centered (>0) or not (==0). "
                  " If equal 0 (not centered)"
                  " then pepVal is 0 and saltVal is 1."
                  " If is greater than 0 (centered),"
                  " then pepVal is -pep_salt_zero_centered and "
                  " saltVal is pep_salt_zero_centered.\n"
        );

    declareOption(ol, "renoising",
                  &StackedAutoassociatorsNet::renoising,
                  OptionBase::buildoption,
                  "Indication that the autoassociator will try to"
                  "'reconstruct' _another_ corrupted version of the input"
                  "(instead of the input itself),"
                  "from an initial encoded corrupted version of the input.\n"
        );

    declareOption(ol, "noisy",
                  &StackedAutoassociatorsNet::noisy, 
                  OptionBase::buildoption,
                  "Indication that example are corrupted before using them for a particular training."
                  "Note that the original example are used for any test."
                  "Choose among:\n"
                  "0 : no example noisy\n"
                  "1 : noisy applied before unsup. pre-training (basic autoassociator will be used (no denoising).\n"
                  "2 : noisy applied before unsup. pre-training and before supervised fine-tuning.\n" 
        );


    declareOption(ol, "prob_salt_noise",
                  &StackedAutoassociatorsNet::prob_salt_noise,
                  OptionBase::buildoption,
                  "Probability that we mask the input by 1 instead of 0.\n"
        );

    declareOption(ol, "gaussian_std",
                  &StackedAutoassociatorsNet::gaussian_std,
                  OptionBase::buildoption,
                  "Standard deviation of Gaussian noise.\n"
        );

    declareOption(ol, "binary_sampling_noise_parameter",
                  &StackedAutoassociatorsNet::binary_sampling_noise_parameter,
                  OptionBase::buildoption,
                  "Parameter \tau for corrupted input sampling:\n"
                  "  \tilde{x}_k ~ B((x_k - 0.5) \tau + 0.5)\n"
        );

    declareOption(ol, "unsupervised_nstages",
                  &StackedAutoassociatorsNet::unsupervised_nstages,
                  OptionBase::buildoption,
                  "Number of samples to use for unsupervised fine-tuning.\n");

    declareOption(ol, "unsupervised_fine_tuning_learning_rate",
                  &StackedAutoassociatorsNet::unsupervised_fine_tuning_learning_rate,
                  OptionBase::buildoption,
                  "The learning rate used during the unsupervised "
                  "fine tuning gradient descent");

    declareOption(ol, "unsupervised_fine_tuning_decrease_ct",
                  &StackedAutoassociatorsNet::unsupervised_fine_tuning_decrease_ct,
                  OptionBase::buildoption,
                  "The decrease constant of the learning rate used during\n"
                  "unsupervised fine tuning gradient descent.\n");

    declareOption(ol, "nb_corrupted_layer",
                  &StackedAutoassociatorsNet::nb_corrupted_layer,
                  OptionBase::buildoption,
                  "Indicate how many layers should be corrupted,\n"
                  "starting with the input one,\n"
                  "during greedy layer-wise learning.\n");

    declareOption(ol, "mask_input_layer_only",
                  &StackedAutoassociatorsNet::mask_input_layer_only,
                  OptionBase::buildoption,
                  "Indication that only the input layer should be corrupted\n"
                  "during greedy layer-wise learning.\n");

    declareOption(ol, "mask_input_layer_only_in_unsupervised_fine_tuning",
                  &StackedAutoassociatorsNet::mask_input_layer_only_in_unsupervised_fine_tuning,
                  OptionBase::buildoption,
                  "Indication that only the input layer should be masked\n"
                  "during unsupervised fine-tuning.\n");

    declareOption(ol, "train_stats_window",
                  &StackedAutoassociatorsNet::train_stats_window,
                  OptionBase::buildoption,
                  "The number of samples to use to compute training stats.\n"
                  "-1 (default) means the number of training samples.\n");


    declareOption(ol, "learnerExpdir",
                  &StackedAutoassociatorsNet::learnerExpdir,
                  OptionBase::buildoption,
                  "Experiment directory where the learner will be save\n"
                  "if save_learner_before_fine_tuning is true."
        );

    declareOption(ol, "save_learner_before_fine_tuning",
                  &StackedAutoassociatorsNet::save_learner_before_fine_tuning,
                  OptionBase::buildoption,
                  "Saves the learner before the supervised fine-tuning."
        );

    declareOption(ol, "keep_online_representations",
                  &StackedAutoassociatorsNet::keep_online_representations,
                  OptionBase::buildoption,
                  "Keep trace of the representations obtained during an "
                  "unsupervised training phase.\n"
        );
    
    declareOption(ol, "greedy_stages",
                  &StackedAutoassociatorsNet::greedy_stages,
                  OptionBase::learntoption,
                  "Number of training samples seen in the different greedy "
                  "phases.\n"
        );

    declareOption(ol, "n_layers", &StackedAutoassociatorsNet::n_layers,
                  OptionBase::learntoption,
                  "Number of layers"
        );

    declareOption(ol, "unsupervised_stage",
                  &StackedAutoassociatorsNet::unsupervised_stage,
                  OptionBase::learntoption,
                  "Number of samples visited so far during unsupervised "
                  "fine-tuning.\n");

    declareOption(ol, "correlation_layers",
                  &StackedAutoassociatorsNet::correlation_layers,
                  OptionBase::learntoption,
                  "Hidden layers for the correlation connections"
        );

    declareOption(ol, "expectation_means",
                  &StackedAutoassociatorsNet::expectation_means,
                  OptionBase::learntoption,
                  "Mean of layers on the training set for each layer"
        );

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void StackedAutoassociatorsNet::declareMethods(RemoteMethodMap& rmm)
{
    // Insert a backpointer to remote methods; note that this is different from
    // declareOptions().
    rmm.inherited(inherited::_getRemoteMethodMap_());

    declareMethod(
        rmm, "fantasizeKTime",
        &StackedAutoassociatorsNet::fantasizeKTime,
        (BodyDoc("On a trained learner, computes a codage-decodage phase (fantasize phase) through a specified number of hidden layer. From one specified source image."),
         ArgDoc ("kTime", "Number of time we want to fantasize. \n" 
                 "Next input image will again be the source Image (if alwaysFromSrcImg is True) \n"
                 "or next input image will be the last fantasize image (if alwaysFromSrcImg is False), and so on for kTime.)"),
         ArgDoc ("srcImg", "Source image vector (should have same width as raws layer)"),
         ArgDoc ("sampling", "Vector of bool indicating whether or not a sampling will be done for each hidden layer\n"
                "during decodage. Its width indicates how many hidden layer will be used.)\n"
                " (should have same width as maskNoiseFractOrProb)\n"
                "smaller element of the vector correspond to lower layer"),
         ArgDoc ("maskNoiseFractOrProb", "Vector of noise fraction or probability\n"
                "(according to the one used during the learning stage)\n"
                "for each layer. (should have same width as sampling or be empty if unuseful.\n"
                "Smaller element of the vector correspond to lower layer"),
         ArgDoc ("alwaysFromSrcImg", "Booleen indicating whether each encode-decode \n"
                "steps are done from the source image (sets to True) or \n"
                "if the next input image is the preceding fantasize image obtained (sets to False). "),
         RetDoc ("Fantasize images obtained for each kTime.")));
    
    declareMethod(
        rmm, "fantasizeKTimeOnMultiSrcImg",
        &StackedAutoassociatorsNet::fantasizeKTimeOnMultiSrcImg,
        (BodyDoc("Call the 'fantasizeKTime' function for each source images found in the matrix 'srcImg'."),
         ArgDoc ("kTime", "Number of time we want to fantasize for each source images. \n"
                 "Next input image will again be the source Image (if alwaysFromSrcImg is True) \n"
                 "or next input image will be the last fantasize image (if alwaysFromSrcImg is False), and so on for kTime.)"),
         ArgDoc ("srcImg", "Source images matrix (should have same width as raws layer)"),
         ArgDoc ("sampling", "Vector of bool indicating whether or not a sampling will be done for each hidden layer\n"
                "during decodage. Its width indicates how many hidden layer will be used.)\n"
                " (should have same width as maskNoiseFractOrProb)\n"
                "smaller element of the vector correspond to lower layer"),
         ArgDoc ("maskNoiseFractOrProb", "Vector of noise fraction or probability\n"
                "(according to the one used during the learning stage)\n"
                "for each layer. (should have same width as sampling or be empty if unuseful.\n"
                "Smaller element of the vector correspond to lower layer"),
         ArgDoc ("alwaysFromSrcImg", "Booleen indicating whether each encode-decode \n"
                "steps are done from the source image (sets to True) or \n"
                "if the next input image is the preceding fantasize image obtained (sets to False). "),
         RetDoc ("For each source images, fantasize images obtained for each kTime.")));
    
    declareMethod(
        rmm, "getTrainRepresentations", &StackedAutoassociatorsNet::getTrainRepresentations,
        (BodyDoc("Returns the representations obtained during last pre-training of the current layer.\n"),
         RetDoc ("Current train representations")));

    declareMethod(
        rmm, "remote_setCurrentlyTrainedLayer", &StackedAutoassociatorsNet::remote_setCurrentlyTrainedLayer,
        (BodyDoc("Modify current_trained_layer.\n"),
        ArgDoc ("input", "Matrix of inputs."),
        RetDoc ("Outputs from each hidden layers.")));
   
}

void StackedAutoassociatorsNet::build_()
{
    MODULE_LOG << "build_() called" << endl;

    if(inputsize_ > 0 && targetsize_ > 0)
    {
        // Initialize some learnt variables
        n_layers = layers.length();

        if(nb_corrupted_layer == -1)
                nb_corrupted_layer = n_layers-1;

        if( nb_corrupted_layer >= n_layers)
            PLERROR("StackedAutoassociatorsNet::build_() - \n"
                    " - \n"
                    "nb_corrupted_layers should be < %d\n",n_layers);

        if( weightsize_ > 0 )
            PLERROR("StackedAutoassociatorsNet::build_() - \n"
                    "usage of weighted samples (weight size > 0) is not\n"
                    "implemented yet.\n");

        if( !online && training_schedule.length() != n_layers-1 )
            PLERROR("StackedAutoassociatorsNet::build_() - \n"
                    "training_schedule should have %d elements.\n",
                    n_layers-1);

        if( partial_costs && partial_costs.length() != n_layers-1 )
            PLERROR("StackedAutoassociatorsNet::build_() - \n"
                    "partial_costs should have %d elements.\n",
                    n_layers-1);

        if( partial_costs && partial_costs_weights &&
            partial_costs_weights.length() != n_layers-1 )
            PLERROR("StackedAutoassociatorsNet::build_() - \n"
                    "partial_costs_weights should have %d elements.\n",
                    n_layers-1);

        if( online && reconstruct_hidden )
            PLERROR("StackedAutoassociatorsNet::build_()"
                    " - \n"
                    "cannot use online setting with reconstruct_hidden=true.\n");

//        if( unsupervised_nstages > 0 && correlation_connections.length() != 0 )
//            PLERROR("StackedAutoassociatorsNet::build_()"
//                    " - \n"
//                    "cannot use unsupervised fine-tuning with correlation connections.\n");

        if( fraction_of_masked_inputs < 0 )
            PLERROR("StackedAutoassociatorsNet::build_()"
                    " - \n"
                    "fraction_of_masked_inputs should be > or equal to 0.\n");

        if( probability_of_masked_inputs < 0 )
            PLERROR("StackedAutoassociatorsNet::build_()"
                    " - \n"
                    "probability_of_masked_inputs should be > or equal to 0.\n");

        if( prob_salt_noise < 0 )
            PLERROR("StackedAutoassociatorsNet::build_()"
                    " - \n"
                    "prob_salt_noise should be > or equal to 0.\n");

        if( probability_of_masked_target < 0 )
            PLERROR("StackedAutoassociatorsNet::build_()"
                    " - \n"
                    "probability_of_masked_target should be > or equal to 0.\n");

        if( data_weight < 0 )
            PLERROR("StackedAutoassociatorsNet::build_()"
                    " - \n"
                    "data_weight should be > or equal to 0.\n");

        if( corrupted_data_weight < 0 )
            PLERROR("StackedAutoassociatorsNet::build_()"
                    " - \n"
                    "corrupted_data_weight should be > or equal to 0.\n");

        if( online && noise_type != "masking_noise" && batch_size != 1)
            PLERROR("StackedAutoassociatorsNet::build_()"
                    " - \n"
                    "corrupted inputs only works with masking noise in online setting,"
                    "in the non-minibatch case.\n");

        if( renoising && noisy > 0 )
            PLERROR("StackedAutoassociatorsNet::build_()"
                    " - \n"
                    "cannot use renoising and noisy at the same time.\n");

        if( renoising && noise_type == "missing_data" )
            PLERROR("StackedAutoassociatorsNet::build_()"
                    " - \n"
                    "cannot use renoising with missing data.\n");

        if( noisy > 0 && noise_type == "missing_data")
            PLERROR("StackedAutoassociatorsNet::build_()"
                    " - \n"
                    "cannot use noisy with missing data.\n");

        if( !online )
        {
            if( greedy_stages.length() == 0)
            {
                greedy_stages.resize(n_layers-1);
                greedy_stages.clear();
            }

            if(stage > 0)
                currently_trained_layer = n_layers;
            else
            {
                currently_trained_layer = n_layers-1;
                while(currently_trained_layer>1
                      && greedy_stages[currently_trained_layer-1] <= 0)
                    currently_trained_layer--;
            }
        }
        else
        {
            currently_trained_layer = n_layers;
        }

        build_layers_and_connections();
        build_costs();
    }
}

void StackedAutoassociatorsNet::build_layers_and_connections()
{
    MODULE_LOG << "build_layers_and_connections() called" << endl;

    if( connections.length() != n_layers-1 )
        PLERROR("StackedAutoassociatorsNet::build_layers_and_connections() - \n"
                "there should be %d connections.\n",
                n_layers-1);

    if( reconstruction_connections.length() != n_layers-1 )
        PLERROR("StackedAutoassociatorsNet::build_layers_and_connections() - \n"
                "there should be %d reconstruction connections.\n",
                n_layers-1);

    if( correlation_connections.length() != 0 &&
        correlation_connections.length() != n_layers-1 )
        PLERROR("StackedAutoassociatorsNet::build_layers_and_connections() - \n"
                "there should be either %d correlation connections or none.\n",
                n_layers-1);

    if( direct_connections.length() != 0 &&
        direct_connections.length() != n_layers-1 )
        PLERROR("StackedAutoassociatorsNet::build_layers_and_connections() - \n"
                "there should be either %d direct connections or none.\n",
                n_layers-1);

    if(reconstruct_hidden && compute_all_test_costs )
        PLERROR("StackedAutoassociatorsNet::build_layers_and_connections() - \n"
                "compute_all_test_costs option is not implemented for\n"
                "reconstruct_hidden option.");

    if( noise_type == "missing_data" || renoising || noisy > 0 )
    {
        if( correlation_connections.length() !=0 )
            PLERROR("StackedAutoassociatorsNet::build_layers_and_connections() - \n"
                    "Missing data, renoising and noisy are not implemented with correlation_connections.\n");
    
        if( direct_connections.length() !=0 )
            PLERROR("StackedAutoassociatorsNet::build_layers_and_connections() - \n"
                    "Missing data, renoising and noisy are not implemented with direct_connections.\n");
        
        if( reconstruct_hidden )
            PLERROR("StackedAutoassociatorsNet::build_layers_and_connections() - \n"
                    "Missing data, renoising and noisy are not implemented with reconstruct_hidden.\n");

        if( online ) 
            PLERROR("StackedAutoassociatorsNet::build_layers_and_connections() - \n"
                    "Missing data, renoising and noisy are not implemented in the online setting.\n");
    }

    if(correlation_connections.length() != 0)
    {
        if( compute_all_test_costs )
            PLERROR("StackedAutoassociatorsNet::build_layers_and_connections() - \n"
                    "compute_all_test_costs option is not implemented for\n"
                    "correlation_connections.");
        correlation_layers.resize( layers.length()-1 );
        for( int i=0 ; i<n_layers-1 ; i++ )
        {
            if( greedy_stages[i] == 0 )
            {
                CopiesMap map;
                correlation_layers[i] =
                    layers[i+1]->deepCopy(map);
            }
        }
        correlation_activations.resize( n_layers-1 );
        correlation_activations_m.resize( n_layers-1 );
        correlation_expectations.resize( n_layers-1 );
        correlation_expectations_m.resize( n_layers-1 );
        correlation_activation_gradients.resize( n_layers-1 );
        correlation_activation_gradients_m.resize( n_layers-1 );
        correlation_expectation_gradients.resize( n_layers-1 );
        correlation_expectation_gradients_m.resize( n_layers-1 );
    }

    if(layers[0]->size != inputsize_)
        PLERROR("StackedAutoassociatorsNet::build_layers_and_connections() - \n"
                "layers[0] should have a size of %d.\n",
                inputsize_);

    activations.resize( n_layers );
    activations_m.resize( n_layers );
    expectations.resize( n_layers );
    expectations_m.resize( n_layers );
    activation_gradients.resize( n_layers );
    activation_gradients_m.resize( n_layers );
    expectation_gradients.resize( n_layers );
    expectation_gradients_m.resize( n_layers );

    // If not defined, reconstruction_layers will  
    // simply point to the layers vector. 
    if( reconstruction_layers.length() == 0 )
        reconstruction_layers = layers;
    else
        if( reconstruction_layers.length() != layers.length()-1 && 
            reconstruction_layers.length() != layers.length() )
            PLERROR("StackedAutoassociatorsNet::build_layers_and_connections() "
                     "- \n"
                    "reconstruction_layers should have a length of layers.length-1 or layers.length, i.e: %d\n.",
                     layers.length()-1);

    for( int i=0 ; i<n_layers-1 ; i++ )
    {
        if( layers[i]->size != reconstruction_layers[i]->size )
            PLERROR("StackedAutoassociatorsNet::build_layers_and_connections() "
                     "- \n"
                    "layers[%i] should have same size of reconstruction_layers[%i], i.e: %d.\n",
                    i, i, layers[i]->size);

        if( noise_type == "missing_data")
        {
            if( layers[i]->size * 2 != connections[i]->down_size )
                PLERROR("StackedAutoassociatorsNet::build_layers_and_connections() "
                     "- \n"
                    "When noise_type==%s, connections[%i] should have a down_size "
                    "2 time the size of layers[%i], i.e: 2 * %d.\n",
                    noise_type.c_str(), i, i, layers[i]->size);

            if( reconstruction_connections[i]->up_size != layers[i]->size*2 )
            PLERROR("StackedAutoassociatorsNet::build_layers_and_connections() "
                    "- \n"
                    "When noise_type==%s, recontruction_connections[%i] should have a up_size "
                    "2 time the size of layers[%i], i.e: 2 * %d.\n",
                    noise_type.c_str(), i, i, layers[i]->size);
        }
        else
        {
            if( layers[i]->size != connections[i]->down_size )
                PLERROR("StackedAutoassociatorsNet::build_layers_and_connections() "
                     "- \n"
                    "connections[%i] should have a down_size of %d.\n",
                    i, layers[i]->size);

            if( reconstruction_connections[i]->up_size != layers[i]->size )
            PLERROR("StackedAutoassociatorsNet::build_layers_and_connections() "
                    "- \n"
                    "recontruction_connections[%i] should have a up_size of "
                    "%d.\n",
                    i, layers[i]->size);
        }

        if( connections[i]->up_size != layers[i+1]->size )
            PLERROR("StackedAutoassociatorsNet::build_layers_and_connections() "
                    "- \n"
                    "connections[%i] should have a up_size of %d.\n",
                    i, layers[i+1]->size);

        if( layers[i+1]->size != reconstruction_connections[i]->down_size )
            PLERROR("StackedAutoassociatorsNet::build_layers_and_connections() "
                    "- \n"
                    "recontruction_connections[%i] should have a down_size of "
                    "%d.\n",
                    i, layers[i+1]->size);

        if(correlation_connections.length() != 0)
        {
            if(reconstruct_hidden)
                PLERROR("StackedAutoassociatorsNet::build_layers_and_connections()"
                        " - \n"
                        "cannot use correlation_connections with reconstruct_hidden=true.\n");

            if( correlation_connections[i]->up_size != layers[i+1]->size ||
                correlation_connections[i]->down_size != layers[i+1]->size )
                PLERROR("StackedAutoassociatorsNet::build_layers_and_connections()"
                        " - \n"
                        "correlation_connections[%i] should have a up_size and "
                        "down_size of %d.\n",
                        i, layers[i+1]->size);
            correlation_activations[i].resize( layers[i+1]->size );
            correlation_expectations[i].resize( layers[i+1]->size );
            correlation_activation_gradients[i].resize( layers[i+1]->size );
            correlation_expectation_gradients[i].resize( layers[i+1]->size );
            if( !(correlation_connections[i]->random_gen) )
            {
                correlation_connections[i]->random_gen = random_gen;
                correlation_connections[i]->forget();
            }

            if( !(correlation_layers[i]->random_gen) )
            {
                correlation_layers[i]->random_gen = random_gen;
                correlation_layers[i]->forget();
            }
        }

        if(direct_connections.length() != 0)
        {
            if( online )
                PLERROR("StackedAutoassociatorsNet::build_layers_and_connections()"
                        " - \n"
                        "cannot use direct_connections in the online setting.\n");


            if(reconstruct_hidden)
                PLERROR("StackedAutoassociatorsNet::build_layers_and_connections()"
                        " - \n"
                        "cannot use direct_connections with reconstruct_hidden=true.\n");

            if( direct_connections[i]->up_size != layers[i]->size ||
                direct_connections[i]->down_size != layers[i]->size )
                PLERROR("StackedAutoassociatorsNet::build_layers_and_connections()"
                        " - \n"
                        "direct_connections[%i] should have a up_size and "
                        "down_size of %d.\n",
                        i, layers[i]->size);
            if( !(direct_connections[i]->random_gen) )
            {
                direct_connections[i]->random_gen = random_gen;
                direct_connections[i]->forget();
            }
        }

        if(greedy_target_connections.length() != 0)
        {
            if(reconstruct_hidden)
                PLERROR("StackedAutoassociatorsNet::build_layers_and_connections()"
                        " - \n"
                        "greedy_target_connections not implemented with reconstruct_hidden=true.\n");

            if( greedy_target_connections[i]->up_size != layers[i+1]->size )
                PLERROR("StackedAutoassociatorsNet::build_layers_and_connections()"
                        " - \n"
                        "greedy_target_connections[%i] should have a up_size of %d.\n",
                        i, layers[i+1]->size);
            if( !(greedy_target_connections[i]->random_gen) )
            {
                greedy_target_connections[i]->random_gen = random_gen;
                greedy_target_connections[i]->forget();
            }
        }

        if( !(layers[i]->random_gen) )
        {
            layers[i]->random_gen = random_gen;
            layers[i]->forget();
        }
        
        if( !(reconstruction_layers[i]->random_gen) )
        {
            reconstruction_layers[i]->random_gen = random_gen;
            reconstruction_layers[i]->forget();
        }

        if( !(connections[i]->random_gen) )
        {
            connections[i]->random_gen = random_gen;
            connections[i]->forget();
        }

        if( !(reconstruction_connections[i]->random_gen) )
        {
            reconstruction_connections[i]->random_gen = random_gen;
            reconstruction_connections[i]->forget();
        }

        activations[i].resize( layers[i]->size );
        expectations[i].resize( layers[i]->size );
        activation_gradients[i].resize( layers[i]->size );
        expectation_gradients[i].resize( layers[i]->size );
    }
    if( !(layers[n_layers-1]->random_gen) )
    {
        layers[n_layers-1]->random_gen = random_gen;
        layers[n_layers-1]->forget();
    }
    activations[n_layers-1].resize( layers[n_layers-1]->size );
    expectations[n_layers-1].resize( layers[n_layers-1]->size );
    activation_gradients[n_layers-1].resize( layers[n_layers-1]->size );
    expectation_gradients[n_layers-1].resize( layers[n_layers-1]->size );

    reconstruction_weights.resize( layers[0]->size );
    // Will be correctly resized if keep_online_representations == True
    train_representations.resize( 1 );

    // For denoising autoencoders
    doubled_expectations.resize( n_layers-1 );
    doubled_expectation_gradients.resize( n_layers-1 );
    corrupted_autoassociator_expectations.resize( n_layers-1 );
    binary_masks.resize( n_layers-1 );
    
    if( (noise_type == "masking_noise" || noise_type == "missing_data") && fraction_of_masked_inputs > 0 )
        autoassociator_expectation_indices.resize( n_layers-1 );
    
    if( renoising || noisy > 0 )
       second_corrupted_autoassociator_expectations.resize( n_layers-1 );

    for( int i=0 ; i<n_layers-1 ; i++ )
    {
        binary_masks[i].resize( layers[i]->size ); // For online learning
        if( noise_type == "missing_data" )
        {
            corrupted_autoassociator_expectations[i].resize( layers[i]->size * 2 );
            doubled_expectations[i].resize( layers[i]->size * 2 );
            doubled_expectation_gradients[i].resize( layers[i]->size * 2 );
        }
        else
        {
            corrupted_autoassociator_expectations[i].resize( layers[i]->size );
            doubled_expectations[i].resize( layers[i]->size );
            doubled_expectation_gradients[i].resize( layers[i]->size );
        }

        if( (noise_type == "masking_noise" || noise_type == "missing_data") && fraction_of_masked_inputs > 0 )
        {
            autoassociator_expectation_indices[i].resize( layers[i]->size );
            for( int j=0 ; j < autoassociator_expectation_indices[i].length() ; j++ )
                autoassociator_expectation_indices[i][j] = j;
        }

        if( renoising || noisy > 0 )
            second_corrupted_autoassociator_expectations[i].resize( layers[i]->size );
    }

    if(greedy_target_connections.length() != 0)
    {
        target_vec.resize(greedy_target_connections[0]->down_size);
        target_vec_gradient.resize(greedy_target_connections[0]->down_size);
        targets_vec.resize(n_layers-1);
        targets_vec_gradient.resize(n_layers-1);
        for( int i=0; i<n_layers-1; i++ )
        {
            targets_vec[i].resize(greedy_target_connections[0]->down_size);
            targets_vec_gradient[i].resize(greedy_target_connections[0]->down_size);
        }
    }
}
void StackedAutoassociatorsNet::build_costs()
{
    MODULE_LOG << "build_final_cost() called" << endl;

    if( !final_cost )
        PLERROR("StackedAutoassociatorsNet::build_costs() - \n"
                "final_cost should be provided.\n");

    final_cost_gradient.resize( final_cost->input_size );
    final_cost->setLearningRate( fine_tuning_learning_rate );

    if( !(final_cost->random_gen) )
    {
        final_cost->random_gen = random_gen;
        final_cost->forget();
    }


    if( !final_module )
        PLERROR("StackedAutoassociatorsNet::build_costs() - \n"
                "final_module should be provided.\n");

    if( layers[n_layers-1]->size != final_module->input_size )
        PLERROR("StackedAutoassociatorsNet::build_costs() - \n"
                "final_module should have an input_size of %d.\n",
                layers[n_layers-1]->size);

    if( final_module->output_size != final_cost->input_size )
        PLERROR("StackedAutoassociatorsNet::build_costs() - \n"
                "final_module should have an output_size of %d.\n",
                final_cost->input_size);

    final_module->setLearningRate( fine_tuning_learning_rate );

    if( !(final_module->random_gen) )
    {
        final_module->random_gen = random_gen;
        final_module->forget();
    }


    if(targetsize_ != 1)
        PLERROR("StackedAutoassociatorsNet::build_costs() - \n"
                "target size of %d is not supported.\n", targetsize_);

    if(partial_costs)
    {

        if( correlation_connections.length() != 0 )
            PLERROR("StackedAutoassociatorsNet::build_costs() - \n"
                    "correlation_connections cannot be used with partial costs.");

        partial_costs_positions.resize(partial_costs.length());
        partial_costs_positions.clear();
        for(int i=0; i<partial_costs.length(); i++)
        {
            if(!partial_costs[i])
                PLERROR("StackedAutoassociatorsNet::build_final_cost() - \n"
                        "partial_costs[%i] should be provided.\n",i);
            if( layers[i+1]->size != partial_costs[i]->input_size )
                PLERROR("StackedAutoassociatorsNet::build_costs() - \n"
                        "partial_costs[%i] should have an input_size of %d.\n",
                        i,layers[i+1]->size);
            if(i==0)
                partial_costs_positions[i] = n_layers-1;
            else
                partial_costs_positions[i] = partial_costs_positions[i-1]
                    + partial_costs[i-1]->costNames().length();

            if( !(partial_costs[i]->random_gen) )
            {
                partial_costs[i]->random_gen = random_gen;
                partial_costs[i]->forget();
            }
        }
    }
}

void StackedAutoassociatorsNet::build()
{
    inherited::build();
    build_();
}


void StackedAutoassociatorsNet::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // deepCopyField(, copies);

    // Public options
    deepCopyField(training_schedule, copies);
    deepCopyField(layers, copies);
    deepCopyField(reconstruction_layers, copies);
    deepCopyField(connections, copies);
    deepCopyField(reconstruction_connections, copies);
    deepCopyField(correlation_connections, copies);
    deepCopyField(direct_connections, copies);
    deepCopyField(final_module, copies);
    deepCopyField(final_cost, copies);
    deepCopyField(partial_costs, copies);
    deepCopyField(partial_costs_weights, copies);
    deepCopyField(greedy_target_connections, copies);

    // Protected options
    deepCopyField(activations, copies);
    deepCopyField(activations_m, copies);
    deepCopyField(expectations, copies);
    deepCopyField(expectations_m, copies);
    deepCopyField(doubled_expectations, copies);
    deepCopyField(activation_gradients, copies);
    deepCopyField(activation_gradients_m, copies);
    deepCopyField(expectation_gradients, copies);
    deepCopyField(doubled_expectation_gradients, copies);
    deepCopyField(expectation_gradients_m, copies);
    deepCopyField(reconstruction_activations, copies);
    deepCopyField(reconstruction_activations_m, copies);
    deepCopyField(reconstruction_activation_gradients, copies);
    deepCopyField(reconstruction_activation_gradients_m, copies);
    deepCopyField(reconstruction_expectation_gradients, copies);
    deepCopyField(reconstruction_expectation_gradients_m, copies);
    deepCopyField(fine_tuning_reconstruction_activations, copies);
    deepCopyField(fine_tuning_reconstruction_expectations, copies);
    deepCopyField(fine_tuning_reconstruction_activation_gradients, copies);
    deepCopyField(fine_tuning_reconstruction_expectation_gradients, copies);
    deepCopyField(reconstruction_activation_gradients_from_hid_rec, copies);
    deepCopyField(reconstruction_expectation_gradients_from_hid_rec, copies);
    deepCopyField(hidden_reconstruction_activations, copies);
    deepCopyField(hidden_reconstruction_activation_gradients, copies);
    deepCopyField(correlation_activations, copies);
    deepCopyField(correlation_activations_m, copies);
    deepCopyField(correlation_expectations, copies);
    deepCopyField(correlation_expectations_m, copies);
    deepCopyField(correlation_activation_gradients, copies);
    deepCopyField(correlation_activation_gradients_m, copies);
    deepCopyField(correlation_expectation_gradients, copies);
    deepCopyField(correlation_expectation_gradients_m, copies);
    deepCopyField(correlation_layers, copies);
    deepCopyField(direct_activations, copies);
    deepCopyField(direct_and_reconstruction_activations, copies);
    deepCopyField(direct_and_reconstruction_activation_gradients, copies);
    deepCopyField(partial_costs_positions, copies);
    deepCopyField(partial_cost_value, copies);
    deepCopyField(partial_cost_values, copies);
    deepCopyField(partial_cost_values_0, copies);
    deepCopyField(final_cost_input, copies);
    deepCopyField(final_cost_inputs, copies);
    deepCopyField(final_cost_value, copies);
    deepCopyField(final_cost_values, copies);
    deepCopyField(final_cost_values_0, copies);
    deepCopyField(final_cost_gradient, copies);
    deepCopyField(final_cost_gradients, copies);
    deepCopyField(corrupted_autoassociator_expectations, copies);
    deepCopyField(second_corrupted_autoassociator_expectations, copies);
    deepCopyField(reconstruction_weights, copies);
    deepCopyField(binary_masks, copies);
    deepCopyField(tmp_mask, copies);
    deepCopyField(autoassociator_expectation_indices, copies);
    deepCopyField(expectation_means, copies);
    deepCopyField(target_vec, copies);
    deepCopyField(target_vec_gradient, copies);
    deepCopyField(targets_vec, copies);
    deepCopyField(targets_vec_gradient, copies);
    deepCopyField(greedy_stages, copies);
}


int StackedAutoassociatorsNet::outputsize() const
{
    if(currently_trained_layer < n_layers)
        return layers[currently_trained_layer]->size;
    return final_module->output_size;
}

void StackedAutoassociatorsNet::forget()
{
    //! (Re-)initialize the PLearner in its fresh state (that state may depend
    //! on the 'seed' option) and sets 'stage' back to 0 (this is the stage of
    //! a fresh learner!)
    /*!
      A typical forget() method should do the following:
      - call inherited::forget() to initialize its random number generator
        with the 'seed' option
      - initialize the learner's parameters, using this random generator
      - stage = 0
    */
    inherited::forget();

    for( int i=0 ; i<n_layers ; i++ )
        layers[i]->forget();

    for( int i=0 ; i<n_layers-1 ; i++ )
    {
        reconstruction_layers[i]->forget();
        connections[i]->forget();
        reconstruction_connections[i]->forget();
    }

    final_module->forget();
    final_cost->forget();

    for( int i=0 ; i<partial_costs.length() ; i++ )
        if( partial_costs[i] )
            partial_costs[i]->forget();

    if(correlation_connections.length() != 0)
    {
        for( int i=0 ; i<n_layers-1 ; i++)
        {
            correlation_connections[i]->forget();
            correlation_layers[i]->forget();
        }
    }

    if(direct_connections.length() != 0)
    {
        for( int i=0 ; i<n_layers-1 ; i++)
            direct_connections[i]->forget();
    }

    for( int i=0; i<greedy_target_connections.length(); i++ )
        greedy_target_connections[i]->forget();

    stage = 0;
    unsupervised_stage = 0;
    greedy_stages.clear();
}

void StackedAutoassociatorsNet::train()
{
    Profiler::pl_profile_start("StackedAutoassociatorsNet::train");
    MODULE_LOG << "train() called " << endl;
    MODULE_LOG << "  training_schedule = " << training_schedule << endl;

    minibatch_size = batch_size > 0 ? batch_size : train_set->length();
    int n_train_stats_samples = (train_stats_window >= 0)
        ? train_stats_window
        : train_set->length();

    Vec input(inputsize());
    Mat inputs(minibatch_size, inputsize());
    Vec target(targetsize());
    Mat targets(minibatch_size, inputsize());
    real weight; // unused
    Vec weights(minibatch_size);

    TVec<string> train_cost_names = getTrainCostNames();
    Vec train_costs(train_cost_names.length(), MISSING_VALUE);
    Mat train_costs_m(minibatch_size, train_cost_names.length(),
                      MISSING_VALUE);

    int nsamples = train_set->length();
    int sample;

    PP<ProgressBar> pb;

    if( !train_stats )
    {
        train_stats = new VecStatsCollector();
        train_stats->setFieldNames(train_cost_names);
    }

    // clear stats of previous epoch
    train_stats->forget();

    real lr = 0;
    int init_stage;

    if( !online )
    {
        Profiler::pl_profile_start("StackedAutoassociatorsNet::train !online");

        /***** initial greedy training *****/
        Profiler::pl_profile_start("StackedAutoassociatorsNet::train greedy");
        for( int i=0 ; i<n_layers-1 ; i++ )
        {
            MODULE_LOG << "Training connection weights between layers " << i
                       << " and " << i+1 << endl;

            int end_stage = training_schedule[i];
            int* this_stage = greedy_stages.subVec(i,1).data();
            init_stage = *this_stage;

            MODULE_LOG << "  stage = " << *this_stage << endl;
            MODULE_LOG << "  end_stage = " << end_stage << endl;
            MODULE_LOG << "  greedy_learning_rate = " << greedy_learning_rate << endl;

            if( *this_stage == 0 && noise_type == "masking_noise" && mask_with_mean )
            {
                Vec in(inputsize());
                Vec tar(train_set->targetsize());
                real w;
                expectation_means.resize(n_layers-1);
                expectation_means[i].resize(expectations[i].length());
                expectation_means[i].clear();
                for( int l = 0; l<train_set->length(); l++ )
                {
                    train_set->getExample(l, in, tar, w);
                    // Get representation
                    expectations[0] << in;
                    if(correlation_connections.length() != 0)
                    {
                        for( int j=0 ; j<i; j++ )
                        {
                            connections[j]->fprop( expectations[j], correlation_activations[j] );
                            layers[j+1]->fprop( correlation_activations[j],
                                                correlation_expectations[j] );
                            correlation_connections[j]->fprop( correlation_expectations[j],
                                                               activations[j+1] );
                            correlation_layers[j]->fprop( activations[j+1],
                                                          expectations[j+1] );
                        }
                    }
                    else
                    {
                        for( int j=0 ; j<i; j++ )
                        {
                            connections[j]->fprop( expectations[j], activations[j+1] );
                            layers[j+1]->fprop(activations[j+1],expectations[j+1]);
                        }
                    }

                    expectation_means[i] += expectations[i];
                }
                expectation_means[i] /= train_set->length();
            }

            if( report_progress && *this_stage < end_stage )
                pb = new ProgressBar( "Training layer "+tostring(i)
                                      +" of "+classname(),
                                      end_stage - init_stage );

            train_costs.fill(MISSING_VALUE);
            lr = greedy_learning_rate;
            layers[i]->setLearningRate( lr );
            reconstruction_layers[i]->setLearningRate( lr );
            connections[i]->setLearningRate( lr );
            reconstruction_connections[i]->setLearningRate( lr );
            if(correlation_connections.length() != 0)
            {
                correlation_connections[i]->setLearningRate( lr );
                correlation_layers[i]->setLearningRate( lr );
            }
            if(direct_connections.length() != 0)
            {
                direct_connections[i]->setLearningRate( lr );
            }
            if( greedy_target_connections.length() && greedy_target_connections[i] )
                greedy_target_connections[i]->setLearningRate( lr );
            layers[i+1]->setLearningRate( lr );
            if(partial_costs.length() != 0 && partial_costs[i])
                        partial_costs[i]->setLearningRate( lr );

            // Make sure that storage not null, will be resized anyways by bprop calls
            reconstruction_activations.resize(layers[i]->size);
            reconstruction_activations_m.resize(minibatch_size,
                                                layers[i]->size);
            reconstruction_activation_gradients.resize(layers[i]->size);
            reconstruction_activation_gradients_m.resize(minibatch_size,
                                                         layers[i]->size);
            reconstruction_expectation_gradients.resize(layers[i]->size);
            reconstruction_expectation_gradients_m.resize(minibatch_size,
                                                          layers[i]->size);

            if(reconstruct_hidden)
            {
                reconstruction_activation_gradients_from_hid_rec.resize(
                    layers[i+1]->size);
                reconstruction_expectation_gradients_from_hid_rec.resize(
                    layers[i+1]->size);
                hidden_reconstruction_activations.resize(layers[i+1]->size);
                hidden_reconstruction_activation_gradients.resize(layers[i+1]->size);
            }

            if(direct_connections.length() != 0)
            {
                direct_activations.resize(layers[i]->size);
                direct_and_reconstruction_activations.resize(layers[i]->size);
                direct_and_reconstruction_activation_gradients.resize(layers[i]->size);
            }

            if( keep_online_representations )
            {
                train_representations.resize(end_stage-(*this_stage));
                train_representations.clear();
            }
            int greedyBatchSize = end_stage - (*this_stage);
            string old_noise_type = noise_type;
            for( ; *this_stage<end_stage ; (*this_stage)++ )
            {
                if( !fast_exact_is_equal( greedy_decrease_ct , 0 ) )
                {
                    lr = greedy_learning_rate/(1 + greedy_decrease_ct
                                               * (*this_stage));
                    layers[i]->setLearningRate( lr );
                    reconstruction_layers[i]->setLearningRate( lr );
                    connections[i]->setLearningRate( lr );
                    reconstruction_connections[i]->setLearningRate( lr );
                    layers[i+1]->setLearningRate( lr );
                    if(correlation_connections.length() != 0)
                    {
                        correlation_connections[i]->setLearningRate( lr );
                        correlation_layers[i]->setLearningRate( lr );
                    }
                    if(direct_connections.length() != 0)
                    {
                        direct_connections[i]->setLearningRate( lr );
                    }
                    if(partial_costs.length() != 0 && partial_costs[i])
                        partial_costs[i]->setLearningRate( lr );
                    if( greedy_target_connections.length() && greedy_target_connections[i] )
                        greedy_target_connections[i]->setLearningRate( lr );
                }
                int train_representations_i = 0;
                sample = *this_stage % nsamples;
                train_set->getExample(sample, input, target, weight);
                if( keep_online_representations )
                {
                    train_representations_i = greedyBatchSize - (end_stage-(*this_stage));
                    train_representations[train_representations_i].resize(layers[i+1]->size);
                }
                if( noisy >= 1 )
                {
                    corrupt_input( input, second_corrupted_autoassociator_expectations[0], 0 );
                    noise_type = "none";
                    greedyStep( second_corrupted_autoassociator_expectations[0], target, i, train_costs, train_representations[train_representations_i]);
                    noise_type = old_noise_type;
                }
                else
                    greedyStep( input, target, i, train_costs, train_representations[train_representations_i]);
                
                train_stats->update( train_costs );

                if( pb )
                    pb->update( *this_stage - init_stage + 1 );
            }
        }
        Profiler::pl_profile_end("StackedAutoassociatorsNet::train greedy");

        /***** unsupervised fine-tuning by gradient descent *****/
        if( unsupervised_stage < unsupervised_nstages )
        {
            Profiler::pl_profile_start("StackedAutoassociatorsNet::train unsupervised");

//            if( unsupervised_nstages > 0 && correlation_connections.length() != 0 )
//                PLERROR("StackedAutoassociatorsNet::train()"
//                        " - \n"
//                        "cannot use unsupervised fine-tuning with correlation connections.\n");

            MODULE_LOG << "Unsupervised fine-tuning all parameters, ";
            MODULE_LOG << "by gradient descent" << endl;
            MODULE_LOG << "  unsupervised_stage = " << unsupervised_stage << endl;
            MODULE_LOG << "  unsupervised_nstages = " <<
                unsupervised_nstages << endl;
            MODULE_LOG << "  unsupervised_fine_tuning_learning_rate = " <<
                unsupervised_fine_tuning_learning_rate << endl;

            init_stage = unsupervised_stage;
            if( report_progress && unsupervised_stage < unsupervised_nstages )
                pb = new ProgressBar( "Fine-tuning parameters of all layers of "
                                      + classname(),
                                      unsupervised_nstages - init_stage );

            fine_tuning_reconstruction_activations.resize( n_layers );
            fine_tuning_reconstruction_expectations.resize( n_layers );
            fine_tuning_reconstruction_activation_gradients.resize( n_layers );
            fine_tuning_reconstruction_expectation_gradients.resize( n_layers );
            for( int i=0 ; i<n_layers ; i++ )
            {
                fine_tuning_reconstruction_activations[i].resize(
                    layers[i]->size );
                fine_tuning_reconstruction_expectations[i].resize(
                    layers[i]->size );
                fine_tuning_reconstruction_activation_gradients[i].resize(
                    layers[i]->size );
                fine_tuning_reconstruction_expectation_gradients[i].resize(
                    layers[i]->size );
            }

            setLearningRate( unsupervised_fine_tuning_learning_rate );
            train_costs.fill(MISSING_VALUE);
            string old_noise_type = noise_type;
            for( ; unsupervised_stage<unsupervised_nstages ; unsupervised_stage++ )
            {
                sample = unsupervised_stage % nsamples;
                if( !fast_exact_is_equal( unsupervised_fine_tuning_decrease_ct, 0. ) )
                    setLearningRate(
                        unsupervised_fine_tuning_learning_rate
                        / (1. + unsupervised_fine_tuning_decrease_ct
                           * unsupervised_stage ) );

                train_set->getExample( sample, input, target, weight );
                if( noisy >= 1)
                {
                    corrupt_input( input, second_corrupted_autoassociator_expectations[0], 0 );
                    noise_type = "none";
                    unsupervisedFineTuningStep(second_corrupted_autoassociator_expectations[0], target, train_costs );
                    noise_type = old_noise_type;
                }
                else
                    unsupervisedFineTuningStep( input, target, train_costs );
                train_stats->update( train_costs );

                if( pb )
                    pb->update( unsupervised_stage - init_stage + 1 );
            }
            Profiler::pl_profile_end("StackedAutoassociatorsNet::train unsupervised");
        }

        if( save_learner_before_fine_tuning )
        {
            if( learnerExpdir == "" )
                PLWARNING("StackedAutoassociatorsNet::train() - \n"
                    "cannot save model before fine-tuning because\n"
                    "no experiment directory has been set.");
            else
                PLearn::save(learnerExpdir + "/learner_before_finetuning.psave",*this);
        }

        /***** fine-tuning by gradient descent *****/
        if( stage < nstages )
        {
            Profiler::pl_profile_start("StackedAutoassociatorsNet::train supervised");

            MODULE_LOG << "Fine-tuning all parameters, by gradient descent" << endl;
            MODULE_LOG << "  stage = " << stage << endl;
            MODULE_LOG << "  nstages = " << nstages << endl;
            MODULE_LOG << "  fine_tuning_learning_rate = " <<
                fine_tuning_learning_rate << endl;

            init_stage = stage;
            if( report_progress && stage < nstages )
                pb = new ProgressBar( "Fine-tuning parameters of all layers of "
                                      + classname(),
                                      nstages - init_stage );

            setLearningRate( fine_tuning_learning_rate );
            train_costs.fill(MISSING_VALUE);
            for( ; stage<nstages ; stage++ )
            {
                sample = stage % nsamples;
                if( !fast_exact_is_equal( fine_tuning_decrease_ct, 0. ) )
                    setLearningRate( fine_tuning_learning_rate
                                     / (1. + fine_tuning_decrease_ct * stage ) );

                train_set->getExample( sample, input, target, weight );
                if( noisy >= 2)
                {
                    corrupt_input( input, second_corrupted_autoassociator_expectations[0], 0 );
                    fineTuningStep( second_corrupted_autoassociator_expectations[0], target, train_costs );
                }
                else
                    fineTuningStep( input, target, train_costs );
                train_stats->update( train_costs );

                if( pb )
                    pb->update( stage - init_stage + 1 );
            }
        }

        train_stats->finalize();
        MODULE_LOG << "  train costs = " << train_stats->getMean() << endl;

        // Update currently_trained_layer
        if(stage > 0)
            currently_trained_layer = n_layers;
        else
        {
            currently_trained_layer = n_layers-1;
            while(currently_trained_layer>1
                  && greedy_stages[currently_trained_layer-1] <= 0)
                currently_trained_layer--;
        }
        Profiler::pl_profile_end("StackedAutoassociatorsNet::train !online");
        Profiler::pl_profile_end("StackedAutoassociatorsNet::train supervised");
    }
    else // online==true
    {
        Profiler::pl_profile_start("StackedAutoassociatorsNet::train online");

        if( unsupervised_nstages > 0 )
            PLERROR("StackedAutoassociatorsNet::train()"
                    " - \n"
                    "unsupervised fine-tuning with online=true is not implemented.\n");

        // Train all layers simultaneously AND fine-tuning as well!
        if( stage < nstages )
        {

            MODULE_LOG << "Training all layers greedy layer-wise AND "
                       << "fine-tuning all parameters, by gradient descent"
                       << endl;
            MODULE_LOG << "  stage = " << stage << endl;
            MODULE_LOG << "  nstages = " << nstages << endl;
            MODULE_LOG << "  fine_tuning_learning_rate = "
                       << fine_tuning_learning_rate << endl;
            MODULE_LOG << "  greedy_learning_rate = "
                       << greedy_learning_rate << endl;

            init_stage = stage;
            if( report_progress && stage < nstages )
                pb = new ProgressBar(
                    "Greedy layer-wise training AND fine-tuning parameters of "
                                      + classname(),
                                      nstages - init_stage );

            setLearningRate( fine_tuning_learning_rate );
            train_costs.fill(MISSING_VALUE);
            for( ; stage<nstages ; stage++ )
            {
                // Do a step every 'minibatch_size' examples
                if (stage % minibatch_size == 0)
                {
                    sample = stage % nsamples;
                    if( !fast_exact_is_equal(fine_tuning_decrease_ct, 0.) )
                        setLearningRate(fine_tuning_learning_rate
                                        /(1. + fine_tuning_decrease_ct*stage));

                    if (minibatch_size > 1 || minibatch_hack)
                    {
                        train_set->getExamples(sample, minibatch_size,
                                               inputs, targets, weights,
                                               NULL, true );
                        onlineStep(inputs, targets, train_costs_m);
                    }
                    else
                    {
                        train_set->getExample(sample, input, target, weight);
                        onlineStep(input, target, train_costs);
                    }

                    // Update stats if we are in the last n_train_stats_samples
                    if (stage >= nstages - n_train_stats_samples){
                        if (minibatch_size > 1 || minibatch_hack)
                            for (int k = 0; k < minibatch_size; k++)
                                train_stats->update(train_costs_m(k));
                        else
                            train_stats->update(train_costs);
                    }
                }

                if (pb)
                    pb->update(stage - init_stage + 1);
            }
        }
        Profiler::pl_profile_end("StackedAutoassociatorsNet::train online");

    }
    Profiler::pl_profile_end("StackedAutoassociatorsNet::train");
}

void StackedAutoassociatorsNet::corrupt_input(const Vec& input, Vec& corrupted_input, int layer)
{
    tmp_mask.resize(input.length());
    corrupt_input(input,corrupted_input,layer,tmp_mask);
}

void StackedAutoassociatorsNet::corrupt_input(const Vec& input, Vec& corrupted_input, int layer, Vec& binary_mask)
{
    binary_mask.fill(1);
    corrupted_input.resize(input.length());
    reconstruction_weights.resize(input.length());
    reconstruction_weights.fill(1);

    if( (mask_input_layer_only && layer != 0) ||
         (!mask_input_layer_only && layer > (nb_corrupted_layer-1)) )
    {
        corrupted_input << input;
        return;
    }

    if( noise_type == "masking_noise" )
    {
        if( probability_of_masked_inputs > 0 )
        {
            if( fraction_of_masked_inputs > 0 )
                PLERROR("In StackedAutoassociatorsNet::corrupt_input():" 
                        " fraction_of_masked_inputs and probability_of_masked_inputs can't be both > 0");
            if( mask_with_pepper_salt )
            {
                real pepVal = 0;
                real saltVal = 1;
                if( pep_salt_zero_centered>0. )
                {
                    pepVal = -pep_salt_zero_centered;
                    saltVal = pep_salt_zero_centered;
                }
                for( int j=0 ; j <input.length() ; j++)
                {
                    if( random_gen->uniform_sample() < probability_of_masked_inputs )
                    {
                        corrupted_input[ j ] = random_gen->bounded_sample(prob_salt_noise,pepVal,saltVal);
                        reconstruction_weights[j] = corrupted_data_weight;
                    }
                    else
                    {
                        corrupted_input[ j ] = input[ j ];  
                        reconstruction_weights[j] = data_weight;
                    }
                }
            }       
            else if( mask_with_mean )
            {
                for( int j=0 ; j <input.length() ; j++)
                {
                    if( random_gen->uniform_sample() < probability_of_masked_inputs )
                    {                    
                        corrupted_input[ j ] = expectation_means[layer][ j ];
                        reconstruction_weights[j] = corrupted_data_weight;
                        binary_mask[ j ] = 0;
                    }
                    else
                    {
                        corrupted_input[ j ] = input[ j ];
                        reconstruction_weights[j] = data_weight;
                    }
                }
            }
            else
            {
                for( int j=0 ; j <input.length() ; j++)
                {
                    if( random_gen->uniform_sample() < probability_of_masked_inputs )
                    {
                        corrupted_input[ j ] = 0;   
                        reconstruction_weights[j] = corrupted_data_weight;
                        binary_mask[ j ] = 0;
                    }
                    else
                    {
                        corrupted_input[ j ] = input[ j ];
                        reconstruction_weights[j] = data_weight;
                    }
                }
            }
        }
        else
        {
            corrupted_input << input;
            reconstruction_weights.fill(data_weight);
            if( fraction_of_masked_inputs > 0. ) 
            {
                random_gen->shuffleElements(autoassociator_expectation_indices[layer]);
                if( mask_with_pepper_salt )
                {
                    real pepVal = 0;
                    real saltVal = 1;
                    if( pep_salt_zero_centered>0. )
                    {
                        pepVal = -pep_salt_zero_centered;
                        saltVal = pep_salt_zero_centered;
                    }
                    for( int j=0 ; j < round(fraction_of_masked_inputs*input.length()) ; j++)
                    {
                        corrupted_input[ autoassociator_expectation_indices[layer][j] ] = random_gen->bounded_sample(prob_salt_noise,pepVal,saltVal);
                        reconstruction_weights[autoassociator_expectation_indices[layer][j]] = corrupted_data_weight;
                    }
                }   
                else if( mask_with_mean )
                {
                    for( int j=0 ; j < round(fraction_of_masked_inputs*input.length()) ; j++)
                    {
                        corrupted_input[ autoassociator_expectation_indices[layer][j] ] = expectation_means[layer][autoassociator_expectation_indices[layer][j]];
                        reconstruction_weights[autoassociator_expectation_indices[layer][j]] = corrupted_data_weight;
                        binary_mask[ autoassociator_expectation_indices[layer][j] ] = 0;
                    }
                }
                else
                {
                    for( int j=0 ; j < round(fraction_of_masked_inputs*input.length()) ; j++)
                    {
                        corrupted_input[ autoassociator_expectation_indices[layer][j] ] = 0;
                        reconstruction_weights[autoassociator_expectation_indices[layer][j]] = corrupted_data_weight;
                        binary_mask[ autoassociator_expectation_indices[layer][j] ] = 0;
                    }
                }
            }
        }
    }
    else if( noise_type == "binary_sampling" )
        for( int i=0; i<corrupted_input.length(); i++ )
            corrupted_input[i] = random_gen->binomial_sample((input[i]-0.5)*binary_sampling_noise_parameter+0.5);
    else if( noise_type == "gaussian" )
        for( int i=0; i<corrupted_input.length(); i++ )
            corrupted_input[i] = input[i] + random_gen->gaussian_01() * gaussian_std;
    else if( noise_type == "missing_data")
    {
        // The entry input is the doubled one according to missing_data_method
        int original_input_length = input.length() / 2;
        reconstruction_weights.resize(original_input_length);
   
        if(missing_data_method == "binomial_complementary" || 
           missing_data_method == "one_if_missing")
        {
            int down_missing_value = 0;
            int up_missing_value = 0;
        
            if(missing_data_method == "one_if_missing")
                up_missing_value = 1;

            if( probability_of_masked_inputs > 0 )
            {
                if( fraction_of_masked_inputs > 0 )
                    PLERROR("In StackedAutoassociatorsNet::corrupt_input():"
                            " fraction_of_masked_inputs and probability_of_masked_inputs can't be both > 0");
                for( int j=0 ; j<original_input_length ; j++ )
                    if( random_gen->uniform_sample() < probability_of_masked_inputs )
                    {
                        corrupted_input[ j*2 ] = down_missing_value;
                        corrupted_input[ j*2+1 ] = up_missing_value;
                        reconstruction_weights[j] = corrupted_data_weight;
                    }
                    else
                    {
                        corrupted_input[ j*2 ] = input[ j*2 ];
                        corrupted_input[ j*2+1] = input[ j*2+1 ];
                        reconstruction_weights[j] = data_weight;
                    }
            }
            else
            {
                corrupted_input << input;
                reconstruction_weights.fill(data_weight);
                if( fraction_of_masked_inputs > 0. )
                {
                    random_gen->shuffleElements(autoassociator_expectation_indices[layer]);
                    for( int j=0 ; j < round(fraction_of_masked_inputs*original_input_length) ; j++)
                    {
                        corrupted_input[ autoassociator_expectation_indices[layer][j]*2 ] = down_missing_value;
                        corrupted_input[ autoassociator_expectation_indices[layer][j]*2 + 1 ] = up_missing_value;
                        reconstruction_weights[autoassociator_expectation_indices[layer][j]] = corrupted_data_weight;
                    }
                }
            }
        }
        else
            PLERROR("In StackedAutoassociatorsNet::corrupt_input(): "
                    "missing_data_method %s not valid with noise_type %s",
                     missing_data_method.c_str(), noise_type.c_str());
    }
    else if( noise_type == "none" )
        corrupted_input << input;
    else
        PLERROR("In StackedAutoassociatorsNet::corrupt_input(): noise_type %s not valid", noise_type.c_str());
}


// ***** binomial_complementary ******
// doubled_input[2*i] = input[i] and
// doubled input[2*i+1] = 1-input[i]
// If input is gradient that we have to double for backpropagation
// (double_grad==true), then:
// doubled_input[2*i] = input[i] and
// oubled input[2*i+1] = -input[i]
// ********** one_if_missing *********
// doubled_input[2*i] = input[i] and
// doubled input[2*i+1] = 0 (gradian or not)
void StackedAutoassociatorsNet::double_input(const Vec& input, Vec& doubled_input, bool double_grad) const
{
    if( noise_type == "missing_data" )
    {
        doubled_input.resize(input.length()*2);
        for( int i=0; i<input.size(); i++ )
        {
            doubled_input[i*2] = input[i];
            if( missing_data_method == "binomial_complementary")
            {
                if( double_grad )
                    doubled_input[i*2+1] = - input[i];
                else
                    doubled_input[i*2+1] = 1 - input[i];
            }
            else if( missing_data_method == "one_if_missing" )
                doubled_input[i*2+1] = 0;
            else
                PLERROR("In StackedAutoassociatorsNet::double_input(): "
                "missing_data_method %s not valid",missing_data_method.c_str());
        }
    }
    else
    {
        doubled_input.resize(input.length());
        doubled_input << input;
    }
}

// ***** binomial_complementary *****
// divided_input[i] = input[2*i] - input[2*i+1]
// even if input is the doubled_gradient
// ********** one_if_missing *********
// divided_input[i] = input[2*i]
void StackedAutoassociatorsNet::divide_input(const Vec& input, Vec& divided_input) const
{
    if( noise_type == "missing_data" )
    {
        divided_input.resize(input.length()/2);
        for( int i=0; i<divided_input.size(); i++ )  
        {
            if( missing_data_method == "binomial_complementary" )
                divided_input[i] = input[i*2] - input[i*2+1];
            else if( missing_data_method == "one_if_missing" )
                divided_input[i] = input[i*2];
            else
                PLERROR("In StackedAutoassociatorsNet::divide_input(): "
                        "missing_data_method %s not valid", missing_data_method.c_str());
        }
    }
    else
    {
        divided_input.resize(input.length());
        divided_input << input;
    }
}


void StackedAutoassociatorsNet::greedyStep(const Vec& input, const Vec& target,
                                           int index, Vec train_costs, Vec& representation)
{
    Profiler::pl_profile_start("StackedAutoassociatorsNet::greedyStep");
    PLASSERT( index < n_layers );

    expectations[0] << input;

    if(correlation_connections.length() != 0)
    {
        for( int i=0 ; i<index + 1; i++ )
        {
            if( i == index )
            {
                corrupt_input( expectations[i], corrupted_autoassociator_expectations[i], i );
                connections[i]->fprop( corrupted_autoassociator_expectations[i], 
                                       correlation_activations[i] );
            }
            else
                connections[i]->fprop( expectations[i], correlation_activations[i] );

            if( i == index && greedy_target_connections.length() && greedy_target_connections[i] )
            {
                target_vec.clear();
                if( probability_of_masked_target == 0. ||
                    random_gen->uniform_sample() >= probability_of_masked_target )
                    target_vec[(int)target[0]] = 1;

                greedy_target_connections[i]->setAsDownInput(target_vec);
                greedy_target_connections[i]->computeProduct(0, correlation_activations[i].length(),
                                                             correlation_activations[i], true);
            }

            layers[i+1]->fprop( correlation_activations[i],
                                correlation_expectations[i] );
            correlation_connections[i]->fprop( correlation_expectations[i],
                                               activations[i+1] );

            correlation_layers[i]->fprop( activations[i+1],
                                          expectations[i+1] );
        }
    }
    else
    {
        for( int i=0 ; i<index + 1; i++ )
        {
            double_input(expectations[i], doubled_expectations[i]);
           
            if( i == index )
            {
                corrupt_input( doubled_expectations[i], corrupted_autoassociator_expectations[i], i );
                connections[i]->fprop( corrupted_autoassociator_expectations[i], activations[i+1] );
            }
            else
                connections[i]->fprop( doubled_expectations[i], activations[i+1] );
            
            if( i == index && greedy_target_connections.length() && greedy_target_connections[i] )
            {
                target_vec.clear();
                if( probability_of_masked_target == 0. ||
                    random_gen->uniform_sample() >= probability_of_masked_target )
                    target_vec[(int)target[0]] = 1;

                greedy_target_connections[i]->setAsDownInput(target_vec);
                greedy_target_connections[i]->computeProduct(0, activations[i+1].length(),
                                                             activations[i+1], true);
            }

            layers[i+1]->fprop(activations[i+1],expectations[i+1]);
            if( keep_online_representations )
                representation << expectations[i+1];
        }
    }


    if( partial_costs && partial_costs[ index ] )
    {
        partial_costs[ index ]->fprop( expectations[ index + 1],
                                       target, partial_cost_value );

        // Update partial cost (might contain some weights for example)
        partial_costs[ index ]->bpropUpdate( expectations[ index + 1 ],
                                             target, partial_cost_value[0],
                                             expectation_gradients[ index + 1 ]
                                             );

        train_costs.subVec(partial_costs_positions[index]+1,
                           partial_cost_value.length()) << partial_cost_value;

        if( !fast_exact_is_equal( partial_costs_weights.length(), 0 ) )
            expectation_gradients[ index + 1 ] *= partial_costs_weights[index];

        // Update hidden layer bias and weights
        layers[ index+1 ]->bpropUpdate( activations[ index + 1 ],
                                        expectations[ index + 1 ],
                                        activation_gradients[ index + 1 ],
                                        expectation_gradients[ index + 1 ] );

        Profiler::pl_profile_start("StackedAutoassociatorsNet::greedyStep bprop connection");
        connections[ index ]->bpropUpdate( corrupted_autoassociator_expectations[index],
                                           activations[ index + 1 ],
                                           expectation_gradients[ index ],
                                           activation_gradients[ index + 1 ] );
        Profiler::pl_profile_end("StackedAutoassociatorsNet::greedyStep bprop connection");
    }

    reconstruction_connections[ index ]->fprop( expectations[ index + 1],
                                                reconstruction_activations);
    if(direct_connections.length() != 0)
    {
        direct_connections[ index ]->fprop( corrupted_autoassociator_expectations[index],
                                            direct_activations );
        direct_and_reconstruction_activations.clear();
        direct_and_reconstruction_activations += direct_activations;
        direct_and_reconstruction_activations += reconstruction_activations;

        reconstruction_layers[ index ]->fprop( direct_and_reconstruction_activations,
                                reconstruction_layers[ index ]->expectation);

        reconstruction_layers[ index ]->activation << direct_and_reconstruction_activations;
        reconstruction_layers[ index ]->activation += reconstruction_layers[ index ]->bias;
        //reconstruction_layers[ index ]->expectation_is_up_to_date = true;  // Won't work for certain RBMLayers
        reconstruction_layers[ index ]->setExpectationByRef( reconstruction_layers[ index ]->expectation );
        train_costs[index] = reconstruction_layers[ index ]->fpropNLL(expectations[index]);

        reconstruction_layers[ index ]->bpropNLL(expectations[index], train_costs[index],
                                  direct_and_reconstruction_activation_gradients);

        reconstruction_layers[ index ]->update(direct_and_reconstruction_activation_gradients);

        direct_connections[ index ]->bpropUpdate(
            corrupted_autoassociator_expectations[index],
            direct_activations,
            reconstruction_expectation_gradients, // Will be overwritten later
            direct_and_reconstruction_activation_gradients);

        reconstruction_connections[ index ]->bpropUpdate(
            expectations[ index + 1],
            reconstruction_activations,
            reconstruction_expectation_gradients,
            direct_and_reconstruction_activation_gradients);
    }
    else
    {
        Vec divided_reconstruction_activations(reconstruction_activations.size());
        Vec divided_reconstruction_activation_gradients(reconstruction_layers[ index ]->size);
        
        divide_input(reconstruction_activations, divided_reconstruction_activations);

        reconstruction_layers[ index ]->fprop( divided_reconstruction_activations,
                               reconstruction_layers[ index ]->expectation);
        reconstruction_layers[ index ]->activation << divided_reconstruction_activations;
        reconstruction_layers[ index ]->activation += reconstruction_layers[ index ]->bias;
        //reconstruction_layers[ index ]->expectation_is_up_to_date = true;
        reconstruction_layers[ index ]->setExpectationByRef( reconstruction_layers[ index ]->expectation );
        real rec_err;

        // If we want to compute reconstruction error according to reconstruction weights.
        //   rec_err = reconstruction_layers[ index ]->fpropNLL(expectations[index], reconstruction_weights);
       
        if( renoising )
        {
            corrupt_input( expectations[index], second_corrupted_autoassociator_expectations[index], index );
            rec_err = reconstruction_layers[ index ]->fpropNLL(second_corrupted_autoassociator_expectations[index]);
            reconstruction_layers[ index ]->bpropNLL(second_corrupted_autoassociator_expectations[index], rec_err, divided_reconstruction_activation_gradients);
        }
        else
        {
            rec_err = reconstruction_layers[ index ]->fpropNLL(expectations[index]);
            reconstruction_layers[ index ]->bpropNLL(expectations[index], rec_err, divided_reconstruction_activation_gradients);
        }
        train_costs[index] = rec_err;

        // apply reconstruction weights which can be different for corrupted
        // (or missing) and non corrupted data. 
        multiply(reconstruction_weights, 
                divided_reconstruction_activation_gradients, 
                divided_reconstruction_activation_gradients);

        double_input(divided_reconstruction_activation_gradients, 
                    reconstruction_activation_gradients, true);   
 
        if(reconstruct_hidden)
        {
            Profiler::pl_profile_start("StackedAutoassociatorsNet::greedyStep reconstruct_hidden");
            connections[ index ]->fprop( reconstruction_layers[ index ]->expectation,
                                         hidden_reconstruction_activations );
            layers[ index+1 ]->fprop( hidden_reconstruction_activations,
                layers[ index+1 ]->expectation );
            layers[ index+1 ]->activation << hidden_reconstruction_activations;
            layers[ index+1 ]->activation += layers[ index+1 ]->bias;
            //layers[ index+1 ]->expectation_is_up_to_date = true;
            layers[ index+1 ]->setExpectationByRef( layers[ index+1 ]->expectation );
            real hid_rec_err = layers[ index+1 ]->fpropNLL(expectations[index+1]);
            train_costs[index] += hid_rec_err;

            layers[ index+1 ]->bpropNLL(expectations[index+1], hid_rec_err,
                                        hidden_reconstruction_activation_gradients);
            layers[ index+1 ]->update(hidden_reconstruction_activation_gradients);

            Profiler::pl_profile_start("StackedAutoassociatorsNet::greedyStep reconstruct_hidden connection bprop");
            connections[ index ]->bpropUpdate(
                reconstruction_layers[ index ]->expectation,
                hidden_reconstruction_activations,
                reconstruction_expectation_gradients_from_hid_rec,
                hidden_reconstruction_activation_gradients);
            Profiler::pl_profile_end("StackedAutoassociatorsNet::greedyStep reconstruct_hidden connection bprop");

            reconstruction_layers[ index ]->bpropUpdate(
                reconstruction_activations,
                reconstruction_layers[ index ]->expectation,
                reconstruction_activation_gradients_from_hid_rec,
                reconstruction_expectation_gradients_from_hid_rec);
            Profiler::pl_profile_end("StackedAutoassociatorsNet::greedyStep reconstruct_hidden");
        }

        reconstruction_layers[ index ]->update(divided_reconstruction_activation_gradients);
        
        if(reconstruct_hidden)
            reconstruction_activation_gradients +=
                reconstruction_activation_gradients_from_hid_rec;

        // // This is a bad update! Propagates gradient through sigmoid again!
        // reconstruction_layers[ index ]->bpropUpdate( reconstruction_activations,
        //                                   reconstruction_layers[ index ]->expectation,
        //                                   reconstruction_activation_gradients,
        //                                   reconstruction_expectation_gradients);
        reconstruction_connections[ index ]->bpropUpdate(
            expectations[ index + 1],
            reconstruction_activations,
            reconstruction_expectation_gradients,
            reconstruction_activation_gradients);
    }


    if(!fast_exact_is_equal(l1_neuron_decay,0))
    {
        // Compute L1 penalty gradient on neurons
        real* hid = expectations[ index + 1 ].data();
        real* grad = reconstruction_expectation_gradients.data();
        int len = expectations[ index + 1 ].length();
        for(int l=0; l<len; l++)
        {
            if(*hid > l1_neuron_decay_center)
                *grad += l1_neuron_decay;
            else if(*hid < l1_neuron_decay_center)
                *grad -= l1_neuron_decay;
            hid++;
            grad++;
        }
    }

    // Update hidden layer bias and weights

    if(correlation_connections.length() != 0)
    {
        correlation_layers[ index ]->bpropUpdate(
            activations[ index + 1 ],
            expectations[ index + 1 ],
            reconstruction_activation_gradients,  // reused
            reconstruction_expectation_gradients
            );

        correlation_connections[ index ]->bpropUpdate(
            correlation_expectations[ index ],
            activations[ index+1 ],
            correlation_expectation_gradients[ index ],
            reconstruction_activation_gradients);

        layers[ index+1 ]->bpropUpdate(
            correlation_activations[ index ],
            correlation_expectations[ index ],
            correlation_activation_gradients [ index ],
            correlation_expectation_gradients [ index ]);

        connections[ index ]->bpropUpdate(
            corrupted_autoassociator_expectations[index],
            correlation_activations[ index ],
            reconstruction_expectation_gradients, //reused
            correlation_activation_gradients [ index ]);

        if( greedy_target_connections.length() && greedy_target_connections[index] )
        {
            greedy_target_connections[index]->bpropUpdate(
                target_vec, 
                correlation_activations[index],
                target_vec_gradient,
                correlation_activation_gradients [ index ]);
        }
    }
    else
    {
        layers[ index+1 ]->bpropUpdate( activations[ index + 1 ],
                                        expectations[ index + 1 ],
                                        // reused
                                        reconstruction_activation_gradients,
                                        reconstruction_expectation_gradients);

        connections[ index ]->bpropUpdate(
            corrupted_autoassociator_expectations[index],
            activations[ index + 1 ],
            reconstruction_expectation_gradients, //reused
            reconstruction_activation_gradients);
        if( greedy_target_connections.length() && greedy_target_connections[index] )
        {
            greedy_target_connections[index]->bpropUpdate(
                target_vec, 
                activations[ index + 1 ],
                target_vec_gradient,
                reconstruction_activation_gradients);
        }
    }

    Profiler::pl_profile_end("StackedAutoassociatorsNet::greedyStep");
}

void StackedAutoassociatorsNet::greedyStep(const Mat& inputs,
                                           const Mat& targets,
                                           int index, Mat& train_costs)
{
    PLCHECK_MSG(false, "Mini-batch not implemented yet.");
}

void StackedAutoassociatorsNet::unsupervisedFineTuningStep(const Vec& input,
                                                           const Vec& target,
                                                           Vec& train_costs)
{
    // fprop
    expectations[0] << input;

    bool old_mask_input_layer_only = mask_input_layer_only;
    mask_input_layer_only = mask_input_layer_only_in_unsupervised_fine_tuning;

    if(correlation_connections.length() != 0)
    {

        for( int i=0 ; i<n_layers-1; i++ )
        {
            corrupt_input( expectations[i], corrupted_autoassociator_expectations[i], i);
            connections[i]->fprop( corrupted_autoassociator_expectations[i],
                                   correlation_activations[i] );
            layers[i+1]->fprop( correlation_activations[i],
                                correlation_expectations[i] );
            correlation_connections[i]->fprop( correlation_expectations[i],
                                               activations[i+1] );
            correlation_layers[i]->fprop( activations[i+1],
                                          expectations[i+1] );
        }
    }
    else
    {
        for( int i=0 ; i<n_layers-1; i++ )
        {
            corrupt_input( expectations[i], corrupted_autoassociator_expectations[i], i);
            connections[i]->fprop( corrupted_autoassociator_expectations[i],
                                   activations[i+1] );
            layers[i+1]->fprop(activations[i+1],expectations[i+1]);
        }
    }
    fine_tuning_reconstruction_expectations[ n_layers-1 ] <<
        expectations[ n_layers-1 ];

    for( int i=n_layers-2 ; i>=0; i-- )
    {
        reconstruction_connections[i]->fprop(
            fine_tuning_reconstruction_expectations[i+1],
            fine_tuning_reconstruction_activations[i] );
        layers[i]->fprop( fine_tuning_reconstruction_activations[i],
                          fine_tuning_reconstruction_expectations[i]);
    }

    layers[ 0 ]->setExpectation( fine_tuning_reconstruction_expectations[ 0 ] );
    layers[ 0 ]->activation << fine_tuning_reconstruction_activations[0];
    layers[ 0 ]->activation += layers[ 0 ]->bias;
    real rec_err = layers[ 0 ]->fpropNLL( input );
    train_costs[n_layers-1] = rec_err;

    layers[ 0 ]->bpropNLL( input, rec_err,
                           fine_tuning_reconstruction_activation_gradients[ 0 ] );

    layers[ 0 ]->update( fine_tuning_reconstruction_activation_gradients[ 0 ] );

    for( int i=0 ; i<n_layers-1; i++ )
    {
        if( i != 0)
            layers[i]->bpropUpdate( fine_tuning_reconstruction_activations[i],
                                    fine_tuning_reconstruction_expectations[i],
                                    fine_tuning_reconstruction_activation_gradients[i],
                                    fine_tuning_reconstruction_expectation_gradients[i]);
        reconstruction_connections[i]->bpropUpdate(
            fine_tuning_reconstruction_expectations[i+1],
            fine_tuning_reconstruction_activations[i],
            fine_tuning_reconstruction_expectation_gradients[i+1],
            fine_tuning_reconstruction_activation_gradients[i]);
    }

    expectation_gradients[ n_layers-1 ] <<
        fine_tuning_reconstruction_expectation_gradients[ n_layers-1 ];

    for( int i=n_layers-2 ; i>=0; i-- )
    {

        if(!fast_exact_is_equal(l1_neuron_decay,0))
        {
            // Compute L1 penalty gradient on neurons
            real* hid = expectations[ i + 1 ].data();
            real* grad = expectation_gradients[ i + 1 ].data();
            int len = expectations[ i + 1 ].length();
            for(int l=0; l<len; l++)
            {
                if(*hid > l1_neuron_decay_center)
                    *grad += l1_neuron_decay;
                else if(*hid < l1_neuron_decay_center)
                    *grad -= l1_neuron_decay;
                hid++;
                grad++;
            }
        }

        if(correlation_connections.length() != 0)
        {
            correlation_layers[ i ]->bpropUpdate(
                activations[ i + 1 ],
                expectations[ i + 1 ],
                activation_gradients[ i + 1 ],
                expectation_gradients[ i + 1 ]
                );

            correlation_connections[ i ]->bpropUpdate(
                correlation_expectations[ i ],
                activations[ i + 1 ],
                correlation_expectation_gradients[ i ],
                activation_gradients[ i + 1 ] );

            layers[ i + 1 ]->bpropUpdate(
                correlation_activations[ i ],
                correlation_expectations[ i ],
                correlation_activation_gradients [ i ],
                correlation_expectation_gradients [ i ]);

            connections[ i ]->bpropUpdate(
                corrupted_autoassociator_expectations[ i ],
                correlation_activations[ i ],
                expectation_gradients[i],
                correlation_activation_gradients [ i ]);
        }
        else
        {

            layers[i+1]->bpropUpdate(
                activations[i+1],expectations[i+1],
                activation_gradients[i+1],expectation_gradients[i+1]);
            connections[i]->bpropUpdate(
                corrupted_autoassociator_expectations[i], activations[i+1],
                expectation_gradients[i], activation_gradients[i+1] );
        }
    }

    mask_input_layer_only = old_mask_input_layer_only;
}

void StackedAutoassociatorsNet::unsupervisedFineTuningStep(const Mat& inputs,
                                                           const Mat& targets,
                                                           Mat& train_costs)
{
    PLCHECK_MSG(false, "Mini-batch not implemented yet.");
}

void StackedAutoassociatorsNet::fineTuningStep(const Vec& input,
                                               const Vec& target,
                                               Vec& train_costs)
{
    Profiler::pl_profile_start("StackedAutoassociatorsNet::fineTuningStep");
    Profiler::pl_profile_start("StackedAutoassociatorsNet::fineTuningStep fprop");

    // fprop
    expectations[0] << input;

    if(correlation_connections.length() != 0)
    {
        for( int i=0 ; i<n_layers-1; i++ )
        {
            connections[i]->fprop( expectations[i], correlation_activations[i] );
            layers[i+1]->fprop( correlation_activations[i],
                                correlation_expectations[i] );
            correlation_connections[i]->fprop( correlation_expectations[i],
                                               activations[i+1] );
            correlation_layers[i]->fprop( activations[i+1],
                                          expectations[i+1] );
        }
    }
    else
    {
        for( int i=0 ; i<n_layers-1; i++ )
        {
            double_input(expectations[i], doubled_expectations[i]);
            Profiler::pl_profile_start("StackedAutoassociatorsNet::fineTuningStep fprop connection");
            connections[i]->fprop( doubled_expectations[i], activations[i+1] );
            Profiler::pl_profile_end("StackedAutoassociatorsNet::fineTuningStep fprop connection");
            layers[i+1]->fprop(activations[i+1],expectations[i+1]);
        }
    }
    Profiler::pl_profile_end("StackedAutoassociatorsNet::fineTuningStep fprop");
    final_module->fprop( expectations[ n_layers-1 ],
                         final_cost_input );
    final_cost->fprop( final_cost_input, target, final_cost_value );

    train_costs.subVec(train_costs.length()-final_cost_value.length(),
                       final_cost_value.length()) <<
        final_cost_value;

    final_cost->bpropUpdate( final_cost_input, target,
                             final_cost_value[0],
                             final_cost_gradient );
    final_module->bpropUpdate( expectations[ n_layers-1 ],
                               final_cost_input,
                               expectation_gradients[ n_layers-1 ],
                               final_cost_gradient );

    Profiler::pl_profile_start("StackedAutoassociatorsNet::fineTuningStep bpropUpdate");
    if( correlation_connections.length() != 0 )
    {
        for( int i=n_layers-1 ; i>0 ; i-- )
        {
            correlation_layers[i-1]->bpropUpdate(
                activations[i],
                expectations[i],
                activation_gradients[i],
                expectation_gradients[i] );

            correlation_connections[i-1]->bpropUpdate(
                correlation_expectations[i-1],
                activations[i],
                correlation_expectation_gradients[i-1],
                activation_gradients[i] );

            layers[i]->bpropUpdate( correlation_activations[i-1],
                                    correlation_expectations[i-1],
                                    correlation_activation_gradients[i-1],
                                    correlation_expectation_gradients[i-1] );

            connections[i-1]->bpropUpdate( expectations[i-1],
                                           correlation_activations[i-1],
                                           expectation_gradients[i-1],
                                           correlation_activation_gradients[i-1] );
        }
    }
    else
    {   
        for( int i=n_layers-1 ; i>0 ; i-- )
        {
            layers[i]->bpropUpdate( activations[i],
                                    expectations[i],
                                    activation_gradients[i],
                                    expectation_gradients[i] );

            Profiler::pl_profile_start("StackedAutoassociatorsNet::fineTuningStep bpropUpdate connection");
            connections[i-1]->bpropUpdate( doubled_expectations[i-1],
                                           activations[i],
                                           doubled_expectation_gradients[i-1],
                                           activation_gradients[i] );

            Profiler::pl_profile_end("StackedAutoassociatorsNet::fineTuningStep bpropUpdate connection");
            divide_input( doubled_expectation_gradients[i-1], expectation_gradients[i-1] );
        }
    }
    Profiler::pl_profile_end("StackedAutoassociatorsNet::fineTuningStep bpropUpdate");
    Profiler::pl_profile_end("StackedAutoassociatorsNet::fineTuningStep");
}

void StackedAutoassociatorsNet::fineTuningStep(const Mat& inputs,
                                               const Mat& targets,
                                               Mat& train_costs)
{
    PLCHECK_MSG(false, "Mini-batch not implemented yet.");
}



void StackedAutoassociatorsNet::onlineStep(const Vec& input,
                                           const Vec& target,
                                           Vec& train_costs)
{
    real lr;
    // fprop
    expectations[0] << input;

    if(correlation_connections.length() != 0)
    {
        for( int i=0 ; i<n_layers-1; i++ )
        {
            corrupt_input( expectations[i], corrupted_autoassociator_expectations[i], 
                           i, binary_masks[i] );
            connections[i]->fprop( corrupted_autoassociator_expectations[i], 
                                   correlation_activations[i] );

            if( greedy_target_connections.length() && greedy_target_connections[i] )
            {
                targets_vec[i].clear();
                if( probability_of_masked_target == 0. ||
                    random_gen->uniform_sample() >= probability_of_masked_target )
                    targets_vec[i][(int)target[0]] = 1;

                greedy_target_connections[i]->setAsDownInput(targets_vec[i]);
                greedy_target_connections[i]->computeProduct(0, correlation_activations[i].length(),
                                                             correlation_activations[i], true);
            }

            layers[i+1]->fprop( correlation_activations[i],
                                correlation_expectations[i] );
            correlation_connections[i]->fprop( correlation_expectations[i],
                                               activations[i+1] );
            correlation_layers[i]->fprop( activations[i+1],
                                          expectations[i+1] );

        }
    }
    else
    {
        for( int i=0 ; i<n_layers-1; i++ )
        {
            corrupt_input( expectations[i], corrupted_autoassociator_expectations[i], 
                           i, binary_masks[i] );
            connections[i]->fprop( corrupted_autoassociator_expectations[i], 
                                   activations[i+1] );
            
            if( greedy_target_connections.length() && greedy_target_connections[i] )
            {
                targets_vec[i].clear();
                if( probability_of_masked_target == 0. ||
                    random_gen->uniform_sample() >= probability_of_masked_target )
                    targets_vec[i][(int)target[0]] = 1;

                greedy_target_connections[i]->setAsDownInput(targets_vec[i]);
                greedy_target_connections[i]->computeProduct(0, activations[i+1].length(),
                                                             activations[i+1], true);
            }

            layers[i+1]->fprop(activations[i+1],expectations[i+1]);
        }
    }

    // Unsupervised greedy layer-wise cost

    // Set learning rates
    if( !fast_exact_is_equal( greedy_decrease_ct , 0 ) )
        lr = greedy_learning_rate / (1 + greedy_decrease_ct * stage) ;
    else
        lr = greedy_learning_rate;

    for( int i=0 ; i<n_layers-1 ; i++ )
    {
        layers[i]->setLearningRate( lr );
        reconstruction_layers[i]->setLearningRate( lr );
        connections[i]->setLearningRate( lr );
        reconstruction_connections[i]->setLearningRate( lr );
        if(correlation_layers.length() != 0)
        {
            correlation_layers[i]->setLearningRate( lr );
            correlation_connections[i]->setLearningRate( lr );
        }
        if( partial_costs.length() != 0 && partial_costs[ i ] )
        {
            partial_costs[ i ]->setLearningRate( lr );
        }
        if( greedy_target_connections.length() && greedy_target_connections[i] )
            greedy_target_connections[i]->setLearningRate( lr );
    }
    layers[n_layers-1]->setLearningRate( lr );

    // Backpropagate unsupervised gradient, layer-wise
    for( int i=n_layers-1 ; i>0 ; i-- )
    {
        reconstruction_connections[ i-1 ]->fprop(
            expectations[ i ],
            reconstruction_activations);

        reconstruction_layers[ i-1 ]->fprop( reconstruction_activations,
                              reconstruction_layers[ i-1 ]->expectation);

        reconstruction_layers[ i-1 ]->activation << reconstruction_activations;
        reconstruction_layers[ i-1 ]->activation += reconstruction_layers[ i-1 ]->bias;
        //reconstruction_layers[ i-1 ]->expectation_is_up_to_date = true;
        reconstruction_layers[ i-1 ]->setExpectationByRef( reconstruction_layers[ i-1 ]->expectation );
        real rec_err = reconstruction_layers[ i-1 ]->fpropNLL( expectations[i-1] );
        train_costs[i-1] = rec_err;

        reconstruction_layers[ i-1 ]->bpropNLL(expectations[i-1], rec_err,
                                  reconstruction_activation_gradients);

        reconstruction_layers[ i-1 ]->update(reconstruction_activation_gradients);

        reconstruction_connections[ i-1 ]->bpropUpdate(
            expectations[ i ],
            reconstruction_activations,
            reconstruction_expectation_gradients,
            reconstruction_activation_gradients);

        if( partial_costs.length() != 0 && partial_costs[ i-1 ] )
        {
            
            partial_costs[ i-1 ]->fprop( expectations[ i],
                                       target, partial_cost_value );
            
            // Update partial cost (might contain some weights for example)
            partial_costs[ i-1 ]->bpropUpdate(
                expectations[ i ],
                target, partial_cost_value[0],
                expectation_gradients[ i ]
                );

            train_costs.subVec(partial_costs_positions[i-1]+1,
                               partial_cost_value.length())
                << partial_cost_value;
            
            if( !fast_exact_is_equal( partial_costs_weights.length(), 0 ) )
                expectation_gradients[ i ] *= partial_costs_weights[i-1];
            reconstruction_expectation_gradients += expectation_gradients[ i ];
        }

        if(!fast_exact_is_equal(l1_neuron_decay,0))
        {
            // Compute L1 penalty gradient on neurons
            real* hid = expectations[ i ].data();
            real* grad = reconstruction_expectation_gradients.data();
            int len = expectations[ i ].length();
            for(int j=0; j<len; j++)
            {
                if(*hid > l1_neuron_decay_center)
                    *grad += l1_neuron_decay;
                else if(*hid < l1_neuron_decay_center)
                    *grad -= l1_neuron_decay;
                hid++;
                grad++;
            }
        }

        if( correlation_connections.length() != 0 )
        {
            correlation_layers[i-1]->bpropUpdate(
                activations[i],
                expectations[i],
                reconstruction_activation_gradients,
                reconstruction_expectation_gradients );

            correlation_connections[i-1]->bpropUpdate(
                correlation_expectations[i-1],
                activations[i],
                correlation_expectation_gradients[i-1],
                reconstruction_activation_gradients);

            layers[i]->bpropUpdate( correlation_activations[i-1],
                                    correlation_expectations[i-1],
                                    correlation_activation_gradients[i-1],
                                    correlation_expectation_gradients[i-1] );

            connections[i-1]->bpropUpdate( corrupted_autoassociator_expectations[i-1],
                                           correlation_activations[i-1],
                                           reconstruction_expectation_gradients,
                                           correlation_activation_gradients[i-1] );

            if( greedy_target_connections.length() && greedy_target_connections[i-1] )
            {
                greedy_target_connections[i-1]->bpropUpdate(
                    targets_vec[i-1], 
                    correlation_activations[i-1],
                    targets_vec_gradient[i-1],
                    correlation_activation_gradients [ i-1 ]);
            }
        }
        else
        {
            layers[i]->bpropUpdate(
                activations[i],
                expectations[i],
                reconstruction_activation_gradients,
                reconstruction_expectation_gradients );

            connections[i-1]->bpropUpdate(
                corrupted_autoassociator_expectations[i-1],
                activations[i],
                reconstruction_expectation_gradients,
                reconstruction_activation_gradients);

            if( greedy_target_connections.length() && greedy_target_connections[i-1] )
            {
                greedy_target_connections[i-1]->bpropUpdate(
                    targets_vec[i-1], 
                    activations[ i ],
                    targets_vec_gradient[i-1],
                    reconstruction_activation_gradients);
            }
        }
    }

    // Put back fine-tuning learning rate
    // Set learning rates
    if( !fast_exact_is_equal( fine_tuning_decrease_ct , 0 ) )
        lr = fine_tuning_learning_rate
            / (1 + fine_tuning_decrease_ct * stage) ;
    else
        lr = fine_tuning_learning_rate ;

    // Set learning rate back for fine-tuning
    for( int i=0 ; i<n_layers-1 ; i++ )
    {
        layers[i]->setLearningRate( lr );
        connections[i]->setLearningRate( lr );
        //reconstruction_connections[i]->setLearningRate( lr );
        if(correlation_layers.length() != 0)
        {
            correlation_layers[i]->setLearningRate( lr );
            correlation_connections[i]->setLearningRate( lr );
        }
        if( greedy_target_connections.length() && greedy_target_connections[i] )
            greedy_target_connections[i]->setLearningRate( lr );
    }
    layers[n_layers-1]->setLearningRate( lr );


    final_module->fprop( expectations[ n_layers-1 ],
                         final_cost_input );
    final_cost->fprop( final_cost_input, target, final_cost_value );

    train_costs.subVec(train_costs.length()-final_cost_value.length(),
                       final_cost_value.length()) <<
        final_cost_value;

    final_cost->bpropUpdate( final_cost_input, target,
                             final_cost_value[0],
                             final_cost_gradient );
    final_module->bpropUpdate( expectations[ n_layers-1 ],
                               final_cost_input,
                               expectation_gradients[ n_layers-1 ],
                               final_cost_gradient );

    // Fine-tuning backpropagation
    if( correlation_connections.length() != 0 )
    {
        for( int i=n_layers-1 ; i>0 ; i-- )
        {
            correlation_layers[i-1]->bpropUpdate(
                activations[i],
                expectations[i],
                activation_gradients[i],
                expectation_gradients[i] );

            correlation_connections[i-1]->bpropUpdate(
                correlation_expectations[i-1],
                activations[i],
                correlation_expectation_gradients[i-1],
                activation_gradients[i] );

            layers[i]->bpropUpdate( correlation_activations[i-1],
                                    correlation_expectations[i-1],
                                    correlation_activation_gradients[i-1],
                                    correlation_expectation_gradients[i-1] );

            connections[i-1]->bpropUpdate(
                corrupted_autoassociator_expectations[i-1],
                correlation_activations[i-1],
                expectation_gradients[i-1],
                correlation_activation_gradients[i-1] );
            expectation_gradients[i-1] *= binary_masks[ i-1 ];
        }
    }
    else
    {
        for( int i=n_layers-1 ; i>0 ; i-- )
        {
            layers[i]->bpropUpdate( activations[i],
                                    expectations[i],
                                    activation_gradients[i],
                                    expectation_gradients[i] );

            connections[i-1]->bpropUpdate( corrupted_autoassociator_expectations[i-1],
                                           activations[i],
                                           expectation_gradients[i-1],
                                           activation_gradients[i] );
            expectation_gradients[i-1] *= binary_masks[ i-1 ];
        }
    }
}

void StackedAutoassociatorsNet::onlineStep(const Mat& inputs,
                                           const Mat& targets,
                                           Mat& train_costs)
{
    real lr;
    int mbatch_size = inputs.length();
    PLASSERT( targets.length() == mbatch_size );
    train_costs.resize(mbatch_size, train_costs.width());

    // fprop
    expectations_m[0].resize(mbatch_size, inputsize());
    expectations_m[0] << inputs;

    if( greedy_target_connections.length() != 0 )
        PLERROR("In StackedAutoassociatorsNet::onlineStep(): greedy_target_connections not "
                "implemented yet in mini-batch online setting.\n");
    
    if(correlation_connections.length() != 0)
    {
        for( int i=0 ; i<n_layers-1; i++ )
        {
            if( partial_costs.length() != 0 && partial_costs[ i ] )
                PLERROR("In StackedAutoassociatorsNet::onlineStep(): partial costs not "
                        "implemented yet for correlation_connections, in mini-batch online "
                        "setting.\n");

            connections[i]->fprop(expectations_m[i],
                                  correlation_activations_m[i]);
            layers[i+1]->fprop(correlation_activations_m[i],
                               correlation_expectations_m[i]);
            correlation_connections[i]->fprop(correlation_expectations_m[i],
                                              activations_m[i+1] );
            correlation_layers[i]->fprop(activations_m[i+1],
                                         expectations_m[i+1]);
        }
    }
    else
    {
        for( int i=0 ; i<n_layers-1; i++ )
        {
            connections[i]->fprop( expectations_m[i], activations_m[i+1] );
            layers[i+1]->fprop(activations_m[i+1], expectations_m[i+1]);

            if( partial_costs.length() != 0 && partial_costs[ i ] )
            {
                // Set learning rates
                if( !fast_exact_is_equal(fine_tuning_decrease_ct, 0 ) )
                    lr = fine_tuning_learning_rate /
                        (1 + fine_tuning_decrease_ct * stage);
                else
                    lr = fine_tuning_learning_rate;

                partial_costs[ i ]->setLearningRate( lr );
                partial_costs[ i ]->fprop( expectations_m[i + 1],
                                           targets, partial_cost_values );
                // Update partial cost (might contain some weights for example)
                partial_cost_values_0.resize(mbatch_size);
                partial_cost_values_0 << partial_cost_values.column(0);
                partial_costs[ i ]->bpropUpdate(
                    expectations_m[ i + 1 ],
                    targets,
                    partial_cost_values_0,
                    expectation_gradients_m[ i + 1 ]
                    );

                train_costs.subMatColumns(partial_costs_positions[i]+1,
                                          partial_cost_values.width())
                    << partial_cost_values;

                if( partial_costs_weights.length() != 0 )
                    expectation_gradients_m[i + 1] *= partial_costs_weights[i];

                // Update hidden layer bias and weights
                layers[ i+1 ]->bpropUpdate( activations_m[ i + 1 ],
                                            expectations_m[ i + 1 ],
                                            activation_gradients_m[ i + 1 ],
                                            expectation_gradients_m[ i + 1 ] );

                connections[ i ]->bpropUpdate( expectations_m[ i ],
                                               activations_m[ i + 1 ],
                                               expectation_gradients_m[ i ],
                                               activation_gradients_m[ i + 1 ]
                                             );
            }
        }
    }

    final_module->fprop( expectations_m[ n_layers-1 ],
                         final_cost_inputs );

    final_cost->fprop( final_cost_inputs, targets, final_cost_values );

    train_costs.subMatColumns(train_costs.width() - final_cost_values.width(),
                              final_cost_values.width())
        << final_cost_values;

    final_cost_values_0.resize(mbatch_size);
    final_cost_values_0 << final_cost_values.column(0);
    final_cost->bpropUpdate( final_cost_inputs, targets,
                             final_cost_values_0,
                             final_cost_gradients );
    final_module->bpropUpdate( expectations_m[ n_layers-1 ],
                               final_cost_inputs,
                               expectation_gradients_m[ n_layers-1 ],
                               final_cost_gradients );

    // Unsupervised greedy layer-wise cost

    // Set learning rates
    if( !fast_exact_is_equal( greedy_decrease_ct, 0 ) )
        lr = greedy_learning_rate / (1 + greedy_decrease_ct * stage) ;
    else
        lr = greedy_learning_rate;

    for( int i=0 ; i<n_layers-1 ; i++ )
    {
        layers[i]->setLearningRate( lr );
        reconstruction_layers[i]->setLearningRate( lr );
        connections[i]->setLearningRate( lr );
        reconstruction_connections[i]->setLearningRate( lr );
        if(correlation_layers.length() != 0)
        {
            correlation_layers[i]->setLearningRate( lr );
            correlation_connections[i]->setLearningRate( lr );
        }
    }
    layers[n_layers-1]->setLearningRate( lr );

    // Backpropagate unsupervised gradient, layer-wise
    for( int i=n_layers-1 ; i>0 ; i-- )
    {
        reconstruction_connections[ i-1 ]->fprop(
            expectations_m[ i ],
            reconstruction_activations_m);

        reconstruction_layers[ i-1 ]->activations.resize(mbatch_size,reconstruction_layers[i-1]->size);
        reconstruction_layers[ i-1 ]->activations << reconstruction_activations_m;
        reconstruction_layers[ i-1 ]->activations += reconstruction_layers[ i-1 ]->bias;

        Mat layer_exp = reconstruction_layers[i-1]->getExpectations();
        reconstruction_layers[ i-1 ]->fprop(reconstruction_activations_m,
                             layer_exp);
        reconstruction_layers[ i-1 ]->setExpectationsByRef(layer_exp);

        reconstruction_layers[ i-1 ]->fpropNLL(expectations_m[i-1],
                                train_costs.column(i-1));

        reconstruction_layers[ i-1 ]->bpropNLL(expectations_m[i-1], train_costs.column(i-1),
                                reconstruction_activation_gradients_m);

        reconstruction_layers[ i-1 ]->update(reconstruction_activation_gradients_m);

        reconstruction_connections[ i-1 ]->bpropUpdate(
            expectations_m[ i ],
            reconstruction_activations_m,
            reconstruction_expectation_gradients_m,
            reconstruction_activation_gradients_m);

        if(!fast_exact_is_equal(l1_neuron_decay,0))
        {
            // Compute L1 penalty gradient on neurons
            for (int k = 0; k < mbatch_size; k++)
            {
                real* hid = expectations_m[i](k).data();
                real* grad = reconstruction_expectation_gradients_m(k).data();
                int width = expectations_m[i].width();
                for(int j = 0; j < width; j++)
                {
                    if(*hid > l1_neuron_decay_center)
                        *grad += l1_neuron_decay;
                    else if(*hid < l1_neuron_decay_center)
                        *grad -= l1_neuron_decay;
                    hid++;
                    grad++;
                }
            }
        }

        if( correlation_connections.length() != 0 )
        {
            correlation_layers[i-1]->bpropUpdate(
                activations_m[i],
                expectations_m[i],
                reconstruction_activation_gradients_m,
                reconstruction_expectation_gradients_m);

            correlation_connections[i-1]->bpropUpdate(
                correlation_expectations_m[i-1],
                activations_m[i],
                correlation_expectation_gradients_m[i-1],
                reconstruction_activation_gradients_m);

            layers[i]->bpropUpdate(
                correlation_activations_m[i-1],
                correlation_expectations_m[i-1],
                correlation_activation_gradients_m[i-1],
                correlation_expectation_gradients_m[i-1]);

            connections[i-1]->bpropUpdate(
                expectations_m[i-1],
                correlation_activations_m[i-1],
                reconstruction_expectation_gradients_m,
                correlation_activation_gradients_m[i-1]);
        }
        else
        {
            layers[i]->bpropUpdate(
                activations_m[i],
                expectations_m[i],
                reconstruction_activation_gradients_m,
                reconstruction_expectation_gradients_m);

            connections[i-1]->bpropUpdate(
                expectations_m[i-1],
                activations_m[i],
                reconstruction_expectation_gradients_m,
                reconstruction_activation_gradients_m);
        }
    }

    // Put back fine-tuning learning rate
    // Set learning rates
    if( !fast_exact_is_equal(fine_tuning_decrease_ct, 0) )
        lr = fine_tuning_learning_rate
            / (1 + fine_tuning_decrease_ct * stage) ;
    else
        lr = fine_tuning_learning_rate ;

    // Set learning rate back for fine-tuning
    for( int i=0 ; i<n_layers-1 ; i++ )
    {
        layers[i]->setLearningRate( lr );
        connections[i]->setLearningRate( lr );
        //reconstruction_connections[i]->setLearningRate( lr );
        if(correlation_layers.length() != 0)
        {
            correlation_layers[i]->setLearningRate( lr );
            correlation_connections[i]->setLearningRate( lr );
        }
    }
    layers[n_layers-1]->setLearningRate( lr );

    // Fine-tuning backpropagation
    if( correlation_connections.length() != 0 )
    {
        for( int i=n_layers-1 ; i>0 ; i-- )
        {
            correlation_layers[i-1]->bpropUpdate(
                activations_m[i],
                expectations_m[i],
                activation_gradients_m[i],
                expectation_gradients_m[i] );

            correlation_connections[i-1]->bpropUpdate(
                correlation_expectations_m[i-1],
                activations_m[i],
                correlation_expectation_gradients_m[i-1],
                activation_gradients_m[i] );

            layers[i]->bpropUpdate( correlation_activations_m[i-1],
                                    correlation_expectations_m[i-1],
                                    correlation_activation_gradients_m[i-1],
                                    correlation_expectation_gradients_m[i-1] );

            connections[i-1]->bpropUpdate(
                expectations_m[i-1],
                correlation_activations_m[i-1],
                expectation_gradients_m[i-1],
                correlation_activation_gradients_m[i-1] );
        }
    }
    else
    {
        for( int i=n_layers-1 ; i>0 ; i-- )
        {
            layers[i]->bpropUpdate( activations_m[i],
                                    expectations_m[i],
                                    activation_gradients_m[i],
                                    expectation_gradients_m[i] );

            connections[i-1]->bpropUpdate( expectations_m[i-1],
                                           activations_m[i],
                                           expectation_gradients_m[i-1],
                                           activation_gradients_m[i] );
        }
    }
}

void StackedAutoassociatorsNet::computeOutput(const Vec& input, Vec& output) const
{
    Profiler::pl_profile_start("StackedAutoassociatorsNet::computeOutput");
    // fprop

    expectations[0] << input;

    if(correlation_connections.length() != 0)
    {
        for( int i=0 ; i<currently_trained_layer-1; i++ )
        {
            connections[i]->fprop( expectations[i], correlation_activations[i] );
            layers[i+1]->fprop( correlation_activations[i],
                                correlation_expectations[i] );
            correlation_connections[i]->fprop( correlation_expectations[i],
                                               activations[i+1] );
            correlation_layers[i]->fprop( activations[i+1],
                                          expectations[i+1] );
        }
    }
    else
    {
        for(int i=0 ; i<currently_trained_layer-1 ; i++ )
        {
            double_input(expectations[i], doubled_expectations[i]);
            connections[i]->fprop( doubled_expectations[i], activations[i+1] );
            layers[i+1]->fprop(activations[i+1],expectations[i+1]);
        }
    }

    if( currently_trained_layer<n_layers )
    {
        if(correlation_connections.length() != 0)
        {
            connections[currently_trained_layer-1]->fprop(
                expectations[currently_trained_layer-1],
                correlation_activations[currently_trained_layer-1] );

            layers[currently_trained_layer]->fprop(
                correlation_activations[currently_trained_layer-1],
                correlation_expectations[currently_trained_layer-1] );

            correlation_connections[currently_trained_layer-1]->fprop(
                correlation_expectations[currently_trained_layer-1],
                activations[currently_trained_layer] );

            correlation_layers[currently_trained_layer-1]->fprop(
                activations[currently_trained_layer],
                output );
        }
        else
        {
            double_input(expectations[currently_trained_layer-1], 
                doubled_expectations[currently_trained_layer-1]);
            connections[currently_trained_layer-1]->fprop(
                doubled_expectations[currently_trained_layer-1],
                activations[currently_trained_layer] );
            layers[currently_trained_layer]->fprop(
                activations[currently_trained_layer],
                output);
        }
    }
    else
        final_module->fprop( expectations[ currently_trained_layer - 1],
                             output );
    Profiler::pl_profile_end("StackedAutoassociatorsNet::computeOutput");
}

void StackedAutoassociatorsNet::computeOutputs(const Mat& input, Mat& output) const
{
    if(correlation_connections.length() != 0
       || compute_all_test_costs
       || noise_type == "missing_data"){
        inherited::computeOutputs(input, output);
    }else{
        Profiler::pl_profile_start("StackedAutoassociatorsNet::computeOutputs");

        expectations_m[0].resize(input.length(), inputsize());
        Mat m = expectations_m[0];
        m<<input;

        for(int i=0 ; i<currently_trained_layer-1 ; i++ )
        {
            connections[i]->fprop( expectations_m[i], activations_m[i+1] );
            layers[i+1]->fprop(activations_m[i+1],expectations_m[i+1]);
        }
        if(currently_trained_layer < n_layers)
        {
            connections[currently_trained_layer-1]->fprop( expectations_m[currently_trained_layer-1],
                 activations_m[currently_trained_layer] );
            layers[currently_trained_layer]->fprop(activations_m[currently_trained_layer],
                 output);
        }
        else
        {
            final_module->fprop( expectations_m[ currently_trained_layer - 1],
                             output );
        }
        Profiler::pl_profile_end("StackedAutoassociatorsNet::computeOutputs");
    }
}

void StackedAutoassociatorsNet::computeOutputsAndCosts(const Mat& input, const Mat& target,
                                                       Mat& output, Mat& costs) const
{
    if(correlation_connections.length() != 0 
       || compute_all_test_costs
       || noise_type == "missing_data"){
        inherited::computeOutputsAndCosts(input, target, output, costs);
    }else{
        Profiler::pl_profile_start("StackedAutoassociatorsNet::computeOutputsAndCosts");

        int n=input.length();
        PLASSERT(target.length()==n);
        output.resize(n,outputsize());
        costs.resize(n,nTestCosts());
        computeOutputs(input, output);
        for (int i=0;i<n;i++)
        {
            Vec in_i = input(i);
            Vec out_i = output(i); 
            Vec target_i = target(i);
            Vec c_i = costs(i);
            computeCostsFromOutputs(in_i, out_i, target_i, c_i);
        }
        Profiler::pl_profile_end("StackedAutoassociatorsNet::computeOutputsAndCosts");
    }
}

void StackedAutoassociatorsNet::computeCostsFromOutputs(const Vec& input, const Vec& output,
                                           const Vec& target, Vec& costs) const
{
    //Assumes that computeOutput has been called

    Profiler::pl_profile_start("StackedAutoassociatorsNet::computeCostsFromOutputs");
    costs.resize( nTestCosts() );
    costs.fill( MISSING_VALUE );

    if(compute_all_test_costs)
    {
        for(int i=0; i<currently_trained_layer-1; i++)
        {
            reconstruction_connections[ i ]->fprop( expectations[ i+1 ],
                                                    reconstruction_activations);
            if( direct_connections.length() != 0 )
            {
                direct_connections[ i ]->fprop(
                    expectations[ i ],
                    direct_activations );
                reconstruction_activations += direct_activations;
            }

            reconstruction_layers[ i ]->fprop( reconstruction_activations,
                                reconstruction_layers[ i ]->expectation);

            reconstruction_layers[ i ]->activation << reconstruction_activations;
            reconstruction_layers[ i ]->activation += reconstruction_layers[ i ]->bias;
            //reconstruction_layers[ i ]->expectation_is_up_to_date = true;
            reconstruction_layers[ i ]->setExpectationByRef( reconstruction_layers[ i ]->expectation );

            costs[i] = reconstruction_layers[ i ]->fpropNLL(expectations[ i ]);

            if( partial_costs && partial_costs[i])
            {
                partial_costs[ i ]->fprop( expectations[ i + 1],
                                           target, partial_cost_value );
                costs.subVec(partial_costs_positions[i],
                             partial_cost_value.length()) <<
                    partial_cost_value;
            }
        }
    }

    if( currently_trained_layer<n_layers )
    {
        reconstruction_connections[ currently_trained_layer-1 ]->fprop(
            output,
            reconstruction_activations);
        if( direct_connections.length() != 0 )
        {
            direct_connections[ currently_trained_layer-1 ]->fprop(
                expectations[ currently_trained_layer-1 ],
                direct_activations );
            reconstruction_activations += direct_activations;
        }
                
        Vec divided_reconstruction_activations(reconstruction_activations.size());
        divide_input(reconstruction_activations, divided_reconstruction_activations);

        reconstruction_layers[ currently_trained_layer-1 ]->fprop(
          divided_reconstruction_activations,
          reconstruction_layers[ currently_trained_layer-1 ]->expectation);
        
        reconstruction_layers[ currently_trained_layer-1 ]->activation <<
            divided_reconstruction_activations;
        reconstruction_layers[ currently_trained_layer-1 ]->activation += 
            reconstruction_layers[ currently_trained_layer-1 ]->bias;
        //reconstruction_layers[ currently_trained_layer-1 ]->expectation_is_up_to_date = true;
        reconstruction_layers[ currently_trained_layer-1 ]->setExpectationByRef(
            reconstruction_layers[ currently_trained_layer-1 ]->expectation );

        costs[ currently_trained_layer-1 ] =
            reconstruction_layers[ currently_trained_layer-1 ]->fpropNLL(
                expectations[ currently_trained_layer-1 ]);

        if(reconstruct_hidden)
        {
            connections[ currently_trained_layer-1 ]->fprop(
                reconstruction_layers[ currently_trained_layer-1 ]->expectation,
                hidden_reconstruction_activations );
            layers[ currently_trained_layer ]->fprop(
                hidden_reconstruction_activations,
                layers[ currently_trained_layer ]->expectation );
            layers[ currently_trained_layer ]->activation <<
                hidden_reconstruction_activations;
            layers[ currently_trained_layer ]->activation += 
                layers[ currently_trained_layer ]->bias;
            //layers[ currently_trained_layer ]->expectation_is_up_to_date = true;
            layers[ currently_trained_layer ]->setExpectationByRef(
                layers[ currently_trained_layer ]->expectation );
            costs[ currently_trained_layer-1 ] +=
                layers[ currently_trained_layer ]->fpropNLL(
                    output);
        }

        if( partial_costs && partial_costs[ currently_trained_layer-1 ] )
        {
            partial_costs[ currently_trained_layer-1 ]->fprop(
                output,
                target, partial_cost_value );
            costs.subVec(partial_costs_positions[currently_trained_layer-1],
                         partial_cost_value.length()) << partial_cost_value;
        }
    }
    else
    {
        final_cost->fprop( output, target, final_cost_value );
        costs.subVec(costs.length()-final_cost_value.length(),
                     final_cost_value.length()) <<
            final_cost_value;
    }
    Profiler::pl_profile_end("StackedAutoassociatorsNet::computeCostsFromOutputs");
}

TVec<string> StackedAutoassociatorsNet::getTestCostNames() const
{
    // Return the names of the costs computed by computeCostsFromOutputs
    // (these may or may not be exactly the same as what's returned by
    // getTrainCostNames).

    TVec<string> cost_names(0);

    for( int i=0; i<layers.size()-1; i++)
        cost_names.push_back("reconstruction_error_" + tostring(i+1));

    for( int i=0 ; i<partial_costs.size() ; i++ )
    {
        TVec<string> names = partial_costs[i]->costNames();
        for(int j=0; j<names.length(); j++)
            cost_names.push_back("partial" + tostring(i) + "." +
                names[j]);
    }

    cost_names.append( final_cost->costNames() );

    return cost_names;
}

TVec<string> StackedAutoassociatorsNet::getTrainCostNames() const
{
    TVec<string> cost_names(0);

    for( int i=0; i<layers.size()-1; i++)
        cost_names.push_back("reconstruction_error_" + tostring(i+1));

    cost_names.push_back("global_reconstruction_error");

    for( int i=0 ; i<partial_costs.size() ; i++ )
    {
        TVec<string> names = partial_costs[i]->costNames();
        for(int j=0; j<names.length(); j++)
            cost_names.push_back("partial" + tostring(i) + "." +
                names[j]);
    }

    cost_names.append( final_cost->costNames() );

    return cost_names;
}


//#####  Helper functions  ##################################################

void StackedAutoassociatorsNet::setLearningRate( real the_learning_rate )
{
    for( int i=0 ; i<n_layers-1 ; i++ )
    {
        layers[i]->setLearningRate( the_learning_rate );
        reconstruction_layers[i]->setLearningRate( the_learning_rate );
        connections[i]->setLearningRate( the_learning_rate );
        if(correlation_layers.length() != 0)
        {
            correlation_layers[i]->setLearningRate( the_learning_rate );
            correlation_connections[i]->setLearningRate( the_learning_rate );
        }
        if(direct_connections.length() != 0)
        {
            direct_connections[i]->setLearningRate( the_learning_rate );
        }
        reconstruction_connections[i]->setLearningRate( the_learning_rate );
    }

    for( int i=0; i<greedy_target_connections.length(); i++ )
        greedy_target_connections[i]->setLearningRate( the_learning_rate );

    layers[n_layers-1]->setLearningRate( the_learning_rate );

    final_cost->setLearningRate( the_learning_rate );
    final_module->setLearningRate( the_learning_rate );
}

TVec<Vec> StackedAutoassociatorsNet::fantasizeKTimeOnMultiSrcImg(const int KTime, const Mat& srcImg, const Vec& sample, const Vec& maskNoiseFractOrProb, bool alwaysFromSrcImg)
{
    int n=srcImg.length();
    TVec<Vec> output(0);

    for( int i=0; i<n; i++ )
    {
        const Vec img_i = srcImg(i);
        TVec<Vec> outputTmp;  
        outputTmp = fantasizeKTime(KTime, img_i, sample, maskNoiseFractOrProb, alwaysFromSrcImg);
        output = concat(output, outputTmp);        
    }

    return output;
}

TVec<Vec> StackedAutoassociatorsNet::fantasizeKTime(const int KTime, const Vec& srcImg, const Vec& sample, const Vec& maskNoiseFractOrProb, bool alwaysFromSrcImg)
{
    bool bFractOrProbUseful=false;

    // Noise type that needs fraction_of_masked_inputs or prob_masked_inputs
    if(noise_type == "masking_noise" || noise_type == "missing_data")
        bFractOrProbUseful=true;

    if(bFractOrProbUseful && maskNoiseFractOrProb.size() == 0)
        PLERROR("In StackedAutoassociatorsNet::fantasize():"
        "maskNoiseFractOrProb should be defined because fraction_of_masked_inputs"
        " or prob_masked_inputs have been used during the learning stage.");

    if(bFractOrProbUseful && maskNoiseFractOrProb.size() != sample.size())
        PLERROR("In StackedAutoassociatorsNet::fantasize():"
        "Size of maskNoiseFractOrProb should be equal to sample's size.");

    if(sample.size() > n_layers-1)
        PLERROR("In StackedAutoassociatorsNet::fantasize():"
        " Size of sample (%i) should be <= "
        "number of hidden layer (%i).",sample.size(), n_layers-1);

    bool bFraction_masked_input = true;
    bool autoassociator_expectation_indices_temp_initialized = false;

    // Number of hidden layer to be 'covered'
    int n_hlayers_used = sample.size();

    // Save actual value
    real old_fraction_masked_inputs = fraction_of_masked_inputs;
    real old_prob_masked_inputs = probability_of_masked_inputs;
    bool old_mask_input_layer_only = mask_input_layer_only;
    int  old_nb_corrupted_layer = nb_corrupted_layer;

    // New values for fantasize
    mask_input_layer_only = false;
    nb_corrupted_layer = n_hlayers_used;

    if(bFractOrProbUseful)
    {
        if(old_prob_masked_inputs > 0.)
            bFraction_masked_input = false;
        else
            if(autoassociator_expectation_indices.size() == 0)
            {
                autoassociator_expectation_indices.resize( n_hlayers_used );
                autoassociator_expectation_indices_temp_initialized = true;
            }
    }
    
    TVec<Vec> fantaImagesObtained(KTime+1);

    fantaImagesObtained[0].resize(srcImg.size());
    fantaImagesObtained[0] << srcImg;
    expectations[0] << srcImg;
    
    // Do fantasize k time.
    for( int k=0 ; k<KTime ; k++ )
    {
        fantaImagesObtained[k+1].resize(srcImg.size());
        for( int i=0 ; i<n_hlayers_used; i++ )
        {
            // Initialisation made only at the first loop.
            if(k == 0)
            {
                // initialize autoassociator_expectation_indices if not already done
                // considering new fraction_of_masked_inputs possibly different (not
                // equal to zero) from the one used during the training.
                if(autoassociator_expectation_indices_temp_initialized)
                {
                    autoassociator_expectation_indices[i].resize( layers[i]->size );
                    for( int j=0 ; j < autoassociator_expectation_indices[i].length() ; j++ )
                         autoassociator_expectation_indices[i][j] = j;
                }
            }

            if(bFractOrProbUseful)
            {
                if(bFraction_masked_input)
                    fraction_of_masked_inputs = maskNoiseFractOrProb[i];
                else
                    probability_of_masked_inputs = maskNoiseFractOrProb[i];
            }
            double_input(expectations[i], doubled_expectations[i]);
            corrupt_input(
                doubled_expectations[i],
                corrupted_autoassociator_expectations[i], i);
            connections[i]->fprop(
                corrupted_autoassociator_expectations[i],
                activations[i+1] );
            layers[i+1]->fprop(activations[i+1],expectations[i+1]);
        }

        for( int i=n_hlayers_used-1 ; i>=0; i-- )
        {
            // Binomial sample
            if( sample[i] == 1 )
                for( int j=0; j<expectations[i+1].size(); j++ )
                    expectations[i+1][j] = random_gen->binomial_sample(expectations[i+1][j]);
    
            reconstruction_connections[i]->fprop(
                expectations[i+1],
                reconstruction_activations );
  
            Vec divided_reconstruction_activations(reconstruction_activations.size());
            divide_input(reconstruction_activations, divided_reconstruction_activations);

            reconstruction_layers[i]->fprop(divided_reconstruction_activations, expectations[i]);
        }
        fantaImagesObtained[k+1] << expectations[0];
        if( alwaysFromSrcImg )
            expectations[0] << srcImg;
    }

    if(bFractOrProbUseful)
    {
        fraction_of_masked_inputs = old_fraction_masked_inputs;
        probability_of_masked_inputs = old_prob_masked_inputs;
    }

    mask_input_layer_only = old_mask_input_layer_only;
    nb_corrupted_layer = old_nb_corrupted_layer;

    return fantaImagesObtained;
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
