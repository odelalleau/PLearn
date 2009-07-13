// -*- C++ -*-

// AdaBoost.cc
//
// Copyright (C) 2003  Pascal Vincent 
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

// Authors: Yoshua Bengio

/*! \file AdaBoost.cc */

#include "AdaBoost.h"
#include <plearn/math/pl_math.h>
#include <plearn/vmat/ConcatColumnsVMatrix.h>
#include <plearn/vmat/SelectRowsVMatrix.h>
#include <plearn/vmat/MemoryVMatrix.h>
#include <plearn/math/random.h>
#include <plearn/io/load_and_save.h>
#include <plearn/base/stringutils.h>
#include <plearn_learners/regressors/RegressionTreeRegisters.h>
#define PL_LOG_MODULE_NAME "AdaBoost"
#include <plearn/io/pl_log.h>

namespace PLearn {
using namespace std;

AdaBoost::AdaBoost()
    : sum_voting_weights(0.0), 
      initial_sum_weights(0.0),
      found_zero_error_weak_learner(0),
      target_error(0.5), 
      provide_learner_expdir(false),
      output_threshold(0.5), 
      compute_training_error(1), 
      pseudo_loss_adaboost(1), 
      conf_rated_adaboost(0), 
      weight_by_resampling(1), 
      early_stopping(1),
      save_often(0),
      forward_sub_learner_test_costs(false),
      modif_train_set_weights(false),
      reuse_test_results(false)
{ }

PLEARN_IMPLEMENT_OBJECT(
    AdaBoost,
    "AdaBoost boosting algorithm for TWO-CLASS classification",
    "Given a classification weak-learner, this algorithm \"boosts\" it in\n"
    "order to obtain a much more powerful classification algorithm.\n"
    "The classifier is two-class, returning 0 or 1, or a number between 0 and 1.\n"
    "In the latter case, the user can use two different versions of AdaBoost:\n"
    " - \"Pseudo-loss\" AdaBoost:    see \"Experiments with a New Boosting \n"
    "                                  Algorithm\" by Freund and Schapire.\n"
    "                                  Set the 'pseudo_loss_adaboost' option\n"
    "                                  to select this version\n"
    "\n"
    " - \"Confidence-rated\" AdaBoost: see \"Improved Boosting Algorithms Using\n"
    "                                Confidence-rated Predictions\" by\n"
    "                                Schapire and Singer.\n"
    "                                Set the 'conf_rated_adaboost' option\n"
    "                                to select this version.\n"
    "These versions compute a more precise notion of error, taking into \n"
    "account the precise value outputted by the soft classifier.\n"
    "Also, \"Confidence-rated\" AdaBoost uses a line search at each stage to\n"
    "compute the weight of the trained weak learner.\n\n"
    "It should be noted that, except for the optimization of the weak learners,\n"
    "\"Confidence-rated\" AdaBoost is equivalent to MarginBoost (see \n"
    "\"Functional Gradient Techniques for Combining Hypotheses\" by \n"
    "Mason et al.) when using the exponential loss on the margin. Hence, the\n"
    "'conf_rated_adaboost' option can be used in that case too, and all that\n"
    "needs to be adjusted is the choice of weak learners.\n\n"
    "The nstages option from PLearner is used to specify the desired\n"
    "number of boosting rounds (but the algorithm can stop earlier if\n"
    "the next weak learner is unable to make significant progress or if\n"
    "the weak learner has 0 error on the training set).\n");

void AdaBoost::declareOptions(OptionList& ol)
{
    declareOption(ol, "weak_learners", &AdaBoost::weak_learners,
                  OptionBase::learntoption,
                  "The vector of learned weak learners");

    declareOption(ol, "voting_weights", &AdaBoost::voting_weights,
                  OptionBase::learntoption,
                  "Weights given to the weak learners (their output is\n"
                  "linearly combined with these weights\n"
                  "to form the output of the AdaBoost learner).\n");

    declareOption(ol, "sum_voting_weights", &AdaBoost::sum_voting_weights,
                  OptionBase::learntoption,
                  "Sum of the weak learners voting weights.\n");
  
    declareOption(ol, "initial_sum_weights", &AdaBoost::initial_sum_weights,
                  OptionBase::learntoption,
                  "Initial sum of weights on the examples. Do not temper with.\n");

    declareOption(ol, "example_weights", &AdaBoost::example_weights,
                  OptionBase::learntoption,
                  "The current weights of the examples.\n");

    declareOption(ol, "learners_error", &AdaBoost::learners_error,
                  OptionBase::learntoption,
                  "The error of each learners.\n");

    declareOption(ol, "weak_learner_template", &AdaBoost::weak_learner_template,
                  OptionBase::buildoption,
                  "Template for the regression weak learner to be"
                  "boosted into a classifier");

    declareOption(ol, "target_error", &AdaBoost::target_error,
                  OptionBase::buildoption,
                  "This is the target average weighted error below"
                  "which each weak learner\n"
                  "must reach after its training (ordinary adaboost:"
                  "target_error=0.5).");

    declareOption(ol, "pseudo_loss_adaboost", &AdaBoost::pseudo_loss_adaboost,
                  OptionBase::buildoption,
                  "Whether to use Pseudo-loss Adaboost (see \"Experiments with\n"
                  "a New Boosting Algorithm\" by Freund and Schapire), which\n"
                  "takes into account the precise value outputted by\n"
                  "the soft classifier.");

    declareOption(ol, "conf_rated_adaboost", &AdaBoost::conf_rated_adaboost,
                  OptionBase::buildoption,
                  "Whether to use Confidence-rated AdaBoost (see \"Improved\n"
                  "Boosting Algorithms Using Confidence-rated Predictions\" by\n"
                  "Schapire and Singer) which takes into account the precise\n"
                  "value outputted by the soft classifier. It also searchs\n"
                  "the weight of a weak learner using a line search according\n"
                  "to a criteria which is more appropriate for soft classifiers.\n"
                  "This option can also be used to obtain MarginBoost with the\n"
                  "exponential loss, provided that an appropriate choice of\n"
                  "weak learner is made by the user (see \"Functional Gradient\n"
                  "Techniques for Combining Hypotheses\" by Mason et al.).\n");

    declareOption(ol, "weight_by_resampling", &AdaBoost::weight_by_resampling,
                  OptionBase::buildoption,
                  "Whether to train the weak learner using resampling"
                  " to represent the weighting\n"
                  "given to examples. If false then give these weights "
                  "explicitly in the training set\n"
                  "of the weak learner (note that some learners can accomodate "
                  "weights well, others not).\n");

    declareOption(ol, "output_threshold", &AdaBoost::output_threshold,
                  OptionBase::buildoption,
                  "To interpret the output of the learner as a class, it is "
                  "compared to this\n"
                  "threshold: class 1 if greater than output_threshold, class "
                  "0 otherwise.\n");

    declareOption(ol, "provide_learner_expdir", &AdaBoost::provide_learner_expdir,
                  OptionBase::buildoption,
                  "If true, each weak learner to be trained will have its\n"
                  "experiment directory set to WeakLearner#kExpdir/");

    declareOption(ol, "early_stopping", &AdaBoost::early_stopping, 
                  OptionBase::buildoption,
                  "If true, then boosting stops when the next weak learner\n"
                  "is too weak (avg error > target_error - .01)\n");

    declareOption(ol, "save_often", &AdaBoost::save_often, 
                  OptionBase::buildoption,
                  "If true, then save the model after training each weak\n"
                  "learner, under <expdir>/model.psave\n");

    declareOption(ol, "compute_training_error", 
                  &AdaBoost::compute_training_error, OptionBase::buildoption,
                  "Whether to compute training error at each stage.\n");

    declareOption(ol, "forward_sub_learner_test_costs", 
                  &AdaBoost::forward_sub_learner_test_costs, OptionBase::buildoption,
                  "Did we add the sub_learner_costs to our costs.\n");

    declareOption(ol, "modif_train_set_weights", 
                  &AdaBoost::modif_train_set_weights, OptionBase::buildoption,
                  "Did we modif directly the train_set weights?\n");

    declareOption(ol, "found_zero_error_weak_learner", 
                  &AdaBoost::found_zero_error_weak_learner, 
                  OptionBase::learntoption,
                  "Indication that a weak learner with 0 training error"
                  "has been found.\n");

    declareOption(ol, "weak_learner_output",
                  &AdaBoost::weak_learner_output,
                  OptionBase::nosave,
                  "A temp vector that contain the weak learner output\n");

    declareOption(ol, "reuse_test_results",
                  &AdaBoost::reuse_test_results,
                  OptionBase::buildoption,
                  "If true we save and reuse previous call to test(). This is"
                  " usefull to have a test time that is independent of the"
                  " number of adaboost itaration.\n");

     declareOption(ol, "saved_testset",
                  &AdaBoost::saved_testset,
                  OptionBase::nosave,
                  "Used with reuse_test_results\n");

     declareOption(ol, "saved_testoutputs",
                  &AdaBoost::saved_testoutputs,
                  OptionBase::nosave,
                  "Used with reuse_test_results\n");

     declareOption(ol, "saved_last_test_stages",
                  &AdaBoost::saved_last_test_stages,
                  OptionBase::nosave,
                  "Used with reuse_test_results\n");

   // Now call the parent class' declareOptions
    inherited::declareOptions(ol);

    declareOption(ol, "train_set",
                  &AdaBoost::train_set,
                  OptionBase::learntoption|OptionBase::nosave,
                  "The training set, so we can reload it.\n");

}

void AdaBoost::build_()
{
    if(conf_rated_adaboost && pseudo_loss_adaboost)
        PLERROR("In Adaboost:build_(): conf_rated_adaboost and pseudo_loss_adaboost cannot both be true, a choice must be made");

    
    int n = 0;
//why we don't always use weak_learner_template?
    if(weak_learners.size()>0)
        n=weak_learners[0]->outputsize();
    else if(weak_learner_template)
        n=weak_learner_template->outputsize();
    weak_learner_output.resize(n);
    
    //for RegressionTreeNode
    if(getTrainingSet())
        setTrainingSet(getTrainingSet(),false);
}

///////////
// build //
///////////
void AdaBoost::build()
{
    inherited::build();
    build_();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void AdaBoost::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(weighted_costs,           copies);
    deepCopyField(sum_weighted_costs,       copies);
    deepCopyField(saved_testset,            copies);
    deepCopyField(saved_testoutputs,        copies);
    deepCopyField(saved_last_test_stages,   copies);

    deepCopyField(learners_error,           copies);
    deepCopyField(example_weights,          copies);
    deepCopyField(weak_learner_output,      copies);
    deepCopyField(voting_weights,           copies);
    deepCopyField(weak_learners,            copies);
    deepCopyField(weak_learner_template,    copies);
}

////////////////
// outputsize //
////////////////
int AdaBoost::outputsize() const
{
    // Outputsize is always 2, since this is a 0-1 classifier
    // and we append the weighted sum to allow the reuse of previous test
    if(reuse_test_results)
        return 2;
    else 
        return 1;
}

void AdaBoost::finalize()
{
    inherited::finalize();
    for(int i=0;i<weak_learners.size();i++){
        weak_learners[i]->finalize();
    }
    if(train_set && train_set->classname()=="RegressionTreeRegisters")
        ((PP<RegressionTreeRegisters>)train_set)->finalize();
}

void AdaBoost::forget()
{
    stage = 0;
    learners_error.resize(0, nstages);
    weak_learners.resize(0, nstages);
    voting_weights.resize(0, nstages);
    sum_voting_weights = 0;
    found_zero_error_weak_learner=false;
    if (seed_ >= 0)
        manual_seed(seed_);
    else
        PLearn::seed();
}

void AdaBoost::train()
{

    if(nstages==stage)
        return;
    else if (nstages < stage){        //!< Asking to revert to previous stage
        PLCHECK(nstages>0); // should use forget
        NORMAL_LOG<<"In AdaBoost::train() - reverting from stage "<<stage
                  <<" to stage "<<nstages<<endl;
        stage = nstages;
        PLCHECK(learners_error.size()>=stage);
        PLCHECK(weak_learners.size()>=stage);
        PLCHECK(voting_weights.size()>=stage);
        PLCHECK(nstages>0);
        learners_error.resize(stage);
        weak_learners.resize(stage);
        voting_weights.resize(stage);
        sum_voting_weights = sum(voting_weights);
        found_zero_error_weak_learner=false;

        example_weights.resize(0);
        return;
        //need examples_weights
        //computeTrainingError();

    }else if(nstages>0 && stage>0 && example_weights.size()==0){
        PLERROR("In AdaBoost::train() -  we can't retrain a reverted learner...");
    }
    
    if(found_zero_error_weak_learner) // Training is over...
        return;

    Profiler::pl_profile_start("AdaBoost::train");

    if(!train_set)
        PLERROR("In AdaBoost::train, you did not setTrainingSet");
    
    if(!train_stats && compute_training_error)
        PLERROR("In AdaBoost::train, you did not setTrainStatsCollector");

    if (train_set->targetsize()!=1)
        PLERROR("In AdaBoost::train, targetsize should be 1, found %d", 
                train_set->targetsize());

    if(modif_train_set_weights && train_set->weightsize()!=1)
        PLERROR("In AdaBoost::train, when modif_train_set_weights is true"
                " the weightsize of the trainset must be one.");
    
    PLCHECK_MSG(train_set->inputsize()>0, "In AdaBoost::train, the inputsize"
                " of the train_set must be know.");


    Vec input;
    Vec output;
    Vec target;
    real weight;

    Vec examples_error;

    const int n = train_set.length();
    TVec<int> train_indices;
    Vec pseudo_loss;

    input.resize(inputsize());
    output.resize(weak_learner_template->outputsize());// We use only the first one as the output from the weak learner
    target.resize(targetsize());
    examples_error.resize(n);

    if (stage==0)
    {
        example_weights.resize(n);
        if (train_set->weightsize()>0)
        {
            PP<ProgressBar> pb;
            initial_sum_weights=0;
            int weight_col = train_set->inputsize()+train_set->targetsize();
            for (int i=0; i<n; ++i) {
                weight=train_set->get(i,weight_col);
                example_weights[i]=weight;
                initial_sum_weights += weight;
            }
            example_weights *= real(1.0)/initial_sum_weights;
        }
        else 
        {
            example_weights.fill(1.0/n);
            initial_sum_weights = 1;
        }
        sum_voting_weights = 0;
        voting_weights.resize(0,nstages);

    } else
        PLCHECK_MSG(example_weights.length()==n,"In AdaBoost::train - the train"
                    " set should not change between each train without a forget!");

    VMat unweighted_data = train_set.subMatColumns(0, inputsize()+1);
    learners_error.resize(nstages);

    for ( ; stage < nstages ; ++stage)
    {
        VMat weak_learner_training_set;
        { 
            // We shall now construct a training set for the new weak learner:
            if (weight_by_resampling)
            {
                PP<ProgressBar> pb;
                if(report_progress) pb = new ProgressBar(
                    "AdaBoost round " + tostring(stage) +
                    ": making training set for weak learner", n);

                // use a "smart" resampling that approximated sampling 
                // with replacement with the probabilities given by 
                // example_weights.
                map<real,int> indices;
                for (int i=0; i<n; ++i) {
                    if(report_progress) pb->update(i);
                    real p_i = example_weights[i];
                    // randomly choose how many repeats of example i
                    int n_samples_of_row_i = 
                        int(rint(gaussian_mu_sigma(n*p_i,sqrt(n*p_i*(1-p_i))))); 
                    for (int j=0;j<n_samples_of_row_i;j++)
                    {
                        if (j==0)
                            indices[i]=i;
                        else
                        {
                            // put the others in random places
                            real k=n*uniform_sample(); 
                            // while avoiding collisions
                            indices[k]=i; 
                        }
                    }
                }
                train_indices.resize(0,n);
                map<real,int>::iterator it = indices.begin();
                map<real,int>::iterator last = indices.end();
                for (;it!=last;++it)
                    train_indices.push_back(it->second);
                weak_learner_training_set = 
                    new SelectRowsVMatrix(unweighted_data, train_indices);
                weak_learner_training_set->defineSizes(inputsize(), 1, 0);
            }
            else if(modif_train_set_weights)
            {
                //No Need for deep copy of the sorted_train_set as after the train it is not used anymore
                // and the data are not modofied, but we need to change the weight
                weak_learner_training_set = train_set;
                int weight_col=train_set->inputsize()+train_set->targetsize();
                for(int i=0;i<train_set->length();i++)
                    train_set->put(i,weight_col,example_weights[i]);
            }
            else
            {
                Mat data_weights_column = example_weights.toMat(n,1).copy();
                // to bring the weights to the same average level as 
                // the original ones
                data_weights_column *= initial_sum_weights; 
                VMat data_weights = VMat(data_weights_column);
                weak_learner_training_set = 
                    new ConcatColumnsVMatrix(unweighted_data,data_weights);
                weak_learner_training_set->defineSizes(inputsize(), 1, 1);
            }
        }

        // Create new weak-learner and train it
        PP<PLearner> new_weak_learner = ::PLearn::deepCopy(weak_learner_template);
        new_weak_learner->setTrainingSet(weak_learner_training_set);
        new_weak_learner->setTrainStatsCollector(new VecStatsCollector);
        if(expdir!="" && provide_learner_expdir)
            new_weak_learner->setExperimentDirectory( expdir / ("WeakLearner"+tostring(stage)+"Expdir") );

        new_weak_learner->train();
        new_weak_learner->finalize();

        // calculate its weighted training error 
        {
            PP<ProgressBar> pb;
            if(report_progress && verbosity >1) pb = new ProgressBar("computing weighted training error of weak learner",n);
            learners_error[stage] = 0;
            for (int i=0; i<n; ++i) {
                if(pb) pb->update(i);
                train_set->getExample(i, input, target, weight);
#ifdef BOUNDCHECK
                if(!(is_equal(target[0],0)||is_equal(target[0],1)))
                    PLERROR("In AdaBoost::train() - target is %f in the training set. It should be 0 or 1 as we implement only two class boosting.",target[0]);
#endif
                new_weak_learner->computeOutput(input,output);
                real y_i=target[0];
                real f_i=output[0];
                if(conf_rated_adaboost)
                {
                    PLASSERT_MSG(f_i>=0,"In AdaBoost.cc::train() - output[0] should be >= 0 ");
                    // an error between 0 and 1 (before weighting)
                    examples_error[i] = 2*(f_i+y_i-2*f_i*y_i);
                    learners_error[stage] += example_weights[i]*
                        examples_error[i]/2;
                }
                else
                {
                    // an error between 0 and 1 (before weighting)
                    if (pseudo_loss_adaboost) 
                    {
                        PLASSERT_MSG(f_i>=0,"In AdaBoost.cc::train() - output[0] should be >= 0 ");
                        examples_error[i] = 2*(f_i+y_i-2*f_i*y_i);
                        learners_error[stage] += example_weights[i]*
                            examples_error[i]/2;
                    }
                    else
                    {
                        if (fast_exact_is_equal(y_i, 1))
                        {
                            if (f_i<output_threshold)
                            {
                                learners_error[stage] += example_weights[i];
                                examples_error[i]=2;
                            }
                            else examples_error[i] = 0;
                        }
                        else
                        {
                            if (f_i>=output_threshold) {
                                learners_error[stage] += example_weights[i];
                                examples_error[i]=2;
                            }
                            else examples_error[i]=0;
                        }
                    }
                }
            }
        }

        if (verbosity>1)
            NORMAL_LOG << "weak learner at stage " << stage 
                       << " has average loss = " << learners_error[stage] << endl;

        weak_learners.push_back(new_weak_learner);

        if (save_often && expdir!="")
            PLearn::save(append_slash(expdir)+"model.psave", *this);
      
        // compute the new learner's weight
        if(conf_rated_adaboost)
        {
            // Find optimal weight with line search
      
            real ax = -10;
            real bx = 1;
            real cx = 100;
            real xmin;
            real tolerance = 0.001;
            int itmax = 100000;

            int iter;
            real xtmp;
            real fa, fb, fc, ftmp;

            // compute function for fa, fb and fc

            fa = 0;
            fb = 0;
            fc = 0;

            for (int i=0; i<n; ++i) {
                train_set->getExample(i, input, target, weight);
                new_weak_learner->computeOutput(input,output);
                real y_i=(2*target[0]-1);
                real f_i=(2*output[0]-1);
                fa += example_weights[i]*exp(-1*ax*f_i*y_i);
                fb += example_weights[i]*exp(-1*bx*f_i*y_i);
                fc += example_weights[i]*exp(-1*cx*f_i*y_i);
            }

        
            for(iter=1;iter<=itmax;iter++)
            {
                if(verbosity>4)
                    NORMAL_LOG << "iteration " << iter << ": fx = " << fb << endl;
                if (abs(cx-ax) <= tolerance)
                {
                    xmin=bx;
                    if(verbosity>3)
                    {
                        NORMAL_LOG << "nIters for minimum: " << iter << endl;
                        NORMAL_LOG << "xmin = " << xmin << endl;
                        NORMAL_LOG << "fx = " << fb << endl;
                    }
                    break;
                }
                if (abs(bx-ax) > abs(bx-cx)) 
                {
                    xtmp = (bx + ax) * 0.5;

                    ftmp = 0;
                    for (int i=0; i<n; ++i) {
                        train_set->getExample(i, input, target, weight);
                        new_weak_learner->computeOutput(input,output);
                        real y_i=(2*target[0]-1);
                        real f_i=(2*output[0]-1);
                        ftmp += example_weights[i]*exp(-1*xtmp*f_i*y_i);
                    }

                    if (ftmp > fb)
                    {
                        ax = xtmp;
                        fa = ftmp;
                    }
                    else
                    {
                        cx = bx;
                        fc = fb;
                        bx = xtmp;
                        fb = ftmp;
                    }
                }
                else
                {
                    xtmp = (bx + cx) * 0.5;
                    ftmp = 0;
                    for (int i=0; i<n; ++i) {
                        train_set->getExample(i, input, target, weight);
                        new_weak_learner->computeOutput(input,output);
                        real y_i=(2*target[0]-1);
                        real f_i=(2*output[0]-1);
                        ftmp += example_weights[i]*exp(-1*xtmp*f_i*y_i);
                    }

                    if (ftmp > fb)
                    {
                        cx = xtmp;
                        fc = ftmp;
                    }
                    else
                    {
                        ax = bx;
                        fa = fb;
                        bx = xtmp;
                        fb = ftmp;
                    }
                }
            }
            if(verbosity>3)
            {
                NORMAL_LOG << "Too many iterations in Brent" << endl;
            }
            xmin=bx;
            voting_weights.push_back(xmin);
            sum_voting_weights += abs(voting_weights[stage]);
        }
        else
        {
            voting_weights.push_back(
                0.5*safeflog(((1-learners_error[stage])*target_error)
                             /(learners_error[stage]*(1-target_error))));
            sum_voting_weights += abs(voting_weights[stage]);
        }

        real sum_w=0;
        for (int i=0;i<n;i++)
        {
            example_weights[i] *= exp(-voting_weights[stage]*
                                      (1-examples_error[i]));
            sum_w += example_weights[i];
        }
        example_weights *= real(1.0)/sum_w;

        computeTrainingError(input, target);

        if(fast_exact_is_equal(learners_error[stage], 0))
        {
            NORMAL_LOG << "AdaBoost::train found weak learner with 0 training "
                       << "error at stage " 
                       << stage << " is " << learners_error[stage] << endl;  

            // Simulate infinite weight on new_weak_learner
            weak_learners.resize(0);
            weak_learners.push_back(new_weak_learner);
            voting_weights.resize(0);
            voting_weights.push_back(1);
            sum_voting_weights = 1;
            found_zero_error_weak_learner = true;
            stage++;
            break;
        }

        // stopping criterion (in addition to n_stages)
        if (early_stopping && learners_error[stage] >= target_error)
        {
            nstages = stage;
            NORMAL_LOG << 
                "AdaBoost::train early stopping because learner's loss at stage " 
                 << stage << " is " << learners_error[stage] << endl;       
            break;
        }


    }
    PLCHECK(stage==weak_learners.length() || found_zero_error_weak_learner);
    Profiler::pl_profile_end("AdaBoost::train");

}

void AdaBoost::test(VMat testset, PP<VecStatsCollector> test_stats,
                    VMat testoutputs, VMat testcosts) const
{
    if(!reuse_test_results){
        inherited::test(testset, test_stats, testoutputs, testcosts);
        return;
    }
    Profiler::pl_profile_start("AdaBoost::test()");
    int index=-1;
    for(int i=0;i<saved_testset.size();i++){
        if(saved_testset[i]==testset){
            index=i;
            break;
        }
    }
    if(index<0){
        //first time the testset is seen
        Profiler::pl_profile_start("AdaBoost::test() first" );
        inherited::test(testset, test_stats, testoutputs, testcosts);
        saved_testset.append(testset);
        saved_testoutputs.append(PLearn::deepCopy(testoutputs));
        PLCHECK(weak_learners.length()==stage || found_zero_error_weak_learner);
        cout << weak_learners.length()<<" "<<stage<<endl;;
        saved_last_test_stages.append(stage);
        Profiler::pl_profile_end("AdaBoost::test() first" );
    }else if(found_zero_error_weak_learner && saved_last_test_stages.last()==stage){
        Vec input;
        Vec output(outputsize());
        Vec target;
        Vec costs(nTestCosts());
        real weight;
        VMat old_outputs=saved_testoutputs[index];
        PLCHECK(old_outputs->width()==testoutputs->width());
        PLCHECK(old_outputs->length()==testset->length());
        for(int row=0;row<testset.length();row++){
            output=old_outputs(row);
            testset.getExample(row, input, target, weight);
            computeCostsFromOutputs(input,output,target,costs);
            if(testoutputs)testoutputs->putOrAppendRow(row,output);
            if(testcosts)testcosts->putOrAppendRow(row,costs);
            if(test_stats)test_stats->update(costs,weight);
        }
    }else{
        Profiler::pl_profile_start("AdaBoost::test() seconds" );
        PLCHECK(weak_learners.size()>1);
        PLCHECK(stage>1);
        PLCHECK(weak_learner_output.size()==weak_learner_template->outputsize());

        PLCHECK(saved_testset.length()>index);
        PLCHECK(saved_testoutputs.length()>index);
        PLCHECK(saved_last_test_stages.length()>index);

        int stages_done = saved_last_test_stages[index];
        PLCHECK(weak_learners.size()>=stages_done);
         
        Vec input;
        Vec output(outputsize());
        Vec target;
        Vec costs(nTestCosts());
        real weight;
        VMat old_outputs=saved_testoutputs[index];
        PLCHECK(old_outputs->width()==testoutputs->width());
        PLCHECK(old_outputs->length()==testset->length());
#ifndef NDEBUG
        Vec output2(outputsize());
        Vec costs2(nTestCosts());
#endif
        for(int row=0;row<testset.length();row++){
            output=old_outputs(row);
            //compute the new testoutputs
            Profiler::pl_profile_start("AdaBoost::test() getExample" );
            testset.getExample(row, input, target, weight);
            Profiler::pl_profile_end("AdaBoost::test() getExample" );
            computeOutput_(input, output, stages_done, output[1]);
            computeCostsFromOutputs(input,output,target,costs);
#ifndef NDEBUG
            computeOutputAndCosts(input,target, output2, costs2);
            PLCHECK(output==output2);
            PLCHECK(costs.isEqual(costs2,true));
#endif
            if(testoutputs)testoutputs->putOrAppendRow(row,output);
            if(testcosts)testcosts->putOrAppendRow(row,costs);
            if(test_stats)test_stats->update(costs,weight);
        }
        saved_testoutputs[index]=PLearn::deepCopy(testoutputs);
        saved_last_test_stages[index]=stage;
        Profiler::pl_profile_end("AdaBoost::test() seconds" );
    }
    Profiler::pl_profile_end("AdaBoost::test()");
}

void AdaBoost::computeOutput_(const Vec& input, Vec& output,
                              const int start, real const sum) const
{
    PLASSERT(weak_learners.size()>0);
    PLASSERT(weak_learner_output.size()==weak_learner_template->outputsize());
    PLASSERT(output.size()==outputsize());
    real sum_out=sum;
    if(!pseudo_loss_adaboost && !conf_rated_adaboost)
        for (int i=start;i<weak_learners.size();i++){
            weak_learners[i]->computeOutput(input,weak_learner_output);
            sum_out += (weak_learner_output[0] < output_threshold ? 0 : 1) 
                *voting_weights[i];
        }
    else
        for (int i=start;i<weak_learners.size();i++){
            weak_learners[i]->computeOutput(input,weak_learner_output);
            sum_out += weak_learner_output[0]*voting_weights[i];
        }

    output[0] = sum_out/sum_voting_weights;
    if(reuse_test_results)
        output[1] = sum_out;
}

void AdaBoost::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                       const Vec& target, Vec& costs) const
{
    //when computing train stats, costs==nTrainCosts() 
    //  and forward_sub_learner_test_costs==false
    if(forward_sub_learner_test_costs)
        PLASSERT(costs.size()==nTestCosts());
    else
        PLASSERT(costs.size()==nTrainCosts()||costs.size()==nTestCosts());
    costs.resize(5);

    // First cost is negative log-likelihood...  output[0] is the likelihood
    // of the first class
#ifdef BOUNDCHECK
    if (target.size() > 1)
        PLERROR("AdaBoost::computeCostsFromOutputs: target must contain "
                "one element only: the 0/1 class");
#endif
    if (fast_exact_is_equal(target[0], 0)) {
        costs[0] = output[0] >= output_threshold; 
    }
    else if (fast_exact_is_equal(target[0], 1)) {
        costs[0] = output[0] < output_threshold; 
    }
    else PLERROR("AdaBoost::computeCostsFromOutputs: target must be "
                 "either 0 or 1; current target=%f", target[0]);
    costs[1] = exp(-1.0*sum_voting_weights*(2*output[0]-1)*(2*target[0]-1));
    costs[2] = costs[0];
    if(train_stats){
        costs[3] = train_stats->getStat("E[avg_weight_class_0]");
        costs[4] = train_stats->getStat("E[avg_weight_class_1]");
    }
    else
        costs[3]=costs[4]=MISSING_VALUE;

    if(forward_sub_learner_test_costs){
        //slow as we already have calculated the output
        //we should haved called computeOutputAndCosts.
        PLWARNING("AdaBoost::computeCostsFromOutputs called with forward_sub_learner_test_costs true. This should be optimized!");
        weighted_costs.resize(weak_learner_template->nTestCosts());
        sum_weighted_costs.resize(weak_learner_template->nTestCosts());
        sum_weighted_costs.clear();
        for(int i=0;i<weak_learners.size();i++){
            weak_learners[i]->computeCostsOnly(input, target, weighted_costs);
            weighted_costs*=voting_weights[i];
            sum_weighted_costs+=weighted_costs;
        }
        costs.append(sum_weighted_costs);
    }

    PLASSERT(costs.size()==nTrainCosts()||costs.size()==nTestCosts());
}

