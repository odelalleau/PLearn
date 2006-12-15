// -*- C++ -*-

// NeighborhoodSmoothnessNNet.cc
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
 * $Id$
 ******************************************************* */

/*! \file PLearnLibrary/PLearnAlgo/NeighborhoodSmoothnessNNet.h */


#include <plearn/var/AffineTransformVariable.h>
#include <plearn/var/AffineTransformWeightPenalty.h>
#include <plearn/var/BinaryClassificationLossVariable.h>
#include <plearn/var/ClassificationLossVariable.h>
#include <plearn/var/ColumnSumVariable.h>
#include <plearn/var/ConcatColumnsVariable.h>
#include <plearn/vmat/ConcatColumnsVMatrix.h>
#include <plearn/var/CrossEntropyVariable.h>
#include <plearn/var/DotProductVariable.h>
#include <plearn/var/ExpVariable.h>
#include <plearn/var/InvertElementsVariable.h>
#include <plearn/var/LogVariable.h>
#include <plearn/var/LiftOutputVariable.h>
#include <plearn/var/LogSoftmaxVariable.h>
#include <plearn/var/MinusVariable.h>
#include <plearn/var/MulticlassLossVariable.h>
#include <plearn/var/NegateElementsVariable.h>
#include <plearn/var/NegCrossEntropySigmoidVariable.h>
#include "NeighborhoodSmoothnessNNet.h"
#include <plearn/var/OneHotSquaredLoss.h>
#include <plearn/base/ProgressBar.h>
#include <plearn/math/random.h>
#include <plearn/var/SigmoidVariable.h>
#include <plearn/var/SoftmaxVariable.h>
#include <plearn/var/SoftplusVariable.h>
#include <plearn/var/SumVariable.h>
#include <plearn/var/SumAbsVariable.h>
#include <plearn/var/SumOfVariable.h>
#include <plearn/var/SumOverBagsVariable.h>
#include <plearn/var/SumSquareVariable.h>
#include <plearn/var/SubMatVariable.h>
#include <plearn/var/SubMatTransposeVariable.h>
#include <plearn/vmat/SubVMatrix.h>
#include <plearn/var/TanhVariable.h>
#include <plearn/var/TimesVariable.h>
#include <plearn/var/TimesScalarVariable.h>
#include <plearn/var/TransposeProductVariable.h>
#include <plearn/var/UnfoldedFuncVariable.h>
#include <plearn/var/UnfoldedSumOfVariable.h>
#include <plearn/var/Var_operators.h>
#include <plearn/var/Var_utils.h>

//#include "DisplayUtils.h"
//#include "GradientOptimizer.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(NeighborhoodSmoothnessNNet, 
                        "Feedforward neural network whose hidden units are smoothed according to input neighborhood\n",
                        "TODO"
    );

NeighborhoodSmoothnessNNet::NeighborhoodSmoothnessNNet() // DEFAULT VALUES FOR ALL OPTIONS
    :
test_bag_size(0),
max_n_instances(1),
nhidden(0),
nhidden2(0),
noutputs(0),
sigma_hidden(0.1),
sne_weight(0),
weight_decay(0),
bias_decay(0),
layer1_weight_decay(0),
layer1_bias_decay(0),
layer2_weight_decay(0),
layer2_bias_decay(0),
output_layer_weight_decay(0),
output_layer_bias_decay(0),
direct_in_to_out_weight_decay(0),
penalty_type("L2_square"),
L1_penalty(false),
direct_in_to_out(false),
output_transfer_func(""),
interval_minval(0), interval_maxval(1),
batch_size(1)
{}

NeighborhoodSmoothnessNNet::~NeighborhoodSmoothnessNNet()
{
}

