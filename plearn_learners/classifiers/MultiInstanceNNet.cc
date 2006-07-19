// -*- C++ -*-

// MultiInstanceNNet.cc
// Copyright (c) 1998-2002 Pascal Vincent
// Copyright (c) 1999-2005 Yoshua Bengio and University of Montreal
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

/*! \file PLearnLibrary/PLearnAlgo/MultiInstanceNNet.h */


#include <plearn/var/AffineTransformVariable.h>
#include <plearn/var/AffineTransformWeightPenalty.h>
#include <plearn/var/BinaryClassificationLossVariable.h>
#include <plearn/var/ClassificationLossVariable.h>
#include <plearn/var/ConcatColumnsVariable.h>
#include <plearn/vmat/ConcatColumnsVMatrix.h>
#include <plearn/var/CrossEntropyVariable.h>
#include <plearn/var/ExpVariable.h>
#include <plearn/var/LogVariable.h>
#include <plearn/var/LiftOutputVariable.h>
#include <plearn/var/LogSoftmaxVariable.h>
#include <plearn/var/MulticlassLossVariable.h>
#include "MultiInstanceNNet.h"
#include <plearn/var/UnfoldedSumOfVariable.h>
#include <plearn/var/SumOverBagsVariable.h>
#include <plearn/var/SumSquareVariable.h>
#include <plearn/math/random.h>
#include <plearn/var/SigmoidVariable.h>
#include <plearn/var/SumVariable.h>
#include <plearn/var/SumAbsVariable.h>
#include <plearn/var/SumOfVariable.h>
#include <plearn/vmat/SubVMatrix.h>
#include <plearn/var/TanhVariable.h>
#include <plearn/var/TransposeProductVariable.h>
#include <plearn/var/Var_operators.h>
#include <plearn/var/Var_utils.h>

//#include "DisplayUtils.h"
//#include "GradientOptimizer.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(MultiInstanceNNet, 
                        "Multi-instance feedforward neural network for probabilistic classification", 
                        "The data has the form of a set of input vectors x_i associated with a single\n"
                        "label y. Each x_i is an instance and the overall set of instance is called a bag.\n"
                        "We don't know which of the inputs is responsible for the label, i.e.\n"
                        "there are hidden (not observed) labels y_i associated with each of the inputs x_i.\n"
                        "We also know that y=1 if at least one of the y_i is 1, otherwise y=0, i.e.\n"
                        "   y = y_1 or y_2 or ... y_m\n"
                        "In terms of probabilities, it means that\n"
                        "   P(Y=0|x_1..x_m) = \\prod_{i=1}^m P(y_i=0|x_i)\n"
                        "which determines the likelihood of the observation (x_1...x_m,y).\n"
                        "The neural network implements the computation of P(y_i=1|x_i). The same\n"
                        "model is assumed for all instances in the bag. The number of instances is variable but\n"
                        "bounded a-priori (max_n_instances). The gradient is computed for a whole bag\n"
                        "at a time. The architectural parameters and hyper-parameters of the model\n"
                        "are otherwise the same as for the generic NNet class.\n"
                        "The bags within each data set are specified with a 2nd target column\n"
                        "(the first column is 0, 1 or missing; it should not be missing for the\n"
                        "last column of the bag). The second target column should be 0,1,2, or 3:\n"
                        "  1: first row of a bag\n"
                        "  2: last row of a bag\n"
                        "  3: simultaneously first and last, there is only one row in this bag\n"
                        "  0: intermediate row of a bag\n"
                        "following the protocol expected by the SumOverBagsVariable.\n"
    );

MultiInstanceNNet::MultiInstanceNNet() // DEFAULT VALUES FOR ALL OPTIONS
    : training_set_has_changed(false),
      max_n_instances(1),
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
      penalty_type("L2_square"),
      L1_penalty(false),
      direct_in_to_out(false),
      interval_minval(0), interval_maxval(1),
      test_bag_size(0),
      batch_size(1)
{}

MultiInstanceNNet::~MultiInstanceNNet()
{
}

