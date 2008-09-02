// -*- C++ -*-

// NeuralProbabilisticLanguageModel.cc
// Copyright (c) 1998-2002 Pascal Vincent
// Copyright (C) 1999-2002 Yoshua Bengio and University of Montreal
// Copyright (c) 2002 Jean-Sebastien Senecal, Xavier Saint-Mleux, Rejean Ducharme
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

/*! \file PLearn/plearn_learners_experimental/NeuralProbabilisticLanguageModel.h */


#include "NeuralProbabilisticLanguageModel.h"
#include <plearn/vmat/SubVMatrix.h>
//#include <plearn/sys/Profiler.h>
#include <time.h>
#include <stdio.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(NeuralProbabilisticLanguageModel, 
                        "Feedforward neural network for language modeling",
                        "Implementation of the Neural Probabilistic Language "
                        "Model proposed by \n"
                        "Bengio, Ducharme, Vincent and Jauvin (JMLR 2003), "
                        "with extentensions to speedup\n"
                        "the model (Bengio and Sénécal, AISTATS 2003) and "
                        "to include prior information\n"
                        "about the distributed representation and permit "
                        "generalization of these\n"
                        "distributed representations to out-of-vocabulary "
                        "words using features \n"
                        "(Larochelle and Bengio, Tech Report 2006).\n");

NeuralProbabilisticLanguageModel::NeuralProbabilisticLanguageModel() 
// DEFAULT VALUES FOR ALL OPTIONS
    :
rgen(new PRandom()),
nhidden(0),
nhidden2(0),
weight_decay(0),
bias_decay(0),
layer1_weight_decay(0),
layer1_bias_decay(0),
layer2_weight_decay(0),
layer2_bias_decay(0),
output_layer_weight_decay(0),
output_layer_bias_decay(0),
direct_in_to_out_weight_decay(0),
output_layer_dist_rep_weight_decay(0),
output_layer_dist_rep_bias_decay(0),
fixed_output_weights(0),
direct_in_to_out(0),
penalty_type("L2_square"),
output_transfer_func(""),
hidden_transfer_func("tanh"),
start_learning_rate(0.01),
decrease_constant(0),
batch_size(1),
stochastic_gradient_descent_speedup(true),
initialization_method("uniform_linear"),
dist_rep_dim(-1),
possible_targets_vary(false),
train_proposal_distribution(true),
sampling_block_size(50),
minimum_effective_sample_size(100)
{}

NeuralProbabilisticLanguageModel::~NeuralProbabilisticLanguageModel()
{
}

void NeuralProbabilisticLanguageModel::declareOptions(OptionList& ol)
{
    declareOption(ol, "nhidden", &NeuralProbabilisticLanguageModel::nhidden, 
                  OptionBase::buildoption, 
                  "Number of hidden units in first hidden layer (0 means no "
                  "hidden layer).\n");
    
    declareOption(ol, "nhidden2", &NeuralProbabilisticLanguageModel::nhidden2, 
                  OptionBase::buildoption, 
                  "Number of hidden units in second hidden layer (0 means no "
                  "hidden layer).\n");
    
    declareOption(ol, "weight_decay", 
                  &NeuralProbabilisticLanguageModel::weight_decay, 
                  OptionBase::buildoption, 
                  "Global weight decay for all layers.\n");
    
    declareOption(ol, "bias_decay", &NeuralProbabilisticLanguageModel::bias_decay,
                  OptionBase::buildoption, 
                  "Global bias decay for all layers.\n");
    
    declareOption(ol, "layer1_weight_decay", 
                  &NeuralProbabilisticLanguageModel::layer1_weight_decay, 
                  OptionBase::buildoption, 
                  "Additional weight decay for the first hidden layer. "
                  "Is added to weight_decay.\n");
    
    declareOption(ol, "layer1_bias_decay", 
                  &NeuralProbabilisticLanguageModel::layer1_bias_decay, 
                  OptionBase::buildoption, 
                  "Additional bias decay for the first hidden layer. "
                  "Is added to bias_decay.\n");
    
    declareOption(ol, "layer2_weight_decay", 
                  &NeuralProbabilisticLanguageModel::layer2_weight_decay, 
                  OptionBase::buildoption, 
                  "Additional weight decay for the second hidden layer. "
                  "Is added to weight_decay.\n");
    
    declareOption(ol, "layer2_bias_decay", 
                  &NeuralProbabilisticLanguageModel::layer2_bias_decay, 
                  OptionBase::buildoption, 
                  "Additional bias decay for the second hidden layer. "
                  "Is added to bias_decay.\n");
    
    declareOption(ol, "output_layer_weight_decay", 
                  &NeuralProbabilisticLanguageModel::output_layer_weight_decay, 
                  OptionBase::buildoption, 
                  "Additional weight decay for the output layer. "
                  "Is added to 'weight_decay'.\n");
    
    declareOption(ol, "output_layer_bias_decay", 
                  &NeuralProbabilisticLanguageModel::output_layer_bias_decay, 
                  OptionBase::buildoption, 
                  "Additional bias decay for the output layer. "
                  "Is added to 'bias_decay'.\n");
    
    declareOption(ol, "direct_in_to_out_weight_decay", 
                  &NeuralProbabilisticLanguageModel::direct_in_to_out_weight_decay,
                  OptionBase::buildoption,
                  "Additional weight decay for the weights going from the "
                  "input directly to the \n output layer.  Is added to "
                  "'weight_decay'.\n");
    
    declareOption(ol, "output_layer_dist_rep_weight_decay", 
                  &NeuralProbabilisticLanguageModel::output_layer_dist_rep_weight_decay, 
                  OptionBase::buildoption, 
                  "Additional weight decay for the output layer of distributed"
                  "representation\n"
                  "predictor.  Is added to 'weight_decay'.\n");
    
    declareOption(ol, "output_layer_dist_rep_bias_decay", 
                  &NeuralProbabilisticLanguageModel::output_layer_dist_rep_bias_decay, 
                  OptionBase::buildoption, 
                  "Additional bias decay for the output layer of distributed"
                  "representation\n"
                  "predictor.  Is added to 'bias_decay'.\n");
    
    declareOption(ol, "fixed_output_weights", 
                  &NeuralProbabilisticLanguageModel::fixed_output_weights, 
                  OptionBase::buildoption, 
                  "If true then the output weights are not learned. They are"
                  "initialized to +1 or -1 randomly.\n");
    
    declareOption(ol, "direct_in_to_out", 
                  &NeuralProbabilisticLanguageModel::direct_in_to_out, 
                  OptionBase::buildoption, 
                  "If true then direct input to output weights will be added "
                  "(if nhidden > 0).\n");
    
    declareOption(ol, "penalty_type", 
                  &NeuralProbabilisticLanguageModel::penalty_type,
                  OptionBase::buildoption,
                  "Penalty to use on the weights (for weight and bias decay).\n"
                  "Can be any of:\n"
                  "  - \"L1\": L1 norm,\n"
                  "  - \"L2_square\" (default): square of the L2 norm.\n");
    
    declareOption(ol, "output_transfer_func", 
                  &NeuralProbabilisticLanguageModel::output_transfer_func, 
                  OptionBase::buildoption, 
                  "what transfer function to use for ouput layer? One of: \n"
                  "  - \"tanh\" \n"
                  "  - \"sigmoid\" \n"
                  "  - \"softmax\" \n"
                  "An empty string or \"none\" means no output transfer function \n");
    
    declareOption(ol, "hidden_transfer_func", 
                  &NeuralProbabilisticLanguageModel::hidden_transfer_func, 
                  OptionBase::buildoption, 
                  "What transfer function to use for hidden units? One of \n"
                  "  - \"linear\" \n"
                  "  - \"tanh\" \n"
                  "  - \"sigmoid\" \n"
                  "  - \"softmax\" \n");
    
    declareOption(ol, "cost_funcs", &NeuralProbabilisticLanguageModel::cost_funcs, 
                  OptionBase::buildoption, 
                  "A list of cost functions to use\n"
                  "in the form \"[ cf1; cf2; cf3; ... ]\" where each function "
                  "is one of: \n"
                  "  - \"NLL\" (negative log likelihood -log(p[c]) for "
                  "classification) \n"
                  "  - \"class_error\" (classification error) \n"
                  "The FIRST function of the list will be used as \n"
                  "the objective function to optimize \n"
                  "(possibly with an added weight decay penalty) \n");
    
    declareOption(ol, "start_learning_rate", 
                  &NeuralProbabilisticLanguageModel::start_learning_rate, 
                  OptionBase::buildoption, 
                  "Start learning rate of gradient descent.\n");
                  
    declareOption(ol, "decrease_constant", 
                  &NeuralProbabilisticLanguageModel::decrease_constant, 
                  OptionBase::buildoption, 
                  "Decrease constant of gradient descent.\n");

    declareOption(ol, "batch_size", 
                  &NeuralProbabilisticLanguageModel::batch_size, 
                  OptionBase::buildoption, 
                  "How many samples to use to estimate the avergage gradient before updating the weights\n"
                  "0 is equivalent to specifying training_set->length() \n");

    declareOption(ol, "stochastic_gradient_descent_speedup", 
                  &NeuralProbabilisticLanguageModel::stochastic_gradient_descent_speedup, 
                  OptionBase::buildoption, 
                  "Indication that a trick to speedup stochastic "
                  "gradient descent\n"
                  "should be used.\n");

    declareOption(ol, "initialization_method", 
                  &NeuralProbabilisticLanguageModel::initialization_method, 
                  OptionBase::buildoption, 
                  "The method used to initialize the weights:\n"
                  " - \"normal_linear\"  = a normal law with variance "
                  "1/n_inputs\n"
                  " - \"normal_sqrt\"    = a normal law with variance "
                  "1/sqrt(n_inputs)\n"
                  " - \"uniform_linear\" = a uniform law in [-1/n_inputs,"
                  "1/n_inputs]\n"
                  " - \"uniform_sqrt\"   = a uniform law in [-1/sqrt(n_inputs),"
                  "1/sqrt(n_inputs)]\n"
                  " - \"zero\"           = all weights are set to 0\n");
    
    declareOption(ol, "dist_rep_dim", 
                  &NeuralProbabilisticLanguageModel::dist_rep_dim, 
                  OptionBase::buildoption, 
                  " Dimensionality (number of components) of distributed "
                  "representations.\n"
                  "If <= 0, than distributed representations will not be used.\n"
        );
    
    declareOption(ol, "possible_targets_vary", 
                  &NeuralProbabilisticLanguageModel::possible_targets_vary, 
                  OptionBase::buildoption, 
                  "Indication that the set of possible targets vary from\n"
                  "one input vector to another.\n"
        );
    
    declareOption(ol, "feat_sets", &NeuralProbabilisticLanguageModel::feat_sets, 
                                OptionBase::buildoption, 
                  "FeatureSets to apply on input. The number of feature\n"
                  "sets should be a divisor of inputsize(). The feature\n"
                  "sets applied to the ith input field is the feature\n"
                  "set at position i % feat_sets.length().\n"
        );

    declareOption(ol, "train_proposal_distribution", 
                  &NeuralProbabilisticLanguageModel::train_proposal_distribution
                  OptionBase::buildoption, 
                  "Indication that the proposal distribution must be trained\n"
                  "(using train_set).\n"
        );

    declareOption(ol, "sampling_block_size", 
                  &NeuralProbabilisticLanguageModel::sampling_block_size, 
                  OptionBase::buildoption, 
                  "Size of the sampling blocks.\n"
        );

    declareOption(ol, "minimum_effective_sample_size", 
                  &NeuralProbabilisticLanguageModel::minimum_effective_sample_size, 
                  OptionBase::buildoption, 
                  "Minimum effective sample size.\n"
        );

    declareOption(ol, "train_set", &NeuralProbabilisticLanguageModel::train_set, 
                  OptionBase::learntoption, 
                  "VMatrix used for training, that also provides information about the data (e.g. Dictionary objects for the different fields).\n");


                  // Networks' learnt parameters
    declareOption(ol, "w1", &NeuralProbabilisticLanguageModel::w1, 
                  OptionBase::learntoption, 
                  "Weights of first hidden layer.\n");
    declareOption(ol, "b1", &NeuralProbabilisticLanguageModel::b1, 
                  OptionBase::learntoption, 
                  "Bias of first hidden layer.\n");
    declareOption(ol, "w2", &NeuralProbabilisticLanguageModel::w2, 
                  OptionBase::learntoption, 
                  "Weights of second hidden layer.\n");
    declareOption(ol, "b2", &NeuralProbabilisticLanguageModel::b2, 
                  OptionBase::learntoption, 
                  "Bias of second hidden layer.\n");
    declareOption(ol, "wout", &NeuralProbabilisticLanguageModel::wout, 
                  OptionBase::learntoption, 
                  "Weights of output layer.\n");
    declareOption(ol, "bout", &NeuralProbabilisticLanguageModel::bout, 
                  OptionBase::learntoption, 
                  "Bias of output layer.\n");
    declareOption(ol, "direct_wout", 
                  &NeuralProbabilisticLanguageModel::direct_wout, 
                  OptionBase::learntoption, 
                  "Direct input to output weights.\n");
    declareOption(ol, "direct_bout", 
                  &NeuralProbabilisticLanguageModel::direct_bout, 
                  OptionBase::learntoption, 
                  "Direct input to output bias.\n");
    declareOption(ol, "wout_dist_rep", 
                  &NeuralProbabilisticLanguageModel::wout_dist_rep, 
                  OptionBase::learntoption, 
                  "Weights of output layer for distributed representation "
                  "predictor.\n");
    declareOption(ol, "bout_dist_rep", 
                  &NeuralProbabilisticLanguageModel::bout_dist_rep, 
                  OptionBase::learntoption, 
                  "Bias of output layer for distributed representation "
                  "predictor.\n");

    inherited::declareOptions(ol);

}

