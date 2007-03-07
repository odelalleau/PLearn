// -*- C++ -*-

// NNet.cc
// Copyright (c) 1998-2002 Pascal Vincent
// Copyright (C) 1999-2005 Yoshua Bengio and University of Montreal
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

/*! \file PLearnLibrary/PLearnAlgo/NNet.h */

#include <plearn/var/AffineTransformVariable.h>
#include <plearn/var/AffineTransformWeightPenalty.h>
#include <plearn/var/BinaryClassificationLossVariable.h>
#include <plearn/var/ClassificationLossVariable.h>
#include <plearn/var/ConcatColumnsVariable.h>
#include <plearn/var/CrossEntropyVariable.h>
#include <plearn/var/ExpVariable.h>
#include <plearn/var/LiftOutputVariable.h>
#include <plearn/var/LogSoftmaxVariable.h>
#include <plearn/var/MarginPerceptronCostVariable.h>
#include <plearn/var/ConfRatedAdaboostCostVariable.h>
#include <plearn/var/GradientAdaboostCostVariable.h>
#include <plearn/var/MulticlassLossVariable.h>
#include <plearn/var/NegCrossEntropySigmoidVariable.h>
#include <plearn/var/NegLogPoissonVariable.h>
#include <plearn/var/OneHotSquaredLoss.h>
// #include "RBFLayerVariable.h" //TODO Put it back when the file exists.
#include <plearn/var/SigmoidVariable.h>
#include <plearn/var/SoftmaxVariable.h>
#include <plearn/var/SoftplusVariable.h>
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
#include <plearn/var/FNetLayerVariable.h>

#include <plearn/vmat/ConcatColumnsVMatrix.h>
//#include <plearn/display/DisplayUtils.h>
//#include "GradientOptimizer.h"
#include "NNet.h"
// #include <plearn/math/random.h>
#include <plearn/vmat/SubVMatrix.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(NNet, "Ordinary Feedforward Neural Network with 1 or 2 hidden layers", 
                        "Neural network with many bells and whistles...");

//////////
// NNet //
//////////
NNet::NNet()
    :
nhidden(0),
nhidden2(0),
noutputs(0),
weight_decay(0),
bias_decay(0),
layer1_weight_decay(0),
layer1_bias_decay(0),
layer2_weight_decay(0),
layer2_bias_decay(0),
output_layer_weight_decay(0),
output_layer_bias_decay(0),
direct_in_to_out_weight_decay(0),
classification_regularizer(0),
margin(1),
fixed_output_weights(0),
rbf_layer_size(0),
first_class_is_junk(1),
penalty_type("L2_square"),
L1_penalty(false),
input_reconstruction_penalty(0),
direct_in_to_out(false),
output_transfer_func(""),
hidden_transfer_func("tanh"),
interval_minval(0), interval_maxval(1),
do_not_change_params(false),
first_hidden_layer_is_output(false),
transpose_first_hidden_layer(false),
n_non_params_in_first_hidden_layer(0),
batch_size(1),
initialization_method("uniform_linear")
{
    // Use the generic PLearner random number generator.
    random_gen = new PRandom();
}

