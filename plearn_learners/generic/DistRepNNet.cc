// -*- C++ -*-

// DistRepNNet.cc
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


/* *******************************************************      
 * $Id: DistRepNNet.cc 3994 2005-08-25 13:35:03Z chapados $
 ******************************************************* */

/*! \file PLearnLibrary/PLearnAlgo/DistRepNNet.h */

#include <plearn/var/SourceVariable.h>
#include <plearn/var/VarRowsVariable.h>
#include <plearn/var/PotentialsVariable.h>
#include <plearn/var/IsMissingVariable.h>
#include <plearn/var/ReIndexedTargetVariable.h>
#include <plearn/var/LogSoftmaxVariable.h>
#include <plearn/var/AffineTransformVariable.h>
#include <plearn/var/AffineTransformWeightPenalty.h>
#include <plearn/var/BinaryClassificationLossVariable.h>
#include <plearn/var/ClassificationLossVariable.h>
#include <plearn/var/ConcatColumnsVariable.h>
#include <plearn/var/CrossEntropyVariable.h>
#include <plearn/var/ExpVariable.h>
#include <plearn/var/MarginPerceptronCostVariable.h>
#include <plearn/var/MulticlassLossVariable.h>
#include <plearn/var/NegCrossEntropySigmoidVariable.h>
#include <plearn/var/InsertZerosVariable.h>
#include <plearn/var/OneHotSquaredLoss.h>
#include <plearn/var/SigmoidVariable.h>
#include <plearn/var/SoftmaxVariable.h>
#include <plearn/var/SoftplusVariable.h>
#include <plearn/var/SumVariable.h>
#include <plearn/var/SumAbsVariable.h>
#include <plearn/var/SumOfVariable.h>
#include <plearn/var/SumSquareVariable.h>
#include <plearn/var/TanhVariable.h>
#include <plearn/var/TransposeVariable.h>
#include <plearn/var/ProductVariable.h>
#include <plearn/var/TransposeProductVariable.h>
#include <plearn/var/UnaryHardSlopeVariable.h>
#include <plearn/var/ArgmaxVariable.h>
#include <plearn/var/Var_operators.h>
#include <plearn/var/Var_utils.h>
#include <plearn/var/FNetLayerVariable.h>
//#include <plearn/display/DisplayUtils.h>

#include <plearn/vmat/ConcatColumnsVMatrix.h>
#include "DistRepNNet.h"
#include <plearn/math/random.h>
#include <plearn/vmat/SubVMatrix.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(DistRepNNet, "Feedforward Neural Network that learns Distributed Representations for symbolic data", 
                        "Inspired from the NNet class, DistRepNNet is simply an extension that deals with\n"
                        "symbolic data by learning a Distributed Representation for each type of symbolic\n" 
                        "data. The possible targets are defined either by the VMatrix's target field\n"
                        "dictionary or by a Dictionary provided by the user. Extra VMatrices corresponding\n"
                        "to extra tasks can also be provided to make use of inductive transfer between\n"
                        "tasks. It this case, the VMatrices need to have a dictionary for their target field.\n");

DistRepNNet::DistRepNNet() // DEFAULT VALUES FOR ALL OPTIONS
    :
nhidden(0),
nhidden2(0),
nhidden_theta_predictor(0),
nhidden_dist_rep_predictor(0),
weight_decay(0),
bias_decay(0),
input_dist_rep_predictor_bias_decay(0),
output_dist_rep_predictor_bias_decay(0),
input_dist_rep_predictor_weight_decay(0),
output_dist_rep_predictor_weight_decay(0),
layer1_weight_decay(0),
layer1_bias_decay(0),
layer1_theta_predictor_weight_decay(0),
layer1_theta_predictor_bias_decay(0),
layer2_weight_decay(0),
layer2_bias_decay(0),
output_layer_weight_decay(0),
output_layer_bias_decay(0),
output_layer_theta_predictor_weight_decay(0),
output_layer_theta_predictor_bias_decay(0),
direct_in_to_out_weight_decay(0),
direct_in_to_out_bias_decay(0),
margin(1),
fixed_output_weights(0),
direct_in_to_out(0),
penalty_type("L2_square"),
output_transfer_func(""),
hidden_transfer_func("tanh"),
do_not_change_params(false),
batch_size(1),
initialization_method("uniform_linear"),
ntokens(-1),
nfeatures_per_token(-1),
//consider_unseen_classes(0),
use_dist_reps(1),
use_output_weights_bases(0)
{}

DistRepNNet::~DistRepNNet()
{
}

