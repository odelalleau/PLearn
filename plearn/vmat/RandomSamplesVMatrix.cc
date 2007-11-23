// -*- C++ -*-

// RandomSamplesVMatrix.cc
//
// Copyright (C) 2006 Olivier Delalleau
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

/*! \file RandomSamplesVMatrix.cc */


#include "RandomSamplesVMatrix.h"
#include <plearn/vmat/VMatLanguage.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    RandomSamplesVMatrix,
    "VMat that samples on-the-fly random examples from its source.",
    "More precisely, this VMat will:\n"
    "- contain all examples from its source that match the 'is_preserved'\n"
    "  VPL program (by default, no example is systematically preserved)\n"
    "- fill the rest of the data with random source examples that do not\n"
    "  match that program\n"
    "\n"
    "It is important to note that random examples are sampled at each call\n"
    "of the 'getNewRow(..)' method, so that the data viewed by this VMatrix\n"
    "is not constant (except for the rows that are preserved).\n"
    "\n"
    "The ordering of the examples is random, and the total length of this\n"
    "VMat is determined from the 'length' option, or from the number of non-\n"
    "preserved examples if the 'n_non_preserved' option is set.\n"
);

//////////////////////////////
// RandomSamplesVMatrix //
//////////////////////////////
RandomSamplesVMatrix::RandomSamplesVMatrix():
    alternate_targets(false),
    is_preserved("0"),
    n_non_preserved(-1),
    seed(1827),
    random_gen(new PRandom())
{
}

