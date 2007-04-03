// -*- C++ -*-

// BootstrapSplitter.cc
//
// Copyright (C) 2003 Olivier Delalleau
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

BootstrapSplitter::BootstrapSplitter()
    :Splitter(),
     frac(0.6667),
     n_splits(0),
     allow_repetitions(false),
     seed(1827),
     rgen(new PRandom())
    /* ### Initialise all fields to their default value */
{
}

PLEARN_IMPLEMENT_OBJECT(BootstrapSplitter,
                        "A splitter whose splits are bootstrap samples of the original dataset",
                        "BootstrapSplitter implements a ...");

void BootstrapSplitter::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    declareOption(ol, "n_splits", &BootstrapSplitter::n_splits, OptionBase::buildoption,
                  "the number of splits wanted");

    declareOption(ol, "frac", &BootstrapSplitter::frac, OptionBase::buildoption,
                  "the fraction of elements to take in each bootstrap");

    declareOption(ol, "allow_repetitions", &BootstrapSplitter::allow_repetitions, 
                  OptionBase::buildoption,
                  "Allows each row to appear more than once per split.");

    declareOption(ol, "seed", &BootstrapSplitter::seed, 
                  OptionBase::buildoption,
                  "Allows each row to appear more than once per split.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void BootstrapSplitter::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation.
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
    // ### You should assume that the parent class' build_() has already been called.
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
                new BootstrapVMatrix(dataset, frac, vmat_rgen, false,
                                     allow_repetitions);
        }
    } else {
        bootstrapped_sets.resize(0,0);
    }
}

// ### Nothing to add here, simply calls build_
void BootstrapSplitter::build()
{
    inherited::build();
    build_();
}

void BootstrapSplitter::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    Splitter::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("BootstrapSplitter::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

int BootstrapSplitter::nsplits() const
{
    // ### Return the number of available splits
    return n_splits;
}

int BootstrapSplitter::nSetsPerSplit() const
{
    // ### Return the number of sets per split
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
