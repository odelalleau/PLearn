
// -*- C++ -*-

// HyperOptimize.cc
//
// Copyright (C) 2003-2006 ApSTAT Technologies Inc.
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
// Documentation: Nicolas Chapados


/* *******************************************************
 * $Id$
 ******************************************************* */

/*! \file HyperOptimize.cc */
#include "HyperOptimize.h"
#include "HyperLearner.h"
#include <plearn/vmat/FileVMatrix.h>
#include <plearn/vmat/MemoryVMatrix.h>
#include <plearn/base/stringutils.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    HyperOptimize,
    "Carry out an hyper-parameter optimization according to an Oracle",
    "HyperOptimize is part of a sequence of HyperCommands (specified within an\n"
    "HyperLearner) to optimize a validation cost over settings of\n"
    "hyper-parameters provided by an Oracle.  [NOTE: The \"underlying learner\" is\n"
    "the PLearner object (specified within the enclosing HyperLearner) whose\n"
    "hyper-parameters we are trying to optimize.]\n"
    "\n"
    "The sequence of steps followed by HyperOptimize is as follows:\n"
    "\n"
    "- 1) Gather a \"trial\" from an Oracle.  A \"trial\" is a full setting of\n"
    "  hyperparameters (option name/value pairs) that the underlying learner\n"
    "  should be trained with.\n"
    "\n"
    "- 2) Set the options within the underlying learner that correspond to\n"
    "  the current trial.\n"
    "\n"
    "- 3) Train and test the underlying learner.  The tester used for this\n"
    "  purpose is a PTester specified in the enclosing HyperLearner.  By\n"
    "  default, we rely on that PTester's Splitter as well; however, an\n"
    "  overriding Splitter may be specified within the HyperCommand.\n"
    "\n"
    "- 4) After training/testing, measure the cost to optimize, given by the\n"
    "  'which_cost' option.  This specifies an index into the test statistics\n"
    "  given by the 'statnames' option in PTester.  The measured cost gives\n"
    "  the performance of the current trial, i.e. how well does perform the\n"
    "  current setting of hyper-parameters.\n"
    "\n"
    "- 5) Repeat steps 1-4 until the Oracle tells us \"no more trials\".\n"
    "\n"
    "- 6) Find the best setting of hyper-parameters among all those tried.\n"
    "  (\"best\" defined as that which minimises the cost measured in Step 4).\n"
    "\n"
    "- 7) Set the underlying learner within the enclosing HyperLearner to be\n"
    "  the BEST ONE found in Step 6.\n"
    "\n"
    "Optionally, instead of a plain Train/Test in Step 3, a SUB-STRATEGY may be\n"
    "invoked.  This can be viewed as a \"sub-routine\" for hyperoptimization and\n"
    "can be used to implement a form of conditioning: given the current setting\n"
    "for hyper-parameters X,Y,Z, find the best setting of hyper-parameters\n"
    "T,U,V.  The most common example is for doing early-stopping when training a\n"
    "neural network: a first-level HyperOptimize command can use an\n"
    "ExplicitListOracle to jointly optimize over weight-decays and the number of\n"
    "hidden units.  A sub-strategy can then be used with an EarlyStoppingOracle\n"
    "to find the optimal number of training stages (epochs) for each combination\n"
    "of weight-decay/hidden units.\n"
    "\n"
    "Note that after optimization, the matrix of all trials is available through\n"
    "the option 'resultsmat' (which is declared as nosave).  This is available\n"
    "even if no expdir has been declared.\n"
    );


HyperOptimize::HyperOptimize()
    : which_cost_pos(-1),
      which_cost(),
      min_n_trials(0),
      provide_tester_expdir(false),
      rerun_after_sub(false),
      provide_sub_expdir(true)
{ }


void HyperOptimize::declareOptions(OptionList& ol)
{
    declareOption(
        ol, "which_cost", &HyperOptimize::which_cost, OptionBase::buildoption,
        "An index or a name in the tester's statnames to be used as the objective cost to minimize");

    declareOption(
        ol, "min_n_trials", &HyperOptimize::min_n_trials, OptionBase::buildoption,
        "Minimum nb of trials before saving best model");

    declareOption(
        ol, "oracle", &HyperOptimize::oracle, OptionBase::buildoption,
        "Oracle to interrogate to get hyper-parameter values to try.");

    declareOption(
        ol, "provide_tester_expdir", &HyperOptimize::provide_tester_expdir, OptionBase::buildoption,
        "Should the tester be provided with an expdir for each option combination to test");

    declareOption(
        ol, "sub_strategy", &HyperOptimize::sub_strategy, OptionBase::buildoption,
        "Optional sub-strategy to optimize other hyper-params (for each combination given by the oracle)");

    declareOption(
        ol, "rerun_after_sub", &HyperOptimize::rerun_after_sub, OptionBase::buildoption,
        "If this is true, a new evaluation will be performed after executing the sub-strategy, \n"
        "using this HyperOptimizer's splitter and which_cost. \n"
        "This is useful if the sub_strategy optimizes a different cost, or uses different splitting.\n");

    declareOption(
        ol, "provide_sub_expdir", &HyperOptimize::provide_sub_expdir, OptionBase::buildoption,
        "Should sub_strategy commands be provided an expdir");

    declareOption(
        ol, "splitter", &HyperOptimize::splitter, OptionBase::buildoption,
        "If not specified, we'll use default splitter specified in the hyper-learner's tester option");

    declareOption(
        ol, "resultsmat", &HyperOptimize::resultsmat,
        OptionBase::learntoption | OptionBase::nosave,
        "Gives access to the results of all trials during the last training.\n"
        "The last row lists the best results found and kept.  Note that this\n"
        "is declared 'nosave' and is intended for programmatic access by other\n"
        "functions through the getOption() mechanism. If an expdir is declared\n"
        "this matrix is available under the name 'results.pmat' in the expdir.");
    
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
    TVec<string> cost_fields = getResultNames();
    TVec<string> option_fields = hlearner->option_fields;
    int w = 2 + option_fields.length() + cost_fields.length();

    // If we have an expdir, create a FileVMatrix to save the results.
    // Otherwise, just a MemoryVMatrix to make the results available as a
    // getOption after training.
    if (! expdir.isEmpty())
    {
        string fname = expdir+"results.pmat";
        resultsmat = new FileVMatrix(fname,0,w);
    }
    else
        resultsmat = new MemoryVMatrix(0,w);

    int j=0;
    resultsmat->declareField(j++, "_trial_");
    resultsmat->declareField(j++, "_objective_");
    for(int k=0; k<option_fields.length(); k++)
        resultsmat->declareField(j++, option_fields[k]);
    for(int k=0; k<cost_fields.length(); k++)
        resultsmat->declareField(j++, cost_fields[k]);

    if (! expdir.isEmpty())
        resultsmat->saveFieldInfos();
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
        newres[j++] = which_cost_pos;

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

    which_cost_pos= getResultNames().find(which_cost);
    if(which_cost_pos < 0)
        which_cost_pos= toint(which_cost);

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
        real objective = results[which_cost_pos];

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
