// -*- C++ -*-

// BestAveragingPLearner.cc
//
// Copyright (C) 2006 Nicolas Chapados
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

// Authors: Nicolas Chapados

/*! \file BestAveragingPLearner.cc */

// From C++ stdlib
#include <algorithm>

// From Boost
#include <boost/scoped_ptr.hpp>

// From PLearn
#include <plearn/base/ProgressBar.h>
#include "BestAveragingPLearner.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    BestAveragingPLearner,
    "Select the M \"best\" of N trained PLearners based on a train cost",
    "This PLearner takes N raw models (themselved PLearners) and trains them all\n"
    "on the same data (or various splits given by an optional Splitter), then\n"
    "selects the M \"best\" models based on a train cost.  At compute-output time,\n"
    "it outputs the arithmetic mean of the outputs of the selected models (which\n"
    "works fine for regression).\n"
    "\n"
    "The train costs of this learner are simply the concatenation of the train\n"
    "costs of all sublearners.  We also add the following costs: the cost\n"
    "'selected_i', where 0 <= i < M, contains the index of the selected model\n"
    "(between 0 and N-1).\n"
    "\n"
    "The test costs of this learner is, for now, just the mse.\n"
    );

BestAveragingPLearner::BestAveragingPLearner()
    : m_initial_seed(-1),
      m_seed_option("seed"),
      m_total_learner_num(0),
      m_best_learner_num(0),
      m_cached_outputsize(-1)
{ }

void BestAveragingPLearner::declareOptions(OptionList& ol)
{
    //#####  Build Options  ###################################################

    declareOption(
        ol, "learner_set", &BestAveragingPLearner::m_learner_set,
        OptionBase::buildoption,
        "The set of all learners to train, given in extension.  If this option\n"
        "is specified, the learner template (see below) is ignored.  Note that\n"
        "these objects ARE NOT deep-copied before being trained.\n");

    declareOption(
        ol, "learner_template", &BestAveragingPLearner::m_learner_template,
        OptionBase::buildoption,
        "If 'learner_set' is not specified, a template PLearner used to\n"
        "instantiate 'learner_set'.  When instantiation is being carried out,\n"
        "the seed is set sequentially from 'initial_seed'.  The instantiation\n"
        "sequence is as follows:\n"
        "\n"
        "- (1) template is deep-copied\n"
        "- (2) seed and expdir are set\n"
        "- (3) build() is called\n"
        "- (4) forget() is called\n"
        "\n"
        "The expdir is set from the BestAveragingPLearner's expdir (if any)\n"
        "by suffixing '/learner_i'.\n");

    declareOption(
        ol, "initial_seed", &BestAveragingPLearner::m_initial_seed,
        OptionBase::buildoption,
        "If learners are instantiated from 'learner_template', the initial seed\n"
        "value to set into the learners before building them.  The seed is\n"
        "incremented by one from that starting point for each successive learner\n"
        "that is being instantiated.  If this value is <= 0, it is used as-is\n"
        "without being incremented.\n");

    declareOption(
        ol, "seed_option", &BestAveragingPLearner::m_seed_option,
        OptionBase::buildoption,
        "Use in conjunction with 'initial_seed'; option name pointing to the\n"
        "seed to be initialized.  The default is just 'seed', which is the\n"
        "PLearner option name for the seed, and is adequate if the\n"
        "learner_template is \"shallow\", such as NNet.  This option is useful if\n"
        "the learner_template is a complex learner (e.g. HyperLearner) and the\n"
        "seed must actually be set inside one of the inner learners.  In the\n"
        "particular case of HyperLearner, one could use 'learner.seed' as the\n"
        "value for this option.\n");
    
    declareOption(
        ol, "total_learner_num", &BestAveragingPLearner::m_total_learner_num,
        OptionBase::buildoption,
        "Total number of learners to instantiate from learner_template (if\n"
        "'learner_set' is not specified.\n");

    declareOption(
        ol, "best_learner_num", &BestAveragingPLearner::m_best_learner_num,
        OptionBase::buildoption,
        "Number of BEST train-time learners to keep and average at\n"
        "compute-output time.\n");

    declareOption(
        ol, "comparison_statspec", &BestAveragingPLearner::m_comparison_statspec,
        OptionBase::buildoption,
        "Statistic specification to use to compare the training performance\n"
        "between learners.  For example, if all learners have a 'mse' measure,\n"
        "this would be \"E[mse]\".  It is assumed that all learners make available\n"
        "the statistic under the same name.\n");

    declareOption(
        ol, "splitter", &BestAveragingPLearner::m_splitter,
        OptionBase::buildoption,
        "Optional splitter that can be used to create the individual training\n"
        "sets for the learners.  If this is specified, it is assumed that the\n"
        "splitter returns a number of splits equal to the number of learners.\n"
        "Each split is used as a learner's training set.  If not specified,\n"
        "all learners receive the same training set (passed to setTrainingSet)\n");

    
    //#####  Learnt Options  ##################################################

    declareOption(
        ol, "cached_outputsize", &BestAveragingPLearner::m_cached_outputsize,
        OptionBase::learntoption,
        "Cached outputsize, determined from the inner learners");
    
    declareOption(
        ol, "learner_train_costs", &BestAveragingPLearner::m_learner_train_costs,
        OptionBase::learntoption,
        "List of train costs values for each learner in 'learner_set'");

    declareOption(
        ol, "best_learners", &BestAveragingPLearner::m_best_learners,
        OptionBase::learntoption,
        "Learners that have been found to be the best and are being kept");
    
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void BestAveragingPLearner::build_()
{
    if (! m_learner_set.size() && m_learner_template.isNull())
        PLERROR("%s: one of 'learner_set' or 'learner_template' must be specified",
                __FUNCTION__);

    // If both 'learner_set' and 'learner_template' are specified, the former
    // silently overrides the latter.  Reason: after instantiation of
    // learner_template, stuff is put into learner_set as a result.
    if (! m_learner_set.size() && m_learner_template) {
        // Sanity check on other options
        if (m_total_learner_num < 1)
            PLERROR("%s: 'total_learner_num' must be strictly positive",
                    __FUNCTION__);

        const int N = m_total_learner_num;
        long cur_seed = m_initial_seed;
        m_learner_set.resize(N);
        for (int i=0 ; i<N ; ++i) {
            PP<PLearner> new_learner = PLearn::deepCopy(m_learner_template);
            new_learner->setOption(m_seed_option, tostring(cur_seed));
            new_learner->build();
            new_learner->forget();
            m_learner_set[i] = new_learner;

            if (cur_seed > 0)
                ++cur_seed;
        }
    }

    // Some more sanity checking
    if (m_best_learner_num < 1)
        PLERROR("%s: 'best_learner_num' must be strictly positive", __FUNCTION__);
    if (m_best_learner_num > m_learner_set.size())
        PLERROR("%s: 'best_learner_num' (=%d) must not be larger than the total "
                "number of learners (=%d)", __FUNCTION__, m_best_learner_num,
                m_learner_set.size());
}

// ### Nothing to add here, simply calls build_
void BestAveragingPLearner::build()
{
    inherited::build();
    build_();
}


void BestAveragingPLearner::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(m_learner_set,         copies);
    deepCopyField(m_learner_template,    copies);
    deepCopyField(m_splitter,            copies);
    deepCopyField(m_learner_train_costs, copies);
    deepCopyField(m_best_learners,       copies);
    deepCopyField(m_output_buffer,       copies);
}