void DistRepNNet::declareOptions(OptionList& ol)
{
    declareOption(ol, "nhidden", &DistRepNNet::nhidden, OptionBase::buildoption, 
                  "Number of hidden units in first hidden layer (0 means no hidden layer).\n");

    declareOption(ol, "nhidden2", &DistRepNNet::nhidden2, OptionBase::buildoption, 
                  "Number of hidden units in second hidden layer (0 means no hidden layer).\n");

    declareOption(ol, "weight_decay", &DistRepNNet::weight_decay, OptionBase::buildoption, 
                  "Global weight decay for all layers.\n");

    declareOption(ol, "bias_decay", &DistRepNNet::bias_decay, OptionBase::buildoption, 
                  "Global bias decay for all layers.\n");

    declareOption(ol, "layer1_weight_decay", &DistRepNNet::layer1_weight_decay, OptionBase::buildoption, 
                  "Additional weight decay for the first hidden layer.  Is added to weight_decay.\n");

    declareOption(ol, "layer1_bias_decay", &DistRepNNet::layer1_bias_decay, OptionBase::buildoption, 
                  "Additional bias decay for the first hidden layer.  Is added to bias_decay.\n");

    declareOption(ol, "layer2_weight_decay", &DistRepNNet::layer2_weight_decay, OptionBase::buildoption, 
                  "Additional weight decay for the second hidden layer.  Is added to weight_decay.\n");

    declareOption(ol, "layer2_bias_decay", &DistRepNNet::layer2_bias_decay, OptionBase::buildoption, 
                  "Additional bias decay for the second hidden layer.  Is added to bias_decay.\n");

    declareOption(ol, "layer1_theta_predictor_weight_decay", &DistRepNNet::layer1_theta_predictor_weight_decay, OptionBase::buildoption, 
                  "Additional weight decay for the first hidden layer of the theta-predictor.  Is added to weight_decay.\n");

    declareOption(ol, "layer1_theta_predictor_bias_decay", &DistRepNNet::layer1_theta_predictor_bias_decay, OptionBase::buildoption, 
                  "Additional bias decay for the first hidden layer of the theta-predictor.  Is added to bias_decay.\n");

    declareOption(ol, "output_layer_weight_decay", &DistRepNNet::output_layer_weight_decay, OptionBase::buildoption, 
                  "Additional weight decay for the output layer.  Is added to 'weight_decay'.\n");

    declareOption(ol, "output_layer_bias_decay", &DistRepNNet::output_layer_bias_decay, OptionBase::buildoption, 
                  "Additional bias decay for the output layer.  Is added to 'bias_decay'.\n");

    declareOption(ol, "output_layer_theta_predictor_weight_decay", &DistRepNNet::output_layer_theta_predictor_weight_decay, OptionBase::buildoption, 
                  "Additional weight decay for the output layer of the theta-predictor.  Is added to 'weight_decay'.\n");

    declareOption(ol, "output_layer_theta_predictor_bias_decay", &DistRepNNet::output_layer_theta_predictor_bias_decay, OptionBase::buildoption, 
                  "Additional bias decay for the output layer of the theta-predictor.  Is added to 'bias_decay'.\n");

    declareOption(ol, "output_dist_rep_predictor_weight_decay", &DistRepNNet::output_dist_rep_predictor_weight_decay, OptionBase::buildoption, 
                  "Additional weight decay for the weights going from the hidden layer of the distributed representation predictor.  Is added to 'weight_decay'.\n");

    declareOption(ol, "output_dist_rep_predictor_bias_decay", &DistRepNNet::output_dist_rep_predictor_bias_decay, OptionBase::buildoption, 
                  "Additional bias decay for the weights going from the hidden layer of the distributed representation predictor.  Is added to 'bias_decay'.\n");

    declareOption(ol, "input_dist_rep_predictor_weight_decay", &DistRepNNet::input_dist_rep_predictor_weight_decay, OptionBase::buildoption, 
                  "Additional weight decay for the weights going from the input layer of the distributed representation predictor.  Is added to 'weight_decay'.\n");

    declareOption(ol, "input_dist_rep_predictor_bias_decay", &DistRepNNet::input_dist_rep_predictor_bias_decay, OptionBase::buildoption, 
                  "Additional bias decay for the weights going from the input layer of the distributed representation predictor.  Is added to 'bias_decay'.\n");

    declareOption(ol, "direct_in_to_out_weight_decay", &DistRepNNet::direct_in_to_out_weight_decay, OptionBase::buildoption,
                  "Additional weight decay for the weights going from the input directly to the output layer.  Is added to 'weight_decay'.\n");

    declareOption(ol, "direct_in_to_out_bias_decay", &DistRepNNet::direct_in_to_out_bias_decay, OptionBase::buildoption,
                  "Additional bias decay for the weights going from the input directly to the output layer.  Is added to 'bias_decay'.\n");


    declareOption(ol, "penalty_type", &DistRepNNet::penalty_type,
                  OptionBase::buildoption,
                  "Penalty to use on the weights (for weight and bias decay).\n"
                  "Can be any of:\n"
                  "  - \"L1\": L1 norm,\n"
                  "  - \"L1_square\": square of the L1 norm,\n"
                  "  - \"L2_square\" (default): square of the L2 norm.\n");

    declareOption(ol, "fixed_output_weights", &DistRepNNet::fixed_output_weights, OptionBase::buildoption, 
                  "If true then the output weights are not learned. They are initialized to +1 or -1 randomly.\n");

    declareOption(ol, "direct_in_to_out", &DistRepNNet::direct_in_to_out, OptionBase::buildoption, 
                  "If true then direct input to output weights will be added (if nhidden > 0).\n");

    declareOption(ol, "output_transfer_func", &DistRepNNet::output_transfer_func, OptionBase::buildoption, 
                  "what transfer function to use for ouput layer? One of: \n"
                  "  - \"tanh\" \n"
                  "  - \"sigmoid\" \n"
                  "  - \"exp\" \n"
                  "  - \"softplus\" \n"
                  "  - \"softmax\" \n"
                  "  - \"log_softmax\" \n"
                  "  - \"hard_slope\" \n"
                  "  - \"symm_hard_slope\" \n"
                  "An empty string or \"none\" means no output transfer function \n");

    declareOption(ol, "hidden_transfer_func", &DistRepNNet::hidden_transfer_func, OptionBase::buildoption, 
                  "What transfer function to use for hidden units? One of \n"
                  "  - \"linear\" \n"
                  "  - \"tanh\" \n"
                  "  - \"sigmoid\" \n"
                  "  - \"exp\" \n"
                  "  - \"softplus\" \n"
                  "  - \"softmax\" \n"
                  "  - \"log_softmax\" \n"
                  "  - \"hard_slope\" \n"
                  "  - \"symm_hard_slope\" \n");

    declareOption(ol, "cost_funcs", &DistRepNNet::cost_funcs, OptionBase::buildoption, 
                  "A list of cost functions to use\n"
                  "in the form \"[ cf1; cf2; cf3; ... ]\" where each function is one of: \n"
                  "  - \"mse_onehot\" (for classification)\n"
                  "  - \"NLL\" (negative log likelihood -log(p[c]) for classification) \n"
                  "  - \"class_error\" (classification error) \n"
                  "  - \"margin_perceptron_cost\" (a hard version of the cross_entropy, uses the 'margin' option)\n"
                  "The FIRST function of the list will be used as \n"
                  "the objective function to optimize \n"
                  "(possibly with an added weight decay penalty) \n");
  
    declareOption(ol, "margin", &DistRepNNet::margin, OptionBase::buildoption, 
                  "Margin requirement, used only with the margin_perceptron_cost cost function.\n"
                  "It should be positive, and larger values regularize more.\n");

    declareOption(ol, "do_not_change_params", &DistRepNNet::do_not_change_params, OptionBase::buildoption, 
                  "If set to 1, the weights won't be loaded nor initialized at build time.");

    declareOption(ol, "optimizer", &DistRepNNet::optimizer, OptionBase::buildoption, 
                  "Specify the optimizer to use\n");

    declareOption(ol, "batch_size", &DistRepNNet::batch_size, OptionBase::buildoption, 
                  "How many samples to use to estimate the avergage gradient before updating the weights\n"
                  "0 is equivalent to specifying training_set->length() \n");

    declareOption(ol, "dist_rep_dim", &DistRepNNet::dist_rep_dim, OptionBase::buildoption, 
                  " Dimensionality (number of components) of distributed representations.\n"
                  "The first element is the dimensionality of the input distributed representations"
                  "and the last one is the dimensionality of the target distributed representations."
//                  "Those values are taken one by one, as the Dictionary objects are extracted.\n"
//                  "When nnet_architecture == \"dist_rep_predictor\", the first element of dist_rep_dim\n"
//                  "indicates the dimensionality to the predicted distributed representation.\n"
        );


    declareOption(ol, "initialization_method", &DistRepNNet::initialization_method, OptionBase::buildoption, 
                  "The method used to initialize the weights:\n"
                  " - \"normal_linear\"  = a normal law with variance 1/n_inputs\n"
                  " - \"normal_sqrt\"    = a normal law with variance 1/sqrt(n_inputs)\n"
                  " - \"uniform_linear\" = a uniform law in [-1/n_inputs, 1/n_inputs]\n"
                  " - \"uniform_sqrt\"   = a uniform law in [-1/sqrt(n_inputs), 1/sqrt(n_inputs)]\n"
                  " - \"zero\"           = all weights are set to 0\n");

    declareOption(ol, "use_dist_reps", &DistRepNNet::use_dist_reps, OptionBase::buildoption, 
                  "Indication that distributed representations should be used");

    declareOption(ol, "use_output_weights_bases", &DistRepNNet::use_output_weights_bases, OptionBase::buildoption, 
                  "Indication that bases for output weights should be used");

    /*
    declareOption(ol, "nnet_architecture", &DistRepNNet::nnet_architecture, OptionBase::buildoption, 
                  "Architecture of the neural network:\n"
                  " - \"standard\"\n"
                  //" - \"csMTL\" (context-sensitive Multiple Task Learning, at NIPS 2005 Inductive Transfer Workshop)\n"
                  " - \"theta_predictor\" (standard NNet with output weights being PREDICTED) \n"
                  " - \"dist_rep_predictor\" (standard NNet with distributed representation being PREDICTED) \n"
                  " - \"linear\" (linear classifier that doesn't learn distributed representations) \n"
        );
    */

    declareOption(ol, "ntokens", &DistRepNNet::ntokens, OptionBase::buildoption, 
                  "Number of tokens, for which to predict a distributed representation.\n");    

    declareOption(ol, "nfeatures_per_token", &DistRepNNet::nfeatures_per_token, OptionBase::buildoption, 
                  "Number of features per token.\n");    

    declareOption(ol, "nhidden_dist_rep_predictor", &DistRepNNet::nhidden_dist_rep_predictor, OptionBase::buildoption, 
                  "Number of hidden units of the neural network predictor for the distributed representation.\n");

    declareOption(ol, "target_dictionary", &DistRepNNet::target_dictionary, OptionBase::buildoption, 
                  "User specified Dictionary for the target field. If null, then it is extracted from the training set VMatrix.\n");

    declareOption(ol, "target_dist_rep", &DistRepNNet::target_dist_rep, OptionBase::buildoption, 
                  "User specified distributed representation for the target field. If null, then it is learned from the training set VMatrix.\n");

    declareOption(ol, "paramsvalues", &DistRepNNet::paramsvalues, OptionBase::learntoption, 
                  "The learned parameter vector\n");

    declareOption(ol, "nhidden_theta_predictor", &DistRepNNet::nhidden_theta_predictor, OptionBase::buildoption, 
                  "Number of hidden units of the neural network predictor for the hidden to output weights.\n");

    declareOption(ol, "extra_tasks", &DistRepNNet::extra_tasks, OptionBase::buildoption, 
                  "Extra tasks' datasets to train on.\n");

    declareOption(ol, "nhidden_extra_tasks", &DistRepNNet::nhidden_extra_tasks, OptionBase::buildoption, 
                  "Number of hidden units in first hidden layer for extra tasks (0 means no hidden layer).\n");

    declareOption(ol, "nhidden2_extra_tasks", &DistRepNNet::nhidden2_extra_tasks, OptionBase::buildoption, 
                  "Number of hidden units in second hidden layer for extra tasks (0 means no hidden layer).\n");

    declareOption(ol, "optimizer_extra_tasks", &DistRepNNet::optimizer_extra_tasks, OptionBase::buildoption, 
                  "Specify the optimizer to use for extra tasks.\n");

    declareOption(ol, "ntokens_extra_tasks", &DistRepNNet::ntokens_extra_tasks, OptionBase::buildoption, 
                  "Number of tokens, for which to predict a distributed representation for extra tasks.\n");

//    declareOption(ol, "consider_unseen_classes", &DistRepNNet::consider_unseen_classes, OptionBase::buildoption, 
//                  "Indication that the test classes may be unseen in the training set.\n");

    declareOption(ol, "train_set", &DistRepNNet::train_set, OptionBase::learntoption, 
                  "VMatrix used for training, that also provides information about the data (e.g. Dictionary objects for the different fields).\n");


    inherited::declareOptions(ol);

}

