// -*- C++ -*-

// LinearInductiveTransferClassifier.cc
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

/*! \file LinearInductiveTransferClassifier.cc */


#include "LinearInductiveTransferClassifier.h"
#include <plearn/var/AffineTransformVariable.h>
#include <plearn/var/ArgmaxVariable.h>
#include <plearn/var/SourceVariable.h>
#include <plearn/var/AffineTransformWeightPenalty.h>
#include <plearn/var/ClassificationLossVariable.h>
#include <plearn/var/ConcatColumnsVariable.h>
#include <plearn/var/ColumnSumVariable.h>
#include <plearn/var/ConcatRowsVariable.h>
#include <plearn/var/CrossEntropyVariable.h>
#include <plearn/var/DotProductVariable.h>
#include <plearn/var/DuplicateRowVariable.h>
#include <plearn/var/DivVariable.h>
#include <plearn/var/ExpVariable.h>
//#include <plearn/var/LogSoftmaxVariable.h>
#include <plearn/var/LiftOutputVariable.h>
#include <plearn/var/MulticlassLossVariable.h>
#include <plearn/var/NegCrossEntropySigmoidVariable.h>
#include <plearn/var/OneHotVariable.h>
//#include <plearn/var/PowVariable.h>
#include <plearn/var/ProductTransposeVariable.h>
#include <plearn/var/ProductVariable.h>
#include <plearn/var/ReshapeVariable.h>
#include <plearn/var/SigmoidVariable.h>
#include <plearn/var/SoftmaxVariable.h>
#include <plearn/var/SumVariable.h>
#include <plearn/var/SumAbsVariable.h>
#include <plearn/var/SumOfVariable.h>
#include <plearn/var/SumSquareVariable.h>
#include <plearn/var/TanhVariable.h>
#include <plearn/var/TimesVariable.h>
#include <plearn/var/TransposeVariable.h>
#include <plearn/var/TransposeProductVariable.h>
#include <plearn/var/VarRowsVariable.h>
#include <plearn/var/Var_operators.h>
#include <plearn/var/Var_utils.h>
#include <plearn/display/DisplayUtils.h>
#include <plearn/vmat/ConcatColumnsVMatrix.h>
#include <plearn/math/random.h>
#include <plearn/math/plapack.h>
#include <plearn_learners/online/RBMMatrixConnection.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    LinearInductiveTransferClassifier,
    "Linear classifier that uses class representations",
    "Linear classifier that uses class representations in\n"
    "order to make use of inductive transfer between classes.");

LinearInductiveTransferClassifier::LinearInductiveTransferClassifier() 
    : batch_size(1), 
      weight_decay(0), 
      penalty_type("L2_square"),
      initialization_method("uniform_linear"), 
      model_type("discriminative"),
      dont_consider_train_targets(false),
      use_bias_in_weights_prediction(false),
      multi_target_classifier(false),
      sigma_min(1e-5),
      nhidden(-1),
      rbm_nstages(0),
      rbm_learning_rate(0.01)
{
    random_gen = new PRandom();
}