///////////
// build //
///////////
void NeuralProbabilisticLanguageModel::build()
{
    inherited::build();
    build_();
}


////////////
// build_ //
////////////
void NeuralProbabilisticLanguageModel::build_()
{
    // Don't do anything if we don't have a train_set
    // It's the only one who knows the inputsize, targetsize and weightsize

    if(inputsize_>=0 && targetsize_>=0 && weightsize_>=0)
    {
        if(targetsize_ != 1)
            PLERROR("In NeuralProbabilisticLanguageModel::build_(): "
                    "targetsize_ must be 1, not %d",targetsize_);

        n_feat_sets = feat_sets.length();

        if(n_feat_sets == 0)
            PLERROR("In NeuralProbabilisticLanguageModel::build_(): "
                    "at least one FeatureSet must be provided\n");
        
        if(inputsize_ % n_feat_sets != 0)
            PLERROR("In NeuralProbabilisticLanguageModel::build_(): "
                    "feat_sets.length() must be a divisor of inputsize()");
        
        // Process penalty type option
        string pt = lowerstring( penalty_type );
        if( pt == "l1" )
            penalty_type = "L1";
        else if( pt == "l2_square" || pt == "l2 square" || pt == "l2square" )
            penalty_type = "L2_square";
        else if( pt == "l2" )
        {
            PLWARNING("In NeuralProbabilisticLanguageModel::build_(): "
                      "L2 penalty not supported, assuming you want L2 square");
            penalty_type = "L2_square";
        }
        else
            PLERROR("In NeuralProbabilisticLanguageModel::build_(): "
                    "penalty_type \"%s\" not supported", penalty_type.c_str());
        
        int ncosts = cost_funcs.size();  
        if(ncosts<=0)
            PLERROR("In NeuralProbabilisticLanguageModel::build_(): "
                    "Empty cost_funcs : must at least specify the cost "
                    "function to optimize!");
        
        if(stage <= 0 ) // Training hasn't started
        {
            // Initialize parameters
            initializeParams();                        
        }
        
        output_comp.resize(total_output_size);
        row.resize(train_set->width());
        row.fill(MISSING_VALUE);
        feats.resize(inputsize_);
        // Making sure that all feats[i] have non null storage...
        for(int i=0; i<feats.length(); i++)
        {
            feats[i].resize(1);
            feats[i].resize(0);
        }
        if(fixed_output_weights && stochastic_gradient_descent_speedup)
            PLERROR("In NeuralProbabilisticLanguageModel::build_(): "
                    "cannot use stochastic gradient descent speedup with "
                    "fixed output weights");
        val_string_reference_set = train_set;
        target_values_reference_set = train_set;

        if(proposal_distribution)
        {
            if(batch_size != 1)
                PLERROR("In NeuralProbabilisticLanguageModel::build_(): "
                        "importance sampling speedup is not implemented for"
                        "batch size != 1");
            sample.resize(1);            
            if(train_proposal_distribution)
            {
                proposal_distribution->setTrainingSet(train_set);
                proposal_distribution->train();
            }
        }
    }
}

void NeuralProbabilisticLanguageModel::fprop(const Vec& inputv, Vec& outputv, 
                                             const Vec& targetv, Vec& costsv, 
                                             real sampleweight) const
{
    
    fpropOutput(inputv,outputv);
    //if(is_missing(outputv[0]))
    //    cout << "What the fuck" << endl;
    fpropCostsFromOutput(inputv, outputv, targetv, costsv, sampleweight);
    //if(is_missing(costsv[0]))
    //    cout << "Re-What the fuck" << endl;

}

void NeuralProbabilisticLanguageModel::fpropOutput(const Vec& inputv, 
                                                   Vec& outputv) const
{
    // Forward propagation until reaches output weights, sets last_layer
    fpropBeforeOutputWeights(inputv);
    
    if(dist_rep_dim > 0) // x -> d(x)
    {        
        // d(x),h1(d(x)),h2(h1(d(x))) -> o(x)

        add_affine_transform(last_layer,wout,bout,outputv,false,
                             possible_targets_vary,target_values);            
        if(direct_in_to_out && nhidden>0)
            add_affine_transform(nnet_input,direct_wout,direct_bout,
                                 outputv,false,possible_targets_vary,
                                 target_values);
    }
    else
    {
        // x, h1(x),h2(h1(x)) -> o(x)
        add_affine_transform(last_layer,wout,bout,outputv,nhidden<=0,
                             possible_targets_vary,target_values);            
        if(direct_in_to_out && nhidden>0)
            add_affine_transform(feat_input,direct_wout,direct_bout,
                                 outputv,true,possible_targets_vary,
                                 target_values);
    }
                               
    if (nhidden2>0 && nhidden<=0)
        PLERROR("NeuralProbabilisticLanguageModel::fprop(): "
                "can't have nhidden2 (=%d) > 0 while nhidden=0",nhidden2);
    
    if(output_transfer_func!="" && output_transfer_func!="none")
       add_transfer_func(outputv, output_transfer_func);
}

void NeuralProbabilisticLanguageModel::fpropBeforeOutputWeights(
    const Vec& inputv) const
{
    // Get possible target values
    if(possible_targets_vary) 
    {
        row.subVec(0,inputsize_) << inputv;
        target_values_reference_set->getValues(row,inputsize_,target_values);
        outputv.resize(target_values.length());
    }

    // Get features
    ni = inputsize_;
    nfeats = 0;
    for(int i=0; i<ni; i++)
    {
        str = val_string_reference_set->getValString(i,inputv[i]);
        feat_sets[i%n_feat_sets]->getFeatures(str,feats[i]);
        nfeats += feats[i].length();
    }
    
    feat_input.resize(nfeats);
    offset = 0;
    id = 0;
    for(int i=0; i<ni; i++)
    {
        f = feats[i].data();
        nj = feats[i].length();
        for(int j=0; j<nj; j++)
            feat_input[id++] = offset + *f++;
        if(dist_rep_dim <= 0 || ((i+1) % n_feat_sets != 0))
            offset += feat_sets[i % n_feat_sets]->size();
        else
            offset = 0;
    }

    // Fprop up to output weights
    if(dist_rep_dim > 0) // x -> d(x)
    {        
        nfeats = 0;
        id = 0;
        for(int i=0; i<inputsize_;)
        {
            ifeats = 0;
            for(int j=0; j<n_feat_sets; j++,i++)
                ifeats += feats[i].length();
            
            add_affine_transform(feat_input.subVec(nfeats,ifeats),
                                 wout_dist_rep, bout_dist_rep,
                                 nnet_input.subVec(id*dist_rep_dim,dist_rep_dim),
                                      true, false);
            nfeats += ifeats;
            id++;
        }

        if(nhidden>0) // d(x) -> h1(d(x))
        {
            add_affine_transform(nnet_input,w1,b1,hiddenv,false,false);
            add_transfer_func(hiddenv);

            if(nhidden2>0) // h1(d(x)) -> h2(h1(d(x)))
            {
                add_affine_transform(hiddenv,w2,b2,hidden2v,false,false);
                add_transfer_func(hidden2v);
                last_layer = hidden2v;
            }
            else
                last_layer = hiddenv;
        }
        else
            last_layer = nnet_input;

    }
    else
    {        
        if(nhidden>0) // x -> h1(x)
        {
            add_affine_transform(feat_input,w1,b1,hiddenv,true,false);
            // Transfert function
            add_transfer_func(hiddenv);

            if(nhidden2>0) // h1(x) -> h2(h1(x))
            {
                add_affine_transform(hiddenv,w2,b2,hidden2v,true,false);
                add_transfer_func(hidden2v);
                last_layer = hidden2v;
            }
            else
                last_layer = hiddenv;
        }
        else
            last_layer = feat_input;
    }
}

void NeuralProbabilisticLanguageModel::fpropCostsFromOutput(const Vec& inputv, const Vec& outputv, const Vec& targetv, Vec& costsv, real sampleweight) const
{
    //Compute cost

    if(possible_targets_vary)
    {
        reind_target = target_values.find(targetv[0]);
        if(reind_target<0)
            PLERROR("In NeuralProbabilisticLanguageModel::fprop(): target %d is not in possible targets", targetv[0]);
    }
    else
        reind_target = (int)targetv[0];

    // Build cost function

    int ncosts = cost_funcs.size();
    for(int k=0; k<ncosts; k++)
    {
        if(cost_funcs[k]=="NLL") 
        {
            costsv[k] = sampleweight*nll(outputv,reind_target);
        }
        else if(cost_funcs[k]=="class_error")
            costsv[k] = sampleweight*classification_loss(outputv, reind_target);
        else 
            PLERROR("In NeuralProbabilisticLanguageModel::fprop(): "
                    "unknown cost_func option: %s",cost_funcs[k].c_str());        
    }
}