///////////
// build //
///////////
void DistRepNNet::build()
{
    inherited::build();
    build_();
}

Var DistRepNNet::buildSparseAffineTransform(VarArray weights, Var input, TVec<int> input_to_dict_index, int begin, VarArray& activated_weights)
{    
    Var out = weights.last();
    for(int j=0; j<weights.length()-1; j++)
    {
        if(input_to_dict_index[j] < 0)
        {
            activated_weights.push_back(weights[j]);
            out = out + weights[j] * input[begin+j];
        }
        else
        {
            activated_weights.push_back(weights[j](isMissing(input[begin+j],true, true, dictionaries[input_to_dict_index[begin+j]]->getId(OOV_TAG))));
            out = out + activated_weights.last();
        }
    }
    return out;
}

void DistRepNNet::buildVarGraph(int task_index)
{
    VMat task_set;
    if(task_index < 0)
        task_set = train_set;
    else
        task_set = extra_tasks[task_index];    

    if(task_set->targetsize() != 1)
        PLERROR("In DistRepNNet::buildVarGraph(): task_set->targetsize() must be 1, not %d",targetsize_);    

    // Initialize the input.
    // This is where we construct the distributed representation
    // mappings (matrices).
    // The input is separated in two parts, one which corresponds
    // to symbolic data (uses distributed representations) and
    // one which corresponds to real valued data
    // Finaly, in order to figure out how many representation
    // mappings have to be constructed (since several input elements
    // might share the same Dictionary), we use the pointer
    // value of the Dictionaries 

    int n_dist_rep_input = 0;
    input_to_dict_index.resize(inputsize_);
    input_to_dict_index.fill(-1);
    target_dict_index = -1;

    //if(direct_in_to_out && nnet_architecture == "csMTL")
    //    PLERROR("In DistRepNNet::buildVarGraph(): direct_in_to_out cannot be used with \"csMTL\" architecture");

    // Associate input components with their corresponding
    // Dictionary and distributed representation
    for(int i=0; i<task_set->inputsize(); i++)
    {
        PP<Dictionary> dict = task_set->getDictionary(i);

        // Check if component has Dictionary
        if(dict)
        {
            // Find if Dictionary has already been added
            int f = dictionaries.find(dict);               
            if(f<0)
            {
                dictionaries.push_back(dict);
                input_to_dict_index[i] = dictionaries.size()-1;
            }
            else input_to_dict_index[i] = f;
            n_dist_rep_input++;
        }
    }

    // Add target Dictionary
    {
        PP<Dictionary> dict;
        if(target_dictionary && task_index < 0) dict = target_dictionary; 
        else dict = task_set->getDictionary(task_set->inputsize());

        // Check if component has Dictionary
        if(!dict) PLERROR("In DistRepNNet::buildVarGraph(): target component of task set has no Dictionary");
        // Find if Dictionary has already been added
        int f = dictionaries.find(dict);               
        if(f<0)
        {
            dictionaries.push_back(dict);
            target_dict_index = dictionaries.size()-1;
        }
        else
            target_dict_index = f;
    }
        
//    if(dist_rep_dim.length() != dist_reps.length())
//        PLWARNING("In DistRepNNet::buildVarGraph(): number of distributed representation sets (%d) and dimensionaly specification (dist_rep_dim.length()=%d) isn't the same", dist_reps.length(), dist_rep_dim.length());
        
    input = Var(task_set->inputsize(), "input");
    
    //if(nnet_architecture == "dist_rep_predictor" || nnet_architecture == "linear")
    //if(!use_dist_reps)
    //{
    //    ntokens = 1;
    //    nfeatures_per_token = task_set->inputsize();
    //    nhidden_dist_rep_predictor = -1;
    //    dim = dictionaries[target_dict_index]->size() + (dictionaries[target_dict_index]->oov_not_in_possible_values ? 0 : 1);
    //}

    if(use_dist_reps)
    {
        int dim = dist_rep_dim[0];
        int this_ntokens;
        if(task_index < 0)
        {
            if(ntokens <= 0) PLERROR("In DistRepNNet::buildVarGraph(): ntokens should be > 0");
            if(nfeatures_per_token <= 0) PLERROR("In DistRepNNet::buildVarGraph(): nfeatures_per_token should be > 0");
            if(ntokens * nfeatures_per_token != task_set->inputsize()) PLERROR("In DistRepNNet::buildVarGraph(): ntokens * nfeatures_per_token != task_set->inputsize()");
            this_ntokens = ntokens;
        }
        else
        {
            if(task_index >= ntokens_extra_tasks.length()) PLERROR("In DistRepNNet::buildVarGraph(): ntokens not defined for task %d", task_index);
            if(ntokens_extra_tasks[task_index] <= 0) PLERROR("In DistRepNNet::buildVarGraph(): ntokens[%d] should be > 0", task_index);
            if(nfeatures_per_token <= 0) PLERROR("In DistRepNNet::buildVarGraph(): nfeatures_per_token should be > 0");
            if(ntokens_extra_tasks[task_index] * nfeatures_per_token != task_set->inputsize()) PLERROR("In DistRepNNet::buildVarGraph(): ntokens_extra_task[%d] * nfeatures_per_token != task_set->inputsize()",task_index);
            this_ntokens = ntokens_extra_tasks[task_index];
        }
                
        activated_weights.resize(0);
        VarArray dist_reps(this_ntokens);
        VarArray dist_rep_hids(this_ntokens);

        if(winputdistrep.length() == 0)
        {
            winputdistrep.resize(nfeatures_per_token+1);
            if(nhidden_dist_rep_predictor > 0) winputdistrep[nfeatures_per_token] = Var(1,nhidden_dist_rep_predictor);
            else winputdistrep[nfeatures_per_token] = Var(1,dim);
            for(int j=0; j<nfeatures_per_token; j++)
            {
                if(input_to_dict_index[j] < 0)
                    if(nhidden_dist_rep_predictor > 0) winputdistrep[j] = Var(nhidden_dist_rep_predictor);
                    else winputdistrep[j] = Var(dim);
                else                            
                    if(nhidden_dist_rep_predictor > 0) winputdistrep[j] = Var(dictionaries[input_to_dict_index[j]]->size()+1,nhidden_dist_rep_predictor);
                    else winputdistrep[j] = Var(dictionaries[input_to_dict_index[j]]->size()+1,dim);
                if(nhidden_dist_rep_predictor > 0) woutdistrep = Var(nhidden_dist_rep_predictor+1,dim);
            }
            params.append(winputdistrep);
            partial_update_vars.append(winputdistrep);
            if(nhidden_dist_rep_predictor > 0) params.append(woutdistrep); 
        }

        // Building var graph from input to distributed representations
        for(int i=0; i<this_ntokens; i++)
        {
            if(nhidden_dist_rep_predictor > 0) dist_rep_hids[i] = buildSparseAffineTransform(winputdistrep, input, input_to_dict_index, i*nfeatures_per_token, activated_weights);
            else dist_reps[i] =  buildSparseAffineTransform(winputdistrep, input, input_to_dict_index, i*nfeatures_per_token, activated_weights);

            if(nhidden_dist_rep_predictor > 0) 
            {
                dist_rep_hids[i] = add_transfer_func(dist_rep_hids[i]);
                dist_reps.append(affine_transform(dist_rep_hids[i],woutdistrep));
            }
        }

        if(task_index < 0)
        {
            // To construct the Func...
            token_features = Var(nfeatures_per_token);
            Var dist_rep_hid;
            VarArray aw;
            if(nhidden_dist_rep_predictor > 0) dist_rep_hid = buildSparseAffineTransform(winputdistrep, token_features, input_to_dict_index, 0, aw);
            else dist_rep =  buildSparseAffineTransform(winputdistrep, token_features, input_to_dict_index, 0, aw);

            if(nhidden_dist_rep_predictor > 0) 
            {
                dist_rep_hid = add_transfer_func(dist_rep_hid);                
                dist_rep = affine_transform(dist_rep_hid,woutdistrep);
            }
        }
        
        dp_input = hconcat(dist_reps);
    }

    if(fixed_output_weights && !use_dist_reps && (task_index < 0 && nhidden <= 0 || task_index>=0 && nhidden_extra_tasks[task_index] <= 0))
        PLERROR("In DistRepNNet::buildVarGraph(): fixed output weights is not implemented for sparse input and no hidden layers");    

    // Build main network graph.
    buildOutputFromInput(task_index);

    target = Var(targetsize_);
    TVec<int> target_cols(1);
    target_cols[0] = task_set->inputsize();

    Var reind_target;
    if(target_dictionary && task_index < 0)
        reind_target = reindexed_target(target,input,target_dictionary,target_cols);
    else
        reind_target = reindexed_target(target,input,task_set,target_cols);
    //reind_target = target;

    if(weightsize_>0)
    {
        if (weightsize_!=1)
            PLERROR("In DistRepNNet::buildVarGraph(): expected weightsize to be 1 or 0 (or unspecified = -1, meaning 0), got %d",weightsize_);
        sampleweight = Var(1, "weight");
    }

    string pt = lowerstring( penalty_type );
    if( pt == "l1" )
        penalty_type = "L1";
    else if( pt == "l1_square" || pt == "l1 square" || pt == "l1square" )
        penalty_type = "L1_square";
    else if( pt == "l2_square" || pt == "l2 square" || pt == "l2square" )
        penalty_type = "L2_square";
    else if( pt == "l2" )
    {
        PLWARNING("L2 penalty not supported, assuming you want L2 square");
        penalty_type = "L2_square";
    }
    else
        PLERROR("penalty_type \"%s\" not supported", penalty_type.c_str());

    buildCosts(output, reind_target, task_index);

    // Build functions.
    if(task_index < 0)
        buildFuncs(invars);
    else
        buildFuncs(invars_extra_tasks[task_index]);

//    if(consider_unseen_classes) cost_paramf.resize(getTrainCostNames().length());
    
}