void NeighborhoodSmoothnessNNet::declareOptions(OptionList& ol)
{
    declareOption(ol, "max_n_instances", &NeighborhoodSmoothnessNNet::max_n_instances, OptionBase::buildoption, 
                  "    maximum number of instances (input vectors x_i) allowed\n");

    declareOption(ol, "nhidden", &NeighborhoodSmoothnessNNet::nhidden, OptionBase::buildoption, 
                  "    number of hidden units in first hidden layer (0 means no hidden layer)\n");

    declareOption(ol, "nhidden2", &NeighborhoodSmoothnessNNet::nhidden2, OptionBase::buildoption, 
                  "    number of hidden units in second hidden layer (0 means no hidden layer)\n");

    declareOption(ol, "sne_weight", &NeighborhoodSmoothnessNNet::sne_weight, OptionBase::buildoption, 
                  "    The weight of the SNE cost in the total cost optimized.");

    declareOption(ol, "sigma_hidden", &NeighborhoodSmoothnessNNet::sigma_hidden, OptionBase::buildoption, 
                  "    The bandwidth of the Gaussian kernel used to compute the similarity\n"
                  "    between hidden layers.");

    declareOption(ol, "noutputs", &NeighborhoodSmoothnessNNet::noutputs, OptionBase::buildoption, 
                  "    number of output units. This gives this learner its outputsize.\n"
                  "    It is typically of the same dimensionality as the target for regression problems \n"
                  "    But for classification problems where target is just the class number, noutputs is \n"
                  "    usually of dimensionality number of classes (as we want to output a score or probability \n"
                  "    vector, one per class)");

    declareOption(ol, "weight_decay", &NeighborhoodSmoothnessNNet::weight_decay, OptionBase::buildoption, 
                  "    global weight decay for all layers\n");

    declareOption(ol, "bias_decay", &NeighborhoodSmoothnessNNet::bias_decay, OptionBase::buildoption, 
                  "    global bias decay for all layers\n");

    declareOption(ol, "layer1_weight_decay", &NeighborhoodSmoothnessNNet::layer1_weight_decay, OptionBase::buildoption, 
                  "    Additional weight decay for the first hidden layer.  Is added to weight_decay.\n");
    declareOption(ol, "layer1_bias_decay", &NeighborhoodSmoothnessNNet::layer1_bias_decay, OptionBase::buildoption, 
                  "    Additional bias decay for the first hidden layer.  Is added to bias_decay.\n");

    declareOption(ol, "layer2_weight_decay", &NeighborhoodSmoothnessNNet::layer2_weight_decay, OptionBase::buildoption, 
                  "    Additional weight decay for the second hidden layer.  Is added to weight_decay.\n");

    declareOption(ol, "layer2_bias_decay", &NeighborhoodSmoothnessNNet::layer2_bias_decay, OptionBase::buildoption, 
                  "    Additional bias decay for the second hidden layer.  Is added to bias_decay.\n");

    declareOption(ol, "output_layer_weight_decay", &NeighborhoodSmoothnessNNet::output_layer_weight_decay, OptionBase::buildoption, 
                  "    Additional weight decay for the output layer.  Is added to 'weight_decay'.\n");

    declareOption(ol, "output_layer_bias_decay", &NeighborhoodSmoothnessNNet::output_layer_bias_decay, OptionBase::buildoption, 
                  "    Additional bias decay for the output layer.  Is added to 'bias_decay'.\n");

    declareOption(ol, "direct_in_to_out_weight_decay", &NeighborhoodSmoothnessNNet::direct_in_to_out_weight_decay, OptionBase::buildoption, 
                  "    Additional weight decay for the direct in-to-out layer.  Is added to 'weight_decay'.\n");

    declareOption(ol, "penalty_type", &NeighborhoodSmoothnessNNet::penalty_type,
                  OptionBase::buildoption,
                  "    Penalty to use on the weights (for weight and bias decay).\n"
                  "    Can be any of:\n"
                  "      - \"L1\": L1 norm,\n"
                  "      - \"L1_square\": square of the L1 norm,\n"
                  "      - \"L2_square\" (default): square of the L2 norm.\n");

    declareOption(ol, "L1_penalty", &NeighborhoodSmoothnessNNet::L1_penalty, OptionBase::buildoption, 
                  "    Deprecated - You should use \"penalty_type\" instead\n"
                  "    should we use L1 penalty instead of the default L2 penalty on the weights?\n");

    declareOption(ol, "direct_in_to_out", &NeighborhoodSmoothnessNNet::direct_in_to_out, OptionBase::buildoption, 
                  "    should we include direct input to output connections?\n");

    declareOption(ol, "output_transfer_func", &NeighborhoodSmoothnessNNet::output_transfer_func, OptionBase::buildoption, 
                  "    what transfer function to use for ouput layer? \n"
                  "    one of: tanh, sigmoid, exp, softplus, softmax \n"
                  "    or interval(<minval>,<maxval>), which stands for\n"
                  "    <minval>+(<maxval>-<minval>)*sigmoid(.).\n"
                  "    An empty string or \"none\" means no output transfer function \n");

    declareOption(ol, "cost_funcs", &NeighborhoodSmoothnessNNet::cost_funcs, OptionBase::buildoption, 
                  "    a list of cost functions to use\n"
                  "    in the form \"[ cf1; cf2; cf3; ... ]\" where each function is one of: \n"
                  "      mse (for regression)\n"
                  "      mse_onehot (for classification)\n"
                  "      NLL (negative log likelihood -log(p[c]) for classification) \n"
                  "      class_error (classification error) \n"
                  "      binary_class_error (classification error for a 0-1 binary classifier)\n"
                  "      multiclass_error\n"
                  "      cross_entropy (for binary classification)\n"
                  "      stable_cross_entropy (more accurate backprop and possible regularization, for binary classification)\n"
                  "      lift_output (not a real cost function, just the output for lift computation)\n"
                  "    The first function of the list will be used as \n"
                  "    the objective function to optimize \n"
                  "    (possibly with an added weight decay penalty) \n");
  
    declareOption(ol, "classification_regularizer", &NeighborhoodSmoothnessNNet::classification_regularizer, OptionBase::buildoption, 
                  "    used only in the stable_cross_entropy cost function, to fight overfitting (0<=r<1)\n");

    declareOption(ol, "optimizer", &NeighborhoodSmoothnessNNet::optimizer, OptionBase::buildoption, 
                  "    specify the optimizer to use\n");

    declareOption(ol, "batch_size", &NeighborhoodSmoothnessNNet::batch_size, OptionBase::buildoption, 
                  "    how many samples to use to estimate the avergage gradient before updating the weights\n"
                  "    0 is equivalent to specifying training_set->n_non_missing_rows() \n");
    // TODO Not really, since the matrix given typically has much more rows (KNNVMatrix) than input samples.

    declareOption(ol, "paramsvalues", &NeighborhoodSmoothnessNNet::paramsvalues, OptionBase::learntoption, 
                  "    The learned parameter vector\n");

    inherited::declareOptions(ol);

}