void BestAveragingPLearner::setTrainingSet(VMat training_set, bool call_forget)
{
    inherited::setTrainingSet(training_set, call_forget);

    // Make intelligent use of splitter if any
    if (m_splitter) {
        m_splitter->setDataSet(training_set);

        // Splitter should return exactly the same number of splits as there
        // are inner learners
        if (m_splitter->nsplits() != m_learner_set.size())
            PLERROR("%s: splitter should return exactly the same number of splits (=%d) "
                    "as there are inner learners (=%d)", __FUNCTION__,
                    m_splitter->nsplits(), m_learner_set.size());

        for (int i=0, n=m_learner_set.size() ; i<n ; ++i) {
            TVec<VMat> split = m_splitter->getSplit(i);
            if (split.size() != 1)
                PLERROR("%s: split %d should return exactly 1 VMat (returned %d)",
                        __FUNCTION__, i, split.size());
            m_learner_set[i]->setTrainingSet(split[0], call_forget);
        }
    }
    else {
        for (int i=0, n=m_learner_set.size() ; i<n ; ++i)
            m_learner_set[i]->setTrainingSet(training_set, call_forget);
    }
}


void BestAveragingPLearner::setTrainStatsCollector(PP<VecStatsCollector> statscol)
{
    inherited::setTrainStatsCollector(statscol);
    for (int i=0, n=m_learner_set.size() ; i<n ; ++i) {
        // Set the statistic names so we can call getStat on the VSC.
        PP<VecStatsCollector> vsc = new VecStatsCollector;
        vsc->setFieldNames(m_learner_set[i]->getTrainCostNames());
        m_learner_set[i]->setTrainStatsCollector(vsc);
    }
}


