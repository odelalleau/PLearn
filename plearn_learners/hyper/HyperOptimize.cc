
// -*- C++ -*-

// HyperOptimize.cc
//
// Copyright (C) 2003-2004 ApSTAT Technologies Inc.
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

// Author: Pascal Vincent

/* *******************************************************
 * $Id$
 ******************************************************* */

/*! \file HyperOptimize.cc */
#include "HyperOptimize.h"
#include "HyperLearner.h"
#include <plearn/vmat/FileVMatrix.h>
#include <plearn/base/stringutils.h>

namespace PLearn {
using namespace std;


//! A generic deep_copy function based on serialization
//! (this does not use the deepCopy method mechanism, and may currently work
//! better for a number of classes, as the makeDeepCopyFromShallowCopy methods are
//! not properly implemented everywhere...)

/*
  template<class T>
  T deep_copy(const T& x)
  {
  ostrstream out_;
  PStream out(&out_);
  out << x;
  char* buf = out_.str();
  int n = out_.pcount();
  string s(buf,n);
  out_.freeze(false); // return ownership to the stream, so that it may free it...

  T y;
  istrstream in_(s.c_str());
  PStream in(&in_);
  in >> y;
  return y;
  }
*/

HyperOptimize::HyperOptimize()
    : which_cost(-1),
      min_n_trials(0),
      provide_tester_expdir(false),
      rerun_after_sub(false),
      provide_sub_expdir(true)
{
    // ...

    // ### You may or may not want to call build_() to finish building the object
    // build_();
}

PLEARN_IMPLEMENT_OBJECT(HyperOptimize, "ONE LINE DESCR", "NO HELP");

void HyperOptimize::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    declareOption(ol, "which_cost", &HyperOptimize::which_cost, OptionBase::buildoption,
                  "an index in the tester's statnames to be used as the objective cost to minimize");
    declareOption(ol, "min_n_trials", &HyperOptimize::min_n_trials, OptionBase::buildoption,
                  "minimum nb of trials before saving best model");
    declareOption(ol, "oracle", &HyperOptimize::oracle, OptionBase::buildoption,
                  "Oracle to interrogate to get hyper-parameter values to try.");
    declareOption(ol, "provide_tester_expdir", &HyperOptimize::provide_tester_expdir, OptionBase::buildoption,
                  "should the tester be provided with an expdir for each option combination to test");
    declareOption(ol, "sub_strategy", &HyperOptimize::sub_strategy, OptionBase::buildoption,
                  "Optional sub-strategy to optimize other hyper-params (for each combination given by the oracle)");
    declareOption(ol, "rerun_after_sub", &HyperOptimize::rerun_after_sub, OptionBase::buildoption,
                  "If this is true, a new evaluation will be performed after executing the sub-strategy, \n"
                  "using this HyperOptimizer's splitter and which_cost \n"
                  "This is useful if the sub_strategy optimizes a different cost, or uses different splitting.\n");
    declareOption(ol, "provide_sub_expdir", &HyperOptimize::provide_sub_expdir, OptionBase::buildoption,
                  "should sub_strategy commands be provided an expdir");
    declareOption(ol, "splitter", &HyperOptimize::splitter, OptionBase::buildoption,
                  "If not specified, we'll use default splitter specified in the hyper-learner's tester option");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void HyperOptimize::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation.
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
    // ### You should assume that the parent class' build_() has already been called.
}

// ### Nothing to add here, simply calls build_
void HyperOptimize::build()
{
    inherited::build();
    build_();
}

void HyperOptimize::setExperimentDirectory(const PPath& the_expdir)
{
    inherited::setExperimentDirectory(the_expdir);
    createResultsMat();
}

void HyperOptimize::createResultsMat()
{
    if(expdir!="")
    {
        string fname = expdir+"results.pmat";
        TVec<string> cost_fields = getResultNames();
        TVec<string> option_fields = hlearner->option_fields;
        int w = 2 + option_fields.length() + cost_fields.length();
        resultsmat = new FileVMatrix(fname,0,w);
        int j=0;
        resultsmat->declareField(j++, "_trial_");
        resultsmat->declareField(j++, "_objective_");
        for(int k=0; k<option_fields.length(); k++)
            resultsmat->declareField(j++, option_fields[k]);
        for(int k=0; k<cost_fields.length(); k++)
            resultsmat->declareField(j++, cost_fields[k]);
        resultsmat->saveFieldInfos();
    }
}

