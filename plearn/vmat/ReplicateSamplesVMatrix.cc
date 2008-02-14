// -*- C++ -*-

// ReplicateSamplesVMatrix.cc
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

/*! \file ReplicateSamplesVMatrix.cc */


#include "ReplicateSamplesVMatrix.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    ReplicateSamplesVMatrix,
    "VMat that replicates samples in order to reweight classes evenly.",
    "If the class with most samples in the source VMat has n samples, then\n"
    "the first 'n-n_j' samples of class j (having n_j samples) will be\n"
    "replicated so that each class also has n samples. If required, samples\n"
    "will be replicated more than once.\n"
    "All samples are also shuffled so as to mix classes together."
);

/////////////////////////////
// ReplicateSamplesVMatrix //
/////////////////////////////
ReplicateSamplesVMatrix::ReplicateSamplesVMatrix():
    seed(1827),
    random_gen(new PRandom())
{}

////////////////////
// declareOptions //
////////////////////
void ReplicateSamplesVMatrix::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    declareOption(ol, "seed", &ReplicateSamplesVMatrix::seed,
                  OptionBase::buildoption,
        "Seed for the random number generator (to shuffle data).");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void ReplicateSamplesVMatrix::build()
{
    // ### Nothing to add here, simply calls build_
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void ReplicateSamplesVMatrix::build_()
{
    if (!source)
        return;
    PLCHECK_MSG(source->targetsize() == 1,
                "In ReplicateSamplesVMatrix::build_ - The source VMat must "
                "have a targetsize equal to 1");
    
    // Build the vector of indices.
    indices.resize(0);
    Vec input, target;
    real weight;
    TVec< TVec<int>  > class_indices;  // Indices of samples in each class.
    for (int i = 0; i < source->length(); i++) {
        source->getExample(i, input, target, weight);
        int c = int(round(target[0]));
        if (c >= class_indices.length()) {
            int n_to_add = c - class_indices.length() + 1;
            for (int j = 0; j < n_to_add; j++)
                class_indices.append(TVec<int>());
        }
        class_indices[c].append(i);
        indices.append(i);
    }
    int max_n = -1;
    for (int c = 0; c < class_indices.length(); c++)
        if (class_indices[c].length() > max_n)
            max_n = class_indices[c].length();
    for (int c = 0; c < class_indices.length(); c++) {
        int n_replicated = max_n - class_indices[c].length();
        for (int i = 0; i < n_replicated; i++) {
            indices.append(class_indices[c][i % class_indices[c].length()]);
        }
    }

    // Shuffle data.
    random_gen->manual_seed(seed);
    random_gen->shuffleElements(indices);
    
    // Re-build since indices have changed.
    inherited::build();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void ReplicateSamplesVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("ReplicateSamplesVMatrix::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
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