///////////
// build //
///////////
void NeighborhoodSmoothnessNNet::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void NeighborhoodSmoothnessNNet::build_()
{
    /*
     * Create Topology Var Graph
     */

    // Don't do anything if we don't have a train_set
    // It's the only one who knows the inputsize and targetsize anyway...

    if(inputsize_>=0 && targetsize_>=0 && weightsize_>=0)
    {

        // init. basic vars
        int true_inputsize = inputsize(); // inputsize is now true inputsize 
        bag_inputs = Var(max_n_instances, inputsize() + 1);
        // The input (with pij) is the first column of the bag inputs.
        Var input_and_pij = subMat(bag_inputs, 0, 0, 1, bag_inputs->width());
        input = new SubMatTransposeVariable(input_and_pij, 0, 0, 1, true_inputsize);
        output = input;
        params.resize(0);

        // first hidden layer
        if(nhidden>0)
        {
            w1 = Var(1 + true_inputsize, nhidden, "w1");      
            output = tanh(affine_transform(output,w1));
            params.append(w1);
            last_hidden = output;
        }

        // second hidden layer
        if(nhidden2>0)
        {
            w2 = Var(1+nhidden, nhidden2, "w2");
            output = tanh(affine_transform(output,w2));
            params.append(w2);
            last_hidden = output;
        }

        if (nhidden==0)
            PLERROR("NeighborhoodSmoothnessNNet:: there must be hidden units!",nhidden2);
      

        // output layer before transfer function

        wout = Var(1+output->size(), outputsize(), "wout");
        output = affine_transform(output,wout);
        params.append(wout);

        // direct in-to-out layer
        if(direct_in_to_out)
        {
            wdirect = Var(true_inputsize, outputsize(), "wdirect");
            output += transposeProduct(wdirect, input);
            params.append(wdirect);
        }

        Var before_transfer_func = output;
   
        /*
         * output_transfer_func
         */
        unsigned int p=0;
        if(output_transfer_func!="" && output_transfer_func!="none")
        {
            if(output_transfer_func=="tanh")
                output = tanh(output);
            else if(output_transfer_func=="sigmoid")
                output = sigmoid(output);
            else if(output_transfer_func=="softplus")
                output = softplus(output);
            else if(output_transfer_func=="exp")
                output = exp(output);
            else if(output_transfer_func=="softmax")
                output = softmax(output);
            else if (output_transfer_func == "log_softmax")
                output = log_softmax(output);
            else if ((p=output_transfer_func.find("interval"))!=string::npos)
            {
                unsigned int q = output_transfer_func.find(",");
                interval_minval = atof(output_transfer_func.substr(p+1,q-(p+1)).c_str());
                unsigned int r = output_transfer_func.find(")");
                interval_maxval = atof(output_transfer_func.substr(q+1,r-(q+1)).c_str());
                output = interval_minval + (interval_maxval - interval_minval)*sigmoid(output);
            }
            else
                PLERROR("In NNet::build_()  unknown output_transfer_func option: %s",output_transfer_func.c_str());
        }

        /*
         * target and weights
         */
      
        target = Var(targetsize()-1, "target");
      
        if(weightsize_>0)
        {
            if (weightsize_!=1)
                PLERROR("NeighborhoodSmoothnessNNet: expected weightsize to be 1 or 0 (or unspecified = -1, meaning 0), got %d",weightsize_);
            sampleweight = Var(1, "weight");
        }

        // checking penalty
        if( L1_penalty )
        {
            PLDEPRECATED("Option \"L1_penalty\" deprecated. Please use \"penalty_type = L1\" instead.");
            L1_penalty = 0;
            penalty_type = "L1";
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

        // create penalties
        penalties.resize(0);  // prevents penalties from being added twice by consecutive builds
        if(w1 && ((layer1_weight_decay + weight_decay)!=0 || (layer1_bias_decay + bias_decay)!=0))
            penalties.append(affine_transform_weight_penalty(w1, (layer1_weight_decay + weight_decay), (layer1_bias_decay + bias_decay), penalty_type));
        if(w2 && ((layer2_weight_decay + weight_decay)!=0 || (layer2_bias_decay + bias_decay)!=0))
            penalties.append(affine_transform_weight_penalty(w2, (layer2_weight_decay + weight_decay), (layer2_bias_decay + bias_decay), penalty_type));
        if(wout && ((output_layer_weight_decay + weight_decay)!=0 || (output_layer_bias_decay + bias_decay)!=0))
            penalties.append(affine_transform_weight_penalty(wout, (output_layer_weight_decay + weight_decay), 
                                                             (output_layer_bias_decay + bias_decay), penalty_type));
        if(wdirect && (direct_in_to_out_weight_decay + weight_decay) != 0)
        {
            if (penalty_type == "L1_square")
                penalties.append(square(sumabs(wdirect))*(direct_in_to_out_weight_decay + weight_decay));
            else if (penalty_type == "L1")
                penalties.append(sumabs(wdirect)*(direct_in_to_out_weight_decay + weight_decay));
            else if (penalty_type == "L2_square")
                penalties.append(sumsquare(wdirect)*(direct_in_to_out_weight_decay + weight_decay));
        }

        // Shared values hack...
        if(paramsvalues && (paramsvalues.size() == params.nelems()))
            params << paramsvalues;
        else
        {
            paramsvalues.resize(params.nelems());
            initializeParams();
        }
        params.makeSharedValue(paramsvalues);

        output->setName("element output");

        f = Func(input, output);
        f_input_to_hidden = Func(input, last_hidden);

        /*
         * costfuncs
         */

        bag_size = Var(1,1);
        bag_hidden = unfoldedFunc(subMat(bag_inputs, 0, 0, bag_inputs.length(), true_inputsize), f_input_to_hidden, false);
        p_ij = subMat(bag_inputs, 1, true_inputsize, bag_inputs->length() - 1, 1);

        // The q_ij function.
        Var hidden_0 = new SubMatTransposeVariable(bag_hidden, 0, 0, 1, bag_hidden->width());
        Var store_hidden(last_hidden.length(), last_hidden.width());
        Var hidden_0_minus_hidden = minus(hidden_0, store_hidden);
        Var k_hidden =
            exp(
                timesScalar(
                    dot(hidden_0_minus_hidden, hidden_0_minus_hidden),
                    var(- 1 / (sigma_hidden * sigma_hidden))
                    )
                );
        Func f_hidden_to_k_hidden(store_hidden, k_hidden);
        Var k_hidden_all =
            unfoldedFunc(
                subMat(
                    bag_hidden, 1, 0, bag_hidden->length() - 1, bag_hidden->width()
                    ),
                f_hidden_to_k_hidden,
                false
                );
        Var one_over_sum_of_k_hidden = invertElements(sum(k_hidden_all));
        Var log_q_ij = log(timesScalar(k_hidden_all, one_over_sum_of_k_hidden));
        Var minus_weight_sum_p_ij_log_q_ij =
            timesScalar(sum(times(p_ij, log_q_ij)), var(-sne_weight));

        int ncosts = cost_funcs.size();  
        if(ncosts<=0)
            PLERROR("In NNet::build_()  Empty cost_funcs : must at least specify the cost function to optimize!");
        costs.resize(ncosts);
      
        for(int k=0; k<ncosts; k++)
        {
            // create costfuncs and apply individual weights if weightpart > 1
            if(cost_funcs[k]=="mse")
                costs[k]= sumsquare(output-target);
            else if(cost_funcs[k]=="mse_onehot")
                costs[k] = onehot_squared_loss(output, target);
            else if(cost_funcs[k]=="NLL") 
            {
                if (output->size() == 1) {
                    // Assume sigmoid output here!
                    costs[k] = cross_entropy(output, target);
                } else {
                    if (output_transfer_func == "log_softmax")
                        costs[k] = -output[target];
                    else
                        costs[k] = neg_log_pi(output, target);
                }
            } 
            else if(cost_funcs[k]=="class_error")
                costs[k] = classification_loss(output, target);
            else if(cost_funcs[k]=="binary_class_error")
                costs[k] = binary_classification_loss(output, target);
            else if(cost_funcs[k]=="multiclass_error")
                costs[k] = multiclass_loss(output, target);
            else if(cost_funcs[k]=="cross_entropy")
                costs[k] = cross_entropy(output, target);
            else if (cost_funcs[k]=="stable_cross_entropy") {
                Var c = stable_cross_entropy(before_transfer_func, target);
                costs[k] = c;
                if (classification_regularizer) {
                    // There is a regularizer to add to the cost function.
                    dynamic_cast<NegCrossEntropySigmoidVariable*>((Variable*) c)->
                        setRegularizer(classification_regularizer);
                }
            }
            else if (cost_funcs[k]=="lift_output")
                costs[k] = lift_output(output, target);
            else  // Assume we got a Variable name and its options
            {
                costs[k]= dynamic_cast<Variable*>(newObject(cost_funcs[k]));
                if(costs[k].isNull())
                    PLERROR("In NNet::build_()  unknown cost_func option: %s",cost_funcs[k].c_str());
                costs[k]->setParents(output & target);
                costs[k]->build();
            }
          
            // take into account the sampleweight
            //if(sampleweight)
            //  costs[k]= costs[k] * sampleweight; // NO, because this is taken into account (more properly) in stats->update
        }

        test_costs = hconcat(costs);

        // Apply penalty to cost.
        // If there is no penalty, we still add costs[0] as the first cost, in
        // order to keep the same number of costs as if there was a penalty.
        Var test_costs_final = test_costs;
        Var first_cost_final = costs[0];
        if (penalties.size() != 0) {
            first_cost_final = sum(hconcat(first_cost_final & penalties));
        }
        if (weightsize_ > 0) {
            test_costs_final = sampleweight * test_costs;
            first_cost_final = sampleweight * first_cost_final;
        }
        // We add the SNE cost.
        // TODO Make sure we optimize the training cost.
        // TODO Actually maybe we should put this before multiplying by sampleweight.
        first_cost_final = first_cost_final + minus_weight_sum_p_ij_log_q_ij;
      
        training_cost = hconcat(first_cost_final & test_costs_final);

/*      if(penalties.size() != 0) {
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
        } */

        training_cost->setName("training_cost");
        test_costs->setName("test_costs");

        if (weightsize_ > 0) {
            invars = bag_inputs & bag_size & target & sampleweight;
        } else {
            invars = bag_inputs & bag_size & target;
        }
        invars_to_training_cost = Func(invars, training_cost);

        invars_to_training_cost->recomputeParents();

        // Other funcs.
        VarArray outvars;
        VarArray testinvars;
        testinvars.push_back(input);
        outvars.push_back(output);
        testinvars.push_back(target);
        outvars.push_back(target);

        test_costf = Func(testinvars, output&test_costs);
        test_costf->recomputeParents();
        output_and_target_to_cost = Func(outvars, test_costs);
        output_and_target_to_cost->recomputeParents();

    }
}

////////////////
// outputsize //
////////////////
int NeighborhoodSmoothnessNNet::outputsize() const
{ return noutputs; }

///////////////////////
// getTrainCostNames //
///////////////////////
TVec<string> NeighborhoodSmoothnessNNet::getTrainCostNames() const
{
    return (cost_funcs[0]+"+penalty+SNE") & cost_funcs;
}

//////////////////////
// getTestCostNames //
//////////////////////
TVec<string> NeighborhoodSmoothnessNNet::getTestCostNames() const
{ 
    return cost_funcs;
}

void NeighborhoodSmoothnessNNet::setTrainingSet(VMat training_set, bool call_forget)
{ 
    // YB: je ne suis pas sur qu'il soit necessaire de faire un build si la LONGUEUR du train_set a change? 
    // les methodes non-parametriques qui utilisent la longueur devrait faire leur "resize" dans train, pas dans build.
    bool training_set_has_changed =
        !train_set
        || train_set->width()      != training_set->width()
        || train_set->length()     != training_set->length()
        || train_set->inputsize()  != training_set->inputsize()
        || train_set->weightsize() != training_set->weightsize()
        || train_set->targetsize() != training_set->targetsize();
    train_set = training_set;

    if (training_set_has_changed && inputsize_<0)
    {
        inputsize_ = train_set->inputsize()-1;
        targetsize_ = train_set->targetsize();
        weightsize_ = train_set->weightsize();
    } else if (train_set->inputsize() != training_set->inputsize()) {
        PLERROR("In NeighborhoodSmoothnessNNet::setTrainingSet - You can't change the inputsize of the training set");
    }
    if (training_set_has_changed || call_forget)
        build(); // MODIF FAITE PAR YOSHUA: sinon apres un setTrainingSet le build n'est pas complete dans un NNet train_set = training_set;
    if (call_forget)
        forget();
}

///////////
// train //
///////////
void NeighborhoodSmoothnessNNet::train()
{
    // NeighborhoodSmoothnessNNet nstages is number of epochs (whole passages through the training set)
    // while optimizer nstages is number of weight updates.
    // So relationship between the 2 depends whether we are in stochastic, batch or minibatch mode

    if(!train_set)
        PLERROR("In NeighborhoodSmoothnessNNet::train, you did not setTrainingSet");
    
    if(!train_stats)
        PLERROR("In NeighborhoodSmoothnessNNet::train, you did not setTrainStatsCollector");

    if(f.isNull()) // Net has not been properly built yet (because build was called before the learner had a proper training set)
        build();

    int n_bags = -1;
    // We must count the nb of bags in the training set.
    {
        n_bags=0;
        int l = train_set->length();
        PP<ProgressBar> pb;
        if(report_progress)
            pb = new ProgressBar("Counting nb bags in train_set for NeighborhoodSmoothnessNNet", l);
        Vec row(train_set->width());
        int tag_column = train_set->inputsize() + train_set->targetsize() - 1;
        for (int i=0;i<l;i++) {
            train_set->getRow(i,row);
            if (int(row[tag_column]) & SumOverBagsVariable::TARGET_COLUMN_FIRST) {
                // Indicates the beginning of a new bag.
                n_bags++;
            }
            if(pb)
                pb->update(i);
        }
    }

    int true_batch_size = batch_size;
    if (true_batch_size <= 0) {
        // The real batch size is actually the number of bags in the training set.
        true_batch_size = n_bags;
    }

    // We can now compute the total cost.
    Var totalcost = sumOverBags(train_set, invars_to_training_cost, max_n_instances, true_batch_size, true);

    // Number of optimizer stages corresponding to one learner stage (one epoch).
    int optstage_per_lstage = 0;
    if (batch_size<=0) {
        optstage_per_lstage = 1;
    } else {
        optstage_per_lstage = n_bags/batch_size;
    }

    if(optimizer) {
        optimizer->setToOptimize(params, totalcost);  
        optimizer->build();
    }

    PP<ProgressBar> pb;
    if(report_progress)
        pb = new ProgressBar("Training NeighborhoodSmoothnessNNet from stage " + tostring(stage) + " to " + tostring(nstages), nstages-stage);

    int initial_stage = stage;
    bool early_stop=false;
    while(stage<nstages && !early_stop)
    {
        optimizer->nstages = optstage_per_lstage;
        train_stats->forget();
        optimizer->early_stop = false;
        optimizer->optimizeN(*train_stats);
        train_stats->finalize();
        if(verbosity>2)
            cout << "Epoch " << stage << " train objective: " << train_stats->getMean() << endl;
        ++stage;
        if(pb)
            pb->update(stage-initial_stage);
    }
    if(verbosity>1)
        cout << "EPOCH " << stage << " train objective: " << train_stats->getMean() << endl;

    // TODO Not sure if this is needed, but just in case...
    output_and_target_to_cost->recomputeParents();
    test_costf->recomputeParents();

}

///////////////////
// computeOutput //
///////////////////
void NeighborhoodSmoothnessNNet::computeOutput(
    const Vec& inputv, Vec& outputv) const
{
    f->fprop(inputv,outputv);
}

///////////////////////////
// computeOutputAndCosts //
///////////////////////////
void NeighborhoodSmoothnessNNet::computeOutputAndCosts(
    const Vec& inputv, const Vec& targetv, Vec& outputv, Vec& costsv) const
{
    test_costf->fprop(inputv&targetv, outputv&costsv);
}

/////////////////////////////
// computeCostsFromOutputs //
/////////////////////////////
void NeighborhoodSmoothnessNNet::computeCostsFromOutputs(
    const Vec& inputv, const Vec& outputv, const Vec& targetv, Vec& costsv) const
{
    output_and_target_to_cost->fprop(outputv&targetv, costsv); 
}

//////////////////////
// initializeParams //
//////////////////////
void NeighborhoodSmoothnessNNet::initializeParams()
{
    if (seed_>=0)
        manual_seed(seed_);
    else
        PLearn::seed();

    real delta = 1. / inputsize();

    /*
      if(direct_in_to_out)
      {
      //fill_random_uniform(wdirect->value, -delta, +delta);
      fill_random_normal(wdirect->value, 0, delta);
      //wdirect->matValue(0).clear();
      }
    */
    if(nhidden>0)
    {
        //fill_random_uniform(w1->value, -delta, +delta);
        //delta = 1./sqrt(nhidden);
        fill_random_normal(w1->value, 0, delta);
        if(direct_in_to_out)
        {
            //fill_random_uniform(wdirect->value, -delta, +delta);
            fill_random_normal(wdirect->value, 0, 0.01*delta);
            wdirect->matValue(0).clear();
        }
        delta = 1./nhidden;
        w1->matValue(0).clear();
    }
    if(nhidden2>0)
    {
        //fill_random_uniform(w2->value, -delta, +delta);
        //delta = 1./sqrt(nhidden2);
        fill_random_normal(w2->value, 0, delta);
        delta = 1./nhidden2;
        w2->matValue(0).clear();
    }
    //fill_random_uniform(wout->value, -delta, +delta);
    fill_random_normal(wout->value, 0, delta);
    wout->matValue(0).clear();

    // Reset optimizer
    if(optimizer)
        optimizer->reset();
}

////////////
// forget //
////////////
void NeighborhoodSmoothnessNNet::forget()
{
    if (train_set) initializeParams();
    stage = 0;
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void NeighborhoodSmoothnessNNet::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(input, copies);
    deepCopyField(target, copies);
    deepCopyField(sampleweight, copies);
    deepCopyField(w1, copies);
    deepCopyField(w2, copies);
    deepCopyField(wout, copies);
    deepCopyField(wdirect, copies);
    deepCopyField(last_hidden, copies);
    deepCopyField(output, copies);
    deepCopyField(bag_size, copies);
    deepCopyField(bag_inputs, copies);
    deepCopyField(bag_output, copies);
    deepCopyField(bag_hidden, copies);
    deepCopyField(invars_to_training_cost, copies);

    deepCopyField(costs, copies);
    deepCopyField(penalties, copies);
    deepCopyField(training_cost, copies);
    deepCopyField(test_costs, copies);
    deepCopyField(invars, copies);
    deepCopyField(params, copies);
    deepCopyField(paramsvalues, copies);

    deepCopyField(p_ij, copies);

    deepCopyField(f, copies);
    deepCopyField(f_input_to_hidden, copies);
    deepCopyField(test_costf, copies);
    deepCopyField(output_and_target_to_cost, copies);
  
    deepCopyField(cost_funcs, copies);

    deepCopyField(optimizer, copies);
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
