// -*- C++ -*-

// BootstrapSplitter.cc
//
// Copyright (C) 2003,2007 Olivier Delalleau
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

/*! \file BootstrapSplitter.cc */

#include "BootstrapSplitter.h"
#include "BootstrapVMatrix.h"

namespace PLearn {
using namespace std;

///////////////////////
// BootstrapSplitter //
///////////////////////
BootstrapSplitter::BootstrapSplitter():
    frac(0.6667),
    n_splits(0),
    allow_repetitions(false),
    seed(1827),
    rgen(new PRandom())
{}

PLEARN_IMPLEMENT_OBJECT(BootstrapSplitter,
        "A splitter whose splits are bootstrap samples of the original dataset.",
        "Each split of this splitter contains a single set, which is a\n"
        "bootstrap sample of the original dataset. Note that, by default,\n"
        "samples are taken without repetition: in order to perform a 'real'\n"
        "bootstrap, the option 'allow_repetitions thus must be set to 1.\n"
);

////////////////////
// declareOptions //
////////////////////
void BootstrapSplitter::declareOptions(OptionList& ol)
{
    declareOption(ol, "n_splits", &BootstrapSplitter::n_splits, OptionBase::buildoption,
                  "Number of splits wanted.");

    declareOption(ol, "frac", &BootstrapSplitter::frac, OptionBase::buildoption,
                  "Fraction of elements to take in each bootstrap.");

    declareOption(ol, "allow_repetitions", &BootstrapSplitter::allow_repetitions, 
                  OptionBase::buildoption,
                  "Allows each row to appear more than once per split.");

    declareOption(ol, "seed", &BootstrapSplitter::seed, 
                  OptionBase::buildoption,
                  "Seed for the random number generator.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void BootstrapSplitter::build_()
{
    if (dataset) {
        rgen->manual_seed(seed);
        bootstrapped_sets.resize(0,0); // First clear the current sets.
        bootstrapped_sets.resize(n_splits,1);
        for (int i = 0; i < n_splits; i++) {
            // Construct a new bootstrap sample from the dataset.
            PP<PRandom> vmat_rgen= rgen->split();
            // Note: indices in the bootstrapped sets are sorted, so that
            // access may be faster (e.g. when reading large data from disk).
            bootstrapped_sets(i,0) = 
                new BootstrapVMatrix(dataset,frac,vmat_rgen, 
                                     false, allow_repetitions);
        }
    } else {
        bootstrapped_sets.resize(0,0);
    }
}

///////////
// build //
///////////
void BootstrapSplitter::build()
{
    inherited::build();
    build_();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void BootstrapSplitter::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    Splitter::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(bootstrapped_sets, copies);
    deepCopyField(rgen,              copies);
}

/////////////
// nsplits //
/////////////
int BootstrapSplitter::nsplits() const
{
    return n_splits;
}

///////////////////
// nSetsPerSplit //
///////////////////
int BootstrapSplitter::nSetsPerSplit() const
{
    // One single set per split.
    return 1;
}

TVec<VMat> BootstrapSplitter::getSplit(int k)
{
    // ### Build and return the kth split
    if (k >= n_splits) {
        PLERROR("BootstrapSplitter::getSplit: k is too high");
    } else if (k >= bootstrapped_sets.length()) {
        PLERROR("BootstrapSplitter::getSplit: you asked for a split but they're not ready yet");
    }
    return bootstrapped_sets(k);
}

////////////////
// setDataSet //
////////////////
void BootstrapSplitter::setDataSet(VMat the_dataset) {
    inherited::setDataSet(the_dataset);
    build(); // necessary to recompute the bootstrap samples.
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
