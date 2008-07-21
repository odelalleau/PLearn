// -*- C++ -*-

// KFoldLogisticClassifier.cc
//
// Copyright (C) 2008 Olivier Delalleau
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

// Authors: Olivier Delalleau

/*! \file KFoldLogisticClassifier.cc */


#include "KFoldLogisticClassifier.h"
#include <plearn/opt/ConjGradientOptimizer.h>
#include <plearn/vmat/ExplicitSplitter.h>
#include <plearn/vmat/KFoldSplitter.h>
#include <plearn_learners/generic/NNet.h>
#include <plearn_learners/hyper/EarlyStoppingOracle.h>
#include <plearn_learners/hyper/HyperLearner.h>
#include <plearn_learners/hyper/HyperOptimize.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    KFoldLogisticClassifier,
    "Average of multiple logistic classifiers from K-Fold split of the data.",
    "The training set is split into 'kfold' folds, and we train one logistic\n"
    "classifier on each fold (whose learning is controlled by early stopping\n"
    "based on the validation NLL).\n"
    "The output of this classifier is then the average of the outputs of the\n"
    "underlying logistic classifiers."
);

/////////////////////////////
// KFoldLogisticClassifier //
/////////////////////////////
KFoldLogisticClassifier::KFoldLogisticClassifier():
    kfold(5),
    max_degraded_steps(20),
    max_epochs(500),
    step_size(1)
{
}