void NNet::declareOptions(OptionList& ol)
{
    declareOption(
        ol, "nhidden", &NNet::nhidden, OptionBase::buildoption, 
        "Number of hidden units in first hidden layer (0 means no hidden layer)\n");

    declareOption(
        ol, "nhidden2", &NNet::nhidden2, OptionBase::buildoption, 
        "Number of hidden units in second hidden layer (0 means no hidden layer)\n");

    declareOption(
        ol, "noutputs", &NNet::noutputs, OptionBase::buildoption, 
        "Number of output units. This gives this learner its outputsize.  It is\n"
        "typically of the same dimensionality as the target for regression\n"
        "problems.  But for classification problems where target is just the\n"
        "class number, noutputs is usually of dimensionality number of classes\n"
        "(as we want to output a score or probability vector, one per class).\n"
        "\n"
        "The default value is 0, which is caught at build-time and gives an\n"
        "error.  If a value of -1 is put, noutputs is set from the targetsize of\n"
        "the trainingset the first time setTrainingSet() is called on the\n"
        "learner (appropriate for regression scenarios).  This allows using the\n"
        "learner as a 'template' without knowing in advance the number of\n"
        "outputs it should have to handle.  Future extensions will cover the\n"
        "case of automatically discovering the outputsize for classification.\n");

    declareOption(
        ol, "weight_decay", &NNet::weight_decay, OptionBase::buildoption, 
        "Global weight decay for all layers\n");

    declareOption(
        ol, "bias_decay", &NNet::bias_decay, OptionBase::buildoption, 
        "Global bias decay for all layers\n");

    declareOption(
        ol, "layer1_weight_decay", &NNet::layer1_weight_decay, OptionBase::buildoption, 
        "Additional weight decay for the first hidden layer.  Is added to weight_decay.\n");

    declareOption(
        ol, "layer1_bias_decay", &NNet::layer1_bias_decay, OptionBase::buildoption, 
        "Additional bias decay for the first hidden layer.  Is added to bias_decay.\n");

    declareOption(
        ol, "layer2_weight_decay", &NNet::layer2_weight_decay, OptionBase::buildoption, 
        "Additional weight decay for the second hidden layer.  Is added to weight_decay.\n");

    declareOption(
        ol, "layer2_bias_decay", &NNet::layer2_bias_decay, OptionBase::buildoption, 
        "Additional bias decay for the second hidden layer.  Is added to bias_decay.\n");

    declareOption(
        ol, "output_layer_weight_decay", &NNet::output_layer_weight_decay, OptionBase::buildoption, 
        "Additional weight decay for the output layer.  Is added to 'weight_decay'.\n");

    declareOption(
        ol, "output_layer_bias_decay", &NNet::output_layer_bias_decay, OptionBase::buildoption, 
        "Additional bias decay for the output layer.  Is added to 'bias_decay'.\n");

    declareOption(
        ol, "direct_in_to_out_weight_decay", &NNet::direct_in_to_out_weight_decay, OptionBase::buildoption, 
        "Additional weight decay for the direct in-to-out layer.  Is added to 'weight_decay'.\n");

    declareOption(
        ol, "penalty_type", &NNet::penalty_type,
        OptionBase::buildoption,
        "Penalty to use on the weights (for weight and bias decay).\n"
        "Can be any of:\n"
        "  - \"L1\": L1 norm,\n"
        "  - \"L1_square\": square of the L1 norm,\n"
        "  - \"L2_square\" (default): square of the L2 norm.\n");

    declareOption(
        ol, "L1_penalty", &NNet::L1_penalty, OptionBase::buildoption,
        "Deprecated - You should use \"penalty_type\" instead\n"
        "should we use L1 penalty instead of the default L2 penalty on the weights?\n");

    declareOption(
        ol, "fixed_output_weights", &NNet::fixed_output_weights, OptionBase::buildoption, 
        "If true then the output weights are not learned. They are initialized to +1 or -1 randomly.\n");

    declareOption(
        ol, "input_reconstruction_penalty", &NNet::input_reconstruction_penalty, OptionBase::buildoption,
        "If >0 then a set of weights will be added from a hidden layer to predict (reconstruct) the inputs\n"
        "and the total loss will include an extra term that is the squared input reconstruction error,\n"
        "multiplied by the input_reconstruction_penalty factor.\n");

    declareOption(
        ol, "direct_in_to_out", &NNet::direct_in_to_out, OptionBase::buildoption, 
        "should we include direct input to output connections?\n");

    declareOption(
        ol, "rbf_layer_size", &NNet::rbf_layer_size, OptionBase::buildoption,
        "If non-zero, add an extra layer which computes N(h(x);mu_i,sigma_i) (Gaussian density) for the\n"
        "i-th output unit with mu_i a free vector and sigma_i a free scalar, and h(x) the vector of\n"
        "activations of the 'representation' output, i.e. what would be the output layer otherwise. The\n"
        "given non-zero value is the number of these 'representation' outputs. Typically this\n"
        "makes sense for classification problems, with a softmax output_transfer_func. If the\n"
        "first_class_is_junk option is set then the first output (first class) does not get a\n"
        "Gaussian density but just a 'pseudo-uniform' density (the single free parameter is the\n"
        "value of that density) and in a softmax it makes sure that when h(x) is far from the\n"
        "centers mu_i for all the other classes then the last class gets the strongest posterior probability.\n");

    declareOption(
        ol, "first_class_is_junk", &NNet::first_class_is_junk, OptionBase::buildoption, 
        "This option is used only when rbf_layer_size>0. If true then the first class is\n"
        "treated differently and gets a pre-transfer-function value that is a learned constant, whereas\n"
        "the others get a normal centered at mu_i.\n");

    declareOption(
        ol, "output_transfer_func", &NNet::output_transfer_func, OptionBase::buildoption, 
        "what transfer function to use for ouput layer? One of: \n"
        "  - \"tanh\" \n"
        "  - \"sigmoid\" \n"
        "  - \"exp\" \n"
        "  - \"softplus\" \n"
        "  - \"softmax\" \n"
        "  - \"log_softmax\" \n"
        "  - \"interval(<minval>,<maxval>)\", which stands for\n"
        "          <minval>+(<maxval>-<minval>)*sigmoid(.).\n"
        "An empty string or \"none\" means no output transfer function \n");

    declareOption(
        ol, "hidden_transfer_func", &NNet::hidden_transfer_func, OptionBase::buildoption, 
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

    declareOption(
        ol, "cost_funcs", &NNet::cost_funcs, OptionBase::buildoption, 
        "A list of cost functions to use\n"
        "in the form \"[ cf1; cf2; cf3; ... ]\" where each function is one of: \n"
        "  - \"mse\" (for regression)\n"
        "  - \"mse_onehot\" (for classification)\n"
        "  - \"NLL\" (negative log likelihood -log(p[c]) for classification) \n"
        "  - \"class_error\" (classification error) \n"
        "  - \"binary_class_error\" (classification error for a 0-1 binary classifier)\n"
        "  - \"multiclass_error\" \n"
        "  - \"cross_entropy\" (for binary classification)\n" 
        "  - \"stable_cross_entropy\" (more accurate backprop and possible regularization, for binary classification)\n"
        "  - \"margin_perceptron_cost\" (a hard version of the cross_entropy, uses the 'margin' option)\n"
        "  - \"lift_output\" (not a real cost function, just the output for lift computation)\n"
        "  - \"conf_rated_adaboost_cost\" (for confidence rated Adaboost)\n"
        "  - \"gradient_adaboost_cost\" (also for confidence rated Adaboost)\n"
        "  - \"poisson_nll\"\n"
        "The FIRST function of the list will be used as \n"
        "the objective function to optimize \n"
        "(possibly with an added weight decay penalty) \n");
  
    declareOption(
        ol, "classification_regularizer", &NNet::classification_regularizer, OptionBase::buildoption, 
        "Used only in the stable_cross_entropy cost function, to fight overfitting (0<=r<1)\n");

    declareOption(
        ol, "first_hidden_layer", &NNet::first_hidden_layer, OptionBase::buildoption, 
        "A user-specified NAry Var that computes the output of the first hidden layer\n"
        "from the network input vector and a set of parameters. Its first argument should\n"
        "be the network input and the remaining arguments the tunable parameters.\n");

    declareOption(
        ol, "first_hidden_layer_is_output",
        &NNet::first_hidden_layer_is_output, OptionBase::buildoption,
        "If true and a 'first_hidden_layer' Var is provided, then this layer\n"
        "will be considered as the NNet output before transfer function.");

    declareOption(
        ol, "n_non_params_in_first_hidden_layer",
        &NNet::n_non_params_in_first_hidden_layer,
        OptionBase::buildoption, 
        "Number of elements in the 'varray' option of 'first_hidden_layer'\n"
        "that are not updated parameters (assumed to be the last elements in\n"
        "'varray').");

    declareOption(
        ol, "transpose_first_hidden_layer",
        &NNet::transpose_first_hidden_layer,
        OptionBase::buildoption, 
        "If true and the 'first_hidden_layer' option is set, this layer will\n"
        "be transposed, and the input variable given to this layer will also\n"
        "be transposed.");

    declareOption(
        ol, "margin", &NNet::margin, OptionBase::buildoption, 
        "Margin requirement, used only with the margin_perceptron_cost cost function.\n"
        "It should be positive, and larger values regularize more.\n");

    declareOption(
        ol, "do_not_change_params", &NNet::do_not_change_params, OptionBase::buildoption, 
        "If set to 1, the weights won't be loaded nor initialized at build time.");

    declareOption(
        ol, "optimizer", &NNet::optimizer, OptionBase::buildoption, 
        "Specify the optimizer to use\n");

    declareOption(
        ol, "batch_size", &NNet::batch_size, OptionBase::buildoption, 
        "How many samples to use to estimate the avergage gradient before updating the weights\n"
        "0 is equivalent to specifying training_set->length() \n");

    declareOption(
        ol, "initialization_method", &NNet::initialization_method, OptionBase::buildoption, 
        "The method used to initialize the weights:\n"
        " - \"normal_linear\"  = a normal law with variance 1/n_inputs\n"
        " - \"normal_sqrt\"    = a normal law with variance 1/sqrt(n_inputs)\n"
        " - \"uniform_linear\" = a uniform law in [-1/n_inputs, 1/n_inputs]\n"
        " - \"uniform_sqrt\"   = a uniform law in [-1/sqrt(n_inputs), 1/sqrt(n_inputs)]\n"
        " - \"zero\"           = all weights are set to 0\n");

    declareOption(
        ol, "paramsvalues", &NNet::paramsvalues, OptionBase::learntoption, 
        "The learned parameter vector\n");

    inherited::declareOptions(ol);

}

///////////
// build //
///////////
void NNet::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void NNet::build_()
{
    /*
     * Create Topology Var Graph
     */

    // Don't do anything if we don't have a train_set
    // It's the only one who knows the inputsize and targetsize anyway...

    if(inputsize_>=0 && targetsize_>=0 && weightsize_>=0)
    {
        // Ensure we have some inputs
        if (noutputs == 0)
            PLERROR("NNet: the option 'noutputs' must be specified");

        // Initialize the input.
        input = Var(inputsize(), "input");

        params.resize(0);
        Var before_transfer_func;

        // Build main network graph.
        buildOutputFromInput(input, hidden_layer, before_transfer_func);

        // Build target and weight variables.
        buildTargetAndWeight();

        // Build costs.
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

        buildCosts(output, target, hidden_layer, before_transfer_func);

        // Shared values hack...
        if (!do_not_change_params) {
            if(paramsvalues.length() == params.nelems())
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

    }
}


////////////////////
// setTrainingSet //
////////////////////

void NNet::setTrainingSet(VMat training_set, bool call_forget)
{
    PLASSERT( training_set );
    
    // Automatically set noutputs from targetsize if not already set
    if (noutputs < 0)
        noutputs = training_set->targetsize();

    inherited::setTrainingSet(training_set, call_forget);
    //cout << "name = " << name << endl << "targetsize = " << targetsize_ << endl << "weightsize = " << weightsize_ << endl;
}


////////////////
// buildCosts //
////////////////
void NNet::buildCosts(const Var& the_output, const Var& the_target, const Var& hidden_layer, const Var& before_transfer_func) {
    int ncosts = cost_funcs.size();  
    if(ncosts<=0)
        PLERROR("In NNet::buildCosts - Empty cost_funcs : must at least specify the cost function to optimize!");
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
                costs[k] = stable_cross_entropy(before_transfer_func, the_target);
            } else {
                if (output_transfer_func == "log_softmax")
                    costs[k] = -the_output[the_target];
                else
                    costs[k] = neg_log_pi(the_output, the_target);
            }
        } 
        else if(cost_funcs[k]=="class_error")
        {
            if (the_output->size()==1)
                costs[k] = binary_classification_loss(the_output, the_target);
            else
                costs[k] = classification_loss(the_output, the_target);
        }
        else if(cost_funcs[k]=="binary_class_error")
            costs[k] = binary_classification_loss(the_output, the_target);
        else if(cost_funcs[k]=="multiclass_error")
            costs[k] = multiclass_loss(the_output, the_target);
        else if(cost_funcs[k]=="cross_entropy")
            costs[k] = cross_entropy(the_output, the_target);
//        else if(cost_funcs[k]=="scaled_cross_entropy") {
//            costs[k] = cross_entropy(the_output, the_target, true);
//        } 
        else if(cost_funcs[k]=="conf_rated_adaboost_cost")
        {
            if(output_transfer_func != "sigmoid")
                PLWARNING("In NNet:buildCosts(): conf_rated_adaboost_cost expects an output in (0,1)");
            alpha_adaboost = Var(1,1); alpha_adaboost->value[0] = 1.0;
            params.append(alpha_adaboost);
            costs[k] = conf_rated_adaboost_cost(the_output, the_target, alpha_adaboost);
        }
        else if(cost_funcs[k]=="gradient_adaboost_cost")
        {
            if(output_transfer_func != "sigmoid")
                PLWARNING("In NNet:buildCosts(): gradient_adaboost_cost expects an output in (0,1)");
            costs[k] = gradient_adaboost_cost(the_output, the_target);
        }
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
        else if (cost_funcs[k]=="poisson_nll") {
            VarArray the_varray(the_output, the_target);
            if (weightsize()>0)
                the_varray.push_back(sampleweight);
            costs[k] = neglogpoissonvariable(the_varray);
        }        
        else  // Assume we got a Variable name and its options
        {
            costs[k]= dynamic_cast<Variable*>(newObject(cost_funcs[k]));
            if(costs[k].isNull())
                PLERROR("In NNet::build_()  unknown cost_func option: %s",cost_funcs[k].c_str());
            costs[k]->setParents(the_output & the_target);
            costs[k]->build();
        }

        // take into account the sampleweight
        //if(sampleweight)
        //  costs[k]= costs[k] * sampleweight; // NO, because this is taken into account (more properly) in stats->update
    }


    /*
     * weight and bias decay penalty
     */

    // create penalties
    buildPenalties(hidden_layer);
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
void NNet::buildFuncs(const Var& the_input, const Var& the_output, const Var& the_target, const Var& the_sampleweight) {
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
    f = Func(the_input, the_output);
    test_costf = Func(testinvars, the_output&test_costs);
    test_costf->recomputeParents();
    output_and_target_to_cost = Func(outvars, test_costs); 
    // Since there will be a fprop() in the network, we need to make sure the
    // input is valid.
    if (train_set && train_set->length() >= 1) {
        Vec input, target;
        real weight;
        train_set->getExample(0, input, target, weight);
        the_input->matValue << input;
    }
    output_and_target_to_cost->recomputeParents();
}