////////////
// build_ //
////////////
void DistRepNNet::build_()
{
    /*
     * Create Topology Var Graph
     */

    // Don't do anything if we don't have a train_set
    // It's the only one who knows the inputsize, targetsize and weightsize,
    // and it contains the Dictionaries...

    if(inputsize_>=0 && targetsize_>=0 && weightsize_>=0)
    {
        if(targetsize_ != 1)
            PLERROR("In DistRepNNet::build_(): targetsize_ must be 1, not %d",targetsize_);
        if(fixed_output_weights && use_output_weights_bases)
            PLERROR("In DistRepNNet::build_(): output weights cannot be fixed (i.e. fixed_output_weights=1) and predicted (i.e. use_output_weights_bases=1)");
        if(direct_in_to_out && use_output_weights_bases)
            PLERROR("In DistRepNNet::build_(): direct input to output weights cannot be used with output weights bases");        
        if(!use_output_weights_bases && !use_dist_reps 
           && extra_tasks.length() != 0)
            PLERROR("In DistRepNNet::build_(): it is useless to have extra tasks and not use distributed\n"
                "representations or output weights bases.");

        dictionaries.resize(0);
        partial_update_vars.resize(0);
        //partial_update_vars_extra_tasks.resize(extra_tasks.length());
        //for(int t=0; t<partial_update_vars_extra_tasks.length(); t++)
        //    partial_update_vars_extra_tasks[t].resize(0);
        params.resize(0);
        training_cost_extra_tasks.resize(0);        
        invars_extra_tasks.resize(extra_tasks.length());

        // Reset shared parameters
        winputdistrep.resize(0);
        woutdistrep = (Variable*) NULL;
        w1theta = (Variable*) NULL;
        wouttheta = (Variable*) NULL;

        for(int t=0; t<extra_tasks.length(); t++)
        {
            // Reset parameters variable
            w1 = (Variable*) NULL;
            w2 = (Variable*) NULL;
            wout = (Variable*) NULL;
            direct_wout = (Variable*) NULL;
            outbias = (Variable*) NULL;        
            winputsparse.resize(0);
            winputsparse_weight_decay = 0;
            winputsparse_bias_decay = 0;

            buildVarGraph(t);
            initializeParams(true,t);
        }

        // Reset parameters variable
        w1 = (Variable*) NULL;
        w2 = (Variable*) NULL;
        wout = (Variable*) NULL;
        direct_wout = (Variable*) NULL;
        outbias = (Variable*) NULL;
        winputsparse.resize(0);
        winputsparse_weight_decay = 0;
        winputsparse_bias_decay = 0;

        buildVarGraph(-1);
        initializeParams();

        // Shared values hack...
        if (!do_not_change_params) {
            if((bool)paramsvalues && (paramsvalues.size() == params.nelems()))
                params << paramsvalues;
            else
            {
                paramsvalues.resize(params.nelems());
                if(optimizer)
                    optimizer->reset();
                for(int t=0; t<optimizer_extra_tasks.length(); t++)
                    optimizer_extra_tasks[t]->reset();
            }
            params.makeSharedValue(paramsvalues);
        }
        
        output_comp.resize(1);
    }
}

////////////////
// buildCosts //
////////////////
void DistRepNNet::buildCosts(const Var& the_output, const Var& the_target, int task_index) {
    int ncosts = cost_funcs.size();  
    if(ncosts<=0)
        PLERROR("In DistRepNNet::buildCosts - Empty cost_funcs : must at least specify the cost function to optimize!");
    costs.resize(ncosts);

    for(int k=0; k<ncosts; k++)
    {
        // create costfuncs and apply individual weights if weightpart > 1
        if(cost_funcs[k]=="mse_onehot")
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
        else if (cost_funcs[k]=="margin_perceptron_cost")
            costs[k] = margin_perceptron_cost(the_output,the_target,margin);
        else  // Assume we got a Variable name and its options
        {
            costs[k]= dynamic_cast<Variable*>(newObject(cost_funcs[k]));
            if(costs[k].isNull())
                PLERROR("In DistRepNNet::build_()  unknown cost_func option: %s",cost_funcs[k].c_str());
            costs[k]->setParents(the_output & the_target);
            costs[k]->build();
        }
    }


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
        if (weightsize_>0)
            // only multiply by sampleweight if there are weights
            training_cost = hconcat(sampleweight*sum(hconcat(costs[0] & penalties))
                                    & (test_costs*sampleweight));
        else {
            training_cost = hconcat(sum(hconcat(costs[0] & penalties)) & test_costs);
        }
    } 
    else {
        if(weightsize_>0) {
            // only multiply by sampleweight if there are weights
            training_cost = hconcat(costs[0]*sampleweight & test_costs*sampleweight);
        } else {
            training_cost = hconcat(costs[0] & test_costs);
        }
    }

    if(task_index >= 0) training_cost_extra_tasks.push_back(training_cost);

    training_cost->setName("training_cost");
    test_costs->setName("test_costs");
    the_output->setName("output");
}