void BestAveragingPLearner::setExperimentDirectory(const PPath& the_expdir)
{
    inherited::setExperimentDirectory(the_expdir);
    if (! the_expdir.isEmpty()) {
        for (int i=0, n=m_learner_set.size() ; i<n ; ++i)
            m_learner_set[i]->setExperimentDirectory(
                the_expdir / "learner_" + tostring(i));
    }
}


int BestAveragingPLearner::outputsize() const
{
    // If the outputsize has not already been determined, get it from the inner
    // learners.
    if (m_cached_outputsize < 0) {
        for (int i=0, n=m_learner_set.size() ; i<n ; ++i) {
            int cur_outputsize = m_learner_set[i]->outputsize();
            if (m_cached_outputsize < 0)
                m_cached_outputsize = cur_outputsize;
            else if (m_cached_outputsize != cur_outputsize)
                PLERROR("%s: all inner learners must have the same outputsize; "
                        "learner %d has an outputsize of %d, contrarily to %d for "
                        "the previous learners",  __FUNCTION__, i, cur_outputsize,
                        m_cached_outputsize);
        }
    }
    return m_cached_outputsize;
}

void BestAveragingPLearner::forget()
{
    inherited::forget();

    for (int i=0, n=m_learner_set.size() ; i<n ; ++i)
        m_learner_set[i]->forget();
}

void BestAveragingPLearner::train()
{
    if (! initTrain())
        return;
 
    const int N = m_learner_set.size();
    m_learner_train_costs.resize(N);
    TVec< pair<real, int> > model_scores(N);
    boost::scoped_ptr<ProgressBar> pb(verbosity?
        new ProgressBar("Training sublearners of BestAveragingPLearner",N) : 0);

    // Basic idea is to train all sublearners, then sample the train statistic
    // used for comparison, and fill out the member variable 'm_best_learners'.
    // Finally we collect the expectation of their sublearners train statistics
    // (to give this learner's train statistics)

    // Actual train-cost vector
    Vec traincosts(nTrainCosts());
    int pos_traincost = 0;
    
    for (int i=0 ; i<N ; ++i) {
        if (pb)
            pb->update(i);
        m_learner_set[i]->train();

        PP<VecStatsCollector> trainstats = m_learner_set[i]->getTrainStatsCollector();
        real cur_comparison = trainstats->getStat(m_comparison_statspec);
        m_learner_train_costs[i] = cur_comparison;
        model_scores[i] = make_pair(cur_comparison, i);

        Vec cur_traincosts = trainstats->getMean();
        traincosts.subVec(pos_traincost, cur_traincosts.size()) << cur_traincosts;
        pos_traincost += cur_traincosts.size();
    }

    // Find the M best train costs
    sort(model_scores.begin(), model_scores.end());
    assert( m_best_learner_num <= model_scores.size() );
    m_best_learners.resize(m_best_learner_num);
    for (int i=0 ; i<m_best_learner_num ; ++i) {
        m_best_learners[i] = m_learner_set[model_scores[i].second];
        traincosts[pos_traincost++] = model_scores[i].second;
    }

    // Accumulate into train statscollector
    assert( getTrainStatsCollector() );
    getTrainStatsCollector()->update(traincosts);
}


void BestAveragingPLearner::computeOutput(const Vec& input, Vec& output) const
{
    output.resize(outputsize());
    output.fill(0.0);
    m_output_buffer.resize(outputsize());

    // Basic strategy: accumulate into output, then divide by number of
    // learners (take unweighted arithmetic mean).  Works fine as long as we
    // don't accumulate millions of terms...
    for (int i=0, n=m_best_learners.size() ; i<n ; ++i) {
        m_best_learners[i]->computeOutput(input, m_output_buffer);
        output += m_output_buffer;
    }
    output /= m_best_learners.size();
}


void BestAveragingPLearner::computeCostsFromOutputs(const Vec& input, const Vec& output,
                                                    const Vec& target, Vec& costs) const
{
    // For now, only MSE is computed...
    real mse = powdistance(output, target);
    costs.resize(1);
    costs[0] = mse;
}


TVec<string> BestAveragingPLearner::getTestCostNames() const
{
    return TVec<string>(1, "mse");
}


TVec<string> BestAveragingPLearner::getTrainCostNames() const
{
    TVec<string> c;
    for (int i=0, n=m_learner_set.size() ; i<n ; ++i) {
        TVec<string> learner_costs = m_learner_set[i]->getTrainCostNames().copy();
        for (int j=0, m=learner_costs.size() ; j<m ; ++j)
            learner_costs[j] = "learner"+tostring(i)+'_'+learner_costs[j];
        c.append(learner_costs);
    }

    for (int i=0 ; i<m_best_learner_num ; ++i)
        c.push_back("selected_" + tostring(i));

    return c;
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
