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
#include <plearn/var/BinaryClassificationLossVariable.h>
#include <plearn/var/ClassificationLossVariable.h>
#include <plearn/var/ConcatColumnsVariable.h>
#include <plearn/var/CrossEntropyVariable.h>
#include <plearn/var/ExpVariable.h>
#include <plearn/var/LiftOutputVariable.h>
#include <plearn/var/LogSoftmaxVariable.h>
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
#include <plearn/var/TransposeProductVariable.h>
#include <plearn/var/UnaryHardSlopeVariable.h>
#include <plearn/var/Var_operators.h>
#include <plearn/var/Var_utils.h>

#include <plearn/vmat/ConcatColumnsVMatrix.h>
#include <plearn/math/random.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    DeepFeatureExtractorNNet,
    "Deep Neural Network that extracts features in an unsupervised way",
    "After the unsupervised phase, this learner can optionally be trained using"
    "a supervised learning criteria (i.e. MSE, class NLL, margin-perceptron cost, etc.).");

DeepFeatureExtractorNNet::DeepFeatureExtractorNNet() 
    : batch_size(1), 
      //hidden_transfer_func("sigmoid"), 
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
      nhidden_schedule_current_position(-1)
{}

void DeepFeatureExtractorNNet::declareOptions(OptionList& ol)
{
    declareOption(ol, "nhidden_schedule", &DeepFeatureExtractorNNet::nhidden_schedule, OptionBase::buildoption,
                  "Number of hidden units of each hidden layers to add");
    declareOption(ol, "optimizer", &DeepFeatureExtractorNNet::optimizer, OptionBase::buildoption,
                  "Optimizer of the neural network");
    declareOption(ol, "batch_size", &DeepFeatureExtractorNNet::batch_size, OptionBase::buildoption, 
                  "How many samples to use to estimate the avergage gradient before updating the weights\n"
                  "0 is equivalent to specifying training_set->length() \n");
    /*
    declareOption(ol, "hidden_transfer_func", &DeepFeatureExtractorNNet::nhidden_transfer_func, OptionBase::buildoption,
                  "Transfer Function to use for hidden units. Choose among: \n"
                  "  - \"linear\" \n"
                  "  - \"tanh\" \n"
                  "  - \"sigmoid\" \n"
                  "  - \"exp\" \n"
                  "  - \"softplus\" \n"
                  "  - \"softmax\" \n"
                  "  - \"log_softmax\" \n"
                  "  - \"hard_slope\" \n"
                  "  - \"symm_hard_slope\" \n");
    */
    declareOption(ol, "output_transfer_func", &DeepFeatureExtractorNNet::output_transfer_func, OptionBase::buildoption,
                  "Output transfer function, when all hidden layers are added. Choose among:\n"
                  "  - \"tanh\" \n"
                  "  - \"sigmoid\" \n"
                  "  - \"exp\" \n"
                  "  - \"softplus\" \n"
                  "  - \"softmax\" \n"
                  "  - \"log_softmax\" \n"
                  "  - \"interval(<minval>,<maxval>)\", which stands for\n"
                  "          <minval>+(<maxval>-<minval>)*sigmoid(.).\n"
                  "An empty string or \"none\" means no output transfer function \n");
    declareOption(ol, "nhidden_schedule_position", &DeepFeatureExtractorNNet::nhidden_schedule_position, OptionBase::buildoption,
                  "Index of the layer(s) that will be trained at the next call of train.\n"
                  "Should be bigger then the last nhidden_schedule_position, which is\n"
                  "initialy -1. Then, all the layers up to nhidden_schedule_position that\n"
                  "were not trained so far will be. Also, when nhidden_schedule_position is\n"
                  "greater than or equal to the size of nhidden_schedule, then the output layer is also\n"
                  "added.");
    declareOption(ol, "nhidden_schedule_current_position", &DeepFeatureExtractorNNet::nhidden_schedule_current_position, OptionBase::learntoption,
                  "Index of the layer that is being trained at the current state");
declareOption(ol, "cost_funcs", &DeepFeatureExtractorNNet::cost_funcs, OptionBase::buildoption, 
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
                  "The FIRST function of the list will be used as \n"
                  "the objective function to optimize \n"
                  "(possibly with an added weight decay penalty) \n");
    declareOption(ol, "weight_decay", &DeepFeatureExtractorNNet::weight_decay, OptionBase::buildoption, 
                  "Global weight decay for all layers\n");
    declareOption(ol, "bias_decay", &DeepFeatureExtractorNNet::bias_decay, OptionBase::buildoption, 
                  "Global bias decay for all layers\n");
    declareOption(ol, "penalty_type", &DeepFeatureExtractorNNet::penalty_type,
                  OptionBase::buildoption,
                  "Penalty to use on the weights (for weight and bias decay).\n"
                  "Can be any of:\n"
                  "  - \"L1\": L1 norm,\n"
                  "  - \"L1_square\": square of the L1 norm,\n"
                  "  - \"L2_square\" (default): square of the L2 norm.\n");
    declareOption(ol, "classification_regularizer", &DeepFeatureExtractorNNet::classification_regularizer, OptionBase::buildoption, 
                  "Used only in the stable_cross_entropy cost function, to fight overfitting (0<=r<1)\n");  
    declareOption(ol, "regularizer", &DeepFeatureExtractorNNet::regularizer, OptionBase::buildoption, 
                  "Used in the stable_cross_entropy cost function for the hidden activations, in the unsupervised stages (0<=r<1)\n");  
    declareOption(ol, "margin", &DeepFeatureExtractorNNet::margin, OptionBase::buildoption, 
                  "Margin requirement, used only with the margin_perceptron_cost cost function.\n"
                  "It should be positive, and larger values regularize more.\n");
    declareOption(ol, "initialization_method", &DeepFeatureExtractorNNet::initialization_method, OptionBase::buildoption, 
                  "The method used to initialize the weights:\n"
                  " - \"normal_linear\"  = a normal law with variance 1/n_inputs\n"
                  " - \"normal_sqrt\"    = a normal law with variance 1/sqrt(n_inputs)\n"
                  " - \"uniform_linear\" = a uniform law in [-1/n_inputs, 1/n_inputs]\n"
                  " - \"uniform_sqrt\"   = a uniform law in [-1/sqrt(n_inputs), 1/sqrt(n_inputs)]\n"
                  " - \"zero\"           = all weights are set to 0\n");    
    declareOption(ol, "paramsvalues", &DeepFeatureExtractorNNet::paramsvalues, OptionBase::learntoption, 
                  "The learned parameter vector\n");
    declareOption(ol, "noutputs", &DeepFeatureExtractorNNet::noutputs, OptionBase::buildoption, 
                  "Number of output units. This gives this learner its outputsize.\n"
                  "It is typically of the same dimensionality as the target for regression problems \n"
                  "But for classification problems where target is just the class number, noutputs is \n"
                  "usually of dimensionality number of classes (as we want to output a score or probability \n"
                  "vector, one per class)\n"
                  "If the network only extracts features in an unsupervised manner,\n"
                  "then let noutputs be 0.");    
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void DeepFeatureExtractorNNet::build_()
{
    /*
     * Create Topology Var Graph
     */

    // Don't do anything if we don't have a train_set
    // It's the only one who knows the inputsize and targetsize anyway...
    // Also, nothing is done if no layers need to be added
    if(inputsize_>=0 && targetsize_>=0 && weightsize_>=0 
       && nhidden_schedule_current_position < nhidden_schedule.length() 
       && nhidden_schedule_current_position < nhidden_schedule_position)
    {

        // Initialize the input.
        if(nhidden_schedule_current_position < 0)
        {
            input = Var(inputsize(), "input");
            output = input;
        }

        Var before_transfer_function;
        params_to_train.resize(0);

        if(nhidden_schedule_current_position == -1)
            unsupervised_target = input;
        else 
            unsupervised_target = hidden_representation;
                
        int n_added_layers = 0;
        while(nhidden_schedule_current_position < nhidden_schedule_position && nhidden_schedule_current_position+1 < nhidden_schedule.length())
        {
            nhidden_schedule_current_position++;
            n_added_layers++;
            Var w = new SourceVariable(output->size()+1,nhidden_schedule[nhidden_schedule_current_position]);
            weights.push_back(w);
            // Initialize weights. If weights applied to a hidden layer, use -0.5 as bias
            // HUGO: Is this a good idea?
            //fillWeights(w,true,nhidden_schedule_current_position == 0 ? 0 : -0.5);
            fillWeights(w,true,0);
            params.push_back(w);
            params_to_train.push_back(w);
            output = hiddenLayer(output,w,"sigmoid",before_transfer_function);
            hidden_representation = output;
        }

        reconstruction_weights.resize(0);
        if(nhidden_schedule_position >= nhidden_schedule.length())
        {
            if(noutputs<=0) PLERROR("In DeepFeatureExtractorNNet::build_(): building the output layer but noutputs<=0");
            nhidden_schedule_current_position++;
            n_added_layers++;
            Var w = new SourceVariable(output->size()+1,noutputs);
            weights.push_back(w);
            //fillWeights(w,true,nhidden_schedule_current_position == 0 ? 0 : -0.5);
            fillWeights(w,true,0);
            params.push_back(w);
            output = hiddenLayer(output,w,output_transfer_func,before_transfer_function);
            
            params_to_train.clear();
            params_to_train.resize(params.length());
            for(int i=0; i<params.length(); i++)
                params_to_train[i] = params[i];
        }
        else
        {
            int it = 0;
            while(n_added_layers > 0)
            {
                n_added_layers--;
                it++;
                Var rw;
                if(nhidden_schedule_current_position-it == -1)
                    rw  = new SourceVariable(output->size()+1,inputsize());
                else
                    rw  = new SourceVariable(output->size()+1,nhidden_schedule[nhidden_schedule_current_position-it]);
                reconstruction_weights.push_back(rw);
                //fillWeights(rw,true,nhidden_schedule_current_position == 0 ? 0 : -0.5);
                fillWeights(rw,true,0);
                params.push_back(rw);
                params_to_train.push_back(rw);
                output = hiddenLayer(output,rw,"sigmoid",before_transfer_function);
            }            
        }

        // Build target and weight variables.
        buildTargetAndWeight();

        // Build costs.
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

        buildCosts(output, target, unsupervised_target, before_transfer_function);

        
        // Shared values hack...
        //if (!do_not_change_params) {
        if((bool)paramsvalues && (paramsvalues.size() == params.nelems()))
            params << paramsvalues;
        else
        {
            paramsvalues.resize(params.nelems());
            if(optimizer)
                optimizer->reset();
        }
        params.makeSharedValue(paramsvalues);
        //}
        
        // Build functions.
        buildFuncs(input, output, target, sampleweight);

        // Reinitialize the optimization phase
        if(optimizer)
            optimizer->reset();
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
    deepCopyField(nhidden_schedule, copies);
    deepCopyField(optimizer, copies);
    deepCopyField(cost_funcs, copies);
    deepCopyField(paramsvalues, copies);
    deepCopyField(params, copies);
    deepCopyField(params_to_train, copies);
    deepCopyField(weights, copies);
    deepCopyField(reconstruction_weights, copies);
    deepCopyField(invars, copies);
    deepCopyField(costs, copies);
    deepCopyField(penalties, copies);

    deepCopyField(f, copies);
    deepCopyField(test_costf, copies);
    deepCopyField(output_and_target_to_cost, copies);

    varDeepCopyField(input, copies);
    varDeepCopyField(output, copies);
    varDeepCopyField(hidden_representation, copies);
    varDeepCopyField(target, copies);
    varDeepCopyField(unsupervised_target, copies);
    varDeepCopyField(sampleweight, copies);
    varDeepCopyField(training_cost, copies);
    varDeepCopyField(test_costs, copies);

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
    stage = 0;
    
    params.clear();
    params.resize(0);
    weights.clear();
    weights.resize(0);
    nhidden_schedule_current_position = -1;
    nhidden_schedule_position = 0;
    build();
}
    
void DeepFeatureExtractorNNet::train()
{
    if(!train_set)
        PLERROR("In DeepFeatureExtractor::train, you did not setTrainingSet");
    
    if(!train_stats)
        PLERROR("In DeepFeatureExtractor::train, you did not setTrainStatsCollector");

    int l = train_set->length();  

    if(f.isNull()) // Net has not been properly built yet (because build was called before the learner had a proper training set)
        build();

    // Update de DeepFeatureExtractor structure if necessary
    if(nhidden_schedule_current_position < nhidden_schedule_position && nhidden_schedule_current_position < nhidden_schedule.length())
        build();

    // number of samples seen by optimizer before each optimizer update
    int nsamples = batch_size>0 ? batch_size : l;
    Func paramf = Func(invars, training_cost); // parameterized function to optimize
    Var totalcost = meanOf(train_set, paramf, nsamples);
    if(optimizer)
    {
        optimizer->setToOptimize(params_to_train, totalcost);  
        optimizer->build();
    }
    else PLERROR("DeepFeatureExtractor::train can't train without setting an optimizer first!");

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

    // Hugo: I don't know why we have to do this?!?
    output_and_target_to_cost->recomputeParents();
    test_costf->recomputeParents();
}

void DeepFeatureExtractorNNet::computeOutput(const Vec& input, Vec& output) const
{
    output.resize(outputsize());
    f->fprop(input,output);
}    

void DeepFeatureExtractorNNet::computeCostsFromOutputs(const Vec& input, const Vec& output, 
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

void DeepFeatureExtractorNNet::computeOutputAndCosts(const Vec& inputv, const Vec& targetv, 
                                 Vec& outputv, Vec& costsv) const
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
    //if(nhidden_schedule_current_position >= nhidden_schedule.length())
    if(targetsize() > 0)
    {        
        target = Var(targetsize(), "target");
        if(weightsize_>0)
        {
            if (weightsize_!=1)
                PLERROR("In NNet::buildTargetAndWeight - Expected weightsize to be 1 or 0 (or unspecified = -1, meaning 0), got %d",weightsize_);
            sampleweight = Var(1, "weight");
        }
    }
}

void DeepFeatureExtractorNNet::buildCosts(const Var& the_output, const Var& the_target, const Var& the_unsupervised_target, const Var& before_transfer_func) {
    costs.clear();
    costs.resize(0);
    if(nhidden_schedule_current_position >= nhidden_schedule.length())
    {
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
                assert( classification_regularizer >= 0 );
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
                    PLERROR("In NNet::build_()  unknown cost_func option: %s",cost_funcs[k].c_str());
                costs[k]->setParents(the_output & the_target);
                costs[k]->build();
            }

            // take into account the sampleweight
            //if(sampleweight)
            //  costs[k]= costs[k] * sampleweight; // NO, because this is taken into account (more properly) in stats->update
        }
        Vec val(1);
        val[0] = REAL_MAX;
        costs.push_back(new SourceVariable(val));        
    }
    else
    {
        int ncosts = cost_funcs.size();  
        costs.resize(ncosts);
        Vec val(1);
        val[0] = REAL_MAX;
        for(int i=0; i<costs.length(); i++)
            costs[i] = new SourceVariable(val);
        
        Var c = stable_cross_entropy(before_transfer_func, the_unsupervised_target);
        costs.push_back(c);
        assert( regularizer >= 0 );
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
        if (weightsize_>0)
            // only multiply by sampleweight if there are weights
            if(nhidden_schedule_current_position < nhidden_schedule.length())
                training_cost = hconcat(sampleweight*sum(hconcat(costs[costs.length()-2] & penalties))
                                        & (test_costs*sampleweight));
            else
                training_cost = hconcat(sampleweight*sum(hconcat(costs[0] & penalties))
                                        & (test_costs*sampleweight));
        else {
            if(nhidden_schedule_current_position < nhidden_schedule.length())
                training_cost = hconcat(sum(hconcat(costs[costs.length()-2] & penalties)) & test_costs);
            else
                training_cost = hconcat(sum(hconcat(costs[0] & penalties)) & test_costs);
        }
    } 
    else {
        if(weightsize_>0) {
            // only multiply by sampleweight if there are weights
            if(nhidden_schedule_current_position < nhidden_schedule.length())
                training_cost = hconcat(costs[costs.length()-2]*sampleweight & test_costs*sampleweight);
            else
                training_cost = hconcat(costs[0]*sampleweight & test_costs*sampleweight);
        } else {
            if(nhidden_schedule_current_position < nhidden_schedule.length())                
                training_cost = hconcat(costs[costs.length()-2] & test_costs);
            else
                training_cost = hconcat(costs[0] & test_costs);
        }
    }

    training_cost->setName("training_cost");
    test_costs->setName("test_costs");
    the_output->setName("output");
}