void AdaBoost::computeOutputAndCosts(const Vec& input, const Vec& target,
                                     Vec& output, Vec& costs) const
{
    PLASSERT(weak_learners.size()>0);
    PLASSERT(weak_learner_output.size()==weak_learner_template->outputsize());
    PLASSERT(output.size()==outputsize());
    real sum_out=0;
    
    if(forward_sub_learner_test_costs){
        weighted_costs.resize(weak_learner_template->nTestCosts());
        sum_weighted_costs.resize(weak_learner_template->nTestCosts());
        sum_weighted_costs.clear();
        if(!pseudo_loss_adaboost && !conf_rated_adaboost){
            for (int i=0;i<weak_learners.size();i++){
                weak_learners[i]->computeOutputAndCosts(input,target,
                                                        weak_learner_output,
                                                        weighted_costs);
                sum_out += (weak_learner_output[0] < output_threshold ? 0 : 1) 
                    *voting_weights[i];
                weighted_costs*=voting_weights[i];
                sum_weighted_costs+=weighted_costs;
            }
        }else{
            for (int i=0;i<weak_learners.size();i++){
                weak_learners[i]->computeOutputAndCosts(input,target,
                                                        weak_learner_output,
                                                        weighted_costs);
                sum_out += weak_learner_output[0]*voting_weights[i];
                weighted_costs*=voting_weights[i];
                sum_weighted_costs+=weighted_costs;
            }
        }
    }else{
        if(!pseudo_loss_adaboost && !conf_rated_adaboost)
            for (int i=0;i<weak_learners.size();i++){
                weak_learners[i]->computeOutput(input,weak_learner_output);
                sum_out += (weak_learner_output[0] < output_threshold ? 0 : 1) 
                    *voting_weights[i];
            }
        else
            for (int i=0;i<weak_learners.size();i++){
                weak_learners[i]->computeOutput(input,weak_learner_output);
                sum_out += weak_learner_output[0]*voting_weights[i];
            }
    }

    output[0] = sum_out/sum_voting_weights;
    if(reuse_test_results)
        output[1] = sum_out;

    //when computing train stats, costs==nTrainCosts() 
    //  and forward_sub_learner_test_costs==false
    if(forward_sub_learner_test_costs)
        PLASSERT(costs.size()==nTestCosts());
    else
        PLASSERT(costs.size()==nTrainCosts()||costs.size()==nTestCosts());
    costs.resize(5);
    costs.clear();

    // First cost is negative log-likelihood...  output[0] is the likelihood
    // of the first class
    if (target.size() > 1)
        PLERROR("AdaBoost::computeCostsFromOutputs: target must contain "
                "one element only: the 0/1 class");
    if (fast_exact_is_equal(target[0], 0)) {
        costs[0] = output[0] >= output_threshold; 
    }
    else if (fast_exact_is_equal(target[0], 1)) {
        costs[0] = output[0] < output_threshold; 
    }
    else PLERROR("AdaBoost::computeCostsFromOutputs: target must be "
                 "either 0 or 1; current target=%f", target[0]);
    costs[1] = exp(-1.0*sum_voting_weights*(2*output[0]-1)*(2*target[0]-1));
    costs[2] = costs[0];
    if(train_stats){
        costs[3] = train_stats->getStat("E[avg_weight_class_0]");
        costs[4] = train_stats->getStat("E[avg_weight_class_1]");
    }
    else
        costs[3]=costs[4]=MISSING_VALUE;

    if(forward_sub_learner_test_costs){
        costs.append(sum_weighted_costs);
    }

    PLASSERT(costs.size()==nTrainCosts()||costs.size()==nTestCosts());
}


