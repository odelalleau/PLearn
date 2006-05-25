// -*- C++ -*-

// HyperRetrain.cc
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

/*! \file HyperRetrain.cc */

#include "HyperRetrain.h"
#include "HyperLearner.h"
#include <plearn_learners/testers/PTester.h>

namespace PLearn {
using namespace std;

HyperRetrain::HyperRetrain()
    : provide_tester_expdir(false)
{
}

PLEARN_IMPLEMENT_OBJECT(
    HyperRetrain,
    "HyperCommand to retrain a learner with a new splitter without changing hyperparameters",
    "This is useful as a last command in an HyperOptimization chain in order\n"
    "to retrain the current \"best\" learner with the entire training+validation\n"
    "sets.");

void HyperRetrain::declareOptions(OptionList& ol)
{
    declareOption(ol, "splitter", &HyperRetrain::splitter,
                  OptionBase::buildoption,
                  "Splitter to use for retraining.  For example, in order to\n"
                  "retrain on entire dataset, one can use the following:\n"
                  "  splitter = FractionSplitter(splits = 1 1 [0:1]); ");

    declareOption(ol, "provide_tester_expdir", &HyperRetrain::provide_tester_expdir,
                  OptionBase::buildoption,
                  "should the tester be provided with an expdir for retraining");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void HyperRetrain::build_()
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
void HyperRetrain::build()
{
    inherited::build();
    build_();
}

void HyperRetrain::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(splitter, copies);
}

Vec HyperRetrain::optimize()
{
    PP<PTester> tester = hlearner->tester;

    string testerexpdir = "";
    if(expdir!="" && provide_tester_expdir)
        testerexpdir = expdir+"retrain/";
    tester->setExperimentDirectory(testerexpdir);

    PP<Splitter> default_splitter = tester->splitter;
    if (splitter.isNull())
        PLERROR("HyperRetrain::optimize: a new splitter must be set inside "
                "HyperRetrain.  \nFor example: "
                "splitter = FractionSplitter(splits = 1 1 [0:1]);");
    tester->splitter = splitter;

    Vec results = tester->perform(true);

    // restore default splitter
    tester->splitter = default_splitter;
    return results;
}

TVec<string> HyperRetrain::getResultNames() const
{
    return hlearner->tester->getStatNames();
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
