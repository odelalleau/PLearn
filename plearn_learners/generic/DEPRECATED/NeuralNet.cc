// -*- C++ -*-

// NeuralNet.cc
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


#include <plearn/var/AffineTransformVariable.h>
#include <plearn/var/AffineTransformWeightPenalty.h>
#include <plearn/var/BinaryClassificationLossVariable.h>
#include <plearn/var/ClassificationLossVariable.h>
#include <plearn/var/ConcatColumnsVariable.h>
#include <plearn/var/CrossEntropyVariable.h>
#include <plearn/var/ExpVariable.h>
#include <plearn/var/IfThenElseVariable.h>
#include <plearn/var/LiftOutputVariable.h>
#include <plearn/var/LogSoftmaxVariable.h>
#include <plearn/var/MulticlassLossVariable.h>
#include <plearn/var/NegCrossEntropySigmoidVariable.h>
#include <plearn/var/OneHotSquaredLoss.h>
#include <plearn/var/SemiSupervisedProbClassCostVariable.h>
#include <plearn/var/SigmoidVariable.h>
#include <plearn/var/SoftmaxVariable.h>
#include <plearn/var/SoftplusVariable.h>
#include <plearn/var/SourceVariable.h>
#include <plearn/var/SubMatVariable.h>
#include <plearn/var/SumVariable.h>
#include <plearn/var/SumOfVariable.h>
#include <plearn/var/SumSquareVariable.h>
#include <plearn/var/TanhVariable.h>
#include <plearn/var/TransposeProductVariable.h>
#include <plearn/var/Var_operators.h>
#include <plearn/var/Var_utils.h>
#include <plearn/var/WeightedSumSquareVariable.h>

#include "NeuralNet.h"
//#include "DisplayUtils.h"
#include <plearn/math/random.h>
//#include "GradientOptimizer.h"
#include <plearn/var/SemiSupervisedProbClassCostVariable.h>
#include <plearn/var/IsMissingVariable.h>