void NeuralProbabilisticLanguageModel::bprop(Vec& inputv, Vec& outputv, 
                                             Vec& targetv, Vec& costsv, 
                                             real learning_rate, 
                                             real sampleweight)
{
    if(possible_targets_vary) 
    {
        gradient_outputv.resize(target_values.length());
        gradient_act_outputv.resize(target_values.length());
        if(!stochastic_gradient_descent_speedup)
            target_values_since_last_update.append(target_values);
    }

    if(!stochastic_gradient_descent_speedup)
        feats_since_last_update.append(feat_input);

    // Gradient through cost
    if(cost_funcs[0]=="NLL") 
    {
        // Permits to avoid numerical precision errors
        if(output_transfer_func == "softmax")
            gradient_outputv[reind_target] = learning_rate*sampleweight;
        else
            gradient_outputv[reind_target] = learning_rate*sampleweight/(outputv[reind_target]);            
    }
    else if(cost_funcs[0]=="class_error")
    {
        PLERROR("NeuralProbabilisticLanguageModel::bprop(): gradient "
                "cannot be computed for \"class_error\" cost");
    }

    // Gradient through output transfer function
    if(output_transfer_func != "linear")
    {
        if(cost_funcs[0]=="NLL" && output_transfer_func == "softmax")
            gradient_transfer_func(outputv,gradient_act_outputv, gradient_outputv,
                                    output_transfer_func, reind_target);
        else
            gradient_transfer_func(outputv,gradient_act_outputv, gradient_outputv,
                                    output_transfer_func);
        gradient_last_layer = gradient_act_outputv;
    }
    else
        gradient_last_layer = gradient_act_outputv;
    
    // Gradient through output affine transform


    if(nhidden2 > 0) {
        gradient_affine_transform(hidden2v, wout, bout, gradient_hidden2v, 
                                  gradient_wout, gradient_bout, 
                                  gradient_last_layer,
                                  false, possible_targets_vary, 
                                  learning_rate*sampleweight, 
                                  weight_decay+output_layer_weight_decay,
                                  bias_decay+output_layer_bias_decay,
                                  target_values);
    }
    else if(nhidden > 0) 
    {
        gradient_affine_transform(hiddenv, wout, bout, gradient_hiddenv,
                                  gradient_wout, gradient_bout, 
                                  gradient_last_layer,
                                  false, possible_targets_vary, 
                                  learning_rate*sampleweight, 
                                  weight_decay+output_layer_weight_decay,
                                  bias_decay+output_layer_bias_decay, 
                                  target_values);
    }
    else
    {
        gradient_affine_transform(nnet_input, wout, bout, gradient_nnet_input, 
                                  gradient_wout, gradient_bout, 
                                  gradient_last_layer,
                                  (dist_rep_dim <= 0), possible_targets_vary, 
                                  learning_rate*sampleweight, 
                                  weight_decay+output_layer_weight_decay,
                                  bias_decay+output_layer_bias_decay, 
                                  target_values);
    }


    if(nhidden>0 && direct_in_to_out)
    {
        gradient_affine_transform(nnet_input, direct_wout, direct_bout,
                                  gradient_nnet_input, 
                                  gradient_direct_wout, gradient_direct_bout,
                                  gradient_last_layer,
                                  dist_rep_dim<=0, possible_targets_vary,
                                  learning_rate*sampleweight, 
                                  weight_decay+direct_in_to_out_weight_decay,
                                  0,
                                  target_values);
    }


    if(nhidden2 > 0)
    {
        gradient_transfer_func(hidden2v,gradient_act_hidden2v,gradient_hidden2v);
        gradient_affine_transform(hiddenv, w2, b2, gradient_hiddenv, 
                                  gradient_w2, gradient_b2, gradient_act_hidden2v,
                                  false, false,learning_rate*sampleweight, 
                                  weight_decay+layer2_weight_decay,
                                  bias_decay+layer2_bias_decay);
    }
    if(nhidden > 0)
    {
        gradient_transfer_func(hiddenv,gradient_act_hiddenv,gradient_hiddenv);  
        gradient_affine_transform(nnet_input, w1, b1, gradient_nnet_input, 
                                  gradient_w1, gradient_b1, gradient_act_hiddenv,
                                  dist_rep_dim<=0, false,learning_rate*sampleweight, 
                                  weight_decay+layer1_weight_decay,
                                  bias_decay+layer1_bias_decay);
    }

    if(dist_rep_dim > 0)
    {
        nfeats = 0;
        id = 0;
        for(int i=0; i<inputsize_; )
        {
            ifeats = 0;
            for(int j=0; j<n_feat_sets; j++,i++)
                ifeats += feats[i].length();
            gradient_affine_transform(feat_input.subVec(nfeats,ifeats),
                                      wout_dist_rep, bout_dist_rep,
                                      //gradient_feat_input.subVec(nfeats,feats[i].length()),
                                      gradient_feat_input,// Useless anyways...
                                      gradient_wout_dist_rep,
                                      gradient_bout_dist_rep,
                                      gradient_nnet_input.subVec(
                                          id*dist_rep_dim,dist_rep_dim),
                                      true, false, learning_rate*sampleweight, 
                                      weight_decay+
                                      output_layer_dist_rep_weight_decay,
                                      bias_decay+output_layer_dist_rep_bias_decay);
            nfeats += ifeats;
            id++;
        }
    }

    clearProppathGradient();
}

void NeuralProbabilisticLanguageModel::bpropBeforeOutputWeights(
    real learning_rate, 
    real sampleweight)
{
}


void NeuralProbabilisticLanguageModel::update()
{

    if(dist_rep_dim > 0)
    {
        update_affine_transform(feats_since_last_update, wout_dist_rep, 
                                bout_dist_rep, gradient_wout_dist_rep,
                                gradient_bout_dist_rep, true, false,
                                target_values_since_last_update);
    }

    if(nhidden>0) 
    {
        update_affine_transform(feats_since_last_update, w1, b1, 
                                gradient_w1, gradient_b1,
                                dist_rep_dim<=0, false,
                                target_values_since_last_update);
        if(nhidden2>0) 
        {
            update_affine_transform(feats_since_last_update, w2, b2, 
                                    gradient_w2, gradient_b2,
                                    false, false,
                                    target_values_since_last_update);
        }

        update_affine_transform(feats_since_last_update, wout, bout, 
                                gradient_wout, gradient_bout,
                                false, possible_targets_vary,
                                target_values_since_last_update);
        if(direct_in_to_out)
        {
            update_affine_transform(feats_since_last_update, direct_wout, 
                                    direct_bout, 
                                    gradient_direct_wout, gradient_direct_bout,
                                    false, possible_targets_vary,
                                    target_values_since_last_update);
        }
    }
    else
    {
        update_affine_transform(feats_since_last_update, wout, bout, 
                                gradient_wout, gradient_bout,
                                dist_rep_dim<=0, possible_targets_vary,
                                target_values_since_last_update);
    }

    feats_since_last_update.resize(0);
    target_values_since_last_update.resize(0);
}

void NeuralProbabilisticLanguageModel::update_affine_transform(
    Vec input, Mat weights, Vec bias,
    Mat gweights, Vec gbias,
    bool input_is_sparse, bool output_is_sparse,
    Vec output_indices) 
{
    // Bias
    if(bias.length() != 0)
    {
        if(output_is_sparse)
        {
            pval1 = gbias.data();
            pval2 = bias.data();
            pval3 = output_indices.data();
            ni = output_indices.length();
            for(int i=0; i<ni; i++)
            {
                pval2[(int)*pval3] += pval1[(int)*pval3];
                pval1[(int)*pval3] = 0;
                pval3++;
            }
        }
        else
        {
            pval1 = gbias.data();
            pval2 = bias.data();
            ni = bias.length();
            for(int i=0; i<ni; i++)
            {
                *pval2 += *pval1;
                *pval1 = 0;
                pval1++; 
                pval2++;
            }
        }
    }

    // Weights
    if(!input_is_sparse && !output_is_sparse)
    {
        if(!gweights.isCompact() || !weights.isCompact())
            PLERROR("In NeuralProbabilisticLanguageModel::"
                    "update_affine_transform(): weights or gweights is"
                    "not a compact TMat");
        ni = weights.length();
        nj = weights.width();
        pval1 = gweights.data();
        pval2 = weights.data();
        for(int i=0; i<ni; i++)
            for(int j=0; j<nj; j++)
            {
                *pval2 += *pval1;
                *pval1 = 0;
                pval1++;
                pval2++;
            }
    }
    else if(!input_is_sparse && output_is_sparse)
    {
        ni = output_indices.length();
        nj = input.length();
        pval3 = output_indices.data();
        for(int i=0; i<ni; i++)
        {
            for(int j=0; j<nj; j++)
            {
                weights(j,(int)*pval3) += gweights(j,(int)*pval3);
                gweights(j,(int)*pval3) = 0;
            }
            pval3++;
        }
    }
    else if(input_is_sparse && !output_is_sparse)
    {
        ni = input.length();
        nj = weights.width();
        pval3 = input.data();
        for(int i=0; i<ni; i++)
        {
            pval1 = gweights[(int)(*pval3)];
            pval2 = weights[(int)(*pval3++)];
            for(int j=0; j<nj;j++)
            {
                *pval2 += *pval1;
                *pval1 = 0;
                pval1++;
                pval2++;
            }
        }
    }
    else if(input_is_sparse && output_is_sparse)
    {
        // Weights
        ni = input.length();
        nj = output_indices.length();
        pval2 = input.data();
        for(int i=0; i<ni; i++)
        {
            pval3 = output_indices.data();
            for(int j=0; j<nj; j++)
            {
                weights((int)(*pval2),(int)*pval3) += 
                    gweights((int)(*pval2),(int)*pval3);
                gweights((int)(*pval2),(int)*pval3) = 0;
                pval3++;
            }
            pval2++;
        }
    }
}

//! Clear network's gradient fields
void NeuralProbabilisticLanguageModel::clearProppathGradient()
{
    // Trick to make clearProppathGradient faster...
    if(cost_funcs[0]=="NLL") 
        gradient_outputv[reind_target] = 0;
    else
        gradient_outputv.clear();
    gradient_act_outputv.clear();
    
    if(dist_rep_dim>0)
        gradient_nnet_input.clear();

    if(nhidden>0) 
    {
        gradient_hiddenv.clear();
        gradient_act_hiddenv.clear();
        if(nhidden2>0) 
        {
            gradient_hidden2v.clear();
            gradient_act_hidden2v.clear();
        }
    }
}


/////////////////////////////
// computeCostsFromOutputs //
/////////////////////////////
void NeuralProbabilisticLanguageModel::computeCostsFromOutputs(const Vec& inputv, 
                                                               const Vec& outputv,
                                                               const Vec& targetv,
                                                               Vec& costsv) const
{
    PLERROR("In NeuralProbabilisticLanguageModel::computeCostsFromOutputs():"
            "output is not enough to compute costs");
}

int NeuralProbabilisticLanguageModel::my_argmax(const Vec& vec, 
                                                int default_compare) const
{
#ifdef BOUNDCHECK
    if(vec.length()==0)
        PLERROR("IN int argmax(const TVec<T>& vec) vec has zero length");
#endif
    real* v = vec.data();
    int indexmax = default_compare;
    real maxval = v[default_compare];
    for(int i=0; i<vec.length(); i++)
        if(v[i]>maxval)
        {
            maxval = v[i];
            indexmax = i;
        }
    return indexmax;
}

///////////////////
// computeOutput //
///////////////////
void NeuralProbabilisticLanguageModel::computeOutput(const Vec& inputv, 
                                                     Vec& outputv) const
{
    fpropOutput(inputv, output_comp);
    if(possible_targets_vary)
    {
        //row.subVec(0,inputsize_) << inputv;
        //target_values_reference_set->getValues(row,inputsize_,target_values);
        outputv[0] = target_values[
            my_argmax(output_comp,rgen->uniform_multinomial_sample(
                          output_comp.length()))];
    }
    else
        outputv[0] = argmax(output_comp);
}

///////////////////////////
// computeOutputAndCosts //
///////////////////////////
void NeuralProbabilisticLanguageModel::computeOutputAndCosts(const Vec& inputv, 
                                                             const Vec& targetv, 
                                                             Vec& outputv, 
                                                             Vec& costsv) const
{
    fprop(inputv,output_comp,targetv,costsv);
    if(possible_targets_vary)
    {
        //row.subVec(0,inputsize_) << inputv;
        //target_values_reference_set->getValues(row,inputsize_,target_values);
        outputv[0] = 
            target_values[
                my_argmax(output_comp,rgen->uniform_multinomial_sample(
                              output_comp.length()))];
    }
    else
        outputv[0] = argmax(output_comp);
}

/////////////////
// fillWeights //
/////////////////
void NeuralProbabilisticLanguageModel::fillWeights(const Mat& weights) {
    if (initialization_method == "zero") {
        weights.clear();
        return;
    }
    real delta;
    int is = weights.length();
    if (initialization_method.find("linear") != string::npos)
        delta = 1.0 / real(is);
    else
        delta = 1.0 / sqrt(real(is));
    if (initialization_method.find("normal") != string::npos)
        rgen->fill_random_normal(weights, 0, delta);
    else
        rgen->fill_random_uniform(weights, -delta, delta);
}

////////////
// forget //
////////////
void NeuralProbabilisticLanguageModel::forget()
{
    if (train_set) build();
    total_updates=0;
    stage = 0;
}

///////////////////////
// getTrainCostNames //
///////////////////////
TVec<string> NeuralProbabilisticLanguageModel::getTrainCostNames() const
{
    return cost_funcs;
}

//////////////////////
// getTestCostNames //
//////////////////////
TVec<string> NeuralProbabilisticLanguageModel::getTestCostNames() const
{ 
    return cost_funcs;
}

///////////////////////
// add_transfer_func //
///////////////////////
void NeuralProbabilisticLanguageModel::add_transfer_func(const Vec& input, 
                                                         string transfer_func) 
    const
{
    if (transfer_func == "default")
        transfer_func = hidden_transfer_func;
    if(transfer_func=="linear")
        return;
    else if(transfer_func=="tanh")
    {
        compute_tanh(input,input);
        return;
    }        
    else if(transfer_func=="sigmoid")
    {
        compute_sigmoid(input,input);
        return;
    }
    else if(transfer_func=="softmax")
    {
        compute_softmax(input,input);
        return;
    }
    else PLERROR("In NeuralProbabilisticLanguageModel::add_transfer_func(): "
                 "Unknown value for transfer_func: %s",transfer_func.c_str());
}