////////////////////
// declareOptions //
////////////////////
void RandomSamplesVMatrix::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here.
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. If you don't provide one of these three,
    // ### this option will be ignored when loading values from a script.
    // ### You can also combine flags, for example with OptionBase::nosave:
    // ### (OptionBase::buildoption | OptionBase::nosave)

    declareOption(ol, "is_preserved", &RandomSamplesVMatrix::is_preserved,
                                      OptionBase::buildoption,
        "VPL program that indicates if a sample is preserved.");

    declareOption(ol, "n_non_preserved",
                  &RandomSamplesVMatrix::n_non_preserved,
                  OptionBase::buildoption,
        "If given a non-negative value, it indicates the total number of\n"
        "non-preserved examples that are added to this VMat, and overrides\n"
        "the 'length' option. Two special negative values can be used:\n"
        " -1: this option is ignored, and 'length' is used instead (or the\n"
        "     source's length if 'length' is not set)\n"
        " -2: the number of non-preserved examples is set exactly to match\n"
        "     the number of preserved examples.");

    declareOption(ol, "alternate_targets",
                  &RandomSamplesVMatrix::alternate_targets,
                  OptionBase::buildoption,
        "If set to 1, then this VMat will never sample two consecutive rows\n"
        "having the same target.");

    declareOption(ol, "seed", &RandomSamplesVMatrix::seed,
                              OptionBase::buildoption,
        "Seed for random number generation.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void RandomSamplesVMatrix::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void RandomSamplesVMatrix::build_()
{
    if (!source)
        return;

    random_gen->manual_seed(seed);

    // Initialize VPL program.
    VMatLanguage program;
    TVec<string> fieldnames;
    program.setSource(source);
    program.compileString(is_preserved, fieldnames);

    // Find out which samples need to be kept.
    int n = source->length();
    Vec row(source->width());
    Vec result(1);
    non_preserved.resize(0);
    indices.resize(0);
    for (int i = 0; i < n; i++) {
        program.run(i, result);
        if (fast_exact_is_equal(result[0], 1))
            // Sample i is to be preserved.
            indices.append(i);
        else
            non_preserved.append(i);
    }

    // Find out length of this VMat if the 'n_non_preserved' option is set.
    PLASSERT( n_non_preserved >= 0 || n_non_preserved == -1 ||
            n_non_preserved == -2 );
    if (n_non_preserved >= 0)
        length_ = indices.length() + n_non_preserved;
    else if (n_non_preserved == -2)
        length_ = indices.length() * 2;
    else if (length_ < 0)
        length_ = source->length();

    // Specific pre-processing for the case where 'alternate_targets' is true.
    if (alternate_targets) {
        // Currently this feature is not implemented when we preserve examples
        // (just because it is a bit more complex).
        if (!indices.isEmpty())
            PLERROR("In RandomSamplesVMatrix::build_ - The 'alternate_targets'"
                    " option is not implemented yet in the case where some "
                    "examples are preserved. Please implement it!");
        if (source->targetsize() != 1)
            PLERROR("In RandomSamplesVMatrix::build_ - The source must have "
                    "a targetsize of 1 in order to use the "
                    "'alternate_targets' option");
        // First we need to find the number of targets.
        map<real, TVec<int> > by_target_map; // Map a target to its samples.
        Vec input, target;
        real weight;
        target_to_idx.clear();
        for (int i = 0; i < source->length(); i++) {
            source->getExample(i, input, target, weight);
            by_target_map[target[0]].append(i);
            if (target_to_idx.count(target[0]) == 0) {
                int n_cur = target_to_idx.size();
                target_to_idx[target[0]] = n_cur;
            }
        }
        int n_targets = by_target_map.size();
        // Convert from map to vector (for convenience).
        by_target.resize(n_targets);
        map<real, TVec<int> >::const_iterator it = by_target_map.begin();
        for (int i = 0; i < n_targets; i++, it++)
            by_target[i] = it->second;
        // Build a map from a target index to all other target indices that may
        // be used afterwards, with corresponding probabilities.
        target_list.resize(n_targets);
        target_distr.resize(n_targets);
        for (int i = 0; i < n_targets; i++) {
            TVec<int>& tl = target_list[i];
            tl.resize(0);
            Vec& td = target_distr[i];
            td.resize(0);
            int n_others = 0;
           
            for (int j = 0; j < n_targets; j++)
                if (j != i) {
                    tl.append(j); // This is the list of all targets,
                                  // excluding 'i'.
                    td.append(by_target[j].length());
                    n_others += by_target[j].length();
                }
            td /= real(n_others); // Normalize probabilities.
        }

        // Resize 'last_targets'.
        last_targets.resize(source->length());
    }

    // Fill in 'indices' with as many -1 as necessary.
    if (indices.length() > length_)
        PLERROR("In RandomSamplesVMatrix::build_ - The number of preserved"
                "samples (%d) is higher than the length of this VMat (%d)",
                indices.length(), length_);
    while (indices.length() != length_)
        indices.append(-1);

    // Shuffle the list of indices.
    random_gen->shuffleElements(indices);

    setMetaInfoFromSource();
}

///////////////
// getNewRow //
///////////////
void RandomSamplesVMatrix::getNewRow(int i, const Vec& v) const
{
    if (indices[i] >= 0)
        source->getRow(indices[i], v);
    else {
        int random_sample;
        if (alternate_targets && i > 0) {
            // Note that the first sample may be any class since it has no
            // previous sample in this VMat.
            real previous_target = last_targets[i - 1];
            int previous_target_idx = target_to_idx[previous_target];
            int random_target = random_gen->multinomial_sample(
                    target_distr[previous_target_idx]);
            const TVec<int>& candidates =
                by_target[target_list[previous_target_idx][random_target]];
            int random_sample_idx = random_gen->uniform_multinomial_sample(
                    candidates.length());
            random_sample = candidates[random_sample_idx];
            PLASSERT( non_preserved.length() == source->length() );
        } else {
            random_sample =
                random_gen->uniform_multinomial_sample(non_preserved.length());
        }
        source->getRow(non_preserved[random_sample], v);
        if (alternate_targets)
            last_targets[i] = v[source->inputsize()];
    }
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void RandomSamplesVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(random_gen,       copies);
    deepCopyField(non_preserved,    copies);
    deepCopyField(indices,          copies);
    deepCopyField(last_targets,     copies);
    deepCopyField(target_list,      copies);
    deepCopyField(target_distr,     copies);
    deepCopyField(by_target,        copies);
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
