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
   * $Id: AdaBoost.cc,v 1.1 2004/04/20 14:27:20 yoshua Exp $
   ******************************************************* */

// Authors: Yoshua Bengio

/*! \file AdaBoost.cc */

#include "AdaBoost.h"
#include "pl_math.h"
#include "ConcatColumnsVMatrix.h"
#include "SelectRowsVMatrix.h"
#include "random.h"

namespace PLearn {
using namespace std;

AdaBoost::AdaBoost()
  : target_error(0.5), output_threshold(0.5)
  { }

PLEARN_IMPLEMENT_OBJECT(
  AdaBoost,
  "AdaBoost boosting algorithm for two-class classification",
  "Given a classification weak-learner, this algorithm \"boosts\" it in\n"
  "order to obtain a much more powerful classification algorithm.\n"
  "The classifier is two-class, returning 0 or 1.\n"
  "The nstages option from PLearner is used to specify the desired\n"
  "number of boosting rounds (but the algorithm can stop earlier if\n"
  "the next weak learner is unable to unable to make significant progress.\n");

void AdaBoost::declareOptions(OptionList& ol)
{
  declareOption(ol, "weak_learners", &AdaBoost::weak_learners,
                OptionBase::learntoption,
                "The vector of learned weak learners");

  declareOption(ol, "weak_learner_template", &AdaBoost::weak_learner_template,
                OptionBase::buildoption,
                "Template for the regression weak learner to be boosted into a classifier");

  declareOption(ol, "target_error", &AdaBoost::target_error,
                OptionBase::buildoption,
                "This is the target average weighted error below which each weak learner"
                "must reach after its training (ordinary adaboost: target_error=0.5).");

  declareOption(ol, "output_threshold", &AdaBoost::output_threshold,
                OptionBase::buildoption,
                "To interpret the output of the learner as a class, it is compared to this\n"
                "threshold: class 1 if greather than output_threshold, class 0 otherwise.\n");

  declareOption(ol, "provide_learner_expdir", &AdaBoost::provide_learner_expdir, OptionBase::buildoption,
                "If true, each weak learner to be trained will have its experiment directory set to WeakLearner#kExpdir/");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void AdaBoost::build_()
{
}

// ### Nothing to add here, simply calls build_
void AdaBoost::build()
{
  inherited::build();
  build_();
}


void AdaBoost::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  deepCopyField(learners_error, copies);
  deepCopyField(example_weights, copies);
  deepCopyField(voting_weights, copies);
  deepCopyField(weak_learners, copies);
  deepCopyField(weak_learner_template, copies);
}


int AdaBoost::outputsize() const
{
  // Outputsize is always 1, since this is a 0-1 classifier
  return 1;
}

void AdaBoost::forget()
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

void AdaBoost::train()
{
  if(!train_set)
    PLERROR("In AdaBoost::train, you did not setTrainingSet");
    
  if(!train_stats)
    PLERROR("In AdaBoost::train, you did not setTrainStatsCollector");

  if (train_set->targetsize()!=1)
    PLERROR("In AdaBoost::train, targetsize should be 1, found %d", train_set->targetsize());

  if (nstages < stage)        //!< Asking to revert to previous stage
    forget();

  static Vec input;
  static Vec output;
  static Vec target;
  real weight;

  static TVec<int> examples_error;

  const int n = train_set.length();
  static TVec<int> train_indices;

  input.resize(inputsize());
  output.resize(1);
  target.resize(targetsize());
  examples_error.resize(n);

  if (stage==0)
  {
    example_weights.resize(n);
    if (train_set->weightsize()>0)
    {
      ProgressBar pb("AdaBoost round " + tostring(stage) +
                     ": extracting initial weights", n);
      real sum_w=0;
      for (int i=0; i<n; ++i) {
        pb.update(i);
        train_set->getExample(i, input, target, weight);
        example_weights[i]=weight;
        sum_w += weight;
      }
      example_weights *= 1.0/sum_w;
    }
    else example_weights.fill(1.0/n);
    sum_voting_weights = 0;
    voting_weights.resize(0,nstages);
  }

  VMat unweighted_data = train_set.subMatColumns(0, inputsize()+1);
  unweighted_data->defineSizes(inputsize(), 1, 0);
  learners_error.resize(nstages);

  for ( ; stage < nstages ; ++stage)
  {
    {
      ProgressBar pb("AdaBoost round " + tostring(stage) +
                     ": making training set for weak learner", n);
      // We shall now construct a training set for the new weak learner:
      // by a "smart" resampling that approximated sampling with replacement
      // with the probabilities given by example_weights.
      int m=0;
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
      map<real,int>::iterator it = indices.begin();
      map<real,int>::iterator last = indices.end();
      for (;it!=last;++it)
        train_indices.push_back(it->second);
    }

    // Create new weak-learner and train it
    PP<PLearner> new_weak_learner = ::PLearn::deepCopy(weak_learner_template);
    new_weak_learner->setTrainingSet(
      new SelectRowsVMatrix(unweighted_data, train_indices));
    new_weak_learner->setTrainStatsCollector(new VecStatsCollector);

    if(expdir!="" && provide_learner_expdir)
      new_weak_learner->setExperimentDirectory(append_slash(expdir+"WeakLearner" + tostring(stage) + "Expdir"));

    new_weak_learner->train();

    // calculate its weighted training error 
    {
      ProgressBar pb("computing weighted training error of weak learner",n);
      learners_error[stage] = 0;
      for (int i=0; i<n; ++i) {
        pb.update(i);
        train_set->getExample(i, input, target, weight);
        new_weak_learner->computeOutput(input,output);
        if (target[0]==1)
        {
          if (output[0]<output_threshold)
          {
            learners_error[stage] += example_weights[i];
            examples_error[i]=1;
          }
          else examples_error[i] = 0;
        }
        else
        {
          if (output[0]>=output_threshold) {
            learners_error[stage] += example_weights[i];
            examples_error[i]=1;
          }
          else examples_error[i]=0;
        }
      }
    }

    // stopping criterion (in addition to n_stages)
    if (learners_error[stage] == 0 || learners_error[stage] > target_error - 0.01)
    {
      nstages = stage;
      cout << "AdaBoost::train early stopping because learner's error at stage " << stage << " is " << learners_error[stage] << endl;
      break;
    }

    weak_learners.push_back(new_weak_learner);

    // compute the new learner's weight

    voting_weights.push_back(safeflog(learners_error[stage]*(1-target_error)/((1-learners_error[stage])*target_error)));
    sum_voting_weights += fabs(voting_weights[stage]);

    // update the example weights

    real sum_w=0;
    for (int i=0;i<n;i++)
    {
      if (examples_error[i])
        example_weights[i] *= exp(-learners_error[stage]);
      sum_w += example_weights[i];
    }
    example_weights *= 1.0/sum_w;

    train_stats->finalize();
  }
}


void AdaBoost::computeOutput(const Vec& input, Vec& output) const
{
  output.resize(1);
  real sum_out=0;
  static Vec weak_learner_output(1);
  for (int i=0;i<voting_weights.length();i++)
  {
    weak_learners[i]->computeOutput(input,weak_learner_output);
    sum_out += weak_learner_output[0]*voting_weights[i];
  }
  output[0] = sum_out/sum_voting_weights;
}

void AdaBoost::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                       const Vec& target, Vec& costs) const
{
  costs.resize(1);

  // First cost is negative log-likelihood...  output[0] is the likelihood
  // of the first class
  if (target.size() > 1)
    PLERROR("AdaBoost::computeCostsFromOutputs: target must contain "
            "one element only: the 0/1 class");
  if (target[0] == 0) {
    costs[0] = output[0] > 0.5;  // classification error if lkd > 0.5;
  }
  else if (target[0] == 1) {
    costs[0] = output[0] <= 0.5; // classification error if lkd <= 0.5;
  }
  else PLERROR("AdaBoost::computeCostsFromOutputs: target must be "
               "either 0 or 1; current target=%f", target[0]);
}

TVec<string> AdaBoost::getTestCostNames() const
{
  return getTrainCostNames();
}

TVec<string> AdaBoost::getTrainCostNames() const
{
  TVec<string> costs(1);
  costs[0] = "class_error";
  return costs;
}

} // end of namespace PLearn
