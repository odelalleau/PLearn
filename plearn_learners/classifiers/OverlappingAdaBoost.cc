// -*- C++ -*-

// OverlappingAdaBoost.cc
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
   * $Id: OverlappingAdaBoost.cc,v 1.2 2004/09/14 16:04:40 chrish42 Exp $
   ******************************************************* */

// Authors: Yoshua Bengio

/*! \file OverlappingAdaBoost.cc */

#include "OverlappingAdaBoost.h"
#include <plearn/math/pl_math.h>
#include <plearn/vmat/ConcatColumnsVMatrix.h>
#include <plearn/vmat/PLearnerOutputVMatrix.h>
#include <plearn/vmat/SelectRowsVMatrix.h>
#include <plearn/math/random.h>

namespace PLearn {
using namespace std;

OverlappingAdaBoost::OverlappingAdaBoost()
  : target_error(0.5), output_threshold(0.5), compute_training_error(1), 
    pseudo_loss_adaboost(1), weight_by_resampling(1), early_stopping(1),
    save_often(0)
  { }

PLEARN_IMPLEMENT_OBJECT(
  OverlappingAdaBoost,
  "OverlappingAdaBoost boosting algorithm for TWO-CLASS classification",
  "NONE");

void OverlappingAdaBoost::declareOptions(OptionList& ol)
{
  declareOption(ol, "weak_learners", &OverlappingAdaBoost::weak_learners,
                OptionBase::learntoption,
                "The vector of learned weak learners");

  declareOption(ol, "weak_learners_output", &OverlappingAdaBoost::weak_learners,
                OptionBase::learntoption,
                "The VMat of the output of learned weak learners on the training set");

  declareOption(ol, "voting_weights", &OverlappingAdaBoost::voting_weights,
                OptionBase::learntoption,
                "Weights given to the weak learners (their output is linearly combined with these weights\n"
                "to form the output of the OverlappingAdaBoost learner).\n");

  declareOption(ol, "initial_sum_weights", &OverlappingAdaBoost::initial_sum_weights,
                OptionBase::learntoption,
                "Initial sum of weights on the examples. Do not temper with.\n");

  declareOption(ol, "weak_learner_template", &OverlappingAdaBoost::weak_learner_template,
                OptionBase::buildoption,
                "Template for the regression weak learner to be boosted into a classifier");

  declareOption(ol, "target_error", &OverlappingAdaBoost::target_error,
                OptionBase::buildoption,
                "This is the target average weighted error below which each weak learner"
                "must reach after its training (ordinary adaboost: target_error=0.5).");

  declareOption(ol, "pseudo_loss_adaboost", &OverlappingAdaBoost::pseudo_loss_adaboost,
                OptionBase::buildoption,
                "Whether to use a variant of OverlappingAdaBoost which is appropriate for soft classifiers\n"
                "whose output is between 0 and 1 rather than being either 0 or 1.\n");

  declareOption(ol, "weight_by_resampling", &OverlappingAdaBoost::weight_by_resampling,
                OptionBase::buildoption,
                "Whether to train the weak learner using resampling to represent the weighting\n"
                "given to examples. If false then give these weights explicitly in the training set\n"
                "of the weak learner (note that some learners can accomodate weights well, others not).\n");

  declareOption(ol, "output_threshold", &OverlappingAdaBoost::output_threshold,
                OptionBase::buildoption,
                "To interpret the output of the learner as a class, it is compared to this\n"
                "threshold: class 1 if greather than output_threshold, class 0 otherwise.\n");

  declareOption(ol, "provide_learner_expdir", &OverlappingAdaBoost::provide_learner_expdir, OptionBase::buildoption,
                "If true, each weak learner to be trained will have its experiment directory set to WeakLearner#kExpdir/");

  declareOption(ol, "early_stopping", &OverlappingAdaBoost::early_stopping, OptionBase::buildoption,
                "If true, then boosting stops when the next weak learner is too weak (avg error > target_error - .01)\n");

  declareOption(ol, "save_often", &OverlappingAdaBoost::save_often, OptionBase::buildoption,
                "If true, then save the model after training each weak learner, under <expdir>/model.psave\n");

  declareOption(ol, "compute_training_error", &OverlappingAdaBoost::compute_training_error, OptionBase::buildoption,
                "Whether to compute training error at each stage.\n");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void OverlappingAdaBoost::build_()
{
}

// ### Nothing to add here, simply calls build_
void OverlappingAdaBoost::build()
{
  inherited::build();
  build_();
}


void OverlappingAdaBoost::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  deepCopyField(learners_error, copies);
  deepCopyField(example_weights, copies);
  deepCopyField(voting_weights, copies);
  deepCopyField(weak_learners, copies);
  deepCopyField(weak_learner_template, copies);
}


int OverlappingAdaBoost::outputsize() const
{
  // Outputsize is always 1, since this is a 0-1 classifier
  return 1;
}

void OverlappingAdaBoost::forget()
{
  stage = 0;
  learners_error.resize(0, nstages);
  weak_learners.resize(0, nstages);
  voting_weights.resize(0, nstages);
  sum_voting_weights = 0;
  if (seed_ >= 0)
    manual_seed(seed_);
  else
    PLearn::seed();
}

void OverlappingAdaBoost::train()
{
  if(!train_set)
    PLERROR("In OverlappingAdaBoost::train, you did not setTrainingSet");
    
  if(!train_stats)
    PLERROR("In OverlappingAdaBoost::train, you did not setTrainStatsCollector");

  if (train_set->targetsize()!=1)
    PLERROR("In OverlappingAdaBoost::train, targetsize should be 1, found %d", train_set->targetsize());

  if (nstages < stage)        //!< Asking to revert to previous stage
    forget();

  static Vec input;
  static Vec output;
  static Vec target;
  real weight;

  static Vec examples_error;

  const int n = train_set.length();
  static TVec<int> train_indices;
  static Vec pseudo_loss;

  input.resize(inputsize());
  output.resize(1);
  target.resize(targetsize());
  examples_error.resize(n);

  if (stage==0)
  {
    example_weights.resize(n);
    if (train_set->weightsize()>0)
    {
      ProgressBar pb("OverlappingAdaBoost round " + tostring(stage) +
                     ": extracting initial weights", n);
      initial_sum_weights=0;
      for (int i=0; i<n; ++i) {
        pb.update(i);
        train_set->getExample(i, input, target, weight);
        example_weights[i]=weight;
        initial_sum_weights += weight;
      }
      example_weights *= 1.0/initial_sum_weights;
    }
    else example_weights.fill(1.0/n);
    sum_voting_weights = 0;
    voting_weights.resize(0,nstages);
  }

  VMat unweighted_data = train_set.subMatColumns(0, inputsize()+1);
  learners_error.resize(nstages);
  weak_learners_output.resize(nstages-1);

  for ( ; stage < nstages ; ++stage)
  {
    VMat weak_learner_training_set;
    {
      ProgressBar pb("OverlappingAdaBoost round " + tostring(stage) +
                     ": making training set for weak learner", n);
      // We shall now construct a training set for the new weak learner:
      /*if (weight_by_resampling)
      {
        // use a "smart" resampling that approximated sampling with replacement
        // with the probabilities given by example_weights.
        map<real,int> indices;
        for (int i=0; i<n; ++i) {
          pb.update(i);
          real p_i = example_weights[i];
          int n_samples_of_row_i = int(rint(gaussian_mu_sigma(n*p_i,sqrt(n*p_i*(1-p_i))))); // randomly choose how many repeats of example i
          for (int j=0;j<n_samples_of_row_i;j++)
          {
            if (j==0)
              indices[i]=i;
            else
            {
              real k=n*uniform_sample(); // put the others in random places
              indices[k]=i; // while avoiding collisions
            }
          }
        }
        train_indices.resize(0,n);
        map<real,int>::iterator it = indices.begin();
        map<real,int>::iterator last = indices.end();
        for (;it!=last;++it)
          train_indices.push_back(it->second);
        weak_learner_training_set = new SelectRowsVMatrix(unweighted_data, train_indices);
        weak_learner_training_set->defineSizes(inputsize(), 1, 0);
      }
      else
      {*/
        Mat data_weights_column = example_weights.toMat(n,1).copy();
        data_weights_column *= initial_sum_weights; // to bring the weights to the same average level as the original ones
        VMat data_weights = VMat(data_weights_column);
        Array<VMat> temp_columns = unweighted_data & data_weights;
        for(int i=0; i<stage; i++)
          temp_columns = weak_learners_output[i] & temp_columns;
        weak_learner_training_set = new ConcatColumnsVMatrix(temp_columns);
        weak_learner_training_set->defineSizes(inputsize()+stage, 1, 1);
        //}
    }

    // Create new weak-learner and train it
    PP<PLearner> new_weak_learner = ::PLearn::deepCopy(weak_learner_template);
    new_weak_learner->setTrainingSet(weak_learner_training_set);
    new_weak_learner->setTrainStatsCollector(new VecStatsCollector);

    if(expdir!="" && provide_learner_expdir)
      new_weak_learner->setExperimentDirectory(append_slash(expdir+"WeakLearner" + tostring(stage) + "Expdir"));

    new_weak_learner->train();
    
    if(stage < nstages)
    {
      TVec< PP<PLearner> > pl(1); pl[0] = new_weak_learner;
      weak_learners_output[stage] = new PLearnerOutputVMatrix(unweighted_data, pl, false, false, true);
    }
    // calculate its weighted training error 
    {
      ProgressBar pb("computing weighted training error of weak learner",n);
      learners_error[stage] = 0;
      for (int i=0; i<n; ++i) {
        pb.update(i);
        weak_learner_training_set->getExample(i, input, target, weight);
        new_weak_learner->computeOutput(input,output);
        real y_i=target[0];
        real f_i=output[0];
        if (pseudo_loss_adaboost) // an error between 0 and 1 (before weighting)
        {
          examples_error[i] = 0.5*(f_i+y_i-2*f_i*y_i);
          learners_error[stage] += example_weights[i]*examples_error[i];
        }
        else
        {
          if (y_i==1)
          {
            if (f_i<output_threshold)
            {
              learners_error[stage] += example_weights[i];
              examples_error[i]=1;
            }
            else examples_error[i] = 0;
          }
          else
          {
            if (f_i>=output_threshold) {
              learners_error[stage] += example_weights[i];
              examples_error[i]=1;
            }
            else examples_error[i]=0;
          }
        }
      }
    }

    input.resize(input.length()+1);

    if (verbosity>1)
      cout << "weak learner at stage " << stage << " has average loss = " << learners_error[stage] << endl;

    // stopping criterion (in addition to n_stages)
    if (early_stopping && (learners_error[stage] == 0 || learners_error[stage] > target_error - 0.01))
    {
      nstages = stage;
      cout << "OverlappingAdaBoost::train early stopping because learner's loss at stage " << stage << " is " << learners_error[stage] << endl;
      break;
    }

    weak_learners.push_back(new_weak_learner);

    if (save_often && expdir!="")
      PLearn::save(append_slash(expdir)+"model.psave", *this);
      
    // compute the new learner's weight

    voting_weights.push_back(safeflog(((1-learners_error[stage])*target_error)/(learners_error[stage]*(1-target_error))));
    sum_voting_weights += fabs(voting_weights[stage]);

    // update the example weights

    real sum_w=0;
    for (int i=0;i<n;i++)
    {
      example_weights[i] *= exp(-voting_weights[stage]*(1-examples_error[i]));
      sum_w += example_weights[i];
    }
    example_weights *= 1.0/sum_w;

    if (compute_training_error)
    {
      {
        ProgressBar pb("computing weighted training error of whole model",n);
        train_stats->forget();
        static Vec err(1);
        for (int i=0;i<n;i++)
        {
          pb.update(i);
          train_set->getExample(i, input, target, weight);
          computeCostsOnly(input,target,err);
          train_stats->update(err);
        }
        train_stats->finalize();
      }
      if (verbosity>2)
        cout << "At stage " << stage << " boosted (weighted) classification error on training set = " << train_stats->getMean() << endl;
    }
  }
}


void OverlappingAdaBoost::computeOutput(const Vec& input, Vec& output) const
{
  output.resize(1);
  real sum_out=0;
  Vec temp_input(input.length()); temp_input << input;
  static Vec weak_learner_output(1);
  for (int i=0;i<voting_weights.length();i++)
  {
    weak_learners[i]->computeOutput(temp_input,weak_learner_output);
    temp_input.append(weak_learner_output[0]);
    sum_out += weak_learner_output[0]*voting_weights[i];
  }
  output[0] = sum_out/sum_voting_weights;
}

void OverlappingAdaBoost::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                       const Vec& target, Vec& costs) const
{
  costs.resize(1);

  // First cost is negative log-likelihood...  output[0] is the likelihood
  // of the first class
  if (target.size() > 1)
    PLERROR("OverlappingAdaBoost::computeCostsFromOutputs: target must contain "
            "one element only: the 0/1 class");
  if (target[0] == 0) {
    costs[0] = output[0] >= output_threshold; 
  }
  else if (target[0] == 1) {
    costs[0] = output[0] < output_threshold; 
  }
  else PLERROR("OverlappingAdaBoost::computeCostsFromOutputs: target must be "
               "either 0 or 1; current target=%f", target[0]);
}

TVec<string> OverlappingAdaBoost::getTestCostNames() const
{
  return getTrainCostNames();
}

TVec<string> OverlappingAdaBoost::getTrainCostNames() const
{
  TVec<string> costs(1);
  costs[0] = "class_error";
  return costs;
}

} // end of namespace PLearn