////////////////////////////
// gradient_transfer_func //
////////////////////////////
void NeuralProbabilisticLanguageModel::gradient_transfer_func(
    Vec& output, 
    Vec& gradient_input,
    Vec& gradient_output,
    string transfer_func,
    int nll_softmax_speed_up_target) 
{
    if (transfer_func == "default")        
        transfer_func = hidden_transfer_func;
    if(transfer_func=="linear")
    {
        pval1 = gradient_output.data();
        pval2 = gradient_input.data();
        ni = output.length();
        for(int i=0; i<ni; i++)
            *pval2++ += *pval1++;
        return;
    }
    else if(transfer_func=="tanh")
    {
        pval1 = gradient_output.data();
        pval2 = output.data();
        pval3 = gradient_input.data();
        ni = output.length();
        for(int i=0; i<ni; i++)
            *pval3++ += (*pval1++)*(1.0-square(*pval2++));
        return;
    }        
    else if(transfer_func=="sigmoid")
    {
        pval1 = gradient_output.data();
        pval2 = output.data();
        pval3 = gradient_input.data();
        ni = output.length();
        for(int i=0; i<ni; i++)
        {
            *pval3++ += (*pval1++)*(*pval2)*(1.0-*pval2);
            pval2++;
        }   
        return;
    }
    else if(transfer_func=="softmax")
    {
        if(nll_softmax_speed_up_target<0)
        {            
            pval3 = gradient_input.data();
            ni = nk = output.length();
            for(int i=0; i<ni; i++)
            {
                val = output[i];
                pval1 = gradient_output.data();
                pval2 = output.data();
                for(int k=0; k<nk; k++)
                    if(k!=i)
                        *pval3 -= *pval1++ * val * (*pval2++);
                    else
                    {
                        *pval3 += *pval1++ * val * (1.0-val);
                        pval2++;
                    }
                pval3++;                
            }   
        }
        else // Permits speedup and avoids numerical precision errors
        {
            pval2 = output.data();
            pval3 = gradient_input.data();
            ni = output.length();
            grad = gradient_output[nll_softmax_speed_up_target];
            val = output[nll_softmax_speed_up_target];
            for(int i=0; i<ni; i++)
            {
                if(nll_softmax_speed_up_target!=i)
                    //*pval3++ -= grad * val * (*pval2++);
                    *pval3++ -= grad * (*pval2++);
                else
                {
                    //*pval3++ += grad * val * (1.0-val);
                    *pval3++ += grad * (1.0-val);
                    pval2++;
                }
            }   
        }
        return;
    }
    else PLERROR("In NeuralProbabilisticLanguageModel::gradient_transfer_func():"
                 "Unknown value for transfer_func: %s",transfer_func.c_str());
}

void NeuralProbabilisticLanguageModel::add_affine_transform(
    Vec input, 
    Mat weights, 
    Vec bias, Vec output, 
    bool input_is_sparse, bool output_is_sparse,
    Vec output_indices) const
{
    // Bias
    if(bias.length() != 0)
    {
        if(output_is_sparse)
        {
            pval1 = output.data();
            pval2 = bias.data();
            pval3 = output_indices.data();
            ni = output.length();
            for(int i=0; i<ni; i++)
                *pval1++ = pval2[(int)*pval3++];
        }
        else
        {
            pval1 = output.data();
            pval2 = bias.data();
            ni = output.length();
            for(int i=0; i<ni; i++)
                *pval1++ = *pval2++;
        }
    }

    // Weights
    if(!input_is_sparse && !output_is_sparse)
    {
        transposeProductAcc(output,weights,input);
    }
    else if(!input_is_sparse && output_is_sparse)
    {
        ni = output.length();
        nj = input.length();
        pval1 = output.data();
        pval3 = output_indices.data();
        for(int i=0; i<ni; i++)
        {
            pval2 = input.data();
            for(int j=0; j<nj; j++)
                *pval1 += (*pval2++)*weights(j,(int)*pval3);
            pval1++;
            pval3++;
        }
    }
    else if(input_is_sparse && !output_is_sparse)
    {
        ni = input.length();
        nj = output.length();
        if(ni != 0)
        {
            pval3 = input.data();
            for(int i=0; i<ni; i++)
            {
                pval1 = output.data();
                pval2 = weights[(int)(*pval3++)];
                for(int j=0; j<nj;j++)
                    *pval1++ += *pval2++;
            }
        }
    }
    else if(input_is_sparse && output_is_sparse)
    {
        // Weights
        ni = input.length();
        nj = output.length();
        if(ni != 0)
        {
            pval2 = input.data();
            for(int i=0; i<ni; i++)
            {
                pval1 = output.data();
                pval3 = output_indices.data();
                for(int j=0; j<nj; j++)
                    *pval1++ += weights((int)(*pval2),(int)*pval3++);
                pval2++;
            }
        }
    }
}

void NeuralProbabilisticLanguageModel::gradient_affine_transform(
    Vec input, Mat weights, Vec bias, 
    Vec ginput, Mat gweights, Vec gbias,
    Vec goutput, bool input_is_sparse, 
    bool output_is_sparse,
    real learning_rate,
    real weight_decay, real bias_decay,
    Vec output_indices)
{
    // Bias
    if(bias.length() != 0)
    {
        if(output_is_sparse)
        {
            pval1 = gbias.data();
            pval2 = goutput.data();
            pval3 = output_indices.data();
            ni = goutput.length();
            
            if(fast_exact_is_equal(bias_decay, 0))
            {
                // Without bias decay
                for(int i=0; i<ni; i++)
                    pval1[(int)*pval3++] += *pval2++;
            }
            else
            {
                // With bias decay
                if(penalty_type == "L2_square")
                {
                    pval4 = bias.data();
                    val = -two(learning_rate)*bias_decay;
                    for(int i=0; i<ni; i++)
                    {
                        pval1[(int)*pval3] += *pval2++ + val*(pval4[(int)*pval3]);
                        pval3++;
                    }
                }
                else if(penalty_type == "L1")
                {
                    pval4 = bias.data();
                    val = -learning_rate*bias_decay;
                    for(int i=0; i<ni; i++)
                    {
                        val2 = pval4[(int)*pval3];
                        if(val2 > 0 )
                            pval1[(int)*pval3] += *pval2 + val;
                        else if(val2 < 0)
                            pval1[(int)*pval3] += *pval2 - val;
                        pval2++;
                        pval3++;
                    }
                }
            }
        }
        else
        {
            pval1 = gbias.data();
            pval2 = goutput.data();
            ni = goutput.length();
            if(fast_exact_is_equal(bias_decay, 0))
            {
                // Without bias decay
                for(int i=0; i<ni; i++)
                    *pval1++ += *pval2++;
            }
            else
            {
                // With bias decay
                if(penalty_type == "L2_square")
                {
                    pval3 = bias.data();
                    val = -two(learning_rate)*bias_decay;
                    for(int i=0; i<ni; i++)
                    {
                        *pval1++ += *pval2++ + val * (*pval3++);
                    }
                }
                else if(penalty_type == "L1")
                {
                    pval3 = bias.data();
                    val = -learning_rate*bias_decay;
                    for(int i=0; i<ni; i++)
                    {
                        if(*pval3 > 0)
                            *pval1 += *pval2 + val;
                        else if(*pval3 < 0)
                            *pval1 += *pval2 - val;
                        pval1++;
                        pval2++;
                        pval3++;
                    }
                }
            }
        }
    }

    // Weights and input (when appropriate)
    if(!input_is_sparse && !output_is_sparse)
    {        
        // Input
        //productAcc(ginput, weights, goutput);
        // Weights
        //externalProductAcc(gweights, input, goutput);

        // Faster code to do this, which limits the accesses
        // to memory

        ni = input.length();
        nj = goutput.length();
        pval3 = ginput.data();
        pval5 = input.data();
        
        if(fast_exact_is_equal(weight_decay, 0))
        {
            // Without weight decay
            for(int i=0; i<ni; i++) {
                
                pval1 = goutput.data();
                pval2 = weights[i];
                pval4 = gweights[i];
                for(int j=0; j<nj; j++) {
                    *pval3 += *pval2 * (*pval1);
                    *pval4 += *pval5 * (*pval1);
                    pval1++;
                    pval2++;
                    pval4++;
                }
                pval3++;
                pval5++;
            }   
        }
        else
        {
            //With weight decay            
            if(penalty_type == "L2_square")
            {
                val = -two(learning_rate)*weight_decay;
                for(int i=0; i<ni; i++) {   
                    pval1 = goutput.data();
                    pval2 = weights[i];
                    pval4 = gweights[i];
                    for(int j=0; j<nj; j++) {
                        *pval3 += *pval2 * (*pval1);
                        *pval4 += *pval5 * (*pval1) + val * (*pval2);
                        pval1++;
                        pval2++;
                        pval4++;
                    }
                    pval3++;
                    pval5++;
                }
            }
            else if(penalty_type == "L1")
            {
                val = -learning_rate*weight_decay;
                for(int i=0; i<ni; i++) {
                    
                    pval1 = goutput.data();
                    pval2 = weights[i];
                    pval4 = gweights[i];
                    for(int j=0; j<nj; j++) {
                        *pval3 += *pval2 * (*pval1);
                        if(*pval2 > 0)
                            *pval4 += *pval5 * (*pval1) + val;
                        else if(*pval2 < 0)
                            *pval4 += *pval5 * (*pval1) - val;
                        pval1++;
                        pval2++;
                        pval4++;
                    }
                    pval3++;
                    pval5++;
                }
            }
        }
    }
    else if(!input_is_sparse && output_is_sparse)
    {
        ni = goutput.length();
        nj = input.length();
        pval1 = goutput.data();
        pval3 = output_indices.data();
        
        if(fast_exact_is_equal(weight_decay, 0))
        {
            // Without weight decay
            for(int i=0; i<ni; i++)
            {
                pval2 = input.data();
                pval4 = ginput.data();
                for(int j=0; j<nj; j++)
                {
                    // Input
                    *pval4++ += weights(j,(int)(*pval3))*(*pval1);
                    // Weights
                    gweights(j,(int)(*pval3)) += (*pval2++)*(*pval1);
                }
                pval1++;
                pval3++;
            }
        }
        else
        {
            // With weight decay
            if(penalty_type == "L2_square")
            {
                val = -two(learning_rate)*weight_decay;
                for(int i=0; i<ni; i++)
                {
                    pval2 = input.data();
                    pval4 = ginput.data();
                    for(int j=0; j<nj; j++)
                    {
                        val2 = weights(j,(int)(*pval3));
                        // Input
                        *pval4++ += val2*(*pval1);
                        // Weights
                        gweights(j,(int)(*pval3)) += (*pval2++)*(*pval1) 
                            + val*val2;
                    }
                    pval1++;
                    pval3++;
                }
            }
            else if(penalty_type == "L1")
            {
                val = -learning_rate*weight_decay;
                for(int i=0; i<ni; i++)
                {
                    pval2 = input.data();
                    pval4 = ginput.data();
                    for(int j=0; j<nj; j++)
                    {
                        val2 = weights(j,(int)(*pval3));
                        // Input
                        *pval4++ += val2*(*pval1);
                        // Weights
                        if(val2 > 0)
                            gweights(j,(int)(*pval3)) += (*pval2)*(*pval1) + val;
                        else if(val2 < 0)
                            gweights(j,(int)(*pval3)) += (*pval2)*(*pval1) - val;
                        pval2++;
                    }
                    pval1++;
                    pval3++;
                }
            }
        }
    }
    else if(input_is_sparse && !output_is_sparse)
    {
        ni = input.length();
        nj = goutput.length();

        if(fast_exact_is_equal(weight_decay, 0))
        {
            // Without weight decay
            if(ni != 0)
            {
                pval3 = input.data();
                for(int i=0; i<ni; i++)
                {
                    pval1 = goutput.data();
                    pval2 = gweights[(int)(*pval3++)];
                    for(int j=0; j<nj;j++)
                        *pval2++ += *pval1++;
                }
            }
        }
        else
        {
            // With weight decay
            if(penalty_type == "L2_square")
            {
                if(ni != 0)
                {
                    pval3 = input.data();                    
                    val = -two(learning_rate)*weight_decay;
                    for(int i=0; i<ni; i++)
                    {
                        pval1 = goutput.data();
                        pval2 = gweights[(int)(*pval3)];
                        pval4 = weights[(int)(*pval3++)];
                        for(int j=0; j<nj;j++)
                        {
                            *pval2++ += *pval1++ + val * (*pval4++);
                        }
                    }
                }
            }
            else if(penalty_type == "L1")
            {
                if(ni != 0)
                {
                    pval3 = input.data();
                    val = learning_rate*weight_decay;
                    for(int i=0; i<ni; i++)
                    {
                        pval1 = goutput.data();
                        pval2 = gweights[(int)(*pval3)];
                        pval4 = weights[(int)(*pval3++)];
                        for(int j=0; j<nj;j++)
                        {
                            if(*pval4 > 0)
                                *pval2 += *pval1 + val;
                            else if(*pval4 < 0)
                                *pval2 += *pval1 - val;
                            pval1++;
                            pval2++;
                            pval4++;
                        }
                    }
                }
            }
        }
    }
    else if(input_is_sparse && output_is_sparse)
    {
        ni = input.length();
        nj = goutput.length();

        if(fast_exact_is_equal(weight_decay, 0))
        {
            // Without weight decay
            if(ni != 0)
            {
                pval2 = input.data();
                for(int i=0; i<ni; i++)
                {
                    pval1 = goutput.data();
                    pval3 = output_indices.data();
                    for(int j=0; j<nj; j++)
                        gweights((int)(*pval2),(int)*pval3++) += *pval1++;
                    pval2++;
                }
            }
        }
        else
        {
            // With weight decay
            if(penalty_type == "L2_square")
            {
                if(ni != 0)
                {
                    pval2 = input.data();
                    val = -two(learning_rate)*weight_decay;                    
                    for(int i=0; i<ni; i++)
                    {
                        pval1 = goutput.data();
                        pval3 = output_indices.data();
                        for(int j=0; j<nj; j++)
                        {
                            gweights((int)(*pval2),(int)*pval3) 
                                += *pval1++ 
                                + val * weights((int)(*pval2),(int)*pval3);
                            pval3++;
                        }
                        pval2++;
                    }
                }
            }
            else if(penalty_type == "L1")
            {
                if(ni != 0)
                {
                    pval2 = input.data();
                    val = -learning_rate*weight_decay;                    
                    for(int i=0; i<ni; i++)
                    {
                        pval1 = goutput.data();
                        pval3 = output_indices.data();
                        for(int j=0; j<nj; j++)
                        {
                            val2 = weights((int)(*pval2),(int)*pval3);
                            if(val2 > 0)
                                gweights((int)(*pval2),(int)*pval3) 
                                    += *pval1 + val;
                            else if(val2 < 0)
                                gweights((int)(*pval2),(int)*pval3) 
                                    += *pval1 - val;
                            pval1++;
                            pval3++;
                        }
                        pval2++;
                    }
                }
            }
        }
    }

//    gradient_penalty(input,weights,bias,gweights,gbias,input_is_sparse,output_is_sparse,
//                     learning_rate,weight_decay,bias_decay,output_indices);
}