TVec<string> AdaBoost::getTestCostNames() const
{
    TVec<string> costs=getTrainCostNames();

    if(forward_sub_learner_test_costs){
        TVec<string> subcosts;
        //We try to find a weak_learner with a train set
        //as a RegressionTree need it to generate the test costs names
        if(weak_learner_template->getTrainingSet())
            subcosts=weak_learner_template->getTestCostNames();
        else if(weak_learners.length()>0)
            subcosts=weak_learners[0]->getTestCostNames();
        else
            subcosts=weak_learner_template->getTestCostNames();
        for(int i=0;i<subcosts.length();i++){
            subcosts[i]="weighted_weak_learner."+subcosts[i];
        }
        costs.append(subcosts);
    }
    return costs;
}

TVec<string> AdaBoost::getTrainCostNames() const
{
    TVec<string> costs(5);
    costs[0] = "binary_class_error";
    costs[1] = "exp_neg_margin";
    costs[2] = "class_error";
    costs[3] = "avg_weight_class_0";
    costs[4] = "avg_weight_class_1";
    return costs;
}

void AdaBoost::computeTrainingError(Vec input, Vec target)
{
    if (compute_training_error)
    {
        PLASSERT(train_set);
        int n=train_set->length();
        PP<ProgressBar> pb;
        if(report_progress) pb = new ProgressBar("computing weighted training error of whole model",n);
        train_stats->forget();
        Vec err(nTrainCosts());
        int nb_class_0=0;
        int nb_class_1=0;
        real cum_weights_0=0;
        real cum_weights_1=0;

        bool save_forward_sub_learner_test_costs = 
            forward_sub_learner_test_costs;
        forward_sub_learner_test_costs=false;
        real weight;
        for (int i=0;i<n;i++)
        {
            if(report_progress) pb->update(i);
            train_set->getExample(i, input, target, weight);
            computeCostsOnly(input,target,err);
            if(fast_is_equal(target[0],0.)){
                cum_weights_0 += example_weights[i];
                nb_class_0++;
            }else{
                cum_weights_1 += example_weights[i];
                nb_class_1++;
            }
            err[3]=cum_weights_0/nb_class_0;
            err[4]=cum_weights_1/nb_class_1;
            train_stats->update(err);
        }
        train_stats->finalize();
        forward_sub_learner_test_costs = 
            save_forward_sub_learner_test_costs;

        if (verbosity>2)
            NORMAL_LOG << "At stage " << stage << 
                " boosted (weighted) classification error on training set = " 
                       << train_stats->getMean() << endl;
     
    }
}

