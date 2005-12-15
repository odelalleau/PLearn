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

#include <plearn/vmat/ConcatColumnsVMatrix.h>
#include "DistRepNNet.h"
#include <plearn/math/random.h>
#include <plearn/vmat/SubVMatrix.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(DistRepNNet, "Feedforward Neural Network that learns Distributed Representations for symbolic data", 
                        "Inspired from the NNet class, DistRepNNet is simply an extension that deals with symbolic data\n"
                        "by learning a Distributed Representation for each type of symbolic data. The training set\n"
                        "VMatrix needs to have Dictionary objects which are such that the symbols are indexed from 0 to\n"
                        "Dictinoary.size() (not Dictionary.size()-1, since OOV_TAG isn't counted as in the Dictionary).\n");

DistRepNNet::DistRepNNet() // DEFAULT VALUES FOR ALL OPTIONS
    :
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
margin(1),
fixed_output_weights(0),
penalty_type("L2_square"),
output_transfer_func(""),
hidden_transfer_func("tanh"),
do_not_change_params(false),
batch_size(1),
initialization_method("uniform_linear"),
use_ebm_nnet(1)
{}

DistRepNNet::~DistRepNNet()
{
}

void DistRepNNet::declareOptions(OptionList& ol)
{
    declareOption(ol, "nhidden", &DistRepNNet::nhidden, OptionBase::buildoption, 
                  "Number of hidden units in first hidden layer (0 means no hidden layer)\n");

    declareOption(ol, "nhidden2", &DistRepNNet::nhidden2, OptionBase::buildoption, 
                  "Number of hidden units in second hidden layer (0 means no hidden layer)\n");

    declareOption(ol, "weight_decay", &DistRepNNet::weight_decay, OptionBase::buildoption, 
                  "Global weight decay for all layers\n");

    declareOption(ol, "bias_decay", &DistRepNNet::bias_decay, OptionBase::buildoption, 
                  "Global bias decay for all layers\n");

    declareOption(ol, "layer1_weight_decay", &DistRepNNet::layer1_weight_decay, OptionBase::buildoption, 
                  "Additional weight decay for the first hidden layer.  Is added to weight_decay.\n");

    declareOption(ol, "layer1_bias_decay", &DistRepNNet::layer1_bias_decay, OptionBase::buildoption, 
                  "Additional bias decay for the first hidden layer.  Is added to bias_decay.\n");

    declareOption(ol, "layer2_weight_decay", &DistRepNNet::layer2_weight_decay, OptionBase::buildoption, 
                  "Additional weight decay for the second hidden layer.  Is added to weight_decay.\n");

    declareOption(ol, "layer2_bias_decay", &DistRepNNet::layer2_bias_decay, OptionBase::buildoption, 
                  "Additional bias decay for the second hidden layer.  Is added to bias_decay.\n");

    declareOption(ol, "output_layer_weight_decay", &DistRepNNet::output_layer_weight_decay, OptionBase::buildoption, 
                  "Additional weight decay for the output layer.  Is added to 'weight_decay'.\n");

    declareOption(ol, "output_layer_bias_decay", &DistRepNNet::output_layer_bias_decay, OptionBase::buildoption, 
                  "Additional bias decay for the output layer.  Is added to 'bias_decay'.\n");

    declareOption(ol, "penalty_type", &DistRepNNet::penalty_type,
                  OptionBase::buildoption,
                  "Penalty to use on the weights (for weight and bias decay).\n"
                  "Can be any of:\n"
                  "  - \"L1\": L1 norm,\n"
                  "  - \"L1_square\": square of the L1 norm,\n"
                  "  - \"L2_square\" (default): square of the L2 norm.\n");

    declareOption(ol, "fixed_output_weights", &DistRepNNet::fixed_output_weights, OptionBase::buildoption, 
                  "If true then the output weights are not learned. They are initialized to +1 or -1 randomly.\n");

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
                  "Those values are taken one by one, as the Dictionary objects are extracted.\n");

    declareOption(ol, "initialization_method", &DistRepNNet::initialization_method, OptionBase::buildoption, 
                  "The method used to initialize the weights:\n"
                  " - \"normal_linear\"  = a normal law with variance 1/n_inputs\n"
                  " - \"normal_sqrt\"    = a normal law with variance 1/sqrt(n_inputs)\n"
                  " - \"uniform_linear\" = a uniform law in [-1/n_inputs, 1/n_inputs]\n"
                  " - \"uniform_sqrt\"   = a uniform law in [-1/sqrt(n_inputs), 1/sqrt(n_inputs)]\n"
                  " - \"zero\"           = all weights are set to 0\n");

    declareOption(ol, "use_ebm_nnet", &DistRepNNet::use_ebm_nnet, OptionBase::buildoption, 
                  "Indication that architecture of the neural network should that of an Energy Based Model (EBM)\n");

    declareOption(ol, "paramsvalues", &DistRepNNet::paramsvalues, OptionBase::learntoption, 
                  "The learned parameter vector\n");

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
        
        // Initialize the input.
        // This is where we construct the distributed representation
        // mappings (matrices).
        // The input is separated in two parts, one which corresponds
        // to symbolic data (uses distributed representations) and
        // one which corresponds to real valued data
        // Finaly, in order to figure out how many representation
        // mappings have to be constructed (since several input elements
        // might share the same Dictionary), we use the pointer
        // value of the Dictionaries (I know, this is a bit hacky...)

        int n_dist_rep_input = 0;
        dist_reps.resize(0);
        dictionaries.resize(0);
        partial_update_vars.resize(0);
        input_to_dict_index.resize(inputsize_);
        input_to_dict_index.fill(-1);

        // Associate input components with their corresponding
        // Dictionary and distributed representation
        for(int i=0; i<inputsize_; i++)
        {
            PP<Dictionary> dict = train_set->getDictionary(i);

            // Check if component has Dictionary
            if(dict)
            {
                // Find if Dictionary has already been added
                int f = dictionaries.find(dict);               
                if(f<0)
                {
                    if(dist_rep_dim.length() <= dist_reps.size())
                        PLERROR("In DistRepNNet::build_(): dist_rep_dim isn't big enough, dimensionality specifications are missing");
                    // Add dictionary. Note that, were are also learning a dist rep 
                    // for OOV_TAG, which is not counted as an element of a 
                    // Dictionary object, we need to add one row to the dist rep matrix 
                    dist_reps.push_back(new SourceVariable(dict->size()+1,dist_rep_dim[dist_reps.size()]));
                    dictionaries.push_back(dict);
                    input_to_dict_index[i] = dictionaries.size()-1;
                }
                else input_to_dict_index[i] = f;
                n_dist_rep_input++;
            }
        }   

        // Add target Dictionary
        {
            PP<Dictionary> dict = train_set->getDictionary(inputsize_);

            // Check if component has Dictionary
            if(!dict) PLERROR("In DistRepNNet::build_(): target component has no Dictionary");
            // Find if Dictionary has already been added
            int f = dictionaries.find(dict);               
            if(f<0)
            {
                if(dist_rep_dim.length() <= dist_reps.size())
                    PLERROR("In DistRepNNet::build_(): dist_rep_dim isn't big enough, dimensionality specifications are missing");
                if(use_ebm_nnet)
                    dist_reps.push_back(new SourceVariable(dict->size()+1,dist_rep_dim[dist_reps.size()]));
                dictionaries.push_back(dict);
                target_dict_index = dictionaries.size()-1;
            }            
            else
                target_dict_index = f;
        }
        
        if(dist_rep_dim.length() != dist_reps.length()) 
            PLWARNING("In DistRepNNet::build_(): number of distributed representation sets (%d) and dimensionaly specification (dist_rep_dim.length()=%d) isn't the same", dist_reps.length(), dist_rep_dim.length());
        
        input = Var(inputsize_, "input");
        VarArray input_components;
        Var non_dist_rep_indexes = Var(inputsize_-n_dist_rep_input);
        // Separate input components that have distributed representation 
        // from those that don't
        for(int i=0, j=0; i<inputsize_; i++)
            if(input_to_dict_index[i] < 0)
                non_dist_rep_indexes->value[j++] = i;
            else
            {
                // If the input is missing, then map to OOV_TAG dist. rep., otherwise map to corresponding dist. rep.
                input_components.push_back(dist_reps[input_to_dict_index[i]](isMissing(input[i],true, true, dictionaries[input_to_dict_index[i]]->getId(OOV_TAG) )));
                partial_update_vars.push_back(input_components.last());                
            }
        // Add input with no distributed representation
        if(non_dist_rep_indexes.length() != 0)
        {
            input_components.push_back(new VarRowsVariable(input,non_dist_rep_indexes));
            partial_update_vars.push_back(input_components.last());
        }

        // Input with distributed representations inserted
        Var dp_input = hconcat(input_components);

        params.resize(0);
        params.append(dist_reps);

        // Build main network graph.
        buildOutputFromInput(dp_input);

        target = Var(targetsize_);
        TVec<int> target_cols(targetsize_);
        for(int i=0; i<targetsize_; i++)
            target_cols[i] = inputsize_+i;
        Var reind_target = reindexed_target(target,input,train_set,target_cols);
        if(weightsize_>0)
        {
            if (weightsize_!=1)
                PLERROR("In DistRepNNet::build_(): expected weightsize to be 1 or 0 (or unspecified = -1, meaning 0), got %d",weightsize_);
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

        buildCosts(output, reind_target);

        // Shared values hack...
        if (!do_not_change_params) {
            if((bool)paramsvalues && (paramsvalues.size() == params.nelems()))
                params << paramsvalues;
            else
            {
                paramsvalues.resize(params.nelems());
                initializeParams();
                if(optimizer)
                    optimizer->reset();
            }
            params.makeSharedValue(paramsvalues);
        }

        // Build functions.
        buildFuncs(input, output, target, sampleweight);
        output_comp.resize(1);
    }
}