void MultiInstanceNNet::declareOptions(OptionList& ol)
{
    declareOption(ol, "max_n_instances", &MultiInstanceNNet::max_n_instances, OptionBase::buildoption, 
                  "    maximum number of instances (input vectors x_i) allowed\n");

    declareOption(ol, "nhidden", &MultiInstanceNNet::nhidden, OptionBase::buildoption, 
                  "    number of hidden units in first hidden layer (0 means no hidden layer)\n");

    declareOption(ol, "nhidden2", &MultiInstanceNNet::nhidden2, OptionBase::buildoption, 
                  "    number of hidden units in second hidden layer (0 means no hidden layer)\n");

    declareOption(ol, "weight_decay", &MultiInstanceNNet::weight_decay, OptionBase::buildoption, 
                  "    global weight decay for all layers\n");

    declareOption(ol, "bias_decay", &MultiInstanceNNet::bias_decay, OptionBase::buildoption, 
                  "    global bias decay for all layers\n");

    declareOption(ol, "layer1_weight_decay", &MultiInstanceNNet::layer1_weight_decay, OptionBase::buildoption, 
                  "    Additional weight decay for the first hidden layer.  Is added to weight_decay.\n");
    declareOption(ol, "layer1_bias_decay", &MultiInstanceNNet::layer1_bias_decay, OptionBase::buildoption, 
                  "    Additional bias decay for the first hidden layer.  Is added to bias_decay.\n");

    declareOption(ol, "layer2_weight_decay", &MultiInstanceNNet::layer2_weight_decay, OptionBase::buildoption, 
                  "    Additional weight decay for the second hidden layer.  Is added to weight_decay.\n");

    declareOption(ol, "layer2_bias_decay", &MultiInstanceNNet::layer2_bias_decay, OptionBase::buildoption, 
                  "    Additional bias decay for the second hidden layer.  Is added to bias_decay.\n");

    declareOption(ol, "output_layer_weight_decay", &MultiInstanceNNet::output_layer_weight_decay, OptionBase::buildoption, 
                  "    Additional weight decay for the output layer.  Is added to 'weight_decay'.\n");

    declareOption(ol, "output_layer_bias_decay", &MultiInstanceNNet::output_layer_bias_decay, OptionBase::buildoption, 
                  "    Additional bias decay for the output layer.  Is added to 'bias_decay'.\n");

    declareOption(ol, "direct_in_to_out_weight_decay", &MultiInstanceNNet::direct_in_to_out_weight_decay, OptionBase::buildoption, 
                  "    Additional weight decay for the direct in-to-out layer.  Is added to 'weight_decay'.\n");

    declareOption(ol, "penalty_type", &MultiInstanceNNet::penalty_type, OptionBase::buildoption,
                  "    Penalty to use on the weights (for weight and bias decay).\n"
                  "    Can be any of:\n"
                  "      - \"L1\": L1 norm,\n"
                  "      - \"L1_square\": square of the L1 norm,\n"
                  "      - \"L2_square\" (default): square of the L2 norm.\n");

    declareOption(ol, "L1_penalty", &MultiInstanceNNet::L1_penalty, OptionBase::buildoption, 
                  "    Deprecated - You should use \"penalty_type\" instead\n"
                  "    should we use L1 penalty instead of the default L2 penalty on the weights?\n");

    declareOption(ol, "direct_in_to_out", &MultiInstanceNNet::direct_in_to_out, OptionBase::buildoption, 
                  "    should we include direct input to output connections?\n");

    declareOption(ol, "optimizer", &MultiInstanceNNet::optimizer, OptionBase::buildoption, 
                  "    specify the optimizer to use\n");

    declareOption(ol, "batch_size", &MultiInstanceNNet::batch_size, OptionBase::buildoption, 
                  "    how many samples to use to estimate the avergage gradient before updating the weights\n"
                  "    0 is equivalent to specifying training_set->n_non_missing_rows() \n");

    declareOption(ol, "paramsvalues", &MultiInstanceNNet::paramsvalues, OptionBase::learntoption, 
                  "    The learned parameter vector\n");

    inherited::declareOptions(ol);

}

void MultiInstanceNNet::build()
{
    inherited::build();
    build_();
}

void MultiInstanceNNet::setTrainingSet(VMat training_set, bool call_forget)
{ 
    training_set_has_changed =
        !train_set || train_set->width()!=training_set->width() ||
        train_set->length()!=training_set->length() || train_set->inputsize()!=training_set->inputsize()
        || train_set->weightsize()!= training_set->weightsize();

    train_set = training_set;
    if (training_set_has_changed)
    {
        inputsize_ = train_set->inputsize();
        targetsize_ = train_set->targetsize();
        weightsize_ = train_set->weightsize();
    }

    if (training_set_has_changed || call_forget)
    {
        build(); // MODIF FAITE PAR YOSHUA: sinon apres un setTrainingSet le build n'est pas complete dans un MultiInstanceNNet train_set = training_set;
        if (call_forget) forget();
    }

}