////////////////
// buildFuncs //
////////////////
void DistRepNNet::buildFuncs(VarArray& invars) {
    invars.resize(0);
    VarArray outvars;
    VarArray testinvars;
    if (input)
    {
        invars.push_back(input);
        testinvars.push_back(input);
    }
    if (output)
        outvars.push_back(output);
    if(target)
    {
        invars.push_back(target);
        testinvars.push_back(target);
        outvars.push_back(target);
    }
    if(sampleweight)
    {
        invars.push_back(sampleweight);
    }
    f = Func(input, argmax(output));
    test_costf = Func(testinvars, argmax(output)&test_costs);
    test_costf->recomputeParents();
    if(dist_rep)
        token_to_dist_rep = Func(token_features,dist_rep);
    paramf = Func(invars, training_cost); 
    //displayFunction(paramf, true, false, 250);
}

//////////////////////////
// buildOutputFromInput //
//////////////////////////
void DistRepNNet::buildOutputFromInput(int task_index) {
    
    /*
    if(nnet_architecture == "csMTL")
    {
        // The idea is to output a "potential" for each
        // target possibility...
        // Hence, we need to make a propagation path from
        // the computations using only the input part
        // (and hence commun to all targets) and the
        // target disptributed representation, to the potential output.
        // In order to know what are the possible targets,
        // the train_set vmat, which contains the target
        // Dictionary, will be used.

        // Computations common to all targets
        if(nhidden>0)
        {
            w1 = Var(1 + dp_input->size(), nhidden, "w1");
            params.append(w1);
            output = affine_transform(dp_input, w1); 
        }
        else
        {
            wout = Var(1 + dp_input->size(), outputsize(), "wout");
            output = affine_transform(dp_input, wout);
            if(!fixed_output_weights)
            {
                params.append(wout);
            }
            else
            {
                outbias = Var(output->size(),"outbias");
                output = output + outbias;
                params.append(outbias);
            }
        }

        Var comp_input = output;
        Var dp_target = Var(1,dist_rep_dim[target_dict_index]);

        VarArray proppath_params;
        if(nhidden>0)
        {
            w1target = Var( dp_target->size(),nhidden, "w1target");      
            params.append(w1target);
            proppath_params.append(w1target);
            output = output + product(dp_target, w1target);
            output = add_transfer_func(output);
        }
        else
        {
            wouttarget = Var(dp_target->size(),outputsize(), "wouttarget");
            if (!fixed_output_weights)        
            {
                params.append(wouttarget);        
                proppath_params.append(wouttarget);        
            }
            output = output + product(dp_target,wouttarget);
            //output = add_transfer_func(output);
        }
    
        // second hidden layer
        if(nhidden2>0)
        {
            w2 = Var(1 + output.length(), nhidden2, "w2");
            params.append(w2);
            proppath_params.append(w2);
            output = affine_transform(output,w2);
            output = add_transfer_func(output);
        }

        if (nhidden2>0 && nhidden==0)
            PLERROR("DistRepNNet:: can't have nhidden2 (=%d) > 0 while nhidden=0",nhidden2);

        // output layer before transfer function when there is at least one hidden layer
        if(nhidden > 0)
        {
            wout = Var(1 + output->size(), outputsize(), "wout");
            output = affine_transform(output, wout);

            if (!fixed_output_weights)
            {
                params.append(wout);
                proppath_params.append(wout);
            }
            else
            {
                outbias = Var(output->size(),"outbias");
                output = output + outbias;
                params.append(outbias);
                proppath_params.append(outbias);
            }
        }

        output = potentials(input,comp_input,dp_target,dist_reps[target_dict_index], output, proppath_params, train_set);
        partial_update_vars.push_back(dist_reps[target_dict_index]);
    }
    else 
    */

    int this_nhidden;
    int this_nhidden2;
    if(task_index < 0)
    {
        this_nhidden = nhidden;
        this_nhidden2 = nhidden2;
    }
    else
    {
        this_nhidden = nhidden_extra_tasks[task_index];
        this_nhidden2 = nhidden2_extra_tasks[task_index];
    }

    if(!use_dist_reps)
    {
        if(!use_output_weights_bases)
        {
            // These weights will be used as the input weights of the neural
            // network, instead of w1.
            int dim;       
            if(this_nhidden > 0) dim = this_nhidden;
            else dim = dictionaries[target_dict_index]->oov_not_in_possible_values ? 0 : 1;
            winputsparse.resize(input->length()+1);
            winputsparse[input->length()] = Var(1,dim);
            for(int j=0; j<winputsparse.length()-1; j++)
            {
                if(input_to_dict_index[j] < 0)
                    winputsparse[j] = Var(dim);
                else
                    winputsparse[j] = Var(dictionaries[input_to_dict_index[j]]->size()+1,dim);
            }
            params.append(winputsparse);
            partial_update_vars.append(winputsparse);            
        }

        if(this_nhidden>0)
        {
            output = buildSparseAffineTransform(winputsparse,input,input_to_dict_index,0,activated_weights); 
            output = add_transfer_func(output);

            winputsparse_weight_decay = weight_decay + layer1_weight_decay;
            winputsparse_bias_decay = bias_decay + layer1_bias_decay;

            // ici: Faire direct in to out

            if(direct_in_to_out)
            {
                PLERROR("In buildOutputFromInput(): direct_in_to_out option not implemented for sparse input.");
                direct_wout = Var(dp_input->size(), dictionaries[target_dict_index]->size() + (dictionaries[target_dict_index]->oov_not_in_possible_values ? 0 : 1),"direct_wout");
                params.append(direct_wout);
            }                           
        }

        // second hidden layer
        if(this_nhidden2>0)
        {
            w2 = Var(1 + output.length(), this_nhidden2, "w2");
            params.append(w2);
            output = affine_transform(output,w2);
            output = add_transfer_func(output);
            wout = Var(1 + this_nhidden2, dictionaries[target_dict_index]->size() + (dictionaries[target_dict_index]->oov_not_in_possible_values ? 0 : 1), "wout");
        }
        else if(this_nhidden > 0) wout = Var(1 + this_nhidden, dictionaries[target_dict_index]->size() + (dictionaries[target_dict_index]->oov_not_in_possible_values ? 0 : 1), "wout");

        if (this_nhidden2>0 && this_nhidden==0)
            PLERROR("DistRepNNet:: can't have nhidden2 (=%d) > 0 while nhidden=0",this_nhidden2);

        if(this_nhidden > 0)
        {            
            // ici: ajouter option sans biais pour sparse product...
            if(direct_in_to_out)
                output = affine_transform(output, wout) + product(dp_input,direct_wout);
            else
                output = affine_transform(output, wout);
        }
        else
        {
            output = buildSparseAffineTransform(winputsparse,input,input_to_dict_index,0,activated_weights); 

            winputsparse_weight_decay = weight_decay + output_layer_weight_decay;
            winputsparse_bias_decay = bias_decay + output_layer_bias_decay;
        }

        if(fixed_output_weights)
        {
            outbias = Var(output->size(),"outbias");
            output = output + outbias;
            params.append(outbias);
        }
        else if(this_nhidden>0) params.append(wout);

        output = transpose(output);
    }
    else
    {
        int before_output_size = dp_input->size();
        if(this_nhidden > 0) before_output_size = this_nhidden;
        if(this_nhidden2 > 0) before_output_size = this_nhidden2;

        /*if(possible_targets_varies)
        {            
            PLERROR("In DistRepNNet::buildOutputFromInput(): possible_targets_varies is not implemented yet");
            if(use_output_weights_bases)
                ;
        }
        else*/
        {
            if(use_output_weights_bases)
            {
                PLERROR("In DistRepNNet::buildOutputFromInput(): use_output_weights_bases is not implemented yet");

                // À faire: faire vérification pour partager les 
                // représentations lorsque les dictionnaires sont les mêmes...
                // Ne pas oublier de partager les poids w1theta et wouttheta

                dp_all_targets = Var(dictionaries[target_dict_index]->oov_not_in_possible_values ? 0 : 1,dist_rep_dim.last(),"dp_all_targets");
                // Penser à l'initialisation!!!! Comment faire!!!
                params.append(dp_all_targets);
                
                if(nhidden_theta_predictor>0)
                {
                    if(!w1theta) 
                    {
                        w1theta = Var(dp_all_targets->length()+1,nhidden_theta_predictor,"w1theta");
                        params.append(w1theta);
                    }
                    wout = new MatrixAffineTransformVariable(dp_all_targets,w1theta);
                    wout = add_transfer_func(wout);
                }
                else
                    wout = dp_all_targets;
            
            
                if(!wouttheta)
                {
                    wouttheta = Var(wout->length()+1,before_output_size+1, "wouttheta");
                    params.append(wouttheta);
                }
                wout = new MatrixAffineTransformVariable(wout,wouttheta);
            }
            else
            {
                wout = Var(1 + before_output_size, dictionaries[target_dict_index]->size() + (dictionaries[target_dict_index]->oov_not_in_possible_values ? 0 : 1), "wout");                
            }
        }

        if(this_nhidden>0)
        {
            w1 = Var(1 + dp_input->size(), this_nhidden, "w1");
            params.append(w1);
            output = affine_transform(dp_input, w1); 
            output = add_transfer_func(output);
            if(direct_in_to_out)
            {
                direct_wout = Var(dp_input->size(), dictionaries[target_dict_index]->size() + (dictionaries[target_dict_index]->oov_not_in_possible_values ? 0 : 1),"direct_wout");
                params.append(direct_wout);
            }                           
        }

        // second hidden layer
        if(this_nhidden2>0)
        {
            w2 = Var(1 + output.length(), this_nhidden2, "w2");
            params.append(w2);
            output = affine_transform(output,w2);
            output = add_transfer_func(output);
        }

        if (this_nhidden2>0 && this_nhidden==0)
            PLERROR("DistRepNNet:: can't have nhidden2 (=%d) > 0 while nhidden=0",this_nhidden2);

        if(this_nhidden > 0)
        {            
            if(direct_in_to_out)
                output = affine_transform(output, wout) + product(dp_input,direct_wout);
            else
                output = affine_transform(output, wout);
        }
        else
        {
            if(use_dist_reps)
                output = affine_transform(dp_input, wout);
            else
                output = buildSparseAffineTransform(winputsparse,input,input_to_dict_index,0,activated_weights); 
        }

        if(fixed_output_weights)
        {
            outbias = Var(output->size(),"outbias");
            output = output + outbias;
            params.append(outbias);
        }
        else if(use_dist_reps)
            params.append(wout);

        output = transpose(output);
    }
    /*
    TVec<bool> class_tag(dictionaries[target_dict_index]->size() + (dictionaries[target_dict_index]->oov_not_in_possible_values ? 0 : 1));
    Vec row(train_set.width());
    int target;
    class_tag.fill(0);
    for(int i=0; i<train_set.length(); i++)
    {
        train_set->getRow(i,row);
        target = (int) row[train_set->inputsize()];
        class_tag[target] = 1;
    }

    Vec seen_target_vec(0);
    seen_target.resize(0);
    unseen_target.resize(0);
    for(int i=0; i<class_tag.length(); i++)
        if(class_tag[i])
        {
            seen_target_vec.push_back(i);
            seen_target.push_back(i);
        }
        else unseen_target.push_back(i);

    if(seen_target_vec.length() != class_tag.length())
        train_output = new VarRowsVariable(output,new SourceVariable(seen_target_vec));
    */

    // output_transfer_func
    if(output_transfer_func!="" && output_transfer_func!="none")
    {
        /*
        if(consider_unseen_classes)
            output = insert_zeros(add_transfer_func(output, output_transfer_func),seen_target);
        else
        */
            output = add_transfer_func(output, output_transfer_func);
    }
    /*
    else
    {
        if(consider_unseen_classes)
            output = insert_zeros(output,seen_target);
    }
    */

    /*
    if(train_output)
        if(output_transfer_func!="" && output_transfer_func!="none")
            train_output = insert_zeros(add_transfer_func(train_output, output_transfer_func),unseen_target);
        else
            train_output = insert_zeros(train_output,unseen_target);
    */
}