namespace PLearn {
using namespace std;


PLEARN_IMPLEMENT_OBJECT(NeuralNet, "DEPRECATED: Use NNet instead", "NO HELP");

NeuralNet::NeuralNet()
    :nhidden(0),
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
     direct_in_to_out(false),
     output_transfer_func(""),
     iseed(-1),
     semisupervised_flatten_factor(1),
     batch_size(1),
     nepochs(10000),
     saveparams("")
{}

NeuralNet::~NeuralNet()
{
}

void NeuralNet::declareOptions(OptionList& ol)
{
    declareOption(ol, "nhidden", &NeuralNet::nhidden, OptionBase::buildoption, 
                  "    number of hidden units in first hidden layer (0 means no hidden layer)\n");

    declareOption(ol, "nhidden2", &NeuralNet::nhidden2, OptionBase::buildoption, 
                  "    number of hidden units in second hidden layer (0 means no hidden layer)\n");

    declareOption(ol, "weight_decay", &NeuralNet::weight_decay, OptionBase::buildoption, 
                  "    global weight decay for all layers\n");

    declareOption(ol, "bias_decay", &NeuralNet::bias_decay, OptionBase::buildoption, 
                  "    global bias decay for all layers\n");

    declareOption(ol, "layer1_weight_decay", &NeuralNet::layer1_weight_decay, OptionBase::buildoption, 
                  "    Additional weight decay for the first hidden layer.  Is added to weight_decay.\n");
    declareOption(ol, "layer1_bias_decay", &NeuralNet::layer1_bias_decay, OptionBase::buildoption, 
                  "    Additional bias decay for the first hidden layer.  Is added to bias_decay.\n");

    declareOption(ol, "layer2_weight_decay", &NeuralNet::layer2_weight_decay, OptionBase::buildoption, 
                  "    Additional weight decay for the second hidden layer.  Is added to weight_decay.\n");

    declareOption(ol, "layer2_bias_decay", &NeuralNet::layer2_bias_decay, OptionBase::buildoption, 
                  "    Additional bias decay for the second hidden layer.  Is added to bias_decay.\n");

    declareOption(ol, "output_layer_weight_decay", &NeuralNet::output_layer_weight_decay, OptionBase::buildoption, 
                  "    Additional weight decay for the output layer.  Is added to 'weight_decay'.\n");

    declareOption(ol, "output_layer_bias_decay", &NeuralNet::output_layer_bias_decay, OptionBase::buildoption, 
                  "    Additional bias decay for the output layer.  Is added to 'bias_decay'.\n");

    declareOption(ol, "direct_in_to_out_weight_decay", &NeuralNet::direct_in_to_out_weight_decay, OptionBase::buildoption, 
                  "    Additional weight decay for the direct in-to-out layer.  Is added to 'weight_decay'.\n");

    declareOption(ol, "direct_in_to_out", &NeuralNet::direct_in_to_out, OptionBase::buildoption, 
                  "    should we include direct input to output connections?\n");

    declareOption(ol, "output_transfer_func", &NeuralNet::output_transfer_func, OptionBase::buildoption, 
                  "    what transfer function to use for ouput layer? \n"
                  "    one of: tanh, sigmoid, exp, softmax \n"
                  "    an empty string means no output transfer function \n");

    declareOption(ol, "seed", &NeuralNet::iseed, OptionBase::buildoption, 
                  "    Seed for the random number generator used to initialize parameters. If -1 then use time of day.\n");

    declareOption(ol, "cost_funcs", &NeuralNet::cost_funcs, OptionBase::buildoption, 
                  "    a list of cost functions to use\n"
                  "    in the form \"[ cf1; cf2; cf3; ... ]\" where each function is one of: \n"
                  "      mse (for regression)\n"
                  "      mse_onehot (for classification)\n"
                  "      NLL (negative log likelihood -log(p[c]) for classification) \n"
                  "      class_error (classification error) \n"
                  "      semisupervised_prob_class\n"
                  "    The first function of the list will be used as \n"
                  "    the objective function to optimize \n"
                  "    (possibly with an added weight decay penalty) \n"
                  "    If semisupervised_prob_class is chosen, then the options\n"
                  "    semisupervised_{flatten_factor,prior} will be used. Note that\n"
                  "    the output_transfer_func should be the softmax, in that case.\n"
        );
  
    declareOption(ol, "semisupervised_flatten_factor", &NeuralNet::semisupervised_flatten_factor, OptionBase::buildoption, 
                  "    Hyper-parameter of the semi-supervised criterion for probabilistic classifiers\n");

    declareOption(ol, "semisupervised_prior", &NeuralNet::semisupervised_prior, OptionBase::buildoption, 
                  "    Hyper-parameter of the semi-supervised criterion = prior classes probabilities\n");

    declareOption(ol, "optimizer", &NeuralNet::optimizer, OptionBase::buildoption, 
                  "    specify the optimizer to use\n");

    declareOption(ol, "batch_size", &NeuralNet::batch_size, OptionBase::buildoption, 
                  "    how many samples to use to estimate the avergage gradient before updating the weights\n"
                  "    0 is equivalent to specifying training_set->length() \n"
                  "    NOTE: this overrides the optimizer's 'n_updates' and 'every_iterations'.\n");

    declareOption(ol, "nepochs", &NeuralNet::nepochs, OptionBase::buildoption, 
                  "    how many times the optimizer gets to see the whole training set.\n");

    declareOption(ol, "paramsvalues", &NeuralNet::paramsvalues, OptionBase::learntoption, 
                  "    The learned parameter vector (in which order?)\n");

    declareOption(ol, "saveparams", &NeuralNet::saveparams, OptionBase::learntoption, 
                  "    This string, if not empty, indicates where in the expdir directory\n"
                  "    to save the final paramsvalues\n");
  
    declareOption(ol, "normalization", &NeuralNet::normalization, OptionBase::buildoption,
                  "    The normalization to be applied to the data\n");
    inherited::declareOptions(ol);

}

void NeuralNet::build()
{
    inherited::build();
    build_();
}

void NeuralNet::build_()
{
    /*
     * Create Topology Var Graph
     */

    // init. basic vars
    input = Var(inputsize(), "input");
    if (normalization.length()) {
        Var means(normalization[0]);
        Var stddevs(normalization[1]);
        output = (input - means) / stddevs;
    } else
        output = input;
    params.resize(0);

    // first hidden layer
    if(nhidden>0)
    {
        w1 = Var(1+inputsize(), nhidden, "w1");      
        output = tanh(affine_transform(output,w1));
        params.append(w1);
    }

    // second hidden layer
    if(nhidden2>0)
    {
        w2 = Var(1+nhidden, nhidden2, "w2");
        output = tanh(affine_transform(output,w2));
        params.append(w2);
    }

    // output layer before transfer function
    wout = Var(1+output->size(), outputsize(), "wout");
    output = affine_transform(output,wout);
    params.append(wout);

    // direct in-to-out layer
    if(direct_in_to_out)
    {
        wdirect = Var(inputsize(), outputsize(), "wdirect");// Var(1+inputsize(), outputsize(), "wdirect");
        output += transposeProduct(wdirect, input);// affine_transform(input,wdirect);
        params.append(wdirect);
    }

    /*
     * output_transfer_func
     */
    if(output_transfer_func!="")
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
        else
            PLERROR("In NeuralNet::build_()  unknown output_transfer_func option: %s",output_transfer_func.c_str());
    }