void MultiInstanceNNet::build_()
{
    /*
     * Create Topology Var Graph
     */

    // Don't do anything if we don't have a train_set
    // It's the only one who knows the inputsize and targetsize anyway...

    if(inputsize_>=0 && targetsize_>=0 && weightsize_>=0)
    {

      
        // init. basic vars
        input = Var(inputsize(), "input");
        output = input;
        params.resize(0);

        if (targetsize()!=2)
            PLERROR("MultiInstanceNNet:: expected the data to have 2 target columns, got %d",
                    targetsize());

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

        if (nhidden2>0 && nhidden==0)
            PLERROR("MultiInstanceNNet:: can't have nhidden2 (=%d) > 0 while nhidden=0",nhidden2);
      
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

        // the output transfer function is FIXED: it must be a sigmoid (0/1 probabilistic classification)

        output = sigmoid(output);

        /*
         * target and weights
         */

        target = Var(1, "target");

        if(weightsize_>0)
        {
            if (weightsize_!=1)
                PLERROR("MultiInstanceNNet: expected weightsize to be 1 or 0 (or unspecified = -1, meaning 0), got %d",weightsize_);
            sampleweight = Var(1, "weight");
        }

        // build costs
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
            PLWARNING("L2 penalty not supported, assuming you want L2 square");             penalty_type = "L2_square";
        }
        else
            PLERROR("penalty_type \"%s\" not supported", penalty_type.c_str());

        // create penalties
        penalties.resize(0);  // prevents penalties from being added twice by consecutive builds
        if(w1 && (!fast_exact_is_equal(layer1_weight_decay + weight_decay,0) ||
                  !fast_exact_is_equal(layer1_bias_decay + bias_decay,    0)))
            penalties.append(affine_transform_weight_penalty(w1, (layer1_weight_decay + weight_decay), (layer1_bias_decay + bias_decay), penalty_type));
        if(w2 && (!fast_exact_is_equal(layer2_weight_decay + weight_decay,0) ||
                  !fast_exact_is_equal(layer2_bias_decay + bias_decay,    0)))
            penalties.append(affine_transform_weight_penalty(w2, (layer2_weight_decay + weight_decay), (layer2_bias_decay + bias_decay), penalty_type));
        if(wout && (!fast_exact_is_equal(output_layer_weight_decay + weight_decay, 0) ||
                    !fast_exact_is_equal(output_layer_bias_decay + bias_decay, 0)))
            penalties.append(affine_transform_weight_penalty(wout, (output_layer_weight_decay + weight_decay), 
                                                             (output_layer_bias_decay + bias_decay), penalty_type));
        if(wdirect && !fast_exact_is_equal(direct_in_to_out_weight_decay + weight_decay, 0))
        {
            if (penalty_type=="L1_square")
                penalties.append(square(sumabs(wdirect))*(direct_in_to_out_weight_decay + weight_decay));
            else if (penalty_type=="L1")
                penalties.append(sumabs(wdirect)*(direct_in_to_out_weight_decay + weight_decay));
            else if (penalty_type=="L2_square")
                penalties.append(sumsquare(wdirect)*(direct_in_to_out_weight_decay + weight_decay));
        }

        // Shared values hack...
        if(paramsvalues.length() == params.nelems())
            params << paramsvalues;
        else
        {
            paramsvalues.resize(params.nelems());
            initializeParams();
        }
        params.makeSharedValue(paramsvalues);

        output->setName("element output");

        f = Func(input, output);

        input_to_logP0 = Func(input, log(1 - output));

        bag_size = Var(1,1);
        bag_inputs = Var(max_n_instances,inputsize());
        bag_output = 1-exp(unfoldedSumOf(bag_inputs,bag_size,input_to_logP0,max_n_instances));

        costs.resize(3); // (negative log-likelihood, classification error, lift output) for the bag

        costs[0] = cross_entropy(bag_output, target);
        costs[1] = binary_classification_loss(bag_output,target);
        costs[2] = lift_output(bag_output, target);
        test_costs = hconcat(costs);

        // Apply penalty to cost.
        // If there is no penalty, we still add costs[0] as the first cost, in
        // order to keep the same number of costs as if there was a penalty.
        if(penalties.size() != 0) {
            if (weightsize_>0)
                // only multiply by sampleweight if there are weights
                training_cost = hconcat(sampleweight*sum(hconcat(costs[0] & penalties)) // don't weight the lift output
                                        & (costs[0]*sampleweight) & (costs[1]*sampleweight) & costs[2]);
            else {
                training_cost = hconcat(sum(hconcat(costs[0] & penalties)) & test_costs);
            }
        } 
        else {
            if(weightsize_>0) {
                // only multiply by sampleweight if there are weights (but don't weight the lift output)
                training_cost = hconcat(costs[0]*sampleweight & costs[0]*sampleweight & costs[1]*sampleweight & costs[2]);
            } else {
                training_cost = hconcat(costs[0] & test_costs);
            }
        }
      
        training_cost->setName("training_cost");
        test_costs->setName("test_costs");

        if (weightsize_>0)
            invars = bag_inputs & bag_size & target & sampleweight;
        else
            invars = bag_inputs & bag_size & target;

        inputs_and_targets_to_test_costs = Func(invars,test_costs);
        inputs_and_targets_to_training_costs = Func(invars,training_cost);

        inputs_and_targets_to_test_costs->recomputeParents();
        inputs_and_targets_to_training_costs->recomputeParents();

        // A UN MOMENT DONNE target NE POINTE PLUS AU MEME ENDROIT!!!
    }
}

