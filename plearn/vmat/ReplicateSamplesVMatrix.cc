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
#include <plearn/var/SumOverBagsVariable.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    ReplicateSamplesVMatrix,
    "VMat that replicates samples in order to reweight classes evenly.",
    "If the class with most samples in the source VMat has n samples, then\n"
    "the first 'n-n_j' samples of class j (having n_j samples) will be\n"
    "replicated so that each class also has n samples. If required, samples\n"
    "will be replicated more than once.\n"
    "The class index is assumed to be the first element of the target. It\n"
    "can be either an integer (negative class indices are also allowed) or\n"
    "a missing value (all examples with missing values are considered as\n"
    "belonging to the same class).\n"
    "When the 'operate_on_bags' option is set to true, the bag information\n"
    "must be stored in the last element of the target.\n"
    "All samples are also shuffled so as to mix classes together."
);

/////////////////////////////
// ReplicateSamplesVMatrix //
/////////////////////////////
ReplicateSamplesVMatrix::ReplicateSamplesVMatrix():
    operate_on_bags(false),
    bag_index(-1),
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

    declareOption(ol, "operate_on_bags",
                  &ReplicateSamplesVMatrix::operate_on_bags,
                  OptionBase::buildoption,
        "If set to 1, then bags in the source VMat will be taken into\n"
        "account so as to preserve their integrity. The classes will also be\n"
        "reweighted so that they have the same number of bags.");

    declareOption(ol, "bag_index",
                  &ReplicateSamplesVMatrix::bag_index,
                  OptionBase::buildoption,
        "Index of the target corresponding to the bag information (useful\n"
        "only when operate_on_bags is True). -1 means the last element.");

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

    PLCHECK_MSG(operate_on_bags || source->targetsize() >= 1,
            "In ReplicateSamplesVMatrix::build_ - The source VMat must have a "
            "targetsize at least 1 when not operating on bags, but its "
            "targetsize is " + tostring(source->targetsize()));

    PLCHECK_MSG(!operate_on_bags || source->targetsize() >= 2,
            "In ReplicateSamplesVMatrix::build_ - The source VMat must have a "
            "targetsize at least 2 when operating on bags, but its targetsize "
            "is " + tostring(source->targetsize()));

    updateMtime(indices_vmat);
    updateMtime(source);

    PLASSERT(bag_index < 0 || bag_index < source->targetsize());

    // Build the vector of indices.
    indices.resize(0);
    Vec input, target;
    real weight;
    TVec< TVec<int>  > class_indices;  // Indices of samples in each class.
    TVec<int> nan_indices; // Indices of missing class
    TVec< TVec<int> > negativeclass_indices;
    map<int, int> bag_sizes; // Map a source index to the size of its bag.
    int bag_start_idx = -1;
    int bag_idx = bag_index >= 0 ? bag_index : source->targetsize() - 1;
    for (int i = 0; i < source->length(); i++) {
        source->getExample(i, input, target, weight);
        real c_real = target[0];
        int c = int(round(c_real));
        if (!is_missing(c_real)) {
            if (c >= class_indices.length()) {
                int n_to_add = c - class_indices.length() + 1;
                for (int j = 0; j < n_to_add; j++)
                    class_indices.append(TVec<int>());
            }
            else if ( -c >= negativeclass_indices.length() ) {
                int n_to_add = -c - negativeclass_indices.length() + 1;
                for (int j = 0; j < n_to_add; j++)
                    negativeclass_indices.append(TVec<int>());
            }
        }
        
        if (!operate_on_bags || int(round(target[bag_idx])) &
                                SumOverBagsVariable::TARGET_COLUMN_FIRST) {
            if( is_missing(c_real) )
                nan_indices.append(i);
            else if( c>= 0 )
                class_indices[c].append(i);
            else if( c< 0 )
                negativeclass_indices[-c].append(i);
            else
                PLERROR("In ReplicateSamplesVMatrix::build_ - Invalid class");
            indices.append(i);
            bag_sizes[i] = 0;
            bag_start_idx = i;
        }
        if (operate_on_bags)
            bag_sizes[bag_start_idx]++;
    }
    if( nan_indices.length() > 0  )
        class_indices.append( nan_indices );
    if( negativeclass_indices.length() > 0 )
        for(int c = 0; c < negativeclass_indices.length(); c ++ )
            if( negativeclass_indices[c].length() > 0 )
                class_indices.append( negativeclass_indices[c] );
    int max_n = -1;
    for (int c = 0; c < class_indices.length(); c++) {
        if (class_indices[c].length() > max_n)
            max_n = class_indices[c].length();
        if (class_indices[c].isEmpty())
            PLERROR("In ReplicateSamplesVMatrix::build_ - Cannot replicate "
                    "samples for class %d since there are zero samples from "
                    "this class", c);
    }
    for (int c = 0; c < class_indices.length(); c++) {
        int n_replicated = max_n - class_indices[c].length();
        for (int i = 0; i < n_replicated; i++) {
            indices.append(class_indices[c][i % class_indices[c].length()]);
        }
    }

    // Shuffle data.
    random_gen->manual_seed(seed);
    random_gen->shuffleElements(indices);

    if (operate_on_bags) {
        // We now need to convert the list of start indices to the list of all
        // indices within each bag.
        TVec<int> start_idx = indices.copy();
        indices.resize(0);
        for (int i = 0; i < start_idx.length(); i++) {
            int start_i = start_idx[i];
            for (int j = 0; j < bag_sizes[start_i]; j++)
                indices.append(start_i + j);
        }
    }
    
    // Re-build since indices have changed.
    inherited::build();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void ReplicateSamplesVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(random_gen, copies);
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
