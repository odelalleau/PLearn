// -*- C++ -*-

// DeepFeatureExtractorNNet.cc
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

/* *******************************************************      
   * $Id: .pyskeleton_header 544 2003-09-01 00:05:31Z plearner $ 
   ******************************************************* */

// Authors: Hugo Larochelle

/*! \file DeepFeatureExtractorNNet.cc */


#include "DeepFeatureExtractorNNet.h"
#include <plearn/var/AffineTransformVariable.h>
#include <plearn/var/SourceVariable.h>
#include <plearn/var/AffineTransformWeightPenalty.h>
#include <plearn/var/BiasWeightAffineTransformVariable.h>
#include <plearn/var/BinaryClassificationLossVariable.h>
#include <plearn/var/ClassificationLossVariable.h>
#include <plearn/var/ConcatColumnsVariable.h>
#include <plearn/var/ConcatRowsVariable.h>
#include <plearn/var/CrossEntropyVariable.h>
#include <plearn/var/ExpVariable.h>
#include <plearn/var/LiftOutputVariable.h>
#include <plearn/var/LogSoftmaxVariable.h>
#include <plearn/var/MarginPerceptronCostVariable.h>
#include <plearn/var/MulticlassLossVariable.h>
#include <plearn/var/NegCrossEntropySigmoidVariable.h>
#include "NLLNeighborhoodWeightsVariable.h"
#include <plearn/var/OneHotSquaredLoss.h>
#include <plearn/var/PowVariable.h>
#include <plearn/var/SigmoidVariable.h>
#include <plearn/var/SoftmaxVariable.h>
#include <plearn/var/SoftplusVariable.h>
#include <plearn/var/SubMatVariable.h>
#include <plearn/var/SumVariable.h>
#include <plearn/var/SumAbsVariable.h>
#include <plearn/var/SumOfVariable.h>
#include <plearn/var/SumSquareVariable.h>
#include <plearn/var/TanhVariable.h>
#include <plearn/var/TransposeVariable.h>
#include <plearn/var/TransposeProductVariable.h>
#include <plearn/var/UnaryHardSlopeVariable.h>
#include <plearn/var/Var_operators.h>
#include <plearn/var/Var_utils.h>
#include <plearn/display/DisplayUtils.h>

#include <plearn/vmat/ConcatColumnsVMatrix.h>
#include <plearn/vmat/GetInputVMatrix.h>
#include <plearn/math/random.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    DeepFeatureExtractorNNet,
    "Deep Neural Network that extracts features in a greedy, mostly unsupervised way",
    "After the greedy unsupervised phase, this learner can optionally be \n"
    "trained using a supervised learning criteria (i.e. MSE, class NLL, \n"
    "margin-perceptron cost, etc.).");

DeepFeatureExtractorNNet::DeepFeatureExtractorNNet() 
    : batch_size(1),
      batch_size_supervised(1), 
      output_transfer_func("softmax"),
      nhidden_schedule_position(0),
      weight_decay(0), 
      bias_decay(0),
      penalty_type("L2_square"),
      classification_regularizer(0),
      regularizer(0),
      margin(1),
      initialization_method("uniform_linear"), 
      noutputs(0),
      use_same_input_and_output_weights(false),
      always_reconstruct_input(false),
      use_activations_with_cubed_input(false),
      use_n_first_as_supervised(-1),
      use_only_supervised_part(false),
      relative_minimum_improvement(-1),
      input_reconstruction_error("cross_entropy"),
      autoassociator_regularisation_weight(0),
      supervised_signal_weight(0),
      k_nearest_neighbors_reconstruction(-1),
      nhidden_schedule_current_position(-1)
{
    random_gen = new PRandom();
}

