
// -*- C++ -*-

// ClassifierFromDensity.cc
//
// Copyright (C) 2003-2005  Pascal Vincent & Olivier Delalleau
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

/*! \file ClassifierFromDensity.cc */
#include "ClassifierFromDensity.h"
#include <plearn/base/tostring.h>
#include <plearn/io/PPath.h>
#include <plearn/ker/ClassErrorCostFunction.h>
#include <plearn/ker/Kernel.h>
#include <plearn/ker/NegLogProbCostFunction.h>
#include <plearn/vmat/ConcatColumnsVMatrix.h>
#include <plearn/vmat/VMat_operations.h>

namespace PLearn {
using namespace std;

ClassifierFromDensity::ClassifierFromDensity() 
    : nclasses(-1),
      output_log_probabilities(false),
      normalize_probabilities(true),
      copy_estimator(false)
{}

PLEARN_IMPLEMENT_OBJECT(
    ClassifierFromDensity,
    "A classifier built from density estimators using Bayes' rule.", 
    "ClassifierFromDensity allows to build a classifier\n"
    "by building one density estimator for each class, \n"
    "and using Bayes rule to combine them. It is assumed that the target\n"
    "variable in the training set represents the class number, coded as\n"
    "an integer from 0 to nclasses-1.");

////////////////////
// declareOptions //
////////////////////
void ClassifierFromDensity::declareOptions(OptionList& ol)
{

    // Build options.

    declareOption(ol, "nclasses", &ClassifierFromDensity::nclasses, OptionBase::buildoption,
                  "The number of classes");

    declareOption(ol, "estimators", &ClassifierFromDensity::estimators, OptionBase::buildoption,
                  "The array of density estimators, one for each class.\n"
                  "You may also specify just one that will be replicated as many times as there are classes.");

    declareOption(ol, "output_log_probabilities", &ClassifierFromDensity::output_log_probabilities, OptionBase::buildoption,
                  "Whether computeOutput yields log-probabilities or probabilities (of classes given inputs)");
 
    declareOption(ol, "normalize_probabilities", &ClassifierFromDensity::normalize_probabilities, OptionBase::buildoption,
                  "Whether to normalize the probabilities (if not just compute likelihood * prior for each class)");

    declareOption(ol, "use_these_priors", &ClassifierFromDensity::use_these_priors, OptionBase::buildoption,
                  "If empty (the default), then prior class probability is determined upon \n"
                  "training from the empirical class count in the training set. Otherwise this\n"
                  "must be a vector of length nclasses, specifying these prior class prob. \n");

    // Learnt options.

    declareOption(ol, "log_priors", &ClassifierFromDensity::log_priors, OptionBase::learntoption,
                  "The log of the class prior probabilities");

    declareOption(ol, "copy_estimator", &ClassifierFromDensity::copy_estimator, OptionBase::learntoption,
                  "Indication that, at build time, the estimators for all classes are a copy of the first one");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void ClassifierFromDensity::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void ClassifierFromDensity::build_()
{
    if(estimators.size()==1 || copy_estimator)
    {
        copy_estimator = true;
        estimators.resize(nclasses);
        for(int i=1; i<nclasses; i++)
            estimators[i] = PLearn::deepCopy(estimators[0]);
    }
    else if(estimators.size() != nclasses)
        PLERROR("In ClassifierFromDensity: specified %d estimators but there are %d classes",estimators.size(), nclasses);
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void ClassifierFromDensity::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(estimators,       copies);
    deepCopyField(log_priors,       copies);
    deepCopyField(use_these_priors, copies);
}

////////////////
// outputsize //
////////////////
int ClassifierFromDensity::outputsize() const
{
    return nclasses;
}

////////////
// forget //
////////////
void ClassifierFromDensity::forget()
{
    stage=0;
    for(int c=0; c<estimators.length(); c++)
        estimators[c]->forget();
}

///////////
// train //
///////////
void ClassifierFromDensity::train()
{
    if(targetsize()!=1)
        PLERROR("In ClassifierFromDensity::train - Expecting a targetsize of 1 (class index between 0 and nclasses-1), not %d !!",targetsize());

    if(nstages<stage) // asking to revert to a previous stage!
        forget();  // reset the learner to stage=0

    if(stage==0)
    {
        map<real, TVec<int> > indices = indicesOfOccurencesInColumn(train_set, inputsize());

        log_priors.resize(nclasses);
        if(use_these_priors.length()==0)
        {
            for(int c=0; c<nclasses; c++)
                log_priors[c] = pl_log(real(indices[real(c)].length())) - pl_log(real(train_set.length())); // how many do we have?
        }
        else
        {
            if(use_these_priors.length()!=nclasses)
                PLERROR("In ClassifierFromDensity::train, when compute_empirical_priors is false, you must specify a proper log_priors of length nclasses");  
            for(int c=0; c<nclasses; c++)
                log_priors[c] = pl_log(use_these_priors[c]);
        }

        PPath expd = getExperimentDirectory();

        for(int c=0; c<nclasses; c++)
        {
            if(verbosity>=1)
                perr << ">>> Training class " << c << endl;
            VMat set_c = train_set.rows(indices[c]);
            int in_sz = set_c->inputsize();
            int targ_sz = set_c->targetsize();
            int we_sz = set_c->weightsize();
            // Removing target from set
            if (we_sz==0)
            {
                set_c = set_c.subMatColumns(0,in_sz);
                set_c->defineSizes(in_sz, 0, 0);
            }
            else // There are some weights.
            {
                set_c = hconcat(set_c.subMatColumns(0,in_sz), set_c.subMatColumns(in_sz+targ_sz,we_sz));
                set_c->defineSizes(in_sz,0,we_sz);
            }
            if (expd!="")
                estimators[c]->setExperimentDirectory(expd / "Class" / tostring(c));
            if (verbosity>=1)
                perr << " ( " << set_c.length() << " samples)" << endl;
            estimators[c]->setTrainingSet(set_c);
            PP<VecStatsCollector> train_stats = new VecStatsCollector();
            train_stats->setFieldNames(estimators[c]->getTrainCostNames());
            estimators[c]->setTrainStatsCollector(train_stats);
            estimators[c]->nstages = nstages;
            estimators[c]->train();
        }
        stage = nstages; // trained!
        if (verbosity >= 2)
            perr << ">>> Training is over" << endl;
    }
    if(stage>0 && stage<nstages)
    {
        for(int c=0; c<nclasses; c++)
        {
            if(verbosity>=1)
                perr << ">>> Training class " << c;
            estimators[c]->nstages = nstages;
            estimators[c]->train();
        }
        stage = nstages;
        if (verbosity >= 2)
            perr << ">>> Training is over" << endl;
    }
    
}

///////////////////
// computeOutput //
///////////////////
void ClassifierFromDensity::computeOutput(const Vec& input, Vec& output) const
{
    output.resize(nclasses);
    double log_of_sumprob = 0.;

    for(int c=0; c<nclasses; c++)
    {
        // Be slightly careful in the case were the log_prior is -Inf, i.e.
        // no observation of the current class appears in the training set.
        // Don't call the estimator in that case, since its output might be
        // ill-defined (NaN or some such), thereby polluting the rest of the
        // output computation

        double logprob_c = log_priors[c];
        if (! isinf(log_priors[c]))
            logprob_c += estimators[c]->log_density(input);  // multiply p by the prior

        output[c] = logprob_c;
        if (normalize_probabilities)
        {
            if(c==0)
                log_of_sumprob = logprob_c;
            else
                log_of_sumprob = logadd(log_of_sumprob, logprob_c);
        }
    }      

    if (normalize_probabilities)
        output -= real(log_of_sumprob); // divide by the sum

    // Make it probabilities rather than log probabilities...
    if (!output_log_probabilities)
        exp(output, output);
}

/////////////////////////////
// computeCostsFromOutputs //
/////////////////////////////
void ClassifierFromDensity::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                                    const Vec& target, Vec& costs) const
{
    static CostFunc cl_er;
    static CostFunc cond_p;
    if(!cl_er)
        cl_er = class_error();
    if(!cond_p)
        cond_p = condprob_cost();
    costs.resize(2);
    costs[0] = cl_er->evaluate(output, target);
    costs[1] = cond_p->evaluate(output, target);
}                                

//////////////////////
// getTestCostNames //
//////////////////////
TVec<string> ClassifierFromDensity::getTestCostNames() const
{
    TVec<string> cnames(2);
    cnames[0] = "class_error";
    cnames[1] = "condprob_cost";
    return cnames;
}

///////////////////////
// getTrainCostNames //
///////////////////////
TVec<string> ClassifierFromDensity::getTrainCostNames() const
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