void LinearInductiveTransferClassifier::declareOptions(OptionList& ol)
{
    declareOption(ol, "optimizer", &LinearInductiveTransferClassifier::optimizer, 
                  OptionBase::buildoption,
                  "Optimizer of the discriminative classifier");
    declareOption(ol, "rbm_nstages", 
                  &LinearInductiveTransferClassifier::rbm_nstages, 
                  OptionBase::buildoption,
                  "Number of RBM training to initialize hidden layer weights");
    declareOption(ol, "rbm_learning_rate", 
                  &LinearInductiveTransferClassifier::rbm_learning_rate, 
                  OptionBase::buildoption,
                  "Learning rate for the RBM");
    declareOption(ol, "visible_layer",
                  &LinearInductiveTransferClassifier::visible_layer, 
                  OptionBase::buildoption,
                  "Visible layer of the RBM");
    declareOption(ol, "hidden_layer",
                  &LinearInductiveTransferClassifier::hidden_layer, 
                  OptionBase::buildoption,
                  "Hidden layer of the RBM");
    declareOption(ol, "batch_size", &LinearInductiveTransferClassifier::batch_size,
                  OptionBase::buildoption, 
                  "How many samples to use to estimate the avergage gradient before updating the weights\n"
                  "0 is equivalent to specifying training_set->length() \n");
    declareOption(ol, "weight_decay", 
                  &LinearInductiveTransferClassifier::weight_decay, 
                  OptionBase::buildoption, 
                  "Global weight decay for all layers\n");
    declareOption(ol, "model_type", &LinearInductiveTransferClassifier::model_type,
                  OptionBase::buildoption, 
                  "Model type. Choose between:\n"
                  " - \"discriminative\"               (multiclass classifier)\n"
                  " - \"discriminative_1_vs_all\"      (1 vs all multitask classier)\n"
                  " - \"generative\"                   (gaussian input)\n"
                  " - \"generative_0-1\"               ([0,1] input)\n"
                  " - \"nnet_discriminative_1_vs_all\" ([0,1] input)\n"
        );
    declareOption(ol, "penalty_type", 
                  &LinearInductiveTransferClassifier::penalty_type,
                  OptionBase::buildoption,
                  "Penalty to use on the weights (for weight and bias decay).\n"
                  "Can be any of:\n"
                  "  - \"L1\": L1 norm,\n"
                  "  - \"L1_square\": square of the L1 norm,\n"
                  "  - \"L2_square\" (default): square of the L2 norm.\n");
    declareOption(ol, "initialization_method", 
                  &LinearInductiveTransferClassifier::initialization_method, 
                  OptionBase::buildoption, 
                  "The method used to initialize the weights:\n"
                  " - \"normal_linear\"  = a normal law with variance 1/n_inputs\n"
                  " - \"normal_sqrt\"    = a normal law with variance 1/sqrt(n_inputs)\n"
                  " - \"uniform_linear\" = a uniform law in [-1/n_inputs, 1/n_inputs]\n"
                  " - \"uniform_sqrt\"   = a uniform law in [-1/sqrt(n_inputs), 1/sqrt(n_inputs)]\n"
                  " - \"zero\"           = all weights are set to 0\n");    
    declareOption(ol, "paramsvalues", 
                  &LinearInductiveTransferClassifier::paramsvalues, 
                  OptionBase::learntoption, 
                  "The learned parameters\n");
    declareOption(ol, "class_reps", &LinearInductiveTransferClassifier::class_reps,
                  OptionBase::buildoption, 
                  "Class vector representations\n");
    declareOption(ol, "dont_consider_train_targets", 
                  &LinearInductiveTransferClassifier::dont_consider_train_targets, 
                  OptionBase::buildoption, 
                  "Indication that the targets seen in the training set\n"
                  "should not be considered when tagging a new set\n");
    declareOption(ol, "use_bias_in_weights_prediction", 
                  &LinearInductiveTransferClassifier::use_bias_in_weights_prediction, 
                  OptionBase::buildoption, 
                  "Indication that a bias should be used for weights prediction\n");
    declareOption(ol, "multi_target_classifier", 
                  &LinearInductiveTransferClassifier::multi_target_classifier, 
                  OptionBase::buildoption, 
                  "Indication that the classifier works with multiple targets,\n"
                  "possibly ON simulatneously.\n");
    declareOption(ol, "sigma_min", &LinearInductiveTransferClassifier::sigma_min, 
                  OptionBase::buildoption, 
                  "Minimum variance for all coordinates, which is added\n"
                  "to the maximum likelihood estimates.\n");
    declareOption(ol, "nhidden", &LinearInductiveTransferClassifier::nhidden, 
                  OptionBase::buildoption, 
                  "Number of hidden units for neural network.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void LinearInductiveTransferClassifier::build_()
{
    /*
     * Create Topology Var Graph
     */

    // Don't do anything if we don't have a train_set
    // It's the only one who knows the inputsize and targetsize anyway...
    // Also, nothing is done if no layers need to be added
    if(inputsize_>=0 && targetsize_>=0 && weightsize_>=0)
    {
        if (seed_ != 0) random_gen->manual_seed(seed_);//random_gen->manual_seed(seed_);

        input = Var(inputsize(), "input");
        target = Var(targetsize(),"target");
        if(class_reps.size()<=0) 
            PLERROR("LinearInductiveTransferClassifier::build_(): class_reps is empty");
        noutputs = class_reps.length();
        buildTargetAndWeight();
        params.resize(0);

        Mat class_reps_to_use;
        if(use_bias_in_weights_prediction)
        {
            // Add column with 1s, to include bias
            Mat class_reps_with_bias(class_reps.length(), class_reps.width()+1);
            for(int i=0; i<class_reps_with_bias.length(); i++)
                for(int j=0; j<class_reps_with_bias.width(); j++)
                {
                    if(j==0)
                        class_reps_with_bias(i,j) = 1;
                    else
                        class_reps_with_bias(i,j) = class_reps(i,j-1);
                }
            class_reps_to_use = class_reps_with_bias;
        }
        else
        {
            class_reps_to_use = class_reps;
        }
                

        if(model_type == "nnet_discriminative_1_vs_all")
        {
            if(nhidden <= 0)
                PLERROR("In LinearInductiveTransferClassifier::build_(): nhidden "
                        "must be > 0.");
//            Ws.resize(nhidden); 
//            As.resize(nhidden);
//            s_hids.resize(nhidden);
//            s = Var(1,nhidden,"sigma_square");
//            for(int i=0; i<Ws.length(); i++)
//            {
//                Ws[i] = Var(inputsize_,class_reps_to_use.width());
//                As[i] = Var(1,class_reps_to_use.width());
//                s_hids[i] = Var(1,inputsize_);
//            }
            W = Var(inputsize_+1,nhidden,"hidden_weights");
            A = Var(nhidden,class_reps_to_use.width());
            s = Var(1,nhidden,"sigma_square");
            params.push_back(W);
            params.push_back(A);
            params.push_back(s);
//            params.append(Ws);
//            params.append(As);
//            params.append(s);
//            params.append(s_hids);
//            A = vconcat(As);
        }
        else
        {
            A = Var(inputsize_,class_reps_to_use.width());
            s = Var(1,inputsize_,"sigma_square");
            //fillWeights(A,false);     
            params.push_back(A);
            params.push_back(s);        
        }
        

        class_reps_var = new SourceVariable(class_reps_to_use);
        Var weights = productTranspose(A,class_reps_var);
        if(model_type == "discriminative" || model_type == "discriminative_1_vs_all")
        { 
            weights =vconcat(-product(exp(s),square(weights)) & weights); // Making sure that the scaling factor is going to be positive
            output = affine_transform(input, weights);
        }
        else if(model_type == "generative_0-1")
        {
            PLERROR("Not implemented yet");
            //weights = vconcat(columnSum(log(A/(exp(A)-1))) & weights);
            //output = affine_transform(input, weights);
        }
        else if(model_type == "generative")
        {
            weights = vconcat(-columnSum(square(weights)/transpose(duplicateRow(s,noutputs))) & 2*weights/transpose(duplicateRow(s,noutputs)));
            if(targetsize() == 1)
                output = affine_transform(input, weights);
            else
                output = exp(affine_transform(input, weights) - duplicateRow(dot(transpose(input)/s,input),noutputs))+REAL_EPSILON;
        }
        else if(model_type == "nnet_discriminative_1_vs_all")
        {
            //hidden_neurons.resize(nhidden);
            //Var weights;
            //for(int i=0; i<nhidden; i++)
            //{
            //    weights = productTranspose(Ws[i],class_reps_var);
            //    weights = vconcat(-product(exp(s_hids[i]),square(weights)) 
            //                      & weights); 
            //    hidden_neurons[i] = tanh(affine_transform(input, weights));
            //}
            //
            //weights = productTranspose(A,class_reps_var);
            //output = -transpose(product(exp(s),square(weights)));
            //
            //for(int i=0; i<nhidden; i++)
            //{
            //    output = output + times(productTranspose(class_reps_var,As[i]),
            //                   hidden_neurons[i]);
            //}
            weights =vconcat(-product(exp(s),square(weights)) & weights); // Making sure that the scaling factor is going to be positive
            if(rbm_nstages>0)
                output = affine_transform(tanh(affine_transform(input,W)), weights);
            else
                output = affine_transform(sigmoid(affine_transform(input,W)), weights);
        }

        else
            PLERROR("In LinearInductiveTransferClassifier::build_(): model_type %s is not valid", model_type.c_str());

        TVec<bool> class_tags(noutputs);
        if(targetsize() == 1)
        {
            Vec row(train_set.width());
            int target_class;
            class_tags.fill(0);
            for(int i=0; i<train_set.length(); i++)
            {
                train_set->getRow(i,row);
                target_class = (int) row[train_set->inputsize()];
                class_tags[target_class] = 1;
            }
            
            seen_targets.resize(0);
            unseen_targets.resize(0);
            for(int i=0; i<class_tags.length(); i++)
                if(class_tags[i])
                    seen_targets.push_back(i);
                else
                    unseen_targets.push_back(i);
        }
        
        if(targetsize() != 1 && !multi_target_classifier)
            PLERROR("In LinearInductiveTransferClassifier::build_(): when targetsize() != 1, multi_target_classifier should be true.");
        if(targetsize() == 1 && multi_target_classifier)
            PLERROR("In LinearInductiveTransferClassifier::build_(): when targetsize() == 1, multi_target_classifier should be false.");
        

        if(targetsize() == 1 && seen_targets.length() != class_tags.length())
        {
            sup_output = new VarRowsVariable(output,new SourceVariable(seen_targets));
            if(dont_consider_train_targets)
                new_output = new VarRowsVariable(output,new SourceVariable(unseen_targets));
            else
                new_output = output;
            Var sup_mapping = new SourceVariable(noutputs,1);
            Var new_mapping = new SourceVariable(noutputs,1);
            int sup_id = 0;
            int new_id = 0;
            for(int k=0; k<class_tags.length(); k++)
            {
                if(class_tags[k])
                {
                    sup_mapping->value[k] = sup_id;
                    new_mapping->value[k] = MISSING_VALUE;
                    sup_id++;
                }
                else
                {
                    sup_mapping->value[k] = MISSING_VALUE;
                    new_mapping->value[k] = new_id;
                    new_id++;
                }
            }
            sup_target = new VarRowsVariable(sup_mapping, target);
            if(dont_consider_train_targets)
                new_target = new VarRowsVariable(new_mapping, target);
            else
                new_target = target;
        }
        else
        {
            sup_output = output;
            new_output = output;
            sup_target = target;
            new_target = target;            
        }

        // Build costs
        if(model_type == "discriminative" || model_type == "discriminative_1_vs_all" || model_type == "generative_0-1" || model_type == "nnet_discriminative_1_vs_all")
        {
            if(model_type == "discriminative")
            {
                if(targetsize() != 1)
                    PLERROR("In LinearInductiveTransferClassifier::build_(): can't use discriminative model with targetsize() != 1");
                costs.resize(2);
                new_costs.resize(2);
                sup_output = softmax(sup_output);
                costs[0] = neg_log_pi(sup_output,sup_target);
                costs[1] = classification_loss(sup_output, sup_target);
                new_output = softmax(new_output);
                new_costs[0] = neg_log_pi(new_output,new_target);
                new_costs[1] = classification_loss(new_output, new_target);
            }
            if(model_type == "discriminative_1_vs_all" 
               || model_type == "nnet_discriminative_1_vs_all")
            {
                costs.resize(2);
                new_costs.resize(2);
                if(targetsize() == 1)
                {
                    costs[0] = stable_cross_entropy(sup_output, onehot(seen_targets.length(),sup_target));
                    costs[1] = classification_loss(sigmoid(sup_output), sup_target);
                }
                else
                {
                    costs[0] = stable_cross_entropy(sup_output, sup_target, true);
                    costs[1] = transpose(lift_output(sigmoid(sup_output)+0.001, sup_target));
                }
                if(targetsize() == 1)
                {
                    if(dont_consider_train_targets)
                        new_costs[0] = stable_cross_entropy(new_output, onehot(unseen_targets.length(),new_target));
                    else
                        new_costs[0] = stable_cross_entropy(new_output, onehot(noutputs,new_target));
                    new_costs[1] = classification_loss(sigmoid(new_output), new_target);
                }
                else
                {
                    new_costs.resize(costs.length());
                    for(int i=0; i<new_costs.length(); i++)
                        new_costs[i] = costs[i];
                }
            }
            if(model_type == "generative_0-1")
            {
                costs.resize(2);
                new_costs.resize(2);
                if(targetsize() == 1)
                {
                    costs[0] = sup_output;
                    costs[1] = classification_loss(sigmoid(sup_output), sup_target);
                }
                else
                {
                    PLERROR("In LinearInductiveTransferClassifier::build_(): can't use generative_0-1 model with targetsize() != 1");
                    costs[0] = sup_output;
                    costs[1] = transpose(lift_output(sigmoid(exp(sup_output)+REAL_EPSILON), sup_target));
                }
                if(targetsize() == 1)
                {
                    new_costs[0] = new_output;
                    new_costs[1] = classification_loss(new_output, new_target);
                }
                else
                {
                    new_costs.resize(costs.length());
                    for(int i=0; i<new_costs.length(); i++)
                        new_costs[i] = costs[i];
                }
            }
        }
        else if(model_type == "generative")
        {
            costs.resize(1);
            if(targetsize() == 1)
                costs[0] = classification_loss(sup_output, sup_target);
            else
                costs[0] = transpose(lift_output(sigmoid(sup_output), sup_target));
            if(targetsize() == 1)
            {
                new_costs.resize(1);
                new_costs[0] = classification_loss(new_output, new_target);
            }
            else
            {
                new_costs.resize(costs.length());
                for(int i=0; i<new_costs.length(); i++)
                    new_costs[i] = costs[i];
            }
        }
        else PLERROR("LinearInductiveTransferClassifier::build_(): model_type \"%s\" invalid",model_type.c_str());


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

        buildPenalties();
        Var train_costs = hconcat(costs);
        test_costs = hconcat(new_costs);

        // Apply penalty to cost.
        // If there is no penalty, we still add costs[0] as the first cost, in
        // order to keep the same number of costs as if there was a penalty.
        if(penalties.size() != 0) {
            if (weightsize_>0)
                training_cost = hconcat(sampleweight*sum(hconcat(costs[0] & penalties))
                                        & (train_costs*sampleweight));
            else 
                training_cost = hconcat(sum(hconcat(costs[0] & penalties)) & train_costs);
        }
        else {
            if(weightsize_>0) {
                training_cost = hconcat(costs[0]*sampleweight & train_costs*sampleweight);
            } else {
                training_cost = hconcat(costs[0] & train_costs);
            }
        }

        training_cost->setName("training_cost");
        test_costs->setName("test_costs");


        if((bool)paramsvalues && (paramsvalues.size() == params.nelems()))
            params << paramsvalues;
        else
            paramsvalues.resize(params.nelems());
        params.makeSharedValue(paramsvalues);
        
        // Build functions.
        buildFuncs(input, output, target, sampleweight);
        
        // Reinitialize the optimization phase
        if(optimizer)
            optimizer->reset();
        stage = 0;        
    }
}


// ### Nothing to add here, simply calls build_
void LinearInductiveTransferClassifier::build()
{
    inherited::build();
    build_();
}


void LinearInductiveTransferClassifier::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(class_reps, copies);
    deepCopyField(optimizer, copies);
    deepCopyField(visible_layer, copies);
    deepCopyField(hidden_layer, copies);

    varDeepCopyField(input, copies);
    varDeepCopyField(output, copies);
    varDeepCopyField(sup_output, copies);
    varDeepCopyField(new_output, copies);
    varDeepCopyField(target, copies);
    varDeepCopyField(sup_target, copies);
    varDeepCopyField(new_target, copies);
    varDeepCopyField(sampleweight, copies);
    varDeepCopyField(A, copies);
    varDeepCopyField(s, copies);
    varDeepCopyField(class_reps_var, copies);

    deepCopyField(costs, copies);
    deepCopyField(new_costs, copies);
    deepCopyField(params, copies);
    deepCopyField(paramsvalues, copies);
    deepCopyField(penalties, copies);

    varDeepCopyField(training_cost, copies);
    varDeepCopyField(test_costs, copies);

    deepCopyField(invars, copies);
    deepCopyField(seen_targets, copies);
    deepCopyField(unseen_targets, copies);

    deepCopyField(f, copies);
    deepCopyField(test_costf, copies);
    deepCopyField(output_and_target_to_cost, copies);
    deepCopyField(sup_test_costf, copies);
    deepCopyField(sup_output_and_target_to_cost, copies);

    varDeepCopyField(W, copies);
    //deepCopyField(As, copies);
    //deepCopyField(Ws, copies);
    //deepCopyField(s_hids, copies);
    //deepCopyField(hidden_neurons, copies);

    //PLERROR("LinearInductiveTransferClassifier::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}


int LinearInductiveTransferClassifier::outputsize() const
{
    if(output)
        return output->size();
    else
        return 0;
}

void LinearInductiveTransferClassifier::forget()
{
    if(optimizer)
        optimizer->reset();
    stage = 0;
    
    if(model_type == "nnet_discriminative_1_vs_all")
    {
//        for(int i=0; i<Ws.length(); i++)
//        {
//            fillWeights(Ws[i],false,1./(inputsize_*class_reps.width()));
//            fillWeights(As[i],false,1./(nhidden*class_reps.width()));
//            s_hids[i]->value.fill(1);
//        }
        fillWeights(W,true);
        fillWeights(A,false,1./(nhidden*class_reps.width()));
        s->value.fill(1);
    }
    else
    {
        //A = Var(inputsize_,class_reps_to_use.width());
        A->value.fill(0);
        s->value.fill(1);
    }

    // Might need to recompute proppaths (if number of task representations changed
    // for instance)
    build();
}
    
void LinearInductiveTransferClassifier::train()
{
    if(!train_set)
        PLERROR("In DeepFeatureExtractor::train, you did not setTrainingSet");
    
    if(!train_stats)
        PLERROR("In DeepFeatureExtractor::train, you did not setTrainStatsCollector");

    int l = train_set->length();  

    if(f.isNull()) // Net has not been properly built yet (because build was called before the learner had a proper training set)
        build();
    
    if(rbm_nstages>0 && stage == 0 && nstages > 0 && model_type == "nnet_discriminative_1_vs_all")
    {
        if(!visible_layer)
            PLERROR("In LinearInductiveTransferClassifier::train(): "
                    "visible_layer must be provided.");
        if(!hidden_layer)
            PLERROR("In LinearInductiveTransferClassifier::train(): "
                    "hidden_layer must be provided.");

        Vec input, target;
        real example_weight;
        real recons = 0;
        RBMMatrixConnection* c = new RBMMatrixConnection();
        PP<RBMMatrixConnection> layer_matrix_connections = c;
        PP<RBMConnection> layer_connections = c;
        hidden_layer->size = nhidden;
        visible_layer->size = inputsize_;
        layer_connections->up_size = inputsize_;
        layer_connections->down_size = nhidden;
        
        hidden_layer->random_gen = random_gen;
        visible_layer->random_gen = random_gen;
        layer_connections->random_gen = random_gen;

        visible_layer->setLearningRate(rbm_learning_rate);
        hidden_layer->setLearningRate(rbm_learning_rate);
        layer_connections->setLearningRate(rbm_learning_rate);

        
        hidden_layer->build();
        visible_layer->build();
        layer_connections->build();
        
        Vec pos_visible,pos_hidden,neg_visible,neg_hidden;
        pos_visible.resize(inputsize_);
        pos_hidden.resize(nhidden);
        neg_visible.resize(inputsize_);
        neg_hidden.resize(nhidden);

        for(int i = 0; i < rbm_nstages; i++)
        {
            for(int j=0; j<train_set->length(); j++)
            {
                train_set->getExample(j,input,target,example_weight);

                pos_visible = input;
                layer_connections->setAsUpInput( input );
                hidden_layer->getAllActivations( layer_connections );
                hidden_layer->computeExpectation();
                hidden_layer->generateSample();
                pos_hidden << hidden_layer->expectation;            

                layer_connections->setAsDownInput( hidden_layer->sample );
                visible_layer->getAllActivations( layer_connections );
                visible_layer->computeExpectation();
                visible_layer->generateSample();
                neg_visible = visible_layer->sample;

                layer_connections->setAsUpInput( visible_layer->sample );
                hidden_layer->getAllActivations( layer_connections );
                hidden_layer->computeExpectation();
                neg_hidden = hidden_layer->expectation;

                // Compute reconstruction error
                layer_connections->setAsDownInput( pos_hidden );
                visible_layer->getAllActivations( layer_connections );
                visible_layer->computeExpectation();
                recons += visible_layer->fpropNLL(input);
                
                // Update
                visible_layer->update(pos_visible, neg_visible);
                hidden_layer->update(pos_hidden, neg_hidden);
                layer_connections->update(pos_hidden, pos_visible,
                                          neg_hidden, neg_visible);
            }
            if(verbosity > 2)
                cout << "Reconstruction error = " << recons/train_set->length() << endl;
            recons = 0;
        }
        W->matValue.subMat(1,0,inputsize_,nhidden) << layer_matrix_connections->weights;
        W->matValue(0) << hidden_layer->bias;
    }

    if(model_type == "discriminative" || model_type == "discriminative_1_vs_all" || model_type == "generative_0-1" || model_type == "nnet_discriminative_1_vs_all")
    {
        // number of samples seen by optimizer before each optimizer update
        int nsamples = batch_size>0 ? batch_size : l;
        Func paramf = Func(invars, training_cost); // parameterized function to optimize
        Var totalcost = meanOf(train_set, paramf, nsamples);
        if(optimizer)
        {
            optimizer->setToOptimize(params, totalcost);  
            optimizer->build();
        }
        else PLERROR("LinearInductiveTransferClassifier::train can't train without setting an optimizer first!");

        // number of optimizer stages corresponding to one learner stage (one epoch)
        int optstage_per_lstage = l/nsamples;

        PP<ProgressBar> pb;
        if(report_progress)
            pb = new ProgressBar("Training " + classname() + " from stage " + tostring(stage) + " to " + tostring(nstages), nstages-stage);

        int initial_stage = stage;
        bool early_stop=false;
        //displayFunction(paramf, true, false, 250);
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
    }
    else
    {
        Mat ww(class_reps_var->width(),class_reps_var->width()); ww.fill(0);
        Mat ww_inv(class_reps_var->width(),class_reps_var->width());
        Mat xw(inputsize(),class_reps_var->width()); xw.fill(0);
        Vec input, target;
        real weight;
        input.resize(train_set->inputsize());
        target.resize(train_set->targetsize());        
        for(int i=0; i<train_set->length(); i++)
        {
            train_set->getExample(i,input,target,weight);
            if(targetsize() == 1)
            {
                if(weightsize()>0)
                {
                    externalProductScaleAcc(ww,class_reps_var->matValue((int)target[0]),class_reps_var->matValue((int)target[0]),weight);
                    externalProductScaleAcc(xw,input,class_reps_var->matValue((int)target[0]),weight);
                }
                else
                {
                    externalProductAcc(ww,class_reps_var->matValue((int)target[0]),class_reps_var->matValue((int)target[0]));
                    externalProductAcc(xw,input,class_reps_var->matValue((int)target[0]));
                }
            }
            else
                for(int j=0; j<target.length(); j++)
                {
                    if(fast_exact_is_equal(target[j], 1))
                        if(weightsize()>0)
                        {
                            externalProductScaleAcc(ww,class_reps_var->matValue(j),class_reps_var->matValue(j),weight);
                            externalProductScaleAcc(xw,input,class_reps_var->matValue(j),weight);
                        }
                        else
                        {
                            externalProductAcc(ww,class_reps_var->matValue(j),class_reps_var->matValue(j));
                            externalProductAcc(xw,input,class_reps_var->matValue(j));
                        }   
                }
        }
        if(weight_decay > 0)
            for(int i=0; i<ww.length(); i++)
                ww(i,i) = ww(i,i) + weight_decay;
        matInvert(ww,ww_inv);
        A->value.fill(0);
        productAcc(A->matValue, xw, ww_inv);
        
        s->value.fill(0);
        Vec sample(s->size());
        Vec weights(inputsize());
        real sum = 0;
        for(int i=0; i<train_set->length(); i++)
        {
            train_set->getExample(i,input,target,weight);
            if(targetsize() == 1)
            {
                product(weights,A->matValue,class_reps_var->matValue((int)target[0]));
                if(weightsize()>0)
                {
                    diffSquareMultiplyAcc(s->value,weights,input,weight);
                    sum += weight;
                }
                else
                {
                    diffSquareMultiplyAcc(s->value,weights,input,real(1.0));
                    sum++;
                }
            }
            else
                for(int j=0; j<target.length(); j++)
                {
                    if(fast_exact_is_equal(target[j], 1))
                    {
                        product(weights,A->matValue,class_reps_var->matValue(j));
                        if(weightsize()>0)
                        {
                            diffSquareMultiplyAcc(s->value,weights,input,weight);
                            sum += weight;
                        }
                        else
                        {
                            diffSquareMultiplyAcc(s->value,weights,input,real(1.0));
                            sum++;
                        }
                    }
                }
        }
        s->value /= sum;
        s->value += sigma_min;

        if(verbosity > 2 && !multi_target_classifier)
        {
            Func paramf = Func(invars, training_cost);
            paramf->recomputeParents();
            real mean_cost = 0;
            Vec cost(2);
            Vec row(train_set->width());
            for(int i=0; i<train_set->length(); i++)
            {
                train_set->getRow(i,row);
                paramf->fprop(row.subVec(0,inputsize()+targetsize()),cost);
                mean_cost += cost[1];
            }
            mean_cost /= train_set->length();
            cout << "Train class error: " << mean_cost << endl;
        }
    }
    // Hugo: I don't know why we have to do this?!?
    output_and_target_to_cost->recomputeParents();
    test_costf->recomputeParents();
}

void LinearInductiveTransferClassifier::computeOutput(const Vec& input, Vec& output) const
{
    output.resize(outputsize());
    f->fprop(input,output);
}    

void LinearInductiveTransferClassifier::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                           const Vec& target, Vec& costs) const
{
    if(targetsize() != 1)
        costs.resize(costs.length()-1+targetsize());
    if(seen_targets.find(target[0])>=0)
        sup_output_and_target_to_cost->fprop(output&target, costs);
    else
        output_and_target_to_cost->fprop(output&target, costs);
    if(targetsize() != 1)
    {
        costs.resize(costs.length()+1);
        int i;
        for(i=0; i<target.length(); i++)
            if(!is_missing(target[i]))
                break;
        if(i>= target.length())
            PLERROR("In LinearInductiveTransferClassifier::computeCostsFromOutputs(): all targets are missing, can't compute cost");
        if(model_type == "generative")
            costs[costs.length()-1] = costs[i];
        else
            costs[costs.length()-1] = costs[i+1];
        costs[costs.length()-targetsize()-1] = costs[costs.length()-1];
        costs.resize(costs.length()-targetsize());
    }
}

void LinearInductiveTransferClassifier::computeOutputAndCosts(const Vec& inputv, const Vec& targetv, 
                                 Vec& outputv, Vec& costsv) const
{
    if(targetsize() != 1)
        costsv.resize(costsv.length()-1+targetsize());

    outputv.resize(outputsize());
    if(seen_targets.find(targetv[0])>=0)
        sup_test_costf->fprop(inputv&targetv, outputv&costsv);
    else
        test_costf->fprop(inputv&targetv, outputv&costsv);

    if(targetsize() != 1)
    {
        costsv.resize(costsv.length()+1);
        int i;
        for(i=0; i<targetv.length(); i++)
            if(!is_missing(targetv[i]))
                break;
        if(i>= targetv.length())
            PLERROR("In LinearInductiveTransferClassifier::computeCostsFromOutputs(): all targets are missing, can't compute cost");
        //for(int j=i+1; j<targetv.length(); j++)
        //    if(!is_missing(targetv[j]))
        //        PLERROR("In LinearInductiveTransferClassifier::computeCostsFromOutputs(): there should be only one non-missing target");
        //cout << "i=" << i << " ";
        if(model_type == "generative")
            costsv[costsv.length()-1] = costsv[i];
        else
            costsv[costsv.length()-1] = costsv[i+1];
        costsv[costsv.length()-targetsize()-1] = costsv[costsv.length()-1];
        costsv.resize(costsv.length()-targetsize());
    }
}

TVec<string> LinearInductiveTransferClassifier::getTestCostNames() const
{
    TVec<string> costs_str;
    if(model_type == "discriminative" || model_type == "discriminative_1_vs_all" || model_type == "generative_0-1" || model_type == "nnet_discriminative_1_vs_all")
    {
        if(model_type == "discriminative" || model_type == "generative_0-1")
        {
            costs_str.resize(2);
            costs_str[0] = "NLL";
            costs_str[1] = "class_error";
        }
        if(model_type == "discriminative_1_vs_all" 
           || model_type == "nnet_discriminative_1_vs_all")
        {
            costs_str.resize(1);
            costs_str[0] = "cross_entropy";
            if(!multi_target_classifier)
            {
                costs_str.resize(2);
                costs_str[1] = "class_error";
            }
            else
            {
                costs_str.resize(2);
                costs_str[1] = "lift_first";
            }
        }
    }
    else if(model_type == "generative")
    {
        if(!multi_target_classifier)
        {
            costs_str.resize(1);
            costs_str[0] = "class_error";
        }
        else
        {
            costs_str.resize(1);            
            costs_str[0] = "lift_first";
        }
    }
    return costs_str;
}

TVec<string> LinearInductiveTransferClassifier::getTrainCostNames() const
{
    return getTestCostNames();
}

void LinearInductiveTransferClassifier::buildTargetAndWeight() {
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

void LinearInductiveTransferClassifier::buildPenalties() {
    penalties.resize(0);  // prevents penalties from being added twice by consecutive builds
    if(weight_decay > 0)
    {
        if(model_type == "nnet_discriminative_1_vs_all")
        {
            //for(int i=0; i<Ws.length(); i++)
            //{
            //    penalties.append(affine_transform_weight_penalty(Ws[i], weight_decay, weight_decay, penalty_type));
            //}
            penalties.append(affine_transform_weight_penalty(W, weight_decay, 0, penalty_type));
        }
        
        penalties.append(affine_transform_weight_penalty(A, weight_decay, weight_decay, penalty_type));
    }
}

void LinearInductiveTransferClassifier::fillWeights(const Var& weights, 
                                                    bool zero_first_row, 
                                                    real scale_with_this) {
    if (initialization_method == "zero") {
        weights->value->clear();
        return;
    }
    real delta;
    if(scale_with_this < 0)
    {
        int is = weights.length();
        if (zero_first_row)
            is--; // -1 to get the same result as before.
        if (initialization_method.find("linear") != string::npos)
            delta = 1.0 / real(is);
        else
            delta = 1.0 / sqrt(real(is));
    }
    else
        delta = scale_with_this;

    if (initialization_method.find("normal") != string::npos)
        random_gen->fill_random_normal(weights->value, 0, delta);
    else
        random_gen->fill_random_uniform(weights->value, -delta, delta);
    if(zero_first_row)
        weights->matValue(0).clear();
}

void LinearInductiveTransferClassifier::buildFuncs(const Var& the_input, const Var& the_output, const Var& the_target, const Var& the_sampleweight){
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

    VarArray sup_outvars;
    sup_test_costf = Func(testinvars, the_output&hconcat(costs));
    sup_test_costf->recomputeParents();
    sup_output_and_target_to_cost = Func(outvars, hconcat(costs)); 
    sup_output_and_target_to_cost->recomputeParents();
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