//////////////////////////
// buildOutputFromInput //
//////////////////////////
void NNet::buildOutputFromInput(const Var& the_input, Var& hidden_layer, Var& before_transfer_func) {
    output = the_input;

    // First hidden layer.

    if (first_hidden_layer)
    {
        NaryVariable* layer_var = dynamic_cast<NaryVariable*>((Variable*)first_hidden_layer);
        if (!layer_var)
            PLERROR("In NNet::buildOutputFromInput - 'first_hidden_layer' should be "
                    "from a subclass of NaryVariable");
        if (layer_var->varray.size() < 1)
            layer_var->varray.resize(1);
        layer_var->varray[0] =
            transpose_first_hidden_layer ? transpose(output)
                                         : output; // Here output = NNet input.
        layer_var->build(); // make sure everything is consistent and finish the build
        if (layer_var->varray.size()<2)
            PLERROR("In NNet::buildOutputFromInput - 'first_hidden_layer' should have parameters");
        int index_max_param =
            layer_var->varray.length() - n_non_params_in_first_hidden_layer;
        for (int i = 1; i < index_max_param; i++)
            params.append(layer_var->varray[i]);
        hidden_layer = transpose_first_hidden_layer ? transpose(layer_var)
                                                    : layer_var;
        output = hidden_layer;
    }
    else if(nhidden>0)
    {
        w1 = Var(1 + the_input->size(), nhidden, "w1");      
        params.append(w1);
        hidden_layer = hiddenLayer(output, w1);
        output = hidden_layer;
        // TODO BEWARE! This is not the same 'hidden_layer' as before.
    }

    // second hidden layer
    if(nhidden2>0)
    {
        PLASSERT( !first_hidden_layer_is_output );
        w2 = Var(1 + output.length(), nhidden2, "w2");
        params.append(w2);
        output = hiddenLayer(output, w2);
    }

    if (nhidden2>0 && nhidden==0 && !first_hidden_layer)
        PLERROR("NNet:: can't have nhidden2 (=%d) > 0 while nhidden=0",nhidden2);

    if (rbf_layer_size>0)
    {
        if (first_class_is_junk)
        {
            rbf_centers = Var(outputsize()-1, rbf_layer_size, "rbf_centers");
            rbf_sigmas = Var(outputsize()-1, "rbf_sigmas");
            PLERROR("In NNet.cc, the code needs to be completed, rbf_layer isn't declared and thus it doesn't compile with the line below");
            // TODO (Also put back the corresponding include).
            //          output = hconcat(rbf_layer(output,rbf_centers,rbf_sigmas)&junk_prob);
            params.append(junk_prob);
        }
        else
        {
            rbf_centers = Var(outputsize(), rbf_layer_size, "rbf_centers");
            rbf_sigmas = Var(outputsize(), "rbf_sigmas");
            PLERROR("In NNet.cc, the code needs to be completed, rbf_layer isn't declared and thus it doesn't compile with the line below");
            //          output = rbf_layer(output,rbf_centers,rbf_sigmas);
        }
        params.append(rbf_centers);
        params.append(rbf_sigmas);
    }

    // Output layer before transfer function.
    if (!first_hidden_layer_is_output) {
        wout = Var(1 + output->size(), outputsize(), "wout");
        output = affine_transform(output, wout);
        if (!fixed_output_weights)
            params.append(wout);
        else
        {
            outbias = Var(output->size(), "outbias");
            output = output + outbias;
            params.append(outbias);
        }
    } else {
        // Verify we have provided a 'first_hidden_layer' Variable: even though
        // one might want to use this option without such a Var, it would be
        // simpler in this case to just set 'nhidden' to 0.
        if (!first_hidden_layer)
            PLERROR("In NNet::buildOutputFromInput - The option "
                    "'first_hidden_layer_is_output' can only be used in "
                    "conjunction with a 'first_hidden_layer' Variable");
    }

    // Direct in-to-out layer.
    if(direct_in_to_out)
    {
        wdirect = Var(the_input->size(), outputsize(), "wdirect");
        output += transposeProduct(wdirect, the_input);
        params.append(wdirect);
        if (nhidden <= 0)
            PLERROR("In NNet::buildOutputFromInput - It seems weird to use direct in-to-out connections if there is no hidden layer anyway");
    }

    before_transfer_func = output;

    /*
     * output_transfer_func
     */
    size_t p=0;
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
            size_t q = output_transfer_func.find(",");
            interval_minval = atof(output_transfer_func.substr(p+1,q-(p+1)).c_str());
            size_t r = output_transfer_func.find(")");
            interval_maxval = atof(output_transfer_func.substr(q+1,r-(q+1)).c_str());
            output = interval_minval + (interval_maxval - interval_minval)*sigmoid(output);
        }
        else
            PLERROR("In NNet::build_()  unknown output_transfer_func option: %s",output_transfer_func.c_str());
    }
}