Var DeepFeatureExtractorNNet::hiddenLayer(const Var& input, const Var& weights, string transfer_func, Var& before_transfer_function) {
    Var hidden = affine_transform(input, weights); 
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
        PLERROR("In NNet::hiddenLayer - Unknown value for transfer_func: %s",transfer_func.c_str());
    return result;
}

void DeepFeatureExtractorNNet::buildPenalties() {
    penalties.resize(0);  // prevents penalties from being added twice by consecutive builds
    if(weight_decay > 0 || bias_decay > 0)
    {
        for(int i=0; i<weights.length(); i++)
        {
            penalties.append(affine_transform_weight_penalty(weights[i], weight_decay, bias_decay, penalty_type));
        }
        
        for(int i=0; i<reconstruction_weights.length(); i++)
        {
            penalties.append(affine_transform_weight_penalty(reconstruction_weights[i], weight_decay, bias_decay, penalty_type));
        }                
    }
}

void DeepFeatureExtractorNNet::fillWeights(const Var& weights, bool fill_first_row, real fill_with_this) {
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
        fill_random_normal(weights->value, 0, delta);
    else
        fill_random_uniform(weights->value, -delta, delta);
    if (fill_first_row)
        weights->matValue(0).fill(fill_with_this);
}

void DeepFeatureExtractorNNet::buildFuncs(const Var& the_input, const Var& the_output, const Var& the_target, const Var& the_sampleweight) {
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
    output_and_target_to_cost->recomputeParents();
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