void HyperOptimize::reportResult(int trialnum,  const Vec& results)
{
    if(expdir!="")
    {
        TVec<string> cost_fields = getResultNames();
        TVec<string> option_fields = hlearner->option_fields;

        if(results.length() != cost_fields.length())
            PLERROR("In HyperOptimize::reportResult - Length of results vector (%d) "
                    "differs from number of cost fields (%d)",
                    results.length(), cost_fields.length());

        // ex: _trial_ _objective_ nepochs nhidden ...     train_error

        Vec newres(resultsmat.width());
        int j=0;
        newres[j++] = trialnum;
        newres[j++] = which_cost;

        for(int k=0; k<option_fields.length(); k++)
        {
            string optstr = hlearner->learner_->getOption(option_fields[k]);
            real optreal = toreal(optstr);
            if(is_missing(optreal)) // it's not directly a real: get a mapping for it
                optreal = resultsmat->addStringMapping(k, optstr);
            newres[j++] = optreal;
        }

        for(int k=0; k<cost_fields.length(); k++)
            newres[j++] = results[k];

        resultsmat->appendRow(newres);
        resultsmat->flush();
    }
}

Vec HyperOptimize::runTest(int trialnum)
{
    PP<PTester> tester = hlearner->tester;

    string testerexpdir = "";
    if(expdir!="" && provide_tester_expdir)
        testerexpdir = expdir / ("Trials"+tostring(trialnum)) / "";
    tester->setExperimentDirectory(testerexpdir);

    PP<Splitter> default_splitter = tester->splitter;
    if(splitter)  // set our own splitter
        tester->splitter = splitter;

    Vec results = tester->perform(false);

    //! restore default splitter
    tester->splitter = default_splitter;
    return results;
}

TVec<string> HyperOptimize::getResultNames() const
{
    return hlearner->tester->getStatNames();
}

Vec HyperOptimize::optimize()
{
    real best_objective = REAL_MAX;
    Vec best_results;
    PP<PLearner> best_learner;

    TVec<string> option_names;
    option_names = oracle->getOptionNames();

    TVec<string> option_vals = oracle->generateFirstTrial();
    if (option_vals.size() != option_names.size())
        PLERROR("HyperOptimize::optimize: the number of option values (%d) "
                "does not match the number of option names (%d)",
                option_vals.size(), option_names.size());

    int trialnum = 0;

    Vec results;
    while(option_vals)
    {
        // This will also call build and forget on the learner unless unnecessary
        // because the modified options don't require it.
        hlearner->setLearnerOptions(option_names, option_vals);

        if(sub_strategy)
        {
            Vec best_sub_results;
            for(int commandnum=0; commandnum<sub_strategy.length(); commandnum++)
            {
                sub_strategy[commandnum]->setHyperLearner(hlearner);
                if(expdir!="" && provide_sub_expdir)
                    sub_strategy[commandnum]->setExperimentDirectory(
                        expdir / ("Trials"+tostring(trialnum)) / ("Step"+tostring(commandnum))
                        );
                best_sub_results = sub_strategy[commandnum]->optimize();
            }
            if(rerun_after_sub)
                results = runTest(trialnum);
            else
                results = best_sub_results;
        }
        else
            results = runTest(trialnum);

        reportResult(trialnum,results);
        real objective = results[which_cost];

        option_vals = oracle->generateNextTrial(option_vals,objective);
        if(!is_missing(objective) &&
           (objective < best_objective || best_results.length()==0) && (trialnum>=min_n_trials || !option_vals))
        {
            best_objective = objective;
            best_results = results;
            CopiesMap copies;
            best_learner = NULL;
            best_learner = hlearner->getLearner()->deepCopy(copies);
        }
        ++trialnum;
    }

    // Detect the case where no trials at all were performed!
    if (trialnum == 0)
        PLWARNING("In HyperOptimize::optimize - No trials at all were completed;\n"
                  "perhaps the oracle settings are wrong?");

    // revert to best_learner
    hlearner->setLearner(best_learner);

    // report best result again
    reportResult(-1,best_results);

    if (best_results.isEmpty())
        // This could happen for instance if all results are NaN.
        PLWARNING("In HyperOptimize::optimize - Could not find a best result, something "
                  "must be wrong");
    return best_results;
}

void HyperOptimize::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(resultsmat, copies);
    deepCopyField(oracle, copies);
    deepCopyField(sub_strategy, copies);
    deepCopyField(splitter, copies);
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