////////////////////
// buildPenalties //
////////////////////
void NNet::buildPenalties(const Var& hidden_layer) {
    penalties.resize(0);  // prevents penalties from being added twice by consecutive builds
    if(w1 && (!fast_exact_is_equal(layer1_weight_decay + weight_decay, 0) ||
              !fast_exact_is_equal(layer1_bias_decay + bias_decay,     0)))
        penalties.append(affine_transform_weight_penalty(w1, (layer1_weight_decay + weight_decay), (layer1_bias_decay + bias_decay), penalty_type));
    if(w2 && (!fast_exact_is_equal(layer2_weight_decay + weight_decay, 0) ||
              !fast_exact_is_equal(layer2_bias_decay + bias_decay,     0)))
        penalties.append(affine_transform_weight_penalty(w2, (layer2_weight_decay + weight_decay), (layer2_bias_decay + bias_decay), penalty_type));
    if(wout && (!fast_exact_is_equal(output_layer_weight_decay + weight_decay, 0) ||
                !fast_exact_is_equal(output_layer_bias_decay + bias_decay,     0)))
        penalties.append(affine_transform_weight_penalty(wout, (output_layer_weight_decay + weight_decay), 
                                                         (output_layer_bias_decay + bias_decay), penalty_type));
    if(wdirect &&
       !fast_exact_is_equal(direct_in_to_out_weight_decay + weight_decay, 0))
    {
        if (penalty_type == "L1_square")
            penalties.append(square(sumabs(wdirect))*(direct_in_to_out_weight_decay + weight_decay));
        else if (penalty_type == "L1")
            penalties.append(sumabs(wdirect)*(direct_in_to_out_weight_decay + weight_decay));
        else if (penalty_type == "L2_square")
            penalties.append(sumsquare(wdirect)*(direct_in_to_out_weight_decay + weight_decay));
    }
    if (input_reconstruction_penalty>0)
    {
        wrec = Var(1 + hidden_layer->size(),input->size(),"wrec");
        predicted_input = affine_transform(hidden_layer, wrec);
        params.append(wrec);
        penalties.append(input_reconstruction_penalty*sumsquare(predicted_input - input));
    }
}