////////////////////
// declareOptions //
////////////////////
void KFoldLogisticClassifier::declareOptions(OptionList& ol)
{
    // Build options.

    declareOption(ol, "kfold", &KFoldLogisticClassifier::kfold,
                  OptionBase::buildoption,
        "Number of splits of the data (and of classifiers being trained).");

    declareOption(ol, "max_degraded_steps",
                  &KFoldLogisticClassifier::max_degraded_steps,
                  OptionBase::buildoption,
        "Maximum number of optimization steps performed after finding a\n"
        "candidate for early stopping.");

    declareOption(ol, "max_epochs",
                  &KFoldLogisticClassifier::max_epochs,
                  OptionBase::buildoption,
        "Maximum number of epochs when training logistic classifiers\n");

    declareOption(ol, "step_size",
                  &KFoldLogisticClassifier::step_size,
                  OptionBase::buildoption,
        "Measure performance every 'step_size' epochs.");

    // Learnt options.

    declareOption(ol, "log_net", &KFoldLogisticClassifier::log_net,
                  OptionBase::learntoption,
        "Underlying logistic classifiers.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void KFoldLogisticClassifier::build_()
{
}

///////////
// build //
///////////
void KFoldLogisticClassifier::build()
{
    inherited::build();
    build_();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void KFoldLogisticClassifier::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("KFoldLogisticClassifier::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

////////////////
// outputsize //
////////////////
int KFoldLogisticClassifier::outputsize() const
{
    if (log_net.isEmpty())
        return -1;
    else
        return log_net[0]->outputsize();
}

////////////
// forget //
////////////
void KFoldLogisticClassifier::forget()
{
    //! (Re-)initialize the PLearner in its fresh state (that state may depend
    //! on the 'seed' option) and sets 'stage' back to 0 (this is the stage of
    //! a fresh learner!)
    /*!
      A typical forget() method should do the following:
      - call inherited::forget() to initialize its random number generator
        with the 'seed' option
      - initialize the learner's parameters, using this random generator
      - stage = 0
    */
    inherited::forget();
    log_net.resize(0);
}

///////////
// train //
///////////
void KFoldLogisticClassifier::train()
{
    if (!initTrain())
        return;

    PLCHECK( stage == 0 );

    // Find out the number of classes in the dataset.
    TVec<bool> all_classes;
    Vec input, target;
    real weight;
    PLCHECK(train_set->targetsize() == 1);
    for (int i = 0; i < train_set->length(); i++) {
        train_set->getExample(i, input, target, weight);
        int t = int(round(target[0]));
        if (t >= all_classes.length()) {
            int n_to_add = t - all_classes.length() + 1;
            for (int j = 0; j < n_to_add; j++)
                all_classes.append(false);
        }
        all_classes[t] = true;
    }
    int n_classes = all_classes.length();
    PLCHECK(n_classes >= 2);
    PLCHECK(all_classes.find(false) == -1);

    // Split the data.
    PP<KFoldSplitter> splitter = new KFoldSplitter();
    splitter->K = this->kfold;
    splitter->build();
    splitter->setDataSet(train_set);

    // Create logistic regressors.
    log_net.resize(0);
    string cost_func;
    for (int k = 0; k < kfold; k++) {
        PP<ConjGradientOptimizer> opt = new ConjGradientOptimizer();
        opt->build();
        PP<NNet> nnet = new NNet();
        nnet->optimizer = opt;
        nnet->seed_ = this->seed_;
        if (n_classes == 2) {
            cost_func = "stable_cross_entropy";
            nnet->output_transfer_func = "sigmoid";
            nnet->noutputs = 1;
        } else {
            cost_func = "NLL";
            nnet->output_transfer_func = "softmax";
            nnet->noutputs = n_classes;
        }
        nnet->cost_funcs = TVec<string>(1, cost_func);
        nnet->batch_size = 0;
        nnet->build();
        log_net.append(get_pointer(nnet));
    }

    // Train logistic regressors.
    for (int k = 0; k < log_net.length(); k++) {
        // Initialize the hyper-learning framework for early stopping.
        // Splitter.
        PP<ExplicitSplitter> hsplitter = new ExplicitSplitter();
        hsplitter->splitsets = TMat<VMat>(1, 2);
        hsplitter->splitsets(0) << splitter->getSplit(k);
        hsplitter->build();
        // PTester.
        PP<PTester> htester = new PTester();
        htester->splitter = hsplitter;
        string cost = "E[test.E[" + cost_func + "]]";
        htester->setStatNames(TVec<string>(1, cost), false);
        htester->build();
        // Oracle.
        PP<EarlyStoppingOracle> early_stop = new EarlyStoppingOracle();
        early_stop->max_degraded_steps = this->max_degraded_steps;
        early_stop->range = TVec<double>(3, this->step_size);
        early_stop->range[1] = this->max_epochs + 1;
        early_stop->option = "nstages";
        early_stop->build();
        // Strategy.
        PP<HyperOptimize> strategy = new HyperOptimize();
        strategy->oracle = early_stop;
        strategy->which_cost = "0";
        strategy->build();
        // HyperLearner.
        PP<HyperLearner> hyper = new HyperLearner();
        hyper->dont_restart_upon_change = TVec<string>(1, "nstages");
        hyper->learner_ = log_net[k];
        hyper->option_fields = hyper->dont_restart_upon_change;
        hyper->save_final_learner = false;
        hyper->strategy = TVec< PP<HyperCommand> >(1, get_pointer(strategy));
        hyper->tester = htester;
        hyper->build();
        // Perform training.
        hyper->train();
        // Make sure we keep only the best model.
        log_net[k] = hyper->getLearner();
    }
    this->stage = 1;
}

///////////////////
// computeOutput //
///////////////////
void KFoldLogisticClassifier::computeOutput(const Vec& input, Vec& output) const
{
    PLCHECK(!log_net.isEmpty());
    log_net[0]->computeOutput(input, output);
    store_output.resize(output.length());
    for (int k = 1; k < log_net.length(); k++) {
        log_net[k]->computeOutput(input, store_output);
        output += store_output;
    }
    output /= real(log_net.length());
}

/////////////////////////////
// computeCostsFromOutputs //
/////////////////////////////
void KFoldLogisticClassifier::computeCostsFromOutputs(const Vec& input, const Vec& output,
                                           const Vec& target, Vec& costs) const
{
    int t = int(round(target[0]));
    costs.resize(1);
    if (output.length() == 1) {
        // Binary classification.
        if (t == 1)
            costs[0] = - pl_log(output[0]);
        else {
            PLASSERT(t == 0);
            costs[0] = - pl_log(1 - output[0]);
        }
    } else {
        // More than two targets.
        PLASSERT(is_equal(sum(output), 1));
        costs[0] = - pl_log(output[t]);
    }
}

//////////////////////
// getTestCostNames //
//////////////////////
TVec<string> KFoldLogisticClassifier::getTestCostNames() const
{
    static TVec<string> costs;
    if (costs.isEmpty())
        costs.append("nll");
    return costs;
}

///////////////////////
// getTrainCostNames //
///////////////////////
TVec<string> KFoldLogisticClassifier::getTrainCostNames() const
{
    return TVec<string>();
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