    /*
     * target & weights
     */
    if(weightsize() != 0 && weightsize() != 1 && targetsize()/2 != weightsize())
        PLERROR("In NeuralNet::build_()  weightsize must be either:\n"
                "\t0: no weights on costs\n"
                "\t1: single weight applied on total cost\n"
                "\ttargetsize/2: vector of weights applied individually to each component of the cost\n"
                "weightsize= %d; targetsize= %d.", weightsize(), targetsize());


    target_and_weights= Var(targetsize(), "target_and_weights");
    target = new SubMatVariable(target_and_weights, 0, 0, targetsize()-weightsize(), 1);
    target->setName("target");
    if(0 < weightsize())
    {
        costweights = new SubMatVariable(target_and_weights, targetsize()-weightsize(), 0, weightsize(), 1);
        costweights->setName("costweights");
    }
    /*
     * costfuncs
     */
    int ncosts = cost_funcs.size();  
    if(ncosts<=0)
        PLERROR("In NeuralNet::build_()  Empty cost_funcs : must at least specify the cost function to optimize!");
    costs.resize(ncosts);

    for(int k=0; k<ncosts; k++)
    {
        bool handles_missing_target=false;
        // create costfuncs and apply individual weights if weightsize() > 1
        if(cost_funcs[k]=="mse")
            if(weightsize() < 2)
                costs[k]= sumsquare(output-target);
            else
                costs[k]= weighted_sumsquare(output-target, costweights);
        else if(cost_funcs[k]=="mse_onehot")
            costs[k] = onehot_squared_loss(output, target);
        else if(cost_funcs[k]=="NLL") {
            if (output_transfer_func == "log_softmax")
                costs[k] = -output[target];
            else
                costs[k] = neg_log_pi(output, target);
        } else if(cost_funcs[k]=="class_error")
            costs[k] = classification_loss(output, target);
        else if(cost_funcs[k]=="multiclass_error")
            if(weightsize() < 2)
                costs[k] = multiclass_loss(output, target);
            else
                PLERROR("In NeuralNet::build()  weighted multiclass error cost not implemented.");
        else if(cost_funcs[k]=="cross_entropy")
            if(weightsize() < 2)
                costs[k] = cross_entropy(output, target);
            else
                PLERROR("In NeuralNet::build()  weighted cross entropy cost not implemented.");
        else if (cost_funcs[k]=="semisupervised_prob_class")
        {
            if (output_transfer_func!="softmax")
                PLWARNING("To properly use the semisupervised_prob_class criterion, the transfer function should probably be a softmax, to guarantee positive probabilities summing to 1");
            if (semisupervised_prior.length()==0) // default value is (1,1,1...)
            {
                semisupervised_prior.resize(outputsize());
                semisupervised_prior.fill(1.0);
            }
            costs[k] = new SemiSupervisedProbClassCostVariable(output,target,new SourceVariable(semisupervised_prior),
                                                               semisupervised_flatten_factor);
            handles_missing_target=true;
        }
        else
        {
            costs[k]= dynamic_cast<Variable*>(newObject(cost_funcs[k]));
            if(costs[k].isNull())
                PLERROR("In NeuralNet::build_()  unknown cost_func option: %s",cost_funcs[k].c_str());
            if(weightsize() < 2)
                costs[k]->setParents(output & target);
            else
                costs[k]->setParents(output & target & costweights);
            costs[k]->build();
        }

        // apply a single global weight if weightsize() == 1
        if(1 == weightsize())
            costs[k]= costs[k] * costweights;

        if (!handles_missing_target)
            costs[k] = ifThenElse(isMissing(target),var(MISSING_VALUE),costs[k]);
    }