////////////////////
// buildPenalties //
////////////////////
void DistRepNNet::buildPenalties() {
    penalties.resize(0);  // prevents penalties from being added twice by consecutive builds
    if(w1 && ((layer1_weight_decay + weight_decay)!=0 || (layer1_bias_decay + bias_decay)!=0))
        penalties.append(affine_transform_weight_penalty(w1, (layer1_weight_decay + weight_decay), (layer1_bias_decay + bias_decay), penalty_type));
    if(w1theta && ((layer1_theta_predictor_weight_decay + weight_decay)!=0 || (layer1_theta_predictor_bias_decay + bias_decay)!=0))
        penalties.append(affine_transform_weight_penalty(w1theta, (layer1_theta_predictor_weight_decay + weight_decay), (layer1_theta_predictor_bias_decay + bias_decay), penalty_type));
    if(w2 && ((layer2_weight_decay + weight_decay)!=0 || (layer2_bias_decay + bias_decay)!=0))
        penalties.append(affine_transform_weight_penalty(w2, (layer2_weight_decay + weight_decay), (layer2_bias_decay + bias_decay), penalty_type));
    if(!use_output_weights_bases && wout && ((output_layer_weight_decay + weight_decay)!=0 || (output_layer_bias_decay + bias_decay)!=0))
        penalties.append(affine_transform_weight_penalty(wout, (output_layer_weight_decay + weight_decay), 
                                                         (output_layer_bias_decay + bias_decay), penalty_type));
    if(wouttheta && ((output_layer_theta_predictor_weight_decay + weight_decay)!=0 || (output_layer_theta_predictor_bias_decay + bias_decay)!=0))
        penalties.append(affine_transform_weight_penalty(wouttheta, (output_layer_theta_predictor_weight_decay + weight_decay), 
                                                         (output_layer_theta_predictor_bias_decay + bias_decay), penalty_type));
    if(woutdistrep && ((output_dist_rep_predictor_weight_decay + weight_decay)!=0 || (output_dist_rep_predictor_bias_decay + bias_decay)!=0))
        penalties.append(affine_transform_weight_penalty(woutdistrep, (output_dist_rep_predictor_weight_decay + weight_decay), 
                                                         (output_dist_rep_predictor_bias_decay + bias_decay), penalty_type));
    if(direct_wout && ((direct_in_to_out_weight_decay + weight_decay)!=0 || (direct_in_to_out_bias_decay + bias_decay)!=0))
        penalties.append(affine_transform_weight_penalty(direct_wout, (direct_in_to_out_weight_decay + weight_decay),
                                                         (direct_in_to_out_bias_decay + bias_decay), penalty_type));

    // Here, affine_transform_weight_penalty is not used differently, since the weight variables don't correspond
    // to an affine_transform (i.e. doesn't contain biases AND a weights)
    if(winputdistrep.length() != 0 && (input_dist_rep_predictor_weight_decay + weight_decay))
    {
        for(int i=0; i<activated_weights.length(); i++)
        {
            if(input_to_dict_index[i%nfeatures_per_token] < 0)
            {
                // Add those weights in the penalties only once
                if(i<nfeatures_per_token)
                    penalties.append(affine_transform_weight_penalty(winputdistrep[i], (input_dist_rep_predictor_weight_decay + weight_decay), 
                                                                     (input_dist_rep_predictor_weight_decay + weight_decay), penalty_type));
            }
            else
                // Approximate version of the weight decay on the input weights, which is more computationally efficient
                penalties.append(affine_transform_weight_penalty(activated_weights[i], (input_dist_rep_predictor_weight_decay + weight_decay), 
                                                                 (input_dist_rep_predictor_weight_decay + weight_decay), penalty_type));                   
        }
    }
    if(winputdistrep.length() != 0 && (input_dist_rep_predictor_bias_decay + bias_decay))
        penalties.append(affine_transform_weight_penalty(winputdistrep[nfeatures_per_token], (input_dist_rep_predictor_bias_decay + bias_decay), 
                                                         (input_dist_rep_predictor_bias_decay + bias_decay), penalty_type));

    if(winputsparse.length() != 0 && winputsparse_weight_decay)
    {
        for(int i=0; i<activated_weights.length(); i++)
        {
            penalties.append(affine_transform_weight_penalty(activated_weights[i], winputsparse_weight_decay, 
                                                             winputsparse_weight_decay, penalty_type));
        }
    }
    if(winputsparse.length() != 0 && winputsparse_bias_decay)
        penalties.append(affine_transform_weight_penalty(winputsparse.last(), winputsparse_bias_decay, 
                                                         winputsparse_bias_decay, penalty_type));

}