void AdaBoost::setTrainingSet(VMat training_set, bool call_forget)
{ 
    PLCHECK(weak_learner_template);
    
    if(weak_learner_template->classname()=="RegressionTree"){
        //we do this for optimization. Otherwise we will creat a RegressionTreeRegister
        //for each weak_learner. This is time consuming as it sort the dataset
        if(training_set->classname()!="RegressionTreeRegisters")
            training_set = new RegressionTreeRegisters(training_set,
                                                       report_progress,
                                                       verbosity,
                                                       !finalized, !finalized);

        //we need to change the weight of the trainning set to reuse the RegressionTreeRegister
        if(!modif_train_set_weights){
            if(training_set->weightsize()==1)
                modif_train_set_weights=1;
            else
                NORMAL_LOG<<"In AdaBoost::setTrainingSet() -"
                          <<" We have RegressionTree as weak_learner, but the"
                          <<" training_set don't have a weigth. This will cause"
                          <<" the creation of a RegressionTreeRegisters at"
                          <<" each stage of AdaBoost!";
        }
        //we do this as RegressionTreeNode need a train_set for getTestCostNames
        if(!weak_learner_template->getTrainingSet())
            weak_learner_template->setTrainingSet(training_set,call_forget);
        for(int i=0;i<weak_learners.length();i++)
            if(!weak_learners[i]->getTrainingSet())
                weak_learners[i]->setTrainingSet(training_set,call_forget);
        
    }

    inherited::setTrainingSet(training_set, call_forget);
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