void NeuralProbabilisticLanguageModel::gradient_penalty(
    Vec input, Mat weights, Vec bias, 
    Mat gweights, Vec gbias,
    bool input_is_sparse, bool output_is_sparse,
    real learning_rate,
    real weight_decay, real bias_decay,
    Vec output_indices)
{
    // Bias
    if(!fast_exact_is_equal(bias_decay, 0) && !fast_exact_is_equal(bias.length(),
                                                                   0) )
    {
        if(output_is_sparse)
        {
            pval1 = gbias.data();
            pval2 = bias.data();
            pval3 = output_indices.data();
            ni = output_indices.length();            
            if(penalty_type == "L2_square")
            {
                val = -two(learning_rate)*bias_decay;
                for(int i=0; i<ni; i++)
                {
                    pval1[(int)*pval3] += val*(pval2[(int)*pval3]);
                    pval3++;
                }
            }
            else if(penalty_type == "L1")
            {
                val = -learning_rate*bias_decay;
                for(int i=0; i<ni; i++)
                {
                    val2 = pval2[(int)*pval3];
                    if(val2 > 0 )
                        pval1[(int)*pval3++] += val;
                    else if(val2 < 0)
                        pval1[(int)*pval3++] -= val;
                }
            }
        }
        else
        {
            pval1 = gbias.data();
            pval2 = bias.data();
            ni = output_indices.length();            
            if(penalty_type == "L2_square")
            {
                val = -two(learning_rate)*bias_decay;
                for(int i=0; i<ni; i++)
                    *pval1++ += val*(*pval2++);
            }
            else if(penalty_type == "L1")
            {
                val = -learning_rate*bias_decay;
                for(int i=0; i<ni; i++)
                {
                    if(*pval2 > 0)
                        *pval1 += val;
                    else if(*pval2 < 0)
                        *pval1 -= val;
                    pval1++;
                    pval2++;
                }
            }
        }
    }

    // Weights
    if(!fast_exact_is_equal(weight_decay, 0))
    {
        if(!input_is_sparse && !output_is_sparse)
        {      
            if(penalty_type == "L2_square")
            {
                multiplyAcc(gweights, weights,-two(learning_rate)*weight_decay);
            }
            else if(penalty_type == "L1")
            {
                val = -learning_rate*weight_decay;
                if(gweights.isCompact() && weights.isCompact())
                {
                    Mat::compact_iterator itm = gweights.compact_begin();
                    Mat::compact_iterator itmend = gweights.compact_end();
                    Mat::compact_iterator itx = weights.compact_begin();
                    for(; itm!=itmend; ++itm, ++itx)
                    {
                        if(*itx > 0)
                            *itm += val;
                        else if(*itx < 0)
                            *itm -= val;
                    }
                }
                else // use non-compact iterators
                {
                    Mat::iterator itm = gweights.begin();
                    Mat::iterator itmend = gweights.end();
                    Mat::iterator itx = weights.begin();
                    for(; itm!=itmend; ++itm, ++itx)
                    {
                        if(*itx > 0)
                            *itm += val;
                        else if(*itx < 0)
                            *itm -= val;
                    }
                }
            }
        }
        else if(!input_is_sparse && output_is_sparse)
        {
            ni = output_indices.length();
            nj = input.length();
            pval1 = output_indices.data();

            if(penalty_type == "L2_square")
            {
                val = -two(learning_rate)*weight_decay;
                for(int i=0; i<ni; i++)
                {
                    for(int j=0; j<nj; j++)
                    {
                        gweights(j,(int)(*pval1)) += val * 
                            weights(j,(int)(*pval1));
                    }
                    pval1++;
                }
            }
            else if(penalty_type == "L1")
            {
                val = -learning_rate*weight_decay;
                for(int i=0; i<ni; i++)
                {
                    for(int j=0; j<nj; j++)
                    {
                        val2 = weights(j,(int)(*pval1));
                        if(val2 > 0)
                            gweights(j,(int)(*pval1)) +=  val;
                        else if(val2 < 0)
                            gweights(j,(int)(*pval1)) -=  val;
                    }
                    pval1++;
                }
            }
        }
        else if(input_is_sparse && !output_is_sparse)
        {
            ni = input.length();
            nj = output_indices.length();
            if(ni != 0)
            {
                pval3 = input.data();
                if(penalty_type == "L2_square")
                {
                    val = -two(learning_rate)*weight_decay;
                    for(int i=0; i<ni; i++)
                    {
                        pval1 = weights[(int)(*pval3)];
                        pval2 = gweights[(int)(*pval3++)];
                        for(int j=0; j<nj;j++)
                            *pval2++ += val * *pval1++;
                    }
                }
                else if(penalty_type == "L1")
                {
                    val = -learning_rate*weight_decay;
                    for(int i=0; i<ni; i++)
                    {
                        pval1 = weights[(int)(*pval3)];
                        pval2 = gweights[(int)(*pval3++)];
                        for(int j=0; j<nj;j++)
                        {
                            if(*pval1 > 0)
                                *pval2 += val;
                            else if(*pval1 < 0)
                                *pval2 -= val;
                            pval2++;
                            pval1++;
                        }
                    }                
                }
            }
        }
        else if(input_is_sparse && output_is_sparse)
        {
            ni = input.length();
            nj = output_indices.length();
            if(ni != 0)
            {
                pval1 = input.data();
                if(penalty_type == "L2_square")
                {
                    val = -two(learning_rate)*weight_decay;
                    for(int i=0; i<ni; i++)
                    {
                        pval2 = output_indices.data();
                        for(int j=0; j<nj; j++)
                        {
                            gweights((int)(*pval1),(int)*pval2) += val*
                                weights((int)(*pval1),(int)*pval2);
                        pval2++;
                        }
                        pval1++;
                    }
                }
                else if(penalty_type == "L1")
                {
                    val = -learning_rate*weight_decay;
                    for(int i=0; i<ni; i++)
                    {
                        pval2 = output_indices.data();
                        for(int j=0; j<nj; j++)
                        {
                            val2 = weights((int)(*pval1),(int)*pval2);
                            if(val2 > 0)
                                gweights((int)(*pval1),(int)*pval2) += val;
                            else if(val2 < 0)
                                gweights((int)(*pval1),(int)*pval2) -= val;
                            pval2++;
                        }
                        pval1++;
                    }
                    
                }
            }
        }
    }
}