void DeepFeatureExtractorNNet::declareOptions(OptionList& ol)
{
    declareOption(ol, "nhidden_schedule", 
                  &DeepFeatureExtractorNNet::nhidden_schedule, 
                  OptionBase::buildoption,
                  "Number of hidden units of each hidden layers to add");
    
    declareOption(ol, "optimizer", &DeepFeatureExtractorNNet::optimizer, 
                  OptionBase::buildoption,
                  "Optimizer of the neural network");

    declareOption(ol, "optimizer_supervised", 
                  &DeepFeatureExtractorNNet::optimizer_supervised, 
                  OptionBase::buildoption,
                  "Optimizer of the supervised phase of the neural network.\n"
                  "If not specified, then the same optimizer will always be\n"
                  "used.\n");

    declareOption(ol, "batch_size", &DeepFeatureExtractorNNet::batch_size, 
                  OptionBase::buildoption, 
                  "How many samples to use to estimate the avergage gradient\n"
                  "before updating the weights\n"
                  "0 is equivalent to specifying training_set->length() \n");
    
    declareOption(ol, "batch_size_supervised", &DeepFeatureExtractorNNet::batch_size_supervised, 
                  OptionBase::buildoption, 
                  "How many samples to use to estimate the avergage gradient\n"
                  "before updating the weights, for the supervised phase.\n"
                  "0 is equivalent to specifying training_set->length() \n");
    
    declareOption(ol, "output_transfer_func", 
                  &DeepFeatureExtractorNNet::output_transfer_func, 
                  OptionBase::buildoption,
                  "Output transfer function, when all hidden layers are \n"
                  "added. Choose among:\n"
                  "  - \"tanh\" \n"
                  "  - \"sigmoid\" \n"
                  "  - \"exp\" \n"
                  "  - \"softplus\" \n"
                  "  - \"softmax\" \n"
                  "  - \"log_softmax\" \n"
                  "  - \"interval(<minval>,<maxval>)\", which stands for\n"
                  "          <minval>+(<maxval>-<minval>)*sigmoid(.).\n"
                  "An empty string or \"none\" means no output \n"
                  "transfer function \n");
    
    declareOption(ol, "nhidden_schedule_position", 
                  &DeepFeatureExtractorNNet::nhidden_schedule_position, 
                  OptionBase::buildoption,
                  "Index of the layer(s) that will be trained at the next\n"
                  "call of train. Should be bigger then the last\n"
                  "nhidden_schedule_position, which is initialy -1. \n"
                  "Then, all the layers up to nhidden_schedule_position that\n"
                  "were not trained so far will be. Also, when\n"
                  "nhidden_schedule_position is greater than or equal\n"
                  "to the size of nhidden_schedule, then the output layer is also\n"
                  "added.");
    
    declareOption(ol, "nhidden_schedule_current_position", 
                  &DeepFeatureExtractorNNet::nhidden_schedule_current_position, 
                  OptionBase::learntoption,
                  "Index of the layer that is being trained at the current state");

    declareOption(ol, "cost_funcs", &DeepFeatureExtractorNNet::cost_funcs, 
                  OptionBase::buildoption, 
                  "A list of cost functions to use\n"
                  "in the form \"[ cf1; cf2; cf3; ... ]\"\n"
                  "where each function is one of: \n"
                  "  - \"mse\" (for regression)\n"
                  "  - \"mse_onehot\" (for classification)\n"
                  "  - \"NLL\" (negative log likelihood -log(p[c])\n"
                  "             for classification) \n"
                  "  - \"class_error\" (classification error) \n"
                  "  - \"binary_class_error\" (classification error for a\n"
                  "                            0-1 binary classifier)\n"
                  "  - \"multiclass_error\" \n"
                  "  - \"cross_entropy\" (for binary classification)\n"
                  "  - \"stable_cross_entropy\" (more accurate backprop and\n"
                  "                              possible regularization, for\n"
                  "                              binary classification)\n"
                  "  - \"margin_perceptron_cost\" (a hard version of the \n"
                  "                                cross_entropy, uses the\n"
                  "                                'margin' option)\n"
                  "  - \"lift_output\" (not a real cost function, just the\n"
                  "                     output for lift computation)\n"
                  "The FIRST function of the list will be used as \n"
                  "the objective function to optimize \n"
                  "(possibly with an added weight decay penalty) \n");
    
    declareOption(ol, "weight_decay", 
                  &DeepFeatureExtractorNNet::weight_decay, OptionBase::buildoption, 
                  "Global weight decay for all layers\n");

    declareOption(ol, "bias_decay", &DeepFeatureExtractorNNet::bias_decay, 
                  OptionBase::buildoption, 
                  "Global bias decay for all layers\n");
    
    declareOption(ol, "penalty_type", &DeepFeatureExtractorNNet::penalty_type,
                  OptionBase::buildoption,
                  "Penalty to use on the weights (for weight and bias decay).\n"
                  "Can be any of:\n"
                  "  - \"L1\": L1 norm,\n"
                  //"  - \"L1_square\": square of the L1 norm,\n"
                  "  - \"L2_square\" (default): square of the L2 norm.\n");
    
    declareOption(ol, "classification_regularizer", 
                  &DeepFeatureExtractorNNet::classification_regularizer, 
                  OptionBase::buildoption, 
                  "Used only in the stable_cross_entropy cost function, to fight overfitting (0<=r<1)\n");  
    
    declareOption(ol, "regularizer", &DeepFeatureExtractorNNet::regularizer, 
                  OptionBase::buildoption, 
                  "Used in the stable_cross_entropy cost function for the hidden activations, in the unsupervised stages (0<=r<1)\n");  
    
    declareOption(ol, "margin", &DeepFeatureExtractorNNet::margin, 
                  OptionBase::buildoption, 
                  "Margin requirement, used only with the \n"
                  "margin_perceptron_cost cost function.\n"
                  "It should be positive, and larger values regularize more.\n");
    
    declareOption(ol, "initialization_method", 
                  &DeepFeatureExtractorNNet::initialization_method, 
                  OptionBase::buildoption, 
                  "The method used to initialize the weights:\n"
                  " - \"normal_linear\"  = a normal law with variance 1/n_inputs\n"
                  " - \"normal_sqrt\"    = a normal law with variance"
                  "1/sqrt(n_inputs)\n"
                  " - \"uniform_linear\" = a uniform law in [-1/n_inputs, "
                  "1/n_inputs]\n"
                  " - \"uniform_sqrt\"   = a uniform law in [-1/sqrt(n_inputs), "
                  "1/sqrt(n_inputs)]\n"
                  " - \"zero\"           = all weights are set to 0\n");
    
    declareOption(ol, "paramsvalues", &DeepFeatureExtractorNNet::paramsvalues, 
                  OptionBase::learntoption, 
                  "The learned parameter vector\n");
    declareOption(ol, "noutputs", &DeepFeatureExtractorNNet::noutputs, 
                  OptionBase::buildoption, 
                  "Number of output units. This gives this learner \n"
                  "its outputsize. It is typically of the same dimensionality\n"
                  "as the target for regression problems\n"
                  "But for classification problems where target is just\n"
                  "the class number, noutputs is usually of dimensionality \n"
                  "number of classes (as we want to output a score or\n"
                  "probability vector, one per class)\n"
                  "If the network only extracts features in an unsupervised\n"
                  "manner, then let noutputs be 0.");    

    declareOption(ol, "use_same_input_and_output_weights", 
                  &DeepFeatureExtractorNNet::use_same_input_and_output_weights, 
                  OptionBase::buildoption, 
                  "Use the same weights for the input and output weights for\n"
                  "the autoassociators.");  

    declareOption(ol, "always_reconstruct_input", 
                  &DeepFeatureExtractorNNet::always_reconstruct_input, 
                  OptionBase::buildoption, 
                  "Always use the reconstruction cost of the input, not of\n"
                  "the last layer. This option should be used if\n"
                  "use_same_input_and_output_weights is true.");  

    declareOption(ol, "use_activations_with_cubed_input", 
                  &DeepFeatureExtractorNNet::use_activations_with_cubed_input, 
                  OptionBase::buildoption, 
                  "Use the cubed value of the input of the activation functions\n"
                  "(not used for reconstruction/auto-associator layers and\n"
                  " output layer).\n");

    declareOption(ol, "use_n_first_as_supervised", 
                  &DeepFeatureExtractorNNet::use_n_first_as_supervised, 
                  OptionBase::buildoption, 
                  "To simulate semi-supervised learning.");

    declareOption(ol, "use_only_supervised_part", 
                  &DeepFeatureExtractorNNet::use_only_supervised_part, 
                  OptionBase::buildoption, 
                  "Indication that only the supervised part should be\n"
                  "used, throughout the whole training, when simulating\n"
                  "semi-supervised learning.");

    declareOption(ol, "relative_minimum_improvement", 
                  &DeepFeatureExtractorNNet::relative_minimum_improvement,
                  OptionBase::buildoption, 
                  "Threshold on training set error relative improvement,\n"
                  "before adding a new layer. If < 0, then the addition\n"
                  "of layers must be done by the user." );

    declareOption(ol, "autoassociator_regularisation_weight", 
                  &DeepFeatureExtractorNNet::autoassociator_regularisation_weight,
                  OptionBase::buildoption, 
                  "Weight of autoassociator regularisation terms\n"
                  "in the fine-tuning phase.\n"
                  "If it is equal to 0,\n"
                  "then the unsupervised signal is ignored.\n");

     declareOption(ol, "input_reconstruction_error", 
                  &DeepFeatureExtractorNNet::input_reconstruction_error,
                  OptionBase::buildoption, 
                   "Input reconstruction error. The reconstruction error\n"
                   "of the hidden layers will always be \"cross_entropy\"."
                   "Choose among:\n"
                   "  - \"cross_entropy\" (default)\n"
                   "  - \"mse\" \n");

     declareOption(ol, "supervised_signal_weight", 
                  &DeepFeatureExtractorNNet::supervised_signal_weight,
                  OptionBase::buildoption, 
                   "Weight of supervised signal used in addition\n"
                  "to unsupervised signal in greedy phase.\n"
                  "This weights should be in [0,1]. If it is equal\n"
                  "to 0, then the supervised signal is ignored.\n"
                  "If it is equal to 1, then the unsupervised signal\n"
                  "is ignored.\n");

     declareOption(ol, "k_nearest_neighbors_reconstruction", 
                  &DeepFeatureExtractorNNet::k_nearest_neighbors_reconstruction,
                  OptionBase::buildoption, 
                   "Number of nearest neighbors to reconstruct in greedy phase.");
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void DeepFeatureExtractorNNet::build_()
{
    /*
     * Create Topology Var Graph
     */

    // nhidden_schedule_position's maximum value is nhidden_schedule.length()+1,
    // which means that the network is in its fine-tuning phase.
    if(nhidden_schedule_position > nhidden_schedule.length()+1)
        nhidden_schedule_position = nhidden_schedule.length()+1;

    // Don't do anything if we don't have a train_set
    // It's the only one who knows the inputsize and targetsize anyway...
    // Also, nothing is done if no layers need to be added
    if(inputsize_>=0 && targetsize_>=0 && weightsize_>=0 
       && nhidden_schedule_current_position < nhidden_schedule.length()+1 
       && nhidden_schedule_current_position < nhidden_schedule_position)
    {

        if(use_n_first_as_supervised > 0)
            sup_train_set = train_set.subMatRows(0,use_n_first_as_supervised);

        // Initialize the input.
        if(nhidden_schedule_current_position < 0)
        {
            input = Var(inputsize(), "input");
            output = input;
            weights.resize(0);
            reconstruction_weights.resize(0);
            params.resize(0);
            biases.resize(0);
            if(use_same_input_and_output_weights)
            {
                Var b = new SourceVariable(1,inputsize());
                b->setName("b0");
                b->value.clear();
                biases.push_back(b);
            }
            if (seed_ != 0) random_gen->manual_seed(seed_);
            if(autoassociator_regularisation_weight > 0) 
            {
                autoassociator_training_costs.resize(nhidden_schedule.length());
                autoassociator_params.resize(nhidden_schedule.length());
            }
        }

        feature_vector = hidden_representation;

        if(nhidden_schedule_current_position < nhidden_schedule_position)
        {
            // Update de network's topology
            if(nhidden_schedule_current_position < nhidden_schedule.length()
               && nhidden_schedule_current_position>=0)
                output = hidden_representation;

            Var before_transfer_function;
            params_to_train.resize(0);  // Will now train new set of weights

            // Will reconstruct input ...
            if(nhidden_schedule_current_position < 0 || always_reconstruct_input)
            {
                if(k_nearest_neighbors_reconstruction>=0)
                    unsupervised_target = 
                        Var((k_nearest_neighbors_reconstruction+1)*inputsize());
                else
                    unsupervised_target = input;
            }
            else // ... or will reconstruct last hidden layer
            {
                if(k_nearest_neighbors_reconstruction>=0)
                    unsupervised_target = 
                        Var((k_nearest_neighbors_reconstruction+1)
                            *nhidden_schedule[nhidden_schedule_current_position]);
                else
                    unsupervised_target = hidden_representation;
            }

            // Number of hidden layers added
            int n_added_layers = 0;

            if((nhidden_schedule_position < nhidden_schedule.length() 
                && supervised_signal_weight != 1) && 
               use_same_input_and_output_weights)
            {
                params_to_train.push_back(biases.last());
            }
        
            // Add new hidden layers until schedule position is reached
            // or all hidden layers have been added
            while(nhidden_schedule_current_position < nhidden_schedule_position 
                  && nhidden_schedule_current_position+1 < 
                  nhidden_schedule.length())
            {
                nhidden_schedule_current_position++;
                n_added_layers++;
                Var w;

                // Share layer and reconstruction weights ...
                if(use_same_input_and_output_weights)
                {
                    // Weights
                    Var w_weights = new SourceVariable(
                        output->size(),
                        nhidden_schedule[nhidden_schedule_current_position]);
                    w_weights->setName("w" + tostring(nhidden_schedule_current_position+1));
                    weights.push_back(w_weights);
                    fillWeights(w_weights,false);
                    params.push_back(w_weights);
                    params_to_train.push_back(w_weights);

                    // Bias
                    Var w_biases = new SourceVariable(
                        1,nhidden_schedule[nhidden_schedule_current_position]);
                    w_biases->setName("b" + tostring(nhidden_schedule_current_position+1));
                    biases.push_back(w_biases);
                    w_biases->value.clear();
                    params.push_back(w_biases);
                    params_to_train.push_back(w_biases);

                    //w = vconcat(w_biases & w_weights);
                    output = hiddenLayer(
                        output,w_weights,w_biases,false,"sigmoid",
                        before_transfer_function,use_activations_with_cubed_input);
                }
                else // ... or have different set of weights.
                {
                    // Weights and bias
                    w = new SourceVariable(
                        output->size()+1,
                        nhidden_schedule[nhidden_schedule_current_position]);
                    w->setName("wb" + tostring(nhidden_schedule_current_position+1));
                    weights.push_back(w);
                    fillWeights(w,true,0);            
                    params.push_back(w);
                    params_to_train.push_back(w);
                    output = hiddenLayer(
                        output,w,"sigmoid",
                        before_transfer_function,use_activations_with_cubed_input);
                }

                hidden_representation = output;
            }

            // Add supervised layer, when all hidden layers have been trained
            // or when a supervised target is also used in the greedy phase.
        
            if(supervised_signal_weight < 0 || supervised_signal_weight > 1)
                PLERROR("In DeepFeatureExtractorNNet::build_(): "
                        "supervised_signal_weight should be in [0,1]");

            Var output_sup;
            if(nhidden_schedule_position < nhidden_schedule.length() 
               && supervised_signal_weight > 0)
                output_sup = output;

            if(nhidden_schedule_current_position < nhidden_schedule_position)
                nhidden_schedule_current_position++;

            if(output_sup || 
               nhidden_schedule_current_position == nhidden_schedule.length())
            {
                if(noutputs<=0) 
                    PLERROR("In DeepFeatureExtractorNNet::build_(): "
                            "building the output layer but noutputs<=0");

                Var w = new SourceVariable(output->size()+1,noutputs);
                w->setName("wbout");
                fillWeights(w,true,0);
            
                // If all hidden layers have been added, these weights
                // can be added to the network
                if(nhidden_schedule_current_position == nhidden_schedule.length())
                {
                    params.push_back(w);
                    weights.push_back(w);
                }

                params_to_train.push_back(w);
                if(output_sup)
                    output_sup = hiddenLayer(
                        output_sup,w,
                        output_transfer_func,before_transfer_function);
                else
                    output = hiddenLayer(output,w,
                                         output_transfer_func,
                                         before_transfer_function);            
            }

            if(nhidden_schedule_current_position < nhidden_schedule_position)
                nhidden_schedule_current_position++;            

            if(nhidden_schedule_current_position == nhidden_schedule.length()+1)
            {
                params_to_train.resize(0);
                // Fine-tune the whole network
                for(int i=0; i<params.length(); i++)
                    params_to_train.push_back(params[i]);
            }

            // Add reconstruction/auto-associator layer
            reconstruction_weights.resize(0);
            if(supervised_signal_weight != 1 
               && nhidden_schedule_current_position < nhidden_schedule.length())
            {
                int it = 0;
                // Add reconstruction/auto-associator layers until last layer
                // is reached, or until input reconstruction is reached
                // if always_reconstruct_input is true
                string rec_trans_func = "some_transfer_func";
                while((!always_reconstruct_input && n_added_layers > 0) 
                      || (always_reconstruct_input && it<weights.size()))
                {                    
                    n_added_layers--;
                    it++;                

                    if((always_reconstruct_input 
                        && nhidden_schedule_current_position-it == -1) 
                       || nhidden_schedule_current_position == 0)
                    {
                        if(input_reconstruction_error == "cross_entropy")
                            rec_trans_func = "sigmoid";
                        else if (input_reconstruction_error == "mse")
                            rec_trans_func = "linear";
                        else PLERROR("In DeepFeatureExtractorNNet::build_(): %s "
                                     "is not a valid reconstruction error", 
                                     input_reconstruction_error.c_str());
                    }
                    else
                        rec_trans_func = "sigmoid";

                    if(use_same_input_and_output_weights)
                    {
                        output =  hiddenLayer(
                            output,weights[weights.size()-it],
                            biases[biases.size()-it-1], 
                            true, rec_trans_func,
                            before_transfer_function,
                            use_activations_with_cubed_input);
                    }
                    else
                    {
                        Var rw;
                        if(nhidden_schedule_current_position-it == -1)
                            rw  = new SourceVariable(output->size()+1,inputsize());
                        else
                            rw  = new SourceVariable(
                                output->size()+1,
                                nhidden_schedule[
                                    nhidden_schedule_current_position-it]);
                        reconstruction_weights.push_back(rw);
                        rw->setName("rwb" + tostring(nhidden_schedule_current_position-it+1));
                        fillWeights(rw,true,0);
                        params_to_train.push_back(rw);
                        output =  hiddenLayer(
                            output,rw, rec_trans_func,
                            before_transfer_function,
                            use_activations_with_cubed_input);
                    }                
                }         
            }

            // Build target and weight variables.
            buildTargetAndWeight();

            // Build costs.
            string pt = lowerstring( penalty_type );
            if( pt == "l1" )
                penalty_type = "L1";
            //else if( pt == "l1_square" || pt == "l1 square" || pt == "l1square" )
            //    penalty_type = "L1_square";
            else if( pt == "l2_square" || pt == "l2 square" || pt == "l2square" )
                penalty_type = "L2_square";
            else if( pt == "l2" )
            {
                PLWARNING("L2 penalty not supported, assuming you want L2 square");
                penalty_type = "L2_square";
            }
            else
                PLERROR("penalty_type \"%s\" not supported", penalty_type.c_str());

            buildCosts(output, target, 
                       unsupervised_target, before_transfer_function, output_sup);
        
            // Build functions.
            buildFuncs(input, output, target, sampleweight);

        }
        
        if((bool)paramsvalues && (paramsvalues.size() == params.nelems()))
            params << paramsvalues;
        else
            paramsvalues.resize(params.nelems());
        params.makeSharedValue(paramsvalues);
        
        // Reinitialize the optimization phase
        if(optimizer)
            optimizer->reset();
        if(optimizer_supervised)
            optimizer_supervised->reset();
        stage = 0;
    }
}

// ### Nothing to add here, simply calls build_
void DeepFeatureExtractorNNet::build()
{
    inherited::build();
    build_();
}


void DeepFeatureExtractorNNet::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // Public

    deepCopyField(nhidden_schedule, copies);
    deepCopyField(optimizer, copies);
    deepCopyField(optimizer_supervised, copies);
    deepCopyField(cost_funcs, copies);
    deepCopyField(paramsvalues, copies);

    // Protected

    deepCopyField(params, copies);
    deepCopyField(params_to_train, copies);
    deepCopyField(weights, copies);
    deepCopyField(reconstruction_weights, copies);
    deepCopyField(biases, copies);
    deepCopyField(invars, copies);
    varDeepCopyField(input, copies);
    varDeepCopyField(output, copies);
    varDeepCopyField(feature_vector, copies);
    varDeepCopyField(hidden_representation, copies);
    varDeepCopyField(neighbor_indices, copies);
    varDeepCopyField(target, copies);
    varDeepCopyField(unsupervised_target, copies);
    varDeepCopyField(sampleweight, copies);
    deepCopyField(costs, copies);
    deepCopyField(penalties, copies);
    varDeepCopyField(training_cost, copies);
    varDeepCopyField(test_costs, copies);
    deepCopyField(sup_train_set, copies);
    deepCopyField(unsup_train_set, copies);
    deepCopyField(knn_train_set, copies);
    deepCopyField(f, copies);
    deepCopyField(test_costf, copies);
    deepCopyField(output_and_target_to_cost, copies);
    deepCopyField(to_feature_vector, copies);
    deepCopyField(autoassociator_params, copies);
    deepCopyField(autoassociator_training_costs, copies);

    

    //PLERROR("DeepFeatureExtractorNNet::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}


int DeepFeatureExtractorNNet::outputsize() const
{
    if(output)
        return output->size();
    else
        return 0;
}

void DeepFeatureExtractorNNet::forget()
{
    if(optimizer)
        optimizer->reset();
    if(optimizer_supervised)
        optimizer_supervised->reset();
    stage = 0;
    
    params.resize(0);
    weights.resize(0);
    nhidden_schedule_current_position = -1;
    build();
}
    
void DeepFeatureExtractorNNet::train()
{
    if(!train_set)
        PLERROR("In DeepFeatureExtractor::train, you did not setTrainingSet");
    
    if(!train_stats)
        PLERROR("In DeepFeatureExtractor::train, you did not setTrainStatsCollector");

    // k nearest neighbors prediction
    if(k_nearest_neighbors_reconstruction>=0 
       && nhidden_schedule_current_position < nhidden_schedule.length())
    {
        if(relative_minimum_improvement <= 0)
            PLERROR("In DeepFeatureExtractorNNEt::build_(): "
                    "relative_minimum_improvement need to be > 0 when "
                    "using nearest neighbors reconstruction");
        if(nhidden_schedule_current_position==0) 
        {
            // Compute nearest neighbors in input space
            if(verbosity > 2) cout << "Computing nearest neighbors" << endl;
            knn_train_set = new AppendNeighborsVMatrix();
            knn_train_set->source = train_set;
            knn_train_set->n_neighbors = k_nearest_neighbors_reconstruction;
            knn_train_set->append_neighbor_indices = false;
            knn_train_set->build();
            unsup_train_set = (VMatrix*) knn_train_set;
            if(verbosity > 2) cout << "Done" << endl;

            // Append input
            unsup_train_set = hconcat(
                new GetInputVMatrix(train_set),unsup_train_set);
            unsup_train_set->defineSizes(train_set->inputsize()*
                                         (k_nearest_neighbors_reconstruction+2),
                                         train_set->targetsize(),
                                         train_set->weightsize()); 
        }
        else
        {
            // Compute nearest neighbors in feature (hidden layer) space
            if(verbosity > 2) cout << "Computing nearest neighbors and performing transformation to hidden representation" << endl;
            knn_train_set->transformation =  to_feature_vector;
            knn_train_set->defineSizes(-1,-1,-1);
            knn_train_set->build();
            unsup_train_set = (VMatrix *)knn_train_set;
            if(verbosity > 2) cout << "Done" << endl;

            int feat_size = to_feature_vector->outputsize;
            // Append input
            unsup_train_set = hconcat(
                new GetInputVMatrix(train_set),unsup_train_set);
            unsup_train_set->defineSizes(
                train_set->inputsize()
                +feat_size*(k_nearest_neighbors_reconstruction+1),
                train_set->targetsize(),train_set->weightsize());            
        }

    }


    int l;
    if(sup_train_set && 
       (supervised_signal_weight == 1
        || nhidden_schedule_current_position >= nhidden_schedule.length()))
        l = sup_train_set->length();  
    else
        if(unsup_train_set 
           && nhidden_schedule_current_position < nhidden_schedule.length())
            l = unsup_train_set->length();  
        else
            l = train_set->length();

    // Net has not been properly built yet 
    // (because build was called before the learner had a proper training set)
    if(f.isNull()) 
        build();

    // Update de DeepFeatureExtractor structure if necessary
    if(nhidden_schedule_current_position < nhidden_schedule_position)
        build();

    // Number of samples seen by optimizer before each optimizer update
    int nsamples;
    if(supervised_signal_weight == 1
       || nhidden_schedule_current_position >= nhidden_schedule.length())
        nsamples = batch_size_supervised>0 ? batch_size_supervised : l;        
    else
        nsamples = batch_size>0 ? batch_size : l;


    // Parameterized function to optimize
    Func paramf = Func(invars, training_cost); 
    Var totalcost;
    
    if(sup_train_set 
       && (supervised_signal_weight == 1
           || nhidden_schedule_current_position >= nhidden_schedule.length()))
        totalcost = meanOf(sup_train_set,paramf,nsamples);
    else
        if(unsup_train_set 
           && nhidden_schedule_current_position < nhidden_schedule.length())
            totalcost = meanOf(unsup_train_set, paramf, nsamples);
        else            
            totalcost = meanOf(train_set, paramf, nsamples);

    PP<Optimizer> this_optimizer;

    if(optimizer_supervised 
       && nhidden_schedule_current_position >= nhidden_schedule.length())
    {
        if(nhidden_schedule_current_position == nhidden_schedule.length()+1
           && autoassociator_regularisation_weight>0)
        {            
            optimizer_supervised->setToOptimize(
                params_to_train, totalcost, autoassociator_training_costs, 
                autoassociator_params, 
                autoassociator_regularisation_weight);
        }
        else
            optimizer_supervised->setToOptimize(params_to_train, totalcost);
        optimizer_supervised->build();
        this_optimizer = optimizer_supervised;
    }
    else if(optimizer)
    {
        if(nhidden_schedule_current_position == nhidden_schedule.length()+1
           && autoassociator_regularisation_weight>0)
            optimizer->setToOptimize(
                params_to_train, totalcost, autoassociator_training_costs, 
                autoassociator_params, autoassociator_regularisation_weight);
        else
            optimizer->setToOptimize(params_to_train, totalcost);

        optimizer->build();
        this_optimizer = optimizer;
    }
    else PLERROR("DeepFeatureExtractor::train can't train without setting "
                 "an optimizer first!");

    // Number of optimizer stages corresponding to one learner stage (one epoch)
    int optstage_per_lstage = l/nsamples;

    PP<ProgressBar> pb;
    if(report_progress)
        pb = new ProgressBar("Training " + classname() + " from stage " 
                             + tostring(stage) + " to " + tostring(nstages), 
                             nstages-stage);

    //displayFunction(paramf, true, false, 250);
    //cout << params_to_train.size() << " params to train" << endl;
    //cout << params.size() << " params" << endl;
    int initial_stage = stage;
    real last_error = REAL_MAX;
    real this_error = 0;
    Vec stats;
    bool flag = (relative_minimum_improvement >= 0 
                 && nhidden_schedule_current_position <= nhidden_schedule.length());

    if(verbosity>2) cout << "Training layer " 
                         << nhidden_schedule_current_position+1 << endl;

    while((stage<nstages || flag))
    {
        this_optimizer->nstages = optstage_per_lstage;
        train_stats->forget();
        this_optimizer->early_stop = false;
        this_optimizer->optimizeN(*train_stats);
        // Uncomment the following if you want to check your new Var.
        // optimizer->verifyGradient(1e-4); 
        train_stats->finalize();
        stats = train_stats->getMean();
        if(verbosity>2)
        {
            if(flag)
                cout << "Initialization epoch, reconstruction train objective: " 
                     << stats << endl;
            else
                cout << "Epoch " << stage << " train objective: " << stats << endl;
        }
        if(pb)
            pb->update(stage-initial_stage);

        this_error = stats[stats.length()-2];
        if(flag 
           && last_error - this_error < relative_minimum_improvement * last_error) 
            break;
        if(!flag) ++stage;
        last_error = this_error;
    }
    if(verbosity>1)
        cout << "EPOCH " << stage << " train objective: " 
             << train_stats->getMean() << endl;

    output_and_target_to_cost->recomputeParents();
    test_costf->recomputeParents();
    
    if(relative_minimum_improvement >= 0 
       && nhidden_schedule_current_position <= nhidden_schedule.length())
    {
        nhidden_schedule_position++;
        totalcost = 0;
        build();
        train();
    }
    //PLERROR("fuck");
}

void DeepFeatureExtractorNNet::computeOutput(const Vec& input, Vec& output) const
{
    output.resize(outputsize());
    f->fprop(input,output);
}    

void DeepFeatureExtractorNNet::computeCostsFromOutputs(const Vec& input, 
                                                       const Vec& output, 
                                           const Vec& target, Vec& costs) const
{
#ifdef BOUNDCHECK
    // Stable cross entropy needs the value *before* the transfer function.
    if (cost_funcs.contains("stable_cross_entropy"))
        PLERROR("In NNet::computeCostsFromOutputs - Cannot directly compute stable "
                "cross entropy from output and target");
#endif
    output_and_target_to_cost->fprop(output&target, costs); 
}

void DeepFeatureExtractorNNet::computeOutputAndCosts(const Vec& inputv, 
                                                     const Vec& targetv, 
                                                     Vec& outputv, 
                                                     Vec& costsv) const
{
    outputv.resize(outputsize());
    test_costf->fprop(inputv&targetv, outputv&costsv);
}

TVec<string> DeepFeatureExtractorNNet::getTestCostNames() const
{
    TVec<string> costs_str = cost_funcs.copy();
    costs_str.push_back("reconstruction_error");
    costs_str.push_back("nhidden_schedule_current_position");
    return costs_str;
}

TVec<string> DeepFeatureExtractorNNet::getTrainCostNames() const
{
    return getTestCostNames();
}

void DeepFeatureExtractorNNet::buildTargetAndWeight() {
    if(targetsize() > 0)
    {        
        target = Var(targetsize(), "target");
        if(weightsize_>0)
        {
            if (weightsize_!=1)
                PLERROR("In NNet::buildTargetAndWeight - Expected weightsize to "
                        "be 1 or 0 (or unspecified = -1, meaning 0), got %d",
                        weightsize_);
            sampleweight = Var(1, "weight");
        }
    }
}

void DeepFeatureExtractorNNet::buildCosts(const Var& the_output, 
                                          const Var& the_target, 
                                          const Var& the_unsupervised_target, 
                                          const Var& before_transfer_func, 
                                          const Var& output_sup) 
{
    costs.resize(0);

    // If in a mainly supervised phase ...
    if(nhidden_schedule_current_position >= nhidden_schedule.length())
    {

        // ... add supervised costs ...
        int ncosts = cost_funcs.size();  
        costs.resize(ncosts);
        
        for(int k=0; k<ncosts; k++)
        {
            // create costfuncs and apply individual weights if weightpart > 1
            if(cost_funcs[k]=="mse")
                costs[k]= sumsquare(the_output-the_target);
            else if(cost_funcs[k]=="mse_onehot")
                costs[k] = onehot_squared_loss(the_output, the_target);
            else if(cost_funcs[k]=="NLL") 
            {
                if (the_output->size() == 1) {
                    // Assume sigmoid output here!
                    costs[k] = cross_entropy(the_output, the_target);
                } else {
                    if (output_transfer_func == "log_softmax")
                        costs[k] = -the_output[the_target];
                    else
                        costs[k] = neg_log_pi(the_output, the_target);
                }
            } 
            else if(cost_funcs[k]=="class_error")
                costs[k] = classification_loss(the_output, the_target);
            else if(cost_funcs[k]=="binary_class_error")
                costs[k] = binary_classification_loss(the_output, the_target);
            else if(cost_funcs[k]=="multiclass_error")
                costs[k] = multiclass_loss(the_output, the_target);
            else if(cost_funcs[k]=="cross_entropy")
                costs[k] = cross_entropy(the_output, the_target);
            else if (cost_funcs[k]=="stable_cross_entropy") {
                Var c = stable_cross_entropy(before_transfer_func, the_target);
                costs[k] = c;
                PLASSERT( classification_regularizer >= 0 );
                if (classification_regularizer > 0) {
                    // There is a regularizer to add to the cost function.
                    dynamic_cast<NegCrossEntropySigmoidVariable*>((Variable*) c)->
                        setRegularizer(classification_regularizer);
                }
            }
            else if (cost_funcs[k]=="margin_perceptron_cost")
                costs[k] = margin_perceptron_cost(the_output,the_target,margin);
            else if (cost_funcs[k]=="lift_output")
                costs[k] = lift_output(the_output, the_target);
            else  // Assume we got a Variable name and its options
            {
                costs[k]= dynamic_cast<Variable*>(newObject(cost_funcs[k]));
                if(costs[k].isNull())
                    PLERROR("In NNet::build_()  unknown cost_func option: %s",
                            cost_funcs[k].c_str());
                costs[k]->setParents(the_output & the_target);
                costs[k]->build();
            }
        }

        // ... and unsupervised cost, which is useless here 
        //     (autoassociator regularisation is incorporated elsewhere, in train())
        Vec val(1);
        val[0] = REAL_MAX;
        costs.push_back(new SourceVariable(val));
    }
    else // If in a mainly unsupervised phase ...
    {
        // ... insert supervised cost if supervised_signal_weight > 0 ...
        if(output_sup)
        {            
            int ncosts = cost_funcs.size();  
            costs.resize(ncosts);
        
            for(int k=0; k<ncosts; k++)
            {
                // create costfuncs and apply individual weights if weightpart > 1
                if(cost_funcs[k]=="mse")
                    costs[k]= sumsquare(output_sup-the_target);
                else if(cost_funcs[k]=="mse_onehot")
                    costs[k] = onehot_squared_loss(output_sup, the_target);
                else if(cost_funcs[k]=="NLL") 
                {
                    if (output_sup->size() == 1) {
                        // Assume sigmoid output here!
                        costs[k] = cross_entropy(output_sup, the_target);
                    } else {
                        if (output_transfer_func == "log_softmax")
                            costs[k] = -output_sup[the_target];
                        else
                            costs[k] = neg_log_pi(output_sup, the_target);
                    }
                } 
                else if(cost_funcs[k]=="class_error")
                    costs[k] = classification_loss(output_sup, the_target);
                else if(cost_funcs[k]=="binary_class_error")
                    costs[k] = binary_classification_loss(output_sup, the_target);
                else if(cost_funcs[k]=="multiclass_error")
                    costs[k] = multiclass_loss(output_sup, the_target);
                else if(cost_funcs[k]=="cross_entropy")
                    costs[k] = cross_entropy(output_sup, the_target);
                else if (cost_funcs[k]=="stable_cross_entropy") {
                    Var c = stable_cross_entropy(before_transfer_func, the_target);
                    costs[k] = c;
                    PLASSERT( classification_regularizer >= 0 );
                    if (classification_regularizer > 0) {
                        // There is a regularizer to add to the cost function.
                        dynamic_cast<NegCrossEntropySigmoidVariable*>((Variable*) c)->
                            setRegularizer(classification_regularizer);
                    }
                }
                else if (cost_funcs[k]=="margin_perceptron_cost")
                    costs[k] = margin_perceptron_cost(output_sup,the_target,margin);
                else if (cost_funcs[k]=="lift_output")
                    costs[k] = lift_output(output_sup, the_target);
                else  // Assume we got a Variable name and its options
                {
                    costs[k]= dynamic_cast<Variable*>(newObject(cost_funcs[k]));
                    if(costs[k].isNull())
                        PLERROR("In NNet::build_()  unknown cost_func option: %s",cost_funcs[k].c_str());
                    costs[k]->setParents(output_sup & the_target);
                    costs[k]->build();
                }

                costs[k] = supervised_signal_weight*costs[k];
            }                    
        }
        else // ... otherwise insert useless maximum cost variables ...
        {
            int ncosts = cost_funcs.size();  
            costs.resize(ncosts);
            Vec val(1);
            val[0] = REAL_MAX;
            for(int i=0; i<costs.length(); i++)
                costs[i] = new SourceVariable(val);
        }
        Var c;

        // ... then insert appropriate unsupervised reconstruction cost ...
        if(supervised_signal_weight == 1) // ... unless only using supervised signal.
        {
            Vec val(1);
            val[0] = REAL_MAX;
            costs.push_back(new SourceVariable(val));
        }
        else
        {
            if(k_nearest_neighbors_reconstruction>=0)
            {
                
                VarArray copies(k_nearest_neighbors_reconstruction+1);
                for(int n=0; n<k_nearest_neighbors_reconstruction+1; n++)
                {
                    if(always_reconstruct_input || nhidden_schedule_position == 0)
                    {
                        if(input_reconstruction_error == "cross_entropy")
                            copies[n] = before_transfer_func;
                        else if (input_reconstruction_error == "mse")
                            copies[n] = the_output;
                    }
                    else
                        copies[n] = before_transfer_func;
                }
                
                Var reconstruct = vconcat(copies);
                
                if(always_reconstruct_input || nhidden_schedule_position == 0)
                {
                    if(input_reconstruction_error == "cross_entropy")
                        c = stable_cross_entropy(reconstruct, the_unsupervised_target);
                    else if (input_reconstruction_error == "mse")
                        c = sumsquare(reconstruct-the_unsupervised_target);
                    else PLERROR("In DeepFeatureExtractorNNet::buildCosts(): %s is not "
                                 "a valid reconstruction error", 
                                 input_reconstruction_error.c_str());
                }
                else
                    c = stable_cross_entropy(reconstruct, the_unsupervised_target);
                
            }
            else
            {
                if(always_reconstruct_input || nhidden_schedule_position == 0)
                {
                    if(input_reconstruction_error == "cross_entropy")
                        c = stable_cross_entropy(before_transfer_func, 
                                                 the_unsupervised_target);
                    else if (input_reconstruction_error == "mse")
                        c = sumsquare(the_output-the_unsupervised_target);
                    else PLERROR("In DeepFeatureExtractorNNet::buildCosts(): %s is not "
                                 "a valid reconstruction error", 
                                 input_reconstruction_error.c_str());
                }
                else
                    c = stable_cross_entropy(before_transfer_func, 
                                             the_unsupervised_target);
            }
        
            if(output_sup) c = (1-supervised_signal_weight) * c + costs[0];
            costs.push_back(c);
        }

        PLASSERT( regularizer >= 0 );
        if (regularizer > 0) {
            // There is a regularizer to add to the cost function.
            dynamic_cast<NegCrossEntropySigmoidVariable*>((Variable*) c)->
                setRegularizer(regularizer);
        }
    }

    // This is so that an EarlyStoppingOracle can be used to
    // do early stopping at each layer
    Vec pos(1);
    pos[0] = -nhidden_schedule_current_position;
    costs.push_back(new SourceVariable(pos));

    /*
     * weight and bias decay penalty
     */

    // create penalties
    buildPenalties();
    test_costs = hconcat(costs);

    // Apply penalty to cost.
    // If there is no penalty, we still add costs[0] as the first cost, in
    // order to keep the same number of costs as if there was a penalty.
    if(penalties.size() != 0) {
        // We only multiply by sampleweight if there are weights
        // and assign the appropriate training cost.
        if (weightsize_>0)
            if(nhidden_schedule_current_position < nhidden_schedule.length() 
               && supervised_signal_weight != 1)
                training_cost = hconcat(
                    sampleweight*sum(hconcat(costs[costs.length()-2] & penalties))
                    & (test_costs*sampleweight));
            else
                training_cost = hconcat(
                    sampleweight*sum(hconcat(costs[0] & penalties))
                    & (test_costs*sampleweight));
        else {
            if(nhidden_schedule_current_position < nhidden_schedule.length() 
               && supervised_signal_weight != 1)
                training_cost = hconcat(sum(hconcat(costs[costs.length()-2] 
                                                    & penalties)) & test_costs);
            else
                training_cost = hconcat(sum(hconcat(costs[0] & penalties)) 
                                        & test_costs);
        }
    }
    else {
        // We only multiply by sampleweight if there are weights
        // and assign the appropriate training cost.
        if(weightsize_>0) {
            if(nhidden_schedule_current_position < nhidden_schedule.length() 
               && supervised_signal_weight != 1)
                training_cost = hconcat(costs[costs.length()-2]*sampleweight 
                                        & test_costs*sampleweight);
            else
                training_cost = hconcat(costs[0]*sampleweight 
                                        & test_costs*sampleweight);
        } else {
            if(nhidden_schedule_current_position < nhidden_schedule.length() 
               && supervised_signal_weight != 1)                
                training_cost = hconcat(costs[costs.length()-2] & test_costs);
            else
                training_cost = hconcat(costs[0] & test_costs);
        }
    }

    training_cost->setName("training_cost");
    test_costs->setName("test_costs");
    the_output->setName("output");
}


Var DeepFeatureExtractorNNet::hiddenLayer(const Var& input, 
                                          const Var& weights, string transfer_func, 
                                          Var& before_transfer_function, 
                                          bool use_cubed_value) {
    Var hidden = affine_transform(input, weights); 
    if(use_cubed_value)
        hidden = pow(hidden,3);    
    before_transfer_function = hidden;
    Var result;
    if(transfer_func=="linear")
        result = hidden;
    else if(transfer_func=="tanh")
        result = tanh(hidden);
    else if(transfer_func=="sigmoid")
        result = sigmoid(hidden);
    else if(transfer_func=="softplus")
        result = softplus(hidden);
    else if(transfer_func=="exp")
        result = exp(hidden);
    else if(transfer_func=="softmax")
        result = softmax(hidden);
    else if (transfer_func == "log_softmax")
        result = log_softmax(hidden);
    else if(transfer_func=="hard_slope")
        result = unary_hard_slope(hidden,0,1);
    else if(transfer_func=="symm_hard_slope")
        result = unary_hard_slope(hidden,-1,1);
    else
        PLERROR("In DeepFeatureExtractorNNet::hiddenLayer - "
                "Unknown value for transfer_func: %s",transfer_func.c_str());
    return result;
}

Var DeepFeatureExtractorNNet::hiddenLayer(const Var& input, 
                                          const Var& weights, const Var& bias, 
                                          bool transpose_weights,
                                          string transfer_func, 
                                          Var& before_transfer_function, 
                                          bool use_cubed_value) {
    Var hidden = bias_weight_affine_transform(input, weights, 
                                              bias,transpose_weights); 
    if(use_cubed_value)
        hidden = pow(hidden,3);    
    before_transfer_function = hidden;
    Var result;
    if(transfer_func=="linear")
        result = hidden;
    else if(transfer_func=="tanh")
        result = tanh(hidden);
    else if(transfer_func=="sigmoid")
        result = sigmoid(hidden);
    else if(transfer_func=="softplus")
        result = softplus(hidden);
    else if(transfer_func=="exp")
        result = exp(hidden);
    else if(transfer_func=="softmax")
        result = softmax(hidden);
    else if (transfer_func == "log_softmax")
        result = log_softmax(hidden);
    else if(transfer_func=="hard_slope")
        result = unary_hard_slope(hidden,0,1);
    else if(transfer_func=="symm_hard_slope")
        result = unary_hard_slope(hidden,-1,1);
    else
        PLERROR("In DeepFeatureExtractorNNet::hiddenLayer - "
                "Unknown value for transfer_func: %s",transfer_func.c_str());
    return result;
}

void DeepFeatureExtractorNNet::buildPenalties() {
    // Prevents penalties from being added twice by consecutive builds
    penalties.resize(0);  
    if(weight_decay > 0 || bias_decay > 0)
    {
        for(int i=0; i<weights.length(); i++)
        {
            // If using same input and output weights,
            // then the weights do not include the bias!
            penalties.append(affine_transform_weight_penalty(
                                 weights[i], weight_decay, 
                                 use_same_input_and_output_weights ? 
                                 weight_decay : bias_decay, 
                                 penalty_type));
        }
        
        if(bias_decay > 0)
            for(int i=0; i<biases.length(); i++)
            {
                penalties.append(affine_transform_weight_penalty(
                                     biases[i], bias_decay, 
                                     bias_decay, 
                                     penalty_type));
            }


        for(int i=0; i<reconstruction_weights.length(); i++)
        {
            penalties.append(affine_transform_weight_penalty(
                                 reconstruction_weights[i], 
                                 weight_decay, bias_decay, penalty_type));
        }                
    }
}

void DeepFeatureExtractorNNet::fillWeights(const Var& weights, 
                                           bool fill_first_row, 
                                           real fill_with_this) {
    if (initialization_method == "zero") {
        weights->value->clear();
        return;
    }
    real delta;
    int is = weights.length();
    if (fill_first_row)
        is--; // -1 to get the same result as before.
    if (initialization_method.find("linear") != string::npos)
        delta = 1.0 / real(is);
    else
        delta = 1.0 / sqrt(real(is));
    if (initialization_method.find("normal") != string::npos)
        random_gen->fill_random_normal(weights->value, 0, delta);
    else
        random_gen->fill_random_uniform(weights->value, -delta, delta);
    if (fill_first_row)
        weights->matValue(0).fill(fill_with_this);
}

void DeepFeatureExtractorNNet::buildFuncs(const Var& the_input, 
                                          const Var& the_output, 
                                          const Var& the_target, 
                                          const Var& the_sampleweight) {
    invars.resize(0);
    VarArray outvars;
    VarArray testinvars;
    if (the_input)
    {
        invars.push_back(the_input);
        testinvars.push_back(the_input);
    }
    if(k_nearest_neighbors_reconstruction>=0 
       && nhidden_schedule_current_position < nhidden_schedule.length())
    {
        invars.push_back(unsupervised_target);
        testinvars.push_back(unsupervised_target);
        if(neighbor_indices)
        {
            invars.push_back(neighbor_indices);
            testinvars.push_back(neighbor_indices);
        }
    }
    if (the_output)
        outvars.push_back(the_output);
    if(the_target)
    {
        invars.push_back(the_target);
        testinvars.push_back(the_target);
        outvars.push_back(the_target);
    }
    if(the_sampleweight)
    {
        invars.push_back(the_sampleweight);
    }
    f = Func(the_input, the_output);
    test_costf = Func(testinvars, the_output&test_costs);
    test_costf->recomputeParents();
    output_and_target_to_cost = Func(outvars, test_costs); 
    output_and_target_to_cost->recomputeParents();

    // To be used later, in the fine-tuning phase
    if(autoassociator_regularisation_weight>0 
       && nhidden_schedule_current_position < nhidden_schedule.length())
    {
        autoassociator_training_costs[nhidden_schedule_current_position] = 
            training_cost;
        autoassociator_params[nhidden_schedule_current_position].resize(
            params_to_train.length());
        for(int i=0; i<params_to_train.length(); i++)
            autoassociator_params[nhidden_schedule_current_position][i] = 
                params_to_train[i];
    }
    to_feature_vector = Func(input,feature_vector);
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