/////////////////////////////
// computeCostsFromOutputs //
/////////////////////////////
void DistRepNNet::computeCostsFromOutputs(const Vec& inputv, const Vec& outputv, 
                                   const Vec& targetv, Vec& costsv) const
{
    PLERROR("In DistRepNNet::computeCostsFromOutputs: output is not enough to compute cost");
    //computeOutputAndCosts(inputv,targetv,outputv,costsv);
}

///////////////////
// computeOutput //
///////////////////
void DistRepNNet::computeOutput(const Vec& inputv, Vec& outputv) const
{
    f->sizefprop(inputv,output_comp);
    row.resize(inputsize_);
    row << inputv;
    row.resize(train_set->width());
    row.subVec(inputsize_,train_set->width()-inputsize_).fill(MISSING_VALUE);
    //if(target_dictionary)
    //    target_values = target_dictionary->getValues();
    //else
    //    target_values = train_set->getValues(row,targetsize_);
    //outputv[0] = target_values[(int)output_comp[0]];
    outputv[0] = (int)output_comp[0];
}

///////////////////////////
// computeOutputAndCosts //
///////////////////////////
void DistRepNNet::computeOutputAndCosts(const Vec& inputv, const Vec& targetv, 
                                 Vec& outputv, Vec& costsv) const
{
    test_costf->sizefprop(inputv&targetv, output_comp&costsv);
    row.resize(inputsize_);
    row << inputv;
    row.resize(train_set->width());
    row.subVec(inputsize_,train_set->width()-inputsize_).fill(MISSING_VALUE);
    //if(target_dictionary)
    //    target_values = target_dictionary->getValues();
    //else
    //    target_values = train_set->getValues(row,inputsize_);
    //outputv[0] = target_values[(int)output_comp[0]];
    outputv[0] = (int)output_comp[0];
    //for(int i=0; i<costsv.length(); i++)
    //    if(is_missing(costsv[i]))
    //        cout << "WHAT THE FUCK!!!" << endl;
}

/////////////////
// fillWeights //
/////////////////
void DistRepNNet::fillWeights(const Var& weights, bool clear_first_row, int use_this_to_scale) {
    if (initialization_method == "zero") {
        weights->value->clear();
        return;
    }
    real delta;
    int is;
    if(use_this_to_scale > 0)
        is = use_this_to_scale;
    else
        is = weights.length();
    if (clear_first_row)
        is--; // -1 to get the same result as before.
    if (initialization_method.find("linear") != string::npos)
        delta = 1.0 / real(is);
    else
        delta = 1.0 / sqrt(real(is));
    if (initialization_method.find("normal") != string::npos)
        fill_random_normal(weights->value, 0, delta);
    else
        fill_random_uniform(weights->value, -delta, delta);
    if (clear_first_row)
        weights->matValue(0).clear();
}

////////////
// forget //
////////////
void DistRepNNet::forget()
{
    if (train_set) 
    {
        paramsvalues.resize(0);
        build();
    }
    if(optimizer)
        optimizer->reset();
    for(int t=0; t<optimizer_extra_tasks.length(); t++)
        optimizer_extra_tasks[t]->reset();
    stage = 0;
}

///////////////////////
// getTrainCostNames //
///////////////////////
TVec<string> DistRepNNet::getTrainCostNames() const
{
    assert( !cost_funcs.isEmpty() );
    int n_costs = cost_funcs.length();
    TVec<string> train_costs(n_costs + 1);
    train_costs[0] = cost_funcs[0] + "+penalty";
    train_costs.subVec(1, n_costs) << cost_funcs;
    return train_costs;
}

//////////////////////
// getTestCostNames //
//////////////////////
TVec<string> DistRepNNet::getTestCostNames() const
{ 
    return cost_funcs;
}

///////////////////////
// add_transfer_func //
///////////////////////
Var DistRepNNet::add_transfer_func(const Var& input, string transfer_func) {
    Var result;
    if (transfer_func == "default")
        transfer_func = hidden_transfer_func;
    if(transfer_func=="linear")
        result = input;
    else if(transfer_func=="tanh")
        result = tanh(input);
    else if(transfer_func=="sigmoid")
        result = sigmoid(input);
    else if(transfer_func=="softplus")
        result = softplus(input);
    else if(transfer_func=="exp")
        result = exp(input);
    else if(transfer_func=="softmax")
        result = softmax(input);
    else if (transfer_func == "log_softmax")
        result = log_softmax(input);
    else if(transfer_func=="hard_slope")
        result = unary_hard_slope(input,0,1);
    else if(transfer_func=="symm_hard_slope")
        result = unary_hard_slope(input,-1,1);
    else
        PLERROR("In DistRepNNet::add_transfer_func(): Unknown value for transfer_func: %s",transfer_func.c_str());
    return result;
}

//////////////////////
// initializeParams //
//////////////////////
void DistRepNNet::initializeParams(bool set_seed, int task_index)
{
    if (set_seed) {
        if (seed_>=0)
            manual_seed(seed_);
        else
            PLearn::seed();
    }

    if(w1) fillWeights(w1, true);
    /*
      if(w1target)
      fillWeights(w1target, false);
    */
    if(w2) fillWeights(w2, true);
    if(w1theta) fillWeights(w1theta,true);
    if(direct_wout) fillWeights(direct_wout,false);

    if(wout)
        if (fixed_output_weights) {
            static Vec values;
            if (values.size()==0)
            {
                values.resize(2);
                values[0]=-1;
                values[1]=1;
            }
            fill_random_discrete(wout->value, values);
            wout->matValue(0).clear();                        
        }
        else fillWeights(wout, true);
    if(outbias) outbias->matValue.clear();

    if(wouttheta) fillWeights(wouttheta,true,wouttheta->width());

    if(woutdistrep) fillWeights(woutdistrep,true);
    if(winputdistrep.size() != 0)
    {
        for(int i=0; i<nfeatures_per_token; i++)
        {
            if(task_index < 0)
                fillWeights(winputdistrep[i],false,train_set->inputsize());
            else
                fillWeights(winputdistrep[i],false,extra_tasks[task_index]->inputsize());
        }
        winputdistrep[nfeatures_per_token]->value.fill(0.0);
    }
    if(winputsparse.size() != 0)
    {
        for(int i=0; i<winputsparse.size(); i++)
        {
            if(task_index < 0)
                fillWeights(winputsparse[i],false,train_set->inputsize());
            else
                fillWeights(winputsparse[i],false,extra_tasks[task_index]->inputsize());
        }
        winputsparse.last()->value.fill(0.0);
    }
    
}