int MultiInstanceNNet::outputsize() const
{ return 1; }

TVec<string> MultiInstanceNNet::getTrainCostNames() const
{
    TVec<string> names(4);
    names[0] = "NLL+penalty";
    names[1] = "NLL";
    names[2] = "class_error";
    names[3] = "lift_output";
    return names;
}

TVec<string> MultiInstanceNNet::getTestCostNames() const
{ 
    TVec<string> names(3);
    names[0] = "NLL";
    names[1] = "class_error";
    names[2] = "lift_output";
    return names;
}


void MultiInstanceNNet::train()
{
    // MultiInstanceNNet nstages is number of epochs (whole passages through the training set)
    // while optimizer nstages is number of weight updates.
    // So relationship between the 2 depends whether we are in stochastic, batch or minibatch mode

    if(!train_set)
        PLERROR("In MultiInstanceNNet::train, you did not setTrainingSet");
    
    if(!train_stats)
        PLERROR("In MultiInstanceNNet::train, you did not setTrainStatsCollector");

    if(f.isNull()) // Net has not been properly built yet (because build was called before the learner had a proper training set)
        build();


    if (training_set_has_changed)
    {
        // number of optimiser stages corresponding to one learner stage (one epoch)
        optstage_per_lstage = 0;
        int n_bags = -1;
        if (batch_size<=0)
            optstage_per_lstage = 1;
        else // must count the nb of bags in the training set
        {
            n_bags=0;
            int l = train_set->length();
            ProgressBar* pb = 0;
            if(report_progress)
                pb = new ProgressBar("Counting nb bags in train_set for MultiInstanceNNet ", l);
            Vec row(train_set->width());
            int tag_column = train_set->inputsize() + train_set->targetsize() - 1;
            for (int i=0;i<l;i++) {
                train_set->getRow(i,row);
                int tag = (int)row[tag_column];
                if (tag & SumOverBagsVariable::TARGET_COLUMN_FIRST) {
                    // indicates the beginning of a new bag.
                    n_bags++;
                }
                if(pb)
                    pb->update(i);
            }
            if(pb)
                delete pb;
            optstage_per_lstage = n_bags/batch_size;
        }
        training_set_has_changed = false;
    }

    Var totalcost = sumOverBags(train_set, inputs_and_targets_to_training_costs, max_n_instances, batch_size);
    if(optimizer)
    {
        optimizer->setToOptimize(params, totalcost);  
        optimizer->build();
    }


    ProgressBar* pb = 0;
    if(report_progress)
        pb = new ProgressBar("Training MultiInstanceNNet from stage " + tostring(stage) + " to " + tostring(nstages), nstages-stage);

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

    if(pb)
        delete pb;

    //if (batch_size==0)
    //  optimizer->verifyGradient(0.001);

    //output_and_target_to_cost->recomputeParents();
    //test_costf->recomputeParents();

    // cerr << "totalcost->value = " << totalcost->value << endl;
    // cout << "Result for benchmark is: " << totalcost->value << endl;
}