void NeuralProbabilisticLanguageModel::importance_sampling_gradient_update(
    Vec& inputv, Vec& targetv, 
    real learning_rate, int n_samples, 
    real train_sample_weight=1)
{
    // TODO: implement NGramDistribution::generate()
    //       adjust deepcopy(...)

    // Do forward propagation that is common to all computations
    fpropBeforeOutputWeights(inputv);

    // Generate the n_samples samples from proposal_distribution
    generated_samples.resize(n_samples+1);
    densities.resize(n_samples);
    
    proposal_distribution->setPredictor(inputv);
    pval1 = generated_samples.data();
    pval2 = sample.data();
    pval3 = densities.data();
    for(int i=0; i<n_samples; i++)
    {
        proposal_distribution->generate(sample);        
        *pval1++ = *pval2;
        *pval3++ = proposal_distribution->density(sample);        
    }

    real sum = 0;
    generated_samples[n_samples] = targetv[0];
    neg_energies.resize(n_samples+1);
    getNegativeEnergyValues(generated_samples, neg_energies);
    
    importance_sampling_ratios.resize(
        importance_sampling_ratios.length() + n_samples);
    pval1 = importance_sampling_ratios.subVec(
        importance_sampling_ratios.length() - n_samples).data();
    pval2 = neg_energies.data();
    pval3 = densities.data();
    for(int i=0; i<n_samples; i++)
    {
        *pval1 = exp(*pval2++)/ (*pval3++);
        sum += *pval1;
    }

    // Compute importance sampling estimate of the gradient

    // Training sample contribution...
    gradient_last_layer.resize(1);
    gradient_last_layer[0] = learning_rate*train_sample_weight;

    if(nhidden2 > 0) {
        gradient_affine_transform(hidden2v, wout, bout, gradient_hidden2v, 
                                  gradient_wout, gradient_bout, 
                                  gradient_last_layer,
                                  false, true, learning_rate*train_sample_weight, 
                                  weight_decay+output_layer_weight_decay,
                                  bias_decay+output_layer_bias_decay,
                                  generated_samples.subVec(n_samples,1));
    }
    else if(nhidden > 0) 
    {
        gradient_affine_transform(hiddenv, wout, bout, gradient_hiddenv,
                                  gradient_wout, gradient_bout, 
                                  gradient_last_layer,
                                  false, true, learning_rate*train_sample_weight, 
                                  weight_decay+output_layer_weight_decay,
                                  bias_decay+output_layer_bias_decay, 
                                  generated_samples.subVec(n_samples,1));
    }
    else
    {
        gradient_affine_transform(nnet_input, wout, bout, gradient_nnet_input, 
                                  gradient_wout, gradient_bout, 
                                  gradient_last_layer,
                                  (dist_rep_dim <= 0), true, 
                                  learning_rate*train_sample_weight, 
                                  weight_decay+output_layer_weight_decay,
                                  bias_decay+output_layer_bias_decay, 
                                  generated_samples.subVec(n_samples,1));
    }


    if(nhidden>0 && direct_in_to_out)
    {
        gradient_affine_transform(nnet_input, direct_wout, direct_bout,
                                  gradient_nnet_input, 
                                  gradient_direct_wout, gradient_direct_bout,
                                  gradient_last_layer,
                                  dist_rep_dim<=0, true,
                                  learning_rate*train_sample_weight, 
                                  weight_decay+direct_in_to_out_weight_decay,
                                  0,
                                  generated_samples.subVec(n_samples,1));
    }

    // Importance sampling contributions
    for(int i=0; i<n_samples; i++)
    {
        gradient_last_layer.resize(1);
        gradient_last_layer[0] = -learning_rate*train_sample_weight*
            importance_sampling_ratios[i]/sum;

        if(nhidden2 > 0) {
            gradient_affine_transform(hidden2v, wout, bout, gradient_hidden2v, 
                                      gradient_wout, gradient_bout, 
                                      gradient_last_layer,
                                      false, true, 
                                      learning_rate*train_sample_weight, 
                                      weight_decay+output_layer_weight_decay,
                                      bias_decay+output_layer_bias_decay,
                                      generated_samples.subVec(i,1));
        }
        else if(nhidden > 0) 
        {
            gradient_affine_transform(hiddenv, wout, bout, gradient_hiddenv,
                                      gradient_wout, gradient_bout, 
                                      gradient_last_layer,
                                      false, true, 
                                      learning_rate*train_sample_weight, 
                                      weight_decay+output_layer_weight_decay,
                                      bias_decay+output_layer_bias_decay, 
                                      generated_samples.subVec(i,1));
        }
        else
        {
            gradient_affine_transform(nnet_input, wout, bout, 
                                      gradient_nnet_input, 
                                      gradient_wout, gradient_bout, 
                                      gradient_last_layer,
                                      (dist_rep_dim <= 0), true, 
                                      learning_rate*train_sample_weight, 
                                      weight_decay+output_layer_weight_decay,
                                      bias_decay+output_layer_bias_decay, 
                                      generated_samples.subVec(i,1));
        }


        if(nhidden>0 && direct_in_to_out)
        {
            gradient_affine_transform(nnet_input, direct_wout, direct_bout,
                                      gradient_nnet_input, 
                                      gradient_direct_wout, gradient_direct_bout,
                                      gradient_last_layer,
                                      dist_rep_dim<=0, true,
                                      learning_rate*train_sample_weight, 
                                      weight_decay+direct_in_to_out_weight_decay,
                                      0,
                                      generated_samples.subVec(i,1));
        }

    }

    // Propagate all contributions through rest of the network

    if(nhidden2 > 0)
    {
        gradient_transfer_func(hidden2v,gradient_act_hidden2v,gradient_hidden2v);
        gradient_affine_transform(hiddenv, w2, b2, gradient_hiddenv, 
                                  gradient_w2, gradient_b2, gradient_act_hidden2v,
                                  false, false,learning_rate*train_sample_weight, 
                                  weight_decay+layer2_weight_decay,
                                  bias_decay+layer2_bias_decay);
    }
    if(nhidden > 0)
    {
        gradient_transfer_func(hiddenv,gradient_act_hiddenv,gradient_hiddenv);  
        gradient_affine_transform(nnet_input, w1, b1, gradient_nnet_input, 
                                  gradient_w1, gradient_b1, gradient_act_hiddenv,
                                  dist_rep_dim<=0, false,learning_rate*train_sample_weight, 
                                  weight_decay+layer1_weight_decay,
                                  bias_decay+layer1_bias_decay);
    }

    if(dist_rep_dim > 0)
    {
        nfeats = 0;
        id = 0;
        for(int i=0; i<inputsize_; )
        {
            ifeats = 0;
            for(int j=0; j<n_feat_sets; j++,i++)
                ifeats += feats[i].length();
            gradient_affine_transform(feat_input.subVec(nfeats,ifeats),
                                      wout_dist_rep, bout_dist_rep,
                                      gradient_feat_input,// Useless anyways...
                                      gradient_wout_dist_rep,
                                      gradient_bout_dist_rep,
                                      gradient_nnet_input.subVec(
                                          id*dist_rep_dim,dist_rep_dim),
                                      true, false, 
                                      learning_rate*train_sample_weight, 
                                      weight_decay+
                                      output_layer_dist_rep_weight_decay,
                                      bias_decay
                                      +output_layer_dist_rep_bias_decay);
            nfeats += ifeats;
            id++;
        }
    }
    clearProppathGradient();

    // Update parameters and clear gradient
    if(!stochastic_gradient_descent_speedup)
        update();
}

void NeuralProbabilisticLanguageModel::getNegativeEnergyValues(
    Vec samples, Vec neg_energies)
{
    if(dist_rep_dim > 0) // x -> d(x)
    {        
        // d(x),h1(d(x)),h2(h1(d(x))) -> o(x)

        add_affine_transform(last_layer,wout,bout,neg_energies,false,
                             true,samples);            
        if(direct_in_to_out && nhidden>0)
            add_affine_transform(nnet_input,direct_wout,direct_bout,
                                 neg_energies,false,true,
                                 samples);
    }
    else
    {
        // x, h1(x),h2(h1(x)) -> o(x)
        add_affine_transform(last_layer,wout,bout,samples,nhidden<=0,
                             true,samples);            
        if(direct_in_to_out && nhidden>0)
            add_affine_transform(feat_input,direct_wout,direct_bout,
                                 neg_energies,true,true,
                                 samples);
    }
}

void NeuralProbabilisticLanguageModel::compute_softmax(const Vec& x, 
                                                       const Vec& y) const
{
    int n = x.length();
    
//    real* yp = y.data();
//    real* xp = x.data();
//    for(int i=0; i<n; i++)
//    {
//        *yp++ = *xp > 1e-5 ? *xp : 1e-5;
//        xp++;
//    }

    if (n>0)
    {
        real* yp = y.data();
        real* xp = x.data();
        real maxx = max(x);
        real s = 0;
        for (int i=0;i<n;i++)
            s += (*yp++ = safeexp(*xp++-maxx));
        if (s == 0) PLERROR("trying to divide by 0 in softmax");
        s = 1.0 / s;
        yp = y.data();
        for (int i=0;i<n;i++)
            *yp++ *= s;
    }
}

real NeuralProbabilisticLanguageModel::nll(const Vec& outputv, int target) const
{
    return -safeflog(outputv[target]);
}
    
real NeuralProbabilisticLanguageModel::classification_loss(const Vec& outputv, 
                                                           int target) const
{
    return (argmax(outputv) == target ? 0 : 1);
}

void NeuralProbabilisticLanguageModel::initializeParams(bool set_seed)
{
    if (set_seed) {
        if (seed_>=0)
            rgen->manual_seed(seed_);
    }


    PP<Dictionary> dict = train_set->getDictionary(inputsize_);
    total_output_size = dict->size();

    total_feats_per_token = 0;
    for(int i=0; i<n_feat_sets; i++)
        total_feats_per_token += feat_sets[i]->size();

    int nnet_inputsize;
    if(dist_rep_dim > 0)
    {
        wout_dist_rep.resize(total_feats_per_token,dist_rep_dim);
        bout_dist_rep.resize(dist_rep_dim);
        nnet_inputsize = dist_rep_dim*inputsize_/n_feat_sets;
        nnet_input.resize(nnet_inputsize);

        fillWeights(wout_dist_rep);
        bout_dist_rep.clear();

        gradient_wout_dist_rep.resize(total_feats_per_token,dist_rep_dim);
        gradient_bout_dist_rep.resize(dist_rep_dim);
        gradient_nnet_input.resize(nnet_inputsize);
        gradient_wout_dist_rep.clear();
        gradient_bout_dist_rep.clear();
        gradient_nnet_input.clear();
    }
    else
    {
        nnet_inputsize = total_feats_per_token*inputsize_/n_feat_sets;
        nnet_input = feat_input;
    }

    if(nhidden>0) 
    {
        w1.resize(nnet_inputsize,nhidden);
        b1.resize(nhidden);
        hiddenv.resize(nhidden);

        fillWeights(w1);
        b1.clear();

        gradient_w1.resize(nnet_inputsize,nhidden);
        gradient_b1.resize(nhidden);
        gradient_hiddenv.resize(nhidden);
        gradient_act_hiddenv.resize(nhidden);
        gradient_w1.clear();
        gradient_b1.clear();
        gradient_hiddenv.clear();
        gradient_act_hiddenv.clear();
        if(nhidden2>0) 
        {
            w2.resize(nhidden,nhidden2);
            b2.resize(nhidden2);
            hidden2v.resize(nhidden2);
            wout.resize(nhidden2,total_output_size);
            bout.resize(total_output_size);

            fillWeights(w2);
            b2.clear();

            gradient_w2.resize(nhidden,nhidden2);
            gradient_b2.resize(nhidden2);
            gradient_hidden2v.resize(nhidden2);
            gradient_act_hidden2v.resize(nhidden2);
            gradient_wout.resize(nhidden2,total_output_size);
            gradient_bout.resize(total_output_size);
            gradient_w2.clear();
            gradient_b2.clear();
            gradient_hidden2v.clear();
            gradient_act_hidden2v.clear();
            gradient_wout.clear();
            gradient_bout.clear();
        }
        else
        {
            wout.resize(nhidden,total_output_size);
            bout.resize(total_output_size);

            gradient_wout.resize(nhidden,total_output_size);
            gradient_bout.resize(total_output_size);
            gradient_wout.clear();
            gradient_bout.clear();
        }
            
        if(direct_in_to_out)
        {
            direct_wout.resize(nnet_inputsize,total_output_size);
            direct_bout.resize(0); // Because it is not used

            fillWeights(direct_wout);
                
            gradient_direct_wout.resize(nnet_inputsize,total_output_size);
            gradient_direct_wout.clear();
            gradient_direct_bout.resize(0); // idem
        }
    }
    else
    {
        wout.resize(nnet_inputsize,total_output_size);
        bout.resize(total_output_size);

        gradient_wout.resize(nnet_inputsize,total_output_size);
        gradient_bout.resize(total_output_size);
        gradient_wout.clear();
        gradient_bout.clear();
    }

    //fillWeights(wout);
    
    if (fixed_output_weights) {
        static Vec values;
        if (values.size()==0)
        {
            values.resize(2);
            values[0]=-1;
            values[1]=1;
        }
        rgen->fill_random_discrete(wout.toVec(), values);
    }
    else 
        fillWeights(wout);

    bout.clear();

    gradient_outputv.resize(total_output_size);
    gradient_act_outputv.resize(total_output_size);
    gradient_outputv.clear();
    gradient_act_outputv.clear();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void NeuralProbabilisticLanguageModel::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // Private variables
    deepCopyField(target_values,copies);
    deepCopyField(output_comp,copies);
    deepCopyField(row,copies);
    deepCopyField(last_layer,copies);
    deepCopyField(gradient_last_layer,copies);
    deepCopyField(feats,copies);
    deepCopyField(gradient,copies);
    deepCopyField(neg_energies,copies);
    deepCopyField(densities,copies);

    // Protected variables
    deepCopyField(feat_input,copies);
    deepCopyField(gradient_feat_input,copies);
    deepCopyField(nnet_input,copies);
    deepCopyField(gradient_nnet_input,copies);
    deepCopyField(hiddenv,copies);
    deepCopyField(gradient_hiddenv,copies);
    deepCopyField(gradient_act_hiddenv,copies);
    deepCopyField(hidden2v,copies);
    deepCopyField(gradient_hidden2v,copies);
    deepCopyField(gradient_act_hidden2v,copies);
    deepCopyField(gradient_outputv,copies);
    deepCopyField(gradient_act_outputv,copies);
    deepCopyField(rgen,copies);
    deepCopyField(feats_since_last_update,copies);
    deepCopyField(target_values_since_last_update,copies);
    deepCopyField(val_string_reference_set,copies);
    deepCopyField(target_values_reference_set,copies);
    deepCopyField(importance_sampling_ratios,copies);
    deepCopyField(sample,copies);
    deepCopyField(generated_samples,copies);

    // Public variables
    deepCopyField(w1,copies);
    deepCopyField(gradient_w1,copies);
    deepCopyField(b1,copies);
    deepCopyField(gradient_b1,copies);
    deepCopyField(w2,copies);
    deepCopyField(gradient_w2,copies);
    deepCopyField(b2,copies);
    deepCopyField(gradient_b2,copies);
    deepCopyField(wout,copies);
    deepCopyField(gradient_wout,copies);
    deepCopyField(bout,copies);
    deepCopyField(gradient_bout,copies);
    deepCopyField(direct_wout,copies);
    deepCopyField(gradient_direct_wout,copies);
    deepCopyField(direct_bout,copies);
    deepCopyField(gradient_direct_bout,copies);
    deepCopyField(wout_dist_rep,copies);
    deepCopyField(gradient_wout_dist_rep,copies);
    deepCopyField(bout_dist_rep,copies);
    deepCopyField(gradient_bout_dist_rep,copies);

    // Public build options
    deepCopyField(cost_funcs,copies);
    deepCopyField(feat_sets,copies);
    deepCopyField(proposal_distribution,copies);

    PLERROR("not up to date");
}