//! To use varDeepCopyField.
#ifdef __INTEL_COMPILER
#pragma warning(disable:1419)  // Get rid of compiler warning.
#endif
extern void varDeepCopyField(Var& field, CopiesMap& copies);
#ifdef __INTEL_COMPILER
#pragma warning(default:1419)
#endif


/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void DistRepNNet::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(target_values,copies);
    deepCopyField(output_comp,copies);
    deepCopyField(row,copies);
    deepCopyField(tf,copies);
    //deepCopyField(cost_paramf,copies);

    varDeepCopyField(output, copies);
    deepCopyField(costs, copies);
    deepCopyField(partial_update_vars, copies);
    deepCopyField(penalties, copies);
    varDeepCopyField(training_cost, copies);
    deepCopyField(training_cost_extra_tasks, copies);
    varDeepCopyField(test_costs, copies);
    deepCopyField(invars, copies);
    deepCopyField(invars_extra_tasks, copies);
    deepCopyField(params, copies);
    deepCopyField(activated_weights, copies);
    deepCopyField(input_to_dict_index,copies);

    deepCopyField(paramsvalues, copies);
    varDeepCopyField(input, copies);
    varDeepCopyField(dp_input, copies);
    varDeepCopyField(target, copies);
    varDeepCopyField(sampleweight, copies);
    varDeepCopyField(w1, copies);
    deepCopyField(winputsparse, copies);
    //varDeepCopyField(w1target, copies);
    varDeepCopyField(w1theta, copies);
    deepCopyField(winputdistrep, copies);
    varDeepCopyField(woutdistrep, copies);
    varDeepCopyField(w2, copies);
    varDeepCopyField(wout, copies);
    varDeepCopyField(direct_wout, copies);
    //varDeepCopyField(wouttarget, copies);
    varDeepCopyField(wouttheta, copies);
    varDeepCopyField(outbias, copies);
    varDeepCopyField(dp_all_targets, copies);
    varDeepCopyField(token_features, copies);
    varDeepCopyField(dist_rep, copies);
    //deepCopyField(dist_reps, copies);
    deepCopyField(dictionaries,copies);
    //deepCopyField(seen_target, copies);
    //deepCopyField(unseen_target, copies);
    deepCopyField(f, copies);
    deepCopyField(test_costf, copies);
    deepCopyField(token_to_dist_rep, copies);
    deepCopyField(paramf, copies);

    deepCopyField(extra_tasks, copies);
    deepCopyField(nhidden_extra_tasks, copies);
    deepCopyField(nhidden2_extra_tasks, copies);
    deepCopyField(cost_funcs, copies);
    deepCopyField(optimizer, copies);
    deepCopyField(optimizer_extra_tasks, copies);
    deepCopyField(dist_rep_dim, copies);
    deepCopyField(ntokens_extra_tasks,copies);
    deepCopyField(target_dictionary,copies);
    deepCopyField(target_dist_rep,copies);
}

////////////////
// outputsize //
////////////////
int DistRepNNet::outputsize() const {
    return targetsize_;
}

void DistRepNNet::getTokenDistRep(TVec<string>& token_features, Vec& dist_rep)
{
    tf.resize(token_features.length());
    for(int i=0; i<tf.length(); i++)
    {
        if(input_to_dict_index[i] < 0)
            tf[i] = toreal(token_features[i]);
        else
            tf[i] = dictionaries[input_to_dict_index[i]]->getId(token_features[i]);
    }
    token_to_dist_rep->fprop(tf,dist_rep);
}

///////////
// train //
///////////
void DistRepNNet::train()
{
    // DistRepNNet nstages is number of epochs (whole passages through the training set)
    // while optimizer nstages is number of weight updates.
    // So relationship between the 2 depends whether we are in stochastic, batch or minibatch mode

    if(!train_set)
        PLERROR("In DistRepNNet::train, you did not setTrainingSet");
    
    if(!train_stats)
        PLERROR("In DistRepNNet::train, you did not setTrainStatsCollector");
    int current_stage = stage;
    for(int t=0; t<extra_tasks.length(); t++)
    {
        int l = extra_tasks[t]->length();  
        
        // number of samples seen by optimizer before each optimizer update
        int nsamples = batch_size>0 ? batch_size : l;
        paramf = Func(invars_extra_tasks[t], training_cost_extra_tasks[t]); // parameterized function to optimize
        Var totalcost = meanOf(extra_tasks[t], paramf, nsamples, true);
        if(optimizer_extra_tasks[t])
        {
            optimizer_extra_tasks[t]->setToOptimize(params, totalcost);
            if(partial_update_vars.length() != 0) optimizer_extra_tasks[t]->setPartialUpdateVars(partial_update_vars);
            optimizer_extra_tasks[t]->build();
        }
        else PLERROR("DistRepNNet::train can't train without setting an optimizer first!");
        
        // number of optimizer stages corresponding to one learner stage (one epoch)
        int optstage_per_lstage = l/nsamples;
        
        ProgressBar* pb = 0;
        if(report_progress)
            pb = new ProgressBar("Extra task " + tostring(t) + ", Training " + classname() + " from stage " + tostring(stage) + " to " + tostring(nstages), nstages-stage);
        
        int initial_stage = stage;
        bool early_stop=false;
        //displayFunction(paramf, true, false, 250);
        //cout << params.size() << " params to train" << endl;
        while(stage<nstages && !early_stop)
        {
            optimizer_extra_tasks[t]->nstages = optstage_per_lstage;
            train_stats->forget();
            optimizer_extra_tasks[t]->early_stop = false;
            optimizer_extra_tasks[t]->optimizeN(*train_stats);
            // optimizer->verifyGradient(1e-4); // Uncomment if you want to check your new Var.
            train_stats->finalize();
            if(verbosity>2)
                cout << "Epoch " << stage << " train objective: " << train_stats->getMean() << endl;
            ++stage;
            if(pb)
                pb->update(stage-initial_stage);
        }
        if(verbosity>1)
            cout << "EPOCH " << stage << " train objective: " << train_stats->getMean() << endl;
        
        if(pb)
            delete pb;
        stage = current_stage;
    }
    

    int l = train_set->length();  

    if(f.isNull()) // Net has not been properly built yet (because build was called before the learner had a proper training set)
        build();

    // number of samples seen by optimizer before each optimizer update
    int nsamples = batch_size>0 ? batch_size : l;
    paramf = Func(invars, training_cost); // parameterized function to optimize
    Var totalcost = meanOf(train_set, paramf, nsamples, true);
    if(optimizer)
    {
        optimizer->setToOptimize(params, totalcost);
        if(partial_update_vars.length() != 0) optimizer->setPartialUpdateVars(partial_update_vars);
        optimizer->build();
    }
    else PLERROR("DistRepNNet::train can't train without setting an optimizer first!");

    // number of optimizer stages corresponding to one learner stage (one epoch)
    int optstage_per_lstage = l/nsamples;

    ProgressBar* pb = 0;
    if(report_progress)
        pb = new ProgressBar("Training " + classname() + " from stage " + tostring(stage) + " to " + tostring(nstages), nstages-stage);

    int initial_stage = stage;
    bool early_stop=false;
    //displayFunction(paramf, true, false, 250);
    //cout << params.size() << " params to train" << endl;
    while(stage<nstages && !early_stop)
    {
        optimizer->nstages = optstage_per_lstage;
        train_stats->forget();
        optimizer->early_stop = false;
        optimizer->optimizeN(*train_stats);
        // optimizer->verifyGradient(1e-4); // Uncomment if you want to check your new Var.
        train_stats->finalize();
        if(verbosity>2)
            cout << "Epoch " << stage << " train objective: " << train_stats->getMean() << endl;
        ++stage;
        if(pb)
            pb->update(stage-initial_stage);
    }
    if(verbosity>1)
        cout << "EPOCH " << stage << " train objective: " << train_stats->getMean() << endl;

    if(pb)
        delete pb;

    // HUGO: Why?
    test_costf->recomputeParents();
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