//////////////////////////
// buildTargetAndWeight //
//////////////////////////
void NNet::buildTargetAndWeight() {
    target = Var(targetsize(), "target");
    if(weightsize_>0)
    {
        if (weightsize_!=1)
            PLERROR("In NNet::buildTargetAndWeight - Expected weightsize to be 1 or 0 (or unspecified = -1, meaning 0), got %d",weightsize_);
        sampleweight = Var(1, "weight");
    }
}

/////////////////////////////
// computeCostsFromOutputs //
/////////////////////////////
void NNet::computeCostsFromOutputs(const Vec& inputv, const Vec& outputv, 
                                   const Vec& targetv, Vec& costsv) const
{
#ifdef BOUNDCHECK
    // Stable cross entropy needs the value *before* the transfer function.
    if (cost_funcs.contains("stable_cross_entropy") or
       (cost_funcs.contains("NLL") and outputsize() == 1))
        PLERROR("In NNet::computeCostsFromOutputs - Cannot directly compute stable "
                "cross entropy from output and target");
#endif
    output_and_target_to_cost->fprop(outputv&targetv, costsv); 
}

///////////////////
// computeOutput //
///////////////////
void NNet::computeOutput(const Vec& inputv, Vec& outputv) const
{
    outputv.resize(outputsize());
    f->fprop(inputv,outputv);
}