////////////////
// outputsize //
////////////////
int NeuralProbabilisticLanguageModel::outputsize() const {
    return targetsize_;
}

///////////
// train //
///////////
void NeuralProbabilisticLanguageModel::train()
{
    //Profiler::activate();
    if(!train_set)
        PLERROR("In NeuralProbabilisticLanguageModel::train, "
                "you did not setTrainingSet");

    if(!train_stats)
        PLERROR("In NeuralProbabilisticLanguageModel::train, "
                "you did not setTrainStatsCollector");
 
    Vec outputv(total_output_size);
    Vec costsv(getTrainCostNames().length());
    Vec inputv(train_set->inputsize());
    Vec targetv(train_set->targetsize());
    real sample_weight = 1;

    int l = train_set->length();  
    int bs = batch_size>0 ? batch_size : l;

    // Importance sampling speedup variables
    
    // Effective sample size statistics
    real effective_sample_size_sum = 0;
    real effective_sample_size_square_sum = 0;
    real importance_sampling_ratio_k = 0;
    // Current true sample size;
    int n_samples = 0;

    real 

    PP<ProgressBar> pb;
    if(report_progress)
        pb = new ProgressBar("Training " + classname() + " from stage " 
                             + tostring(stage) + " to " 
                             + tostring(nstages), nstages-stage);

    //if(stage == 0)
    //{
    //    for(int t=0; t<l;t++)
    //    {
    //        cout << "t=" << t << " ";
    //        train_set->getExample(t,inputv,targetv,sample_weight);
    //        row.subVec(0,inputsize_) << inputv;
    //        train_set->getValues(row,inputsize_,target_values);
    //        if(target_values.length() != 1)
    //            verify_gradient(inputv,targetv,1e-6);
    //    }
    //    return;
    //}

    Mat old_gradient_wout;
    Vec old_gradient_bout;
    Mat old_gradient_wout_dist_rep;
    Vec old_gradient_bout_dist_rep;
    Mat old_gradient_w1;
    Vec old_gradient_b1;
    Mat old_gradient_w2;
    Vec old_gradient_b2;
    Mat old_gradient_direct_wout;

    if(stochastic_gradient_descent_speedup)
    {
        // Trick to make stochastic gradient descent faster

        old_gradient_wout = gradient_wout;
        old_gradient_bout = gradient_bout;
        gradient_wout = wout;
        gradient_bout = bout;
        
        if(dist_rep_dim > 0)
        {
            old_gradient_wout_dist_rep = gradient_wout_dist_rep;
            old_gradient_bout_dist_rep = gradient_bout_dist_rep;
            gradient_wout_dist_rep = wout_dist_rep;
            gradient_bout_dist_rep = bout_dist_rep;
        }

        if(nhidden>0) 
        {
            old_gradient_w1 = gradient_w1;
            old_gradient_b1 = gradient_b1;
            gradient_w1 = w1;
            gradient_b1 = b1;
            if(nhidden2>0) 
            {
                old_gradient_w2 = gradient_w2;
                old_gradient_b2 = gradient_b2;
                gradient_w2 = w2;
                gradient_b2 = b2;
            }
            
            if(direct_in_to_out)
            {
                old_gradient_direct_wout = gradient_direct_wout;
                gradient_direct_wout = direct_wout;
            }
        }
    }

    int initial_stage = stage;
    while(stage<nstages)
    {
        for(int t=0; t<l;)
        {
            //if(t%1000 == 0)
            //{
            //    cout << "Time: " << clock()/CLOCKS_PER_SEC << " seconds." << endl;
            //}
            for(int i=0; i<bs; i++)
            {
                //if(t == 71705)
                //    cout << "It's going to fuck !!!" << endl;
                
                //if(t == 71704)
                //    cout << "It's going to fuck !!!" << endl;
                
                train_set->getExample(t%l,inputv,targetv,sample_weight);

                if(proposal_distributions)
                {
                    n_samples = 0;
                    importance_sampling_ratios.resize(0);
                    effective_sample_size_sum = 0;
                    effective_sample_size_square_sum = 0;                    
                    while(effective_sample_size < minimum_effective_sample_size)
                    {
                        if(n_samples >= total_output_size)
                        {
                            gradient_last_layer.resize(total_output_size);
                            
                            fprop(inputv,outputv,targetv,costsv,sample_weight);
                            bprop(inputv,outputv,targetv,costsv,
                                  start_learning_rate/
                                  (bs*(1.0+decrease_constant*total_updates)),
                                  sample_weight);
                            train_stats->update(costsv);
                            break;
                        }
                        
                        importance_sampling_gradient_update(
                            inputv,targetv,
                            start_learning_rate/
                            (bs*(1.0+decrease_constant*total_updates)),
                            sampling_block_size,
                            sampleweight
                            );

                        // Update effective sample size
                        pval1 = importance_sampling_ratios.subVec(
                            nsamples,sampling_block_size).data();
                        for(int k=0; k<sampling_block_size; k++)
                        {                            
                            effective_sample_size_sum += *pval1;
                            effective_sample_size_square_sum += *pval1 * (*pval1);
                            pval1++;
                        }
                        
                        effective_sample_size = 
                            (effective_sample_size_sum*effective_sample_size_sum)/
                            effective_sample_size_square_sum;
                        n_samples += sampling_block_size;
                    }
                }
                else
                {
                    //Profiler::start("fprop()");
                    fprop(inputv,outputv,targetv,costsv,sample_weight);
                    //Profiler::end("fprop()");
                    //Profiler::start("bprop()");
                    bprop(inputv,outputv,targetv,costsv,
                          start_learning_rate/
                          (bs*(1.0+decrease_constant*total_updates)),
                          sample_weight);
                    //Profiler::end("bprop()");
                    train_stats->update(costsv);
                }
                t++;
            }
            // Update
            if(!stochastic_gradient_descent_speedup)
                update();
            total_updates++;
        }
        train_stats->finalize();
        ++stage;
        if(verbosity>2)
            cout << "Epoch " << stage << " train objective: " 
                 << train_stats->getMean() << endl;
        if(pb) pb->update(stage-initial_stage);
    }

    if(stochastic_gradient_descent_speedup)
    {
        // Trick to make stochastic gradient descent faster

        gradient_wout = old_gradient_wout;
        gradient_bout = old_gradient_bout;
        
        if(dist_rep_dim > 0)
        {
            gradient_wout_dist_rep = old_gradient_wout_dist_rep;
            gradient_bout_dist_rep = old_gradient_bout_dist_rep;
        }

        if(nhidden>0) 
        {
            gradient_w1 = old_gradient_w1;
            gradient_b1 = old_gradient_b1;
            if(nhidden2>0) 
            {
                gradient_w2 = old_gradient_w2;
                gradient_b2 = old_gradient_b2;
            }
            
            if(direct_in_to_out)
            {
                gradient_direct_wout = old_gradient_direct_wout;
            }
        }
    }
    //Profiler::report(cout);
}

void NeuralProbabilisticLanguageModel::verify_gradient(
    Vec& input, Vec targetv, real step)
{
    Vec costsv(getTrainCostNames().length());
    real sampleweight = 1;
    real verify_step = step;
    
    // To avoid the interaction between fprop and this function
    int nfeats = 0;
    int id = 0;
    int ifeats = 0;

    Vec est_gradient_bout;
    Mat est_gradient_wout;
    Vec est_gradient_bout_dist_rep;
    Mat est_gradient_wout_dist_rep;
    Vec est_gradient_b1;
    Mat est_gradient_w1;
    Vec est_gradient_b2;
    Mat est_gradient_w2;
    Vec est_gradient_direct_bout;
    Mat est_gradient_direct_wout;

    int nnet_inputsize;
    if(dist_rep_dim > 0)
    {
        nnet_inputsize = dist_rep_dim*inputsize_/n_feat_sets;
        est_gradient_wout_dist_rep.resize(total_feats_per_token,dist_rep_dim);
        est_gradient_bout_dist_rep.resize(dist_rep_dim);
        est_gradient_wout_dist_rep.clear();
        est_gradient_bout_dist_rep.clear();
        gradient_wout_dist_rep.clear();
        gradient_bout_dist_rep.clear();
    }
    else
    {
        nnet_inputsize = total_feats_per_token*inputsize_/n_feat_sets;
    }
    
    if(nhidden>0) 
    {
        est_gradient_w1.resize(nnet_inputsize,nhidden);
        est_gradient_b1.resize(nhidden);
        est_gradient_w1.clear();
        est_gradient_b1.clear();
        gradient_w1.clear();
        gradient_b1.clear();
        if(nhidden2>0) 
        {
            est_gradient_w2.resize(nhidden,nhidden2);
            est_gradient_b2.resize(nhidden2);
            est_gradient_wout.resize(nhidden2,total_output_size);
            est_gradient_bout.resize(total_output_size);
            est_gradient_w2.clear();
            est_gradient_b2.clear();
            est_gradient_wout.clear();
            est_gradient_bout.clear();
            gradient_w2.clear();
            gradient_b2.clear();
            gradient_wout.clear();
            gradient_bout.clear();
        }
        else
        {
            est_gradient_wout.resize(nhidden,total_output_size);
            est_gradient_bout.resize(total_output_size);
            est_gradient_wout.clear();
            est_gradient_bout.clear();
            gradient_wout.clear();
            gradient_bout.clear();
        }
            
        if(direct_in_to_out)
        {
            est_gradient_direct_wout.resize(nnet_inputsize,total_output_size);
            est_gradient_direct_wout.clear();
            est_gradient_direct_bout.resize(0); // idem
            gradient_direct_wout.clear();                        
        }
    }
    else
    {
        est_gradient_wout.resize(nnet_inputsize,total_output_size);
        est_gradient_bout.resize(total_output_size);
        est_gradient_wout.clear();
        est_gradient_bout.clear();
        gradient_wout.clear();
        gradient_bout.clear();
    }

    fprop(input, output_comp, targetv, costsv);
    bprop(input,output_comp,targetv,costsv,
          -1, sampleweight);
    clearProppathGradient();
    
    // Compute estimated gradient

    if(dist_rep_dim > 0) 
    {        
        nfeats = 0;
        id = 0;
        for(int i=0; i<inputsize_;)
        {
            ifeats = 0;
            for(int j=0; j<n_feat_sets; j++,i++)
                ifeats += feats[i].length();
            verify_gradient_affine_transform(
                input,output_comp, targetv, costsv, sampleweight,
                feat_input.subVec(nfeats,ifeats),
                wout_dist_rep, bout_dist_rep,
                est_gradient_wout_dist_rep, est_gradient_bout_dist_rep,
                true, false, verify_step);
            nfeats += ifeats;
            id++;
        }

        cout << "Verify wout_dist_rep" << endl;
        output_gradient_verification(gradient_wout_dist_rep.toVec(), 
                                     est_gradient_wout_dist_rep.toVec());
        cout << "Verify bout_dist_rep" << endl;
        output_gradient_verification(gradient_bout_dist_rep, 
                                     est_gradient_bout_dist_rep);
        gradient_wout_dist_rep.clear();
        gradient_bout_dist_rep.clear();

        if(nhidden>0) 
        {
            verify_gradient_affine_transform(
                input,output_comp, targetv, costsv, sampleweight,
                nnet_input,w1,b1,
                est_gradient_w1, est_gradient_b1, false,false, verify_step);

            cout << "Verify w1" << endl;
            output_gradient_verification(gradient_w1.toVec(), 
                                         est_gradient_w1.toVec());
            cout << "Verify b1" << endl;
            output_gradient_verification(gradient_b1, est_gradient_b1);
            
            if(nhidden2>0) 
            {
                verify_gradient_affine_transform(
                    input,output_comp, targetv, costsv, sampleweight,    
                    hiddenv,w2,b2,
                    est_gradient_w2, est_gradient_b2,
                    false,false, verify_step);
                cout << "Verify w2" << endl;
                output_gradient_verification(gradient_w2.toVec(), 
                                             est_gradient_w2.toVec());
                cout << "Verify b2" << endl;
                output_gradient_verification(gradient_b2, est_gradient_b2);

                last_layer = hidden2v;
            }
            else
                last_layer = hiddenv;
        }
        else
            last_layer = nnet_input;

        verify_gradient_affine_transform(
            input,output_comp, targetv, costsv, sampleweight,
            last_layer,wout,bout,
            est_gradient_wout, est_gradient_bout, false,
            possible_targets_vary,verify_step,target_values);

        cout << "Verify wout" << endl;
        output_gradient_verification(gradient_wout.toVec(), 
                                     est_gradient_wout.toVec());
        cout << "Verify bout" << endl;
        output_gradient_verification(gradient_bout, est_gradient_bout);
 
        if(direct_in_to_out && nhidden>0)
        {
            verify_gradient_affine_transform(
                input,output_comp, targetv, costsv, sampleweight,
                nnet_input,direct_wout,direct_bout,
                est_gradient_direct_wout, est_gradient_direct_bout,false,
                possible_targets_vary, verify_step, target_values);
            cout << "Verify direct_wout" << endl;
            output_gradient_verification(gradient_direct_wout.toVec(), 
                                         est_gradient_direct_wout.toVec());
            //cout << "Verify direct_bout" << endl;
            //output_gradient_verification(gradient_direct_bout, est_gradient_direct_bout);
        }
    }
    else
    {        
        if(nhidden>0)
        {
            verify_gradient_affine_transform(
                input,output_comp, targetv, costsv, sampleweight,
                feat_input,w1,b1,
                est_gradient_w1, est_gradient_b1,
                true,false, verify_step);

            cout << "Verify w1" << endl;
            output_gradient_verification(gradient_w1.toVec(), 
                                         est_gradient_w1.toVec());
            cout << "Verify b1" << endl;
            output_gradient_verification(gradient_b1, est_gradient_b1);

            if(nhidden2>0)
            {
                verify_gradient_affine_transform(
                    input,output_comp, targetv, costsv, sampleweight,
                    hiddenv,w2,b2,
                    est_gradient_w2, est_gradient_b2,true,false,
                    verify_step);

                cout << "Verify w2" << endl;
                output_gradient_verification(gradient_w2.toVec(), 
                                             est_gradient_w2.toVec());
                cout << "Verify b2" << endl;
                output_gradient_verification(gradient_b2, est_gradient_b2);
                
                last_layer = hidden2v;
            }
            else
                last_layer = hiddenv;
        }
        else
            last_layer = feat_input;
        
        verify_gradient_affine_transform(
            input,output_comp, targetv, costsv, sampleweight,
            last_layer,wout,bout,
            est_gradient_wout, est_gradient_bout, nhidden<=0,
            possible_targets_vary,verify_step, target_values);

        cout << "Verify wout" << endl;
        output_gradient_verification(gradient_wout.toVec(), 
                                     est_gradient_wout.toVec());
        cout << "Verify bout" << endl;
        output_gradient_verification(gradient_bout, est_gradient_bout);
        
        if(direct_in_to_out && nhidden>0)
        {
            verify_gradient_affine_transform(
                input,output_comp, targetv, costsv, sampleweight,
                feat_input,direct_wout,direct_bout,
                est_gradient_wout, est_gradient_bout,true,
                possible_targets_vary, verify_step,target_values);
            cout << "Verify direct_wout" << endl;
            output_gradient_verification(gradient_direct_wout.toVec(), 
                                         est_gradient_direct_wout.toVec());
            cout << "Verify direct_bout" << endl;
            output_gradient_verification(gradient_direct_bout, 
                                         est_gradient_direct_bout);
        }
    }

}