    /*
     * weight and bias decay penalty
     */

    // create penalties
    VarArray penalties;
    if(w1 && ((layer1_weight_decay + weight_decay)!=0 || (layer1_bias_decay + bias_decay)!=0))
        penalties.append(affine_transform_weight_penalty(w1, (layer1_weight_decay + weight_decay), (layer1_bias_decay + bias_decay)));
    if(w2 && ((layer2_weight_decay + weight_decay)!=0 || (layer2_bias_decay + bias_decay)!=0))
        penalties.append(affine_transform_weight_penalty(w2, (layer2_weight_decay + weight_decay), (layer2_bias_decay + bias_decay)));
    if(wout && ((output_layer_weight_decay + weight_decay)!=0 || (output_layer_bias_decay + bias_decay)!=0))
        penalties.append(affine_transform_weight_penalty(wout, (output_layer_weight_decay + weight_decay), (output_layer_bias_decay + bias_decay)));
    if(wdirect && (direct_in_to_out_weight_decay + weight_decay) != 0)
        penalties.append(sumsquare(wdirect)*(direct_in_to_out_weight_decay + weight_decay));

    // apply penalty to cost
    if(penalties.size() != 0)
        cost = hconcat( sum(hconcat(costs[0] & penalties)) & costs );
    else    
        cost = hconcat(costs[0] & costs);
  
  
    cost->setName("cost");
    output->setName("output");

    // norman: ambiguous conversion (bool or char*?)
    //if(paramsvalues && (paramsvalues.size() == params.nelems()))
    if((bool)(paramsvalues) && (paramsvalues.size() == params.nelems()))
    {
        params << paramsvalues;
        initial_paramsvalues.resize(paramsvalues.length());
        initial_paramsvalues << paramsvalues;
    }
    else
    {
        paramsvalues.resize(params.nelems());
        initializeParams();
    }
    params.makeSharedValue(paramsvalues);

    // Funcs

    f = Func(input, output);
    costf = Func(input&target_and_weights, output&cost);
    costf->recomputeParents();
    output_and_target_to_cost = Func(output&target_and_weights, cost); 
    output_and_target_to_cost->recomputeParents();
}

Array<string> NeuralNet::costNames() const
{
    return (cost_funcs[0]+"+penalty") & cost_funcs;
}

int NeuralNet::costsize() const 
{ return cost->size(); }

void NeuralNet::train(VMat training_set)
{
    setTrainingSet(training_set);
    int l = training_set->length();  
    int nsamples = batch_size>0 ? batch_size : l;
    Func paramf = Func(input&target_and_weights, cost); // parameterized function to optimize
    Var totalcost = meanOf(training_set,paramf, nsamples);
    optimizer->setToOptimize(params, totalcost);
    optimizer->nupdates = (nepochs*l)/nsamples;
    optimizer->every = l/nsamples;
    optimizer->addMeasurer(*this);
    optimizer->build();
    optimizer->optimize();
  
    output_and_target_to_cost->recomputeParents();
    costf->recomputeParents();
    // cerr << "totalcost->value = " << totalcost->value << endl;
    setTrainCost(totalcost->value);
    if (saveparams!="")
        PLearn::save(expdir+saveparams,paramsvalues);
}


void NeuralNet::initializeParams()
{
    if (iseed<0)
        seed();
    else
        manual_seed(iseed);
    //real delta = 1./sqrt(inputsize());
    real delta = 1./inputsize();
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
            fill_random_normal(wdirect->value, 0, delta);
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
}

void NeuralNet::use(const Vec& in, Vec& prediction)
{
    f->fprop(in,prediction);
}

void NeuralNet::useAndCost(const Vec& inputvec, const Vec& targetvec, Vec outputvec, Vec costvec)
{
    costf->fprop(inputvec&targetvec, outputvec&costvec);
}

void NeuralNet::computeCost(const Vec& inputvec, const Vec& targetvec, const Vec& outputvec, const Vec& costvec)
{
    output_and_target_to_cost->fprop(outputvec&targetvec, costvec); 
}

void NeuralNet::forget()
{
    if(initial_paramsvalues)
        params << initial_paramsvalues;
    else
        initializeParams();
    inherited::forget();
}

void NeuralNet::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
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