///////////////////////////
// computeOutputAndCosts //
///////////////////////////
void NNet::computeOutputAndCosts(const Vec& inputv, const Vec& targetv, 
                                 Vec& outputv, Vec& costsv) const
{
    outputv.resize(outputsize());
    test_costf->fprop(inputv&targetv, outputv&costsv);
}

/////////////////
// fillWeights //
/////////////////
void NNet::fillWeights(const Var& weights, bool clear_first_row) {
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
        random_gen->fill_random_normal(weights->value, 0, delta);
    else
        random_gen->fill_random_uniform(weights->value, -delta, delta);
    if (clear_first_row)
        weights->matValue(0).clear();
}

////////////
// forget //
////////////
void NNet::forget()
{
    inherited::forget();
    if (train_set) initializeParams();
    if(optimizer)
        optimizer->reset();
    stage = 0;
}

///////////////////////
// getTrainCostNames //
///////////////////////
TVec<string> NNet::getTrainCostNames() const
{
    PLASSERT( !cost_funcs.isEmpty() );
    int n_costs = cost_funcs.length();
    TVec<string> train_costs(n_costs + 1);
    train_costs[0] = cost_funcs[0] + "+penalty";
    train_costs.subVec(1, n_costs) << cost_funcs;
    return train_costs;
}