void NeuralProbabilisticLanguageModel::verify_gradient_affine_transform(
    Vec global_input, Vec& global_output, Vec& global_targetv,
    Vec& global_costs, real sampleweight,
    Vec input, Mat weights, Vec bias,
    Mat est_gweights, Vec est_gbias,  
    bool input_is_sparse, bool output_is_sparse,
    real step,
    Vec output_indices) const
{
    real *pval1, *pval2, *pval3;
    int ni,nj;
    real out1,out2;
    // Bias
    if(bias.length() != 0)
    {
        if(output_is_sparse)
        {
            pval1 = est_gbias.data();
            pval2 = bias.data();
            pval3 = output_indices.data();
            ni = output_indices.length();
            for(int i=0; i<ni; i++)
            {
                pval2[(int)*pval3] += step;
                fprop(global_input, global_output, global_targetv, 
                      global_costs, sampleweight);
                out1 = global_costs[0];
                pval2[(int)*pval3] -= 2*step;
                fprop(global_input, global_output, global_targetv, 
                      global_costs, sampleweight);
                out2 = global_costs[0];
                pval1[(int)*pval3] = (out1-out2)/(2*step);
                pval2[(int)*pval3] += step;
                pval3++;
            }
        }
        else
        {
            pval1 = est_gbias.data();
            pval2 = bias.data();
            ni = bias.length();
            for(int i=0; i<ni; i++)
            {
                *pval2 += step;
                fprop(global_input, global_output, global_targetv, 
                      global_costs, sampleweight);
                out1 = global_costs[0];
                *pval2 -= 2*step;
                fprop(global_input, global_output, global_targetv, 
                      global_costs, sampleweight);
                out2 = global_costs[0];
                *pval1 = (out1-out2)/(2*step);
                *pval2 += step;
                pval1++; 
                pval2++;
            }
        }
    }

    // Weights
    if(!input_is_sparse && !output_is_sparse)
    {
        ni = weights.length();
        nj = weights.width();
        for(int i=0; i<ni; i++)
            for(int j=0; j<nj; j++)
            {
                weights(i,j) += step;
                fprop(global_input, global_output, global_targetv, 
                      global_costs, sampleweight);
                out1 = global_costs[0];
                weights(i,j) -= 2*step;
                fprop(global_input, global_output, global_targetv, 
                      global_costs, sampleweight);
                out2 = global_costs[0];
                weights(i,j) += step;
                est_gweights(i,j) = (out1-out2)/(2*step);
            }
    }
    else if(!input_is_sparse && output_is_sparse)
    {
        ni = output_indices.length();
        nj = input.length();
        pval3 = output_indices.data();
        for(int i=0; i<ni; i++)
        {
            for(int j=0; j<nj; j++)
            {
                weights(j,(int)*pval3) += step;
                fprop(global_input, global_output, global_targetv, 
                      global_costs, sampleweight);
                out1 = global_costs[0];
                weights(j,(int)*pval3) -= 2*step;
                fprop(global_input, global_output, global_targetv, 
                      global_costs, sampleweight);
                out2 = global_costs[0];
                weights(j,(int)*pval3) += step;
                est_gweights(j,(int)*pval3) = (out1-out2)/(2*step);
//                if(target_values.length() != 1 && input[j] != 0 && (out1-out2)/(2*step) == 0)
//                {                    
//                    print_what_the_fuck();
//                    weights(j,(int)*pval3) += 1;
//                    fprop(global_input, global_output, global_targetv, global_costs, sampleweight);
//                    weights(j,(int)*pval3) -= 1;
//                    cout << "out1 - global_costs[0] =" << out1-global_costs[0] << endl;
//                }
            }
            pval3++;
        }
    }
    else if(input_is_sparse && !output_is_sparse)
    {
        ni = input.length();
        nj = weights.width();
        if(ni != 0 )
        {
            pval3 = input.data();
            for(int i=0; i<ni; i++)
            {
                pval1 = est_gweights[(int)(*pval3)];
                pval2 = weights[(int)(*pval3++)];
                for(int j=0; j<nj;j++)
                {
                    *pval2 += step;
                    fprop(global_input, global_output, global_targetv, 
                          global_costs, sampleweight);
                    out1 = global_costs[0];
                    *pval2 -= 2*step;
                    fprop(global_input, global_output, global_targetv, 
                          global_costs, sampleweight);
                    out2 = global_costs[0];
                    *pval1 = (out1-out2)/(2*step);
                    *pval2 += step;
                    pval1++;
                    pval2++;
                }
            }
        }
    }
    else if(input_is_sparse && output_is_sparse)
    {
        // Weights
        ni = input.length();
        nj = output_indices.length();
        if(ni != 0)
        {
            pval2 = input.data();
            for(int i=0; i<ni; i++)
            {
                pval3 = output_indices.data();
                for(int j=0; j<nj; j++)
                {
                    weights((int)(*pval2),(int)*pval3) += step;
                    fprop(global_input, global_output, global_targetv, 
                          global_costs, sampleweight);
                    out1 = global_costs[0];
                    weights((int)(*pval2),(int)*pval3) -= 2*step;
                    fprop(global_input, global_output, global_targetv, 
                          global_costs, sampleweight);
                    out2 = global_costs[0];
                    est_gweights((int)(*pval2),(int)*pval3)  = 
                        (out1-out2)/(2*step);
                    weights((int)(*pval2),(int)*pval3) += step;
                    pval3++;
                }
                pval2++;
            }
        }
    }
}


void NeuralProbabilisticLanguageModel::output_gradient_verification(
    Vec grad, Vec est_grad)
{
    // Inspired from Func::verifyGradient()

    Vec num = apply(grad - est_grad,(tRealFunc)FABS);
    Vec denom = real(0.5)*apply(grad + est_grad,(tRealFunc)FABS);
    for (int i = 0; i < num.length(); i++)
    {
        if (!fast_exact_is_equal(num[i], 0))
            num[i] /= denom[i];
        else
            if(!fast_exact_is_equal(denom[i],0))
                cout << "at position " << i << " num[i] == 0 but denom[i] = " 
                     << denom[i] << endl;
    }
    int pos = argmax(num);
    cout << max(num) << " (at position " << pos << "/" << num.length()
         << ", computed = " << grad[pos] << " and estimated = "
         << est_grad[pos] << ")" << endl;

    real norm_grad = norm(grad);
    real norm_est_grad = norm(est_grad);
    real cos_angle = fast_exact_is_equal(norm_grad*norm_est_grad,
                                         0)
        ? MISSING_VALUE
        : dot(grad,est_grad) /
        (norm_grad*norm_est_grad);
    if (cos_angle > 1)
        cos_angle = 1;      // Numerical imprecisions can lead to such situation.
    cout << "grad.length() = " << grad.length() << endl;
    cout << "cos(angle) : " << cos_angle << endl;
    cout << "angle : " << ( is_missing(cos_angle) ? MISSING_VALUE
                            : acos(cos_angle) ) << endl;
}

void NeuralProbabilisticLanguageModel::batchComputeOutputAndConfidence(
    VMat inputs, real probability,
    VMat outputs_and_confidence) const
{
    val_string_reference_set = inputs;
    inherited::batchComputeOutputAndConfidence(inputs,
                                               probability,
                                               outputs_and_confidence);
    val_string_reference_set = train_set;
}

void NeuralProbabilisticLanguageModel::use(VMat testset, VMat outputs) const
{
    val_string_reference_set = testset;
    if(testset->width() > train_set->inputsize())
        target_values_reference_set = testset;
    target_values_reference_set = testset;
    inherited::use(testset,outputs);
    val_string_reference_set = train_set;
    if(testset->width() > train_set->inputsize())
        target_values_reference_set = train_set;
}

void NeuralProbabilisticLanguageModel::test(VMat testset, 
                                            PP<VecStatsCollector> test_stats, 
                      VMat testoutputs, VMat testcosts) const
{
    val_string_reference_set = testset;
    target_values_reference_set = testset;
    inherited::test(testset,test_stats,testoutputs,testcosts);
    val_string_reference_set = train_set;
    target_values_reference_set = train_set;
}

VMat NeuralProbabilisticLanguageModel::processDataSet(VMat dataset) const
{
    VMat ret;
    val_string_reference_set = dataset;
    // Assumes it contains the target part information
    if(dataset->width() > train_set->inputsize())
        target_values_reference_set = dataset;
    ret = inherited::processDataSet(dataset);
    val_string_reference_set = train_set;
    if(dataset->width() > train_set->inputsize())
        target_values_reference_set = train_set;
    return ret;
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