////////////////
// buildCosts //
////////////////
void DistRepNNet::buildCosts(const Var& the_output, const Var& the_target) {
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

    training_cost->setName("training_cost");
    test_costs->setName("test_costs");
    the_output->setName("output");
}

////////////////
// buildFuncs //
////////////////
void DistRepNNet::buildFuncs(const Var& the_input, const Var& the_output, const Var& the_target, const Var& the_sampleweight) {
    invars.resize(0);
    VarArray outvars;
    VarArray testinvars;
    if (the_input)
    {
        invars.push_back(the_input);
        testinvars.push_back(the_input);
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
    f = Func(the_input, argmax(the_output));
    test_costf = Func(testinvars, argmax(the_output)&test_costs);
    test_costf->recomputeParents();
}

//////////////////////////
// buildOutputFromInput //
//////////////////////////
void DistRepNNet::buildOutputFromInput(const Var& dp_input) {

    if(use_ebm_nnet)
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
            output = add_transfer_func(output);
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
    {
        // Computations common to all targets
        if(nhidden>0)
        {
            w1 = Var(1 + dp_input->size(), nhidden, "w1");
            params.append(w1);
            output = affine_transform(dp_input, w1); 
        }
        else
        {
            wout = Var(1 + dp_input->size(), dictionaries[target_dict_index]->size()+1, "wout");        
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

        // second hidden layer
        if(nhidden2>0)
        {
            w2 = Var(1 + output.length(), nhidden2, "w2");
            params.append(w2);
            output = affine_transform(output,w2);
            output = add_transfer_func(output);
        }

        if (nhidden2>0 && nhidden==0)
            PLERROR("DistRepNNet:: can't have nhidden2 (=%d) > 0 while nhidden=0",nhidden2);

        // output layer before transfer function when there is at least one hidden layer
        if(nhidden > 0)
        {
            wout = Var(1 + output->size(), dictionaries[target_dict_index]->size()+1, "wout");
            output = affine_transform(output, wout);
            if (!fixed_output_weights)
                params.append(wout);
            else
            {
                outbias = Var(output->size(),"outbias");
                output = output + outbias;
                params.append(outbias);
            }
        }

        output = transpose(output);
    }


    // output_transfer_func
    if(output_transfer_func!="" && output_transfer_func!="none")
        output = add_transfer_func(output, output_transfer_func);
}

////////////////////
// buildPenalties //
////////////////////
void DistRepNNet::buildPenalties() {
    penalties.resize(0);  // prevents penalties from being added twice by consecutive builds
    if(w1 && ((layer1_weight_decay + weight_decay)!=0 || (layer1_bias_decay + bias_decay)!=0))
        penalties.append(affine_transform_weight_penalty(w1, (layer1_weight_decay + weight_decay), (layer1_bias_decay + bias_decay), penalty_type));
    if(w1target && ((layer1_weight_decay + weight_decay)!=0 || (layer1_bias_decay + bias_decay)!=0))
        penalties.append(affine_transform_weight_penalty(w1target, (layer1_weight_decay + weight_decay), (layer1_bias_decay + bias_decay), penalty_type));
    if(w2 && ((layer2_weight_decay + weight_decay)!=0 || (layer2_bias_decay + bias_decay)!=0))
        penalties.append(affine_transform_weight_penalty(w2, (layer2_weight_decay + weight_decay), (layer2_bias_decay + bias_decay), penalty_type));
    if(wout && ((output_layer_weight_decay + weight_decay)!=0 || (output_layer_bias_decay + bias_decay)!=0))
        penalties.append(affine_transform_weight_penalty(wout, (output_layer_weight_decay + weight_decay), 
                                                         (output_layer_bias_decay + bias_decay), penalty_type));
    if(wouttarget && ((output_layer_weight_decay + weight_decay)!=0 || (output_layer_bias_decay + bias_decay)!=0))
        penalties.append(affine_transform_weight_penalty(wouttarget, (output_layer_weight_decay + weight_decay), 
                                                         (output_layer_bias_decay + bias_decay), penalty_type));
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
    target_values = train_set->getValues(row,targetsize_);
    outputv[0] = target_values[(int)output_comp[0]];
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
    target_values = train_set->getValues(row,targetsize_);
    outputv[0] = target_values[(int)output_comp[0]];
}

/////////////////
// fillWeights //
/////////////////
void DistRepNNet::fillWeights(const Var& weights, bool clear_first_row) {
    if (initialization_method == "zero") {
        weights->value->clear();
        return;
    }
    real delta;
    int is = weights.length();
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
    if (train_set) initializeParams();
    if(optimizer)
        optimizer->reset();
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
void DistRepNNet::initializeParams(bool set_seed)
{
    if (set_seed) {
        if (seed_>=0)
            manual_seed(seed_);
        else
            PLearn::seed();
    }

    if(nhidden>0) {
        fillWeights(w1, true);
        if(w1target)
            fillWeights(w1target, false);
    }

    if(nhidden2>0) {
        fillWeights(w2, true);
    }

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
        if(wouttarget) fill_random_discrete(wouttarget->value, values);
            
    }
    else {
        fillWeights(wout, true);
        if(wouttarget) fillWeights(wouttarget, false);
    }

    // Initialize distributed representations
    for(int i=0; i<dist_reps.length(); i++)
    {
        fillWeights(dist_reps[i],false);
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
    varDeepCopyField(input, copies);
    varDeepCopyField(target, copies);
    varDeepCopyField(sampleweight, copies);
    varDeepCopyField(w1, copies);
    varDeepCopyField(w1target, copies);
    varDeepCopyField(w2, copies);
    varDeepCopyField(wout, copies);
    varDeepCopyField(wouttarget, copies);
    varDeepCopyField(outbias, copies);
    deepCopyField(dist_reps, copies);
    deepCopyField(dictionaries,copies);
    deepCopyField(input_to_dict_index,copies);
    varDeepCopyField(output, copies);
    deepCopyField(costs, copies);
    deepCopyField(partial_update_vars, copies);
    deepCopyField(penalties, copies);
    varDeepCopyField(training_cost, copies);
    varDeepCopyField(test_costs, copies);
    deepCopyField(invars, copies);
    deepCopyField(params, copies);
    deepCopyField(paramsvalues, copies);
    deepCopyField(f, copies);
    deepCopyField(test_costf, copies);
    deepCopyField(optimizer, copies);
    deepCopyField(cost_funcs, copies);
    deepCopyField(dist_rep_dim, copies);
}

////////////////
// outputsize //
////////////////
int DistRepNNet::outputsize() const {
    return targetsize_;
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

    int l = train_set->length();  

    if(f.isNull()) // Net has not been properly built yet (because build was called before the learner had a proper training set)
        build();

    // number of samples seen by optimizer before each optimizer update
    int nsamples = batch_size>0 ? batch_size : l;
    Func paramf = Func(invars, training_cost); // parameterized function to optimize
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