//////////////////////
// getTestCostNames //
//////////////////////
TVec<string> NNet::getTestCostNames() const
{ 
    return cost_funcs;
}

/////////////////
// hiddenLayer //
/////////////////
Var NNet::hiddenLayer(const Var& input, const Var& weights, string transfer_func) {
    Var hidden = affine_transform(input, weights); 
    Var result;
    if (transfer_func == "default")
        transfer_func = hidden_transfer_func;
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
        PLERROR("In NNet::hiddenLayer - Unknown value for transfer_func: %s",transfer_func.c_str());
    return result;
}

//////////////////////
// initializeParams //
//////////////////////
void NNet::initializeParams(bool set_seed)
{
    if (set_seed && seed_ != 0)
        random_gen->manual_seed(seed_);

    if (nhidden>0) {
        if (!first_hidden_layer)
            fillWeights(w1, true);
        if (direct_in_to_out)
            fillWeights(wdirect, false);
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
        random_gen->fill_random_discrete(wout->value, values);
        wout->matValue(0).clear();
    }
    else {
        if (wout)
            fillWeights(wout, true);
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
void NNet::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    varDeepCopyField(input, copies);
    varDeepCopyField(target, copies);
    varDeepCopyField(sampleweight, copies);
    varDeepCopyField(w1, copies);
    varDeepCopyField(w2, copies);
    varDeepCopyField(wout, copies);
    varDeepCopyField(outbias, copies);
    varDeepCopyField(wdirect, copies);
    varDeepCopyField(wrec, copies);
    varDeepCopyField(hidden_layer, copies);
    varDeepCopyField(rbf_centers, copies);
    varDeepCopyField(rbf_sigmas, copies);
    varDeepCopyField(junk_prob, copies);
    varDeepCopyField(alpha_adaboost,copies);
    varDeepCopyField(output, copies);
    varDeepCopyField(predicted_input, copies);
    deepCopyField(costs, copies);
    deepCopyField(penalties, copies);
    varDeepCopyField(training_cost, copies);
    varDeepCopyField(test_costs, copies);
    deepCopyField(invars, copies);
    deepCopyField(params, copies);
    deepCopyField(paramsvalues, copies);
    deepCopyField(f, copies);
    deepCopyField(test_costf, copies);
    deepCopyField(output_and_target_to_cost, copies);
    varDeepCopyField(first_hidden_layer, copies);
    deepCopyField(optimizer, copies);
}

////////////////
// outputsize //
////////////////
int NNet::outputsize() const {
    return noutputs;
}

///////////
// train //
///////////
void NNet::train()
{
    // NNet nstages is number of epochs (whole passages through the training set)
    // while optimizer nstages is number of weight updates.
    // So relationship between the 2 depends whether we are in stochastic, batch or minibatch mode

    if(!train_set)
        PLERROR("In NNet::train, you did not setTrainingSet");
    
    if(!train_stats)
        setTrainStatsCollector(new VecStatsCollector());
    // PLERROR("In NNet::train, you did not setTrainStatsCollector");

    int l = train_set->length();  
    
    if(f.isNull()) // Net has not been properly built yet (because build was called before the learner had a proper training set)
        build();

    // number of samples seen by optimizer before each optimizer update
    int nsamples = batch_size>0 ? batch_size : l;
    Func paramf = Func(invars, training_cost); // parameterized function to optimize
    Var totalcost = meanOf(train_set, paramf, nsamples);
    if(optimizer)
    {
        optimizer->setToOptimize(params, totalcost);  
        optimizer->build();
    }
    else PLERROR("NNet::train can't train without setting an optimizer first!");

    // number of optimizer stages corresponding to one learner stage (one epoch)
    int optstage_per_lstage = l/nsamples;

    PP<ProgressBar> pb;
    if(report_progress)
        pb = new ProgressBar("Training " + classname() + " from stage " + tostring(stage) + " to " + tostring(nstages), nstages-stage);

    int initial_stage = stage;
    bool early_stop=false;
    while(stage<nstages && !early_stop)
    {
        optimizer->nstages = optstage_per_lstage;
        train_stats->forget();
        optimizer->early_stop = false;
        early_stop = optimizer->optimizeN(*train_stats);
        // optimizer->verifyGradient(1e-6); // Uncomment if you want to check your new Var.
        train_stats->finalize();
        if(verbosity>2)
            cout << "Epoch " << stage << " train objective: " << train_stats->getMean() << endl;
        ++stage;
        if(pb)
            pb->update(stage-initial_stage);
    }
    if(verbosity>1)
        cout << "EPOCH " << stage << " train objective: " << train_stats->getMean() << endl;

    output_and_target_to_cost->recomputeParents();
    test_costf->recomputeParents();
    // cerr << "totalcost->value = " << totalcost->value << endl;
    // cout << "Result for benchmark is: " << totalcost->value << endl;
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