void MultiInstanceNNet::computeOutput(const Vec& inputv, Vec& outputv) const
{
    f->fprop(inputv,outputv);
}

///////////////////////////
// computeOutputAndCosts //
///////////////////////////
void MultiInstanceNNet::computeOutputAndCosts(const Vec& inputv, const Vec& targetv, 
                                              Vec& outputv, Vec& costsv) const
{
    f->fprop(inputv,outputv); // this is the individual P(y_i|x_i), MAYBE UNNECESSARY CALCULATION
    // since the outputs will be re-computed when doing the fprop below at the end of the bag
    // (but if we want to provide them after each call...). The solution would
    // be to do like in computeCostsFromOutputs, keeping track of the outputs.
    int bag_signal = int(targetv[1]);
    if (bag_signal & 1) // first instance, start counting
        test_bag_size=0;
    bag_inputs->matValue(test_bag_size++) << inputv;
    if (!(bag_signal & 2)) // not reached the last instance
        costsv.fill(MISSING_VALUE);
    else // end of bag, we have a target and we can compute a cost
    {
        bag_size->valuedata[0]=test_bag_size;
        target->valuedata[0] = targetv[0];
        if (weightsize_>0) sampleweight->valuedata[0]=1; // the test weights are known and used higher up
        inputs_and_targets_to_test_costs->fproppath.fprop();
        inputs_and_targets_to_test_costs->outputs.copyTo(costsv);
    }
}

/////////////////////////////
// computeCostsFromOutputs //
/////////////////////////////
void MultiInstanceNNet::computeCostsFromOutputs(const Vec& inputv, const Vec& outputv, 
                                                const Vec& targetv, Vec& costsv) const
{
    instance_logP0.resize(max_n_instances);
    int bag_signal = int(targetv[1]);
    if (bag_signal & 1) // first instance, start counting
        test_bag_size=0;
    instance_logP0[test_bag_size++] = safeflog(1-outputv[0]);
    if (!(bag_signal & 2)) // not reached the last instance
        costsv.fill(MISSING_VALUE);
    else // end of bag, we have a target and we can compute a cost
    {
        instance_logP0.resize(test_bag_size);
        real bag_P0 = safeexp(sum(instance_logP0));
        int classe = int(targetv[0]);
        int predicted_classe = (bag_P0>0.5)?0:1;
        real nll = (classe==0)?-safeflog(bag_P0):-safeflog(1-bag_P0);
        int classification_error = (classe != predicted_classe);
        costsv[0] = nll;
        costsv[1] = classification_error;
        // Add the lift output.
        // Probably not working: it looks like it only takes into account the
        // output for the last instance in the bag.
        PLERROR("In MultiInstanceNNet::computeCostsFromOutputs - Probably "
                "bugged, please check code");
        if (targetv[0] > 0) {
            costsv[2] = outputv[0];
        } else {
            costsv[2] = -outputv[0];
        }
    }
}

//////////////////////
// initializeParams //
//////////////////////
void MultiInstanceNNet::initializeParams()
{
    if (seed_>=0)
        manual_seed(seed_);
    else
        PLearn::seed();

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

void MultiInstanceNNet::forget()
{
    if (train_set) initializeParams();
    stage = 0;
}

//! To use varDeepCopyField.
#ifdef __INTEL_COMPILER
#pragma warning(disable:1419)  // Get rid of compiler warning.
#endif
extern void varDeepCopyField(Var& field, CopiesMap& copies);
#ifdef __INTEL_COMPILER
#pragma warning(default:1419)
#endif

void MultiInstanceNNet::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(instance_logP0, copies);
    varDeepCopyField(input, copies);
    varDeepCopyField(target, copies);
    varDeepCopyField(sampleweight, copies);
    varDeepCopyField(w1, copies);
    varDeepCopyField(w2, copies);
    varDeepCopyField(wout, copies);
    varDeepCopyField(wdirect, copies);
    varDeepCopyField(output, copies);
    varDeepCopyField(bag_size, copies);
    varDeepCopyField(bag_inputs, copies);
    varDeepCopyField(bag_output, copies);
    deepCopyField(inputs_and_targets_to_test_costs, copies);
    deepCopyField(inputs_and_targets_to_training_costs, copies);
    deepCopyField(input_to_logP0, copies);
    varDeepCopyField(nll, copies);
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
