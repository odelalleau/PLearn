// -*- C++ -*-

// RepeatSplitter.cc
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

/*! \file RepeatSplitter.cc */

#include <plearn/math/random.h>
#include "RepeatSplitter.h"
#include "SelectRowsVMatrix.h"

namespace PLearn {
using namespace std;

////////////////////
// RepeatSplitter //
////////////////////
RepeatSplitter::RepeatSplitter()
    :
    last_n(-1),
    do_not_shuffle_first(0),
    force_proportion(-1),
    n(1),
    seed(-1),
    shuffle(0)
{
}

PLEARN_IMPLEMENT_OBJECT(RepeatSplitter,
                        "Repeat a given splitter a certain amount of times, with the possibility to\n"
                        "shuffle randomly the dataset each time",
                        "NO HELP");

////////////////////
// declareOptions //
////////////////////
void RepeatSplitter::declareOptions(OptionList& ol)
{
    declareOption(ol, "do_not_shuffle_first", &RepeatSplitter::do_not_shuffle_first, OptionBase::buildoption,
                  "If set to 1, then the dataset won't be shuffled the first time we do the splitting.\n"
                  "It only makes sense to use this option if 'shuffle' is set to 1.");

    declareOption(ol, "force_proportion", &RepeatSplitter::force_proportion, OptionBase::buildoption,
                  "If a target value appears at least once every x samples, will ensure that after\n"
                  "shuffling it appears at least once every (x * 'force_proportion') samples, and not\n"
                  "more than once every (x / 'force_proportion') samples. Will be ignored if < 1.\n"
                  "Note that this currently only works for a binary target! (and hasn't been 100% tested).");

    declareOption(ol, "n", &RepeatSplitter::n, OptionBase::buildoption,
                  "How many times we want to repeat.");

    declareOption(ol, "seed", &RepeatSplitter::seed, OptionBase::buildoption,
                  "Initializes the random number generator (only if shuffle is set to 1).\n"
                  "If set to -1, the initialization will depend on the clock.");

    declareOption(ol, "shuffle", &RepeatSplitter::shuffle, OptionBase::buildoption,
                  "If set to 1, the dataset will be shuffled differently at each repetition.");

    declareOption(ol, "to_repeat", &RepeatSplitter::to_repeat, OptionBase::buildoption,
                  "The splitter we want to repeat.");

    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void RepeatSplitter::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void RepeatSplitter::build_()
{
    if (shuffle && dataset) {
        // Prepare the shuffled indices.
        if (seed >= 0)
            manual_seed(seed);
        else
            PLearn::seed();
        int n_splits = nsplits();
        indices = TMat<int>(n_splits, dataset.length());
        TVec<int> shuffled;
        for (int i = 0; i < n_splits; i++) {
            shuffled = TVec<int>(0, dataset.length()-1, 1);
            // Don't shuffle if (i == 0) and do_not_shuffle_first is set to 1.
            if (!do_not_shuffle_first || i > 0) {
                shuffleElements(shuffled);
                if (force_proportion >= 1) {
                    // We need to ensure the proportions of target values are respected.
                    // First compute the target stats.
                    StatsCollector tsc(2000);
                    if (dataset->targetsize() != 1) {
                        PLERROR("In RepeatSplitter::build_ - 'force_proportion' is only implemented for a 1-dimensional target");
                    }
                    real t;
                    for (int j = 0; j < dataset->length(); j++) {
                        t = dataset->get(j, dataset->inputsize()); // We get the target.
                        tsc.update(t);
                    }
                    tsc.finalize();
                    // Make sure the target is binary.
                    int count = (int) tsc.getCounts()->size() - 1;
                    if (count != 2) {
                        PLERROR("In RepeatSplitter::build_ - 'force_proportion' is only implemented for a binary target");
                    }
                    // Ensure the proportion of the targets respect the constraints.
                    int index = 0;
                    for (map<real,StatsCollectorCounts>::iterator it =
                            tsc.getCounts()->begin(); index < count; index++)
                    {
                        t = it->first;
                        real prop_t = real(it->second.n) /
                                      real(dataset->length());
                        // Find the step to use to check the proportion is ok.
                        // We want a step such that each 'step' examples, there
                        // should be at least two with this target, but less
                        // than 'step - 10'.  For instance, for a proportion of
                        // 0.1, 'step' would be 20, and for a proportion of
                        // 0.95, it would be 200.  We also want the
                        // approximation made when rounding to be negligible.
                        int step = 20;
                        bool ok = false;
                        while (!ok) {
                            int n = int(step * prop_t + 0.5);
                            if (n >= 2  && n <= step - 10
                                && abs(step * prop_t - real(n)) / real(step) < 0.01) {
                                ok = true;
                            } else {
                                // We try a higher step.
                                step *= 2;
                            }
                        }
                        int expected_count = int(step * prop_t + 0.5);
                        // cout << "step = " << step << ", expected_count = " << expected_count << endl;
                        // Now verify the proportion.
                        ok = false;
                        int tc = dataset->inputsize(); // The target column.
                        while (!ok) {
                            ok = true;
                            // First pass: ensure there is enough.
                            int first_pass_step = int(step * force_proportion + 0.5);
                            int k,l;
                            for (k = 0; k < shuffled.length(); k += first_pass_step) {
                                int count_target = 0;
                                for (l = k; l < k + first_pass_step && l < shuffled.length(); l++) {
                                    if (fast_exact_is_equal(
                                            dataset->get(shuffled[l], tc), t))
                                        count_target++;
                                }
                                if (l - k == first_pass_step && count_target < expected_count) {
                                    // Not enough, need to add more.
                                    ok = false;
                                    // cout << "At l = " << l << ", need to add " << expected_count - count_target << " samples" << endl;
                                    for (int m = 0; m < expected_count - count_target; m++) {
                                        bool can_swap = false;
                                        int to_swap = -1;
                                        // Find a sample to swap in the current window.
                                        while (!can_swap) {
                                            to_swap = int(uniform_sample() * first_pass_step);
                                            if (!fast_exact_is_equal(dataset->get(shuffled[k + to_swap], tc), t)) {
                                                can_swap = true;
                                            }
                                        }
                                        to_swap += k;
                                        // Find a sample to swap in the next samples.
                                        int next = k + first_pass_step - 1;
                                        can_swap = false;
                                        while (!can_swap) {
                                            next++;
                                            if (next >= shuffled.length()) {
                                                next = 0;
                                            }
                                            if (fast_exact_is_equal(dataset->get(shuffled[next], tc), t)) {
                                                can_swap = true;
                                            }
                                        }
                                        // And swap baby!
                                        int tmp = shuffled[next];
                                        shuffled[next] = shuffled[to_swap];
                                        shuffled[to_swap] = tmp;
                                    }
                                }
                            }
                            // Second pass: ensure there aren't too many.
                            int second_pass_step = int(step / force_proportion + 0.5);
                            for (k = 0; k < shuffled.length(); k += second_pass_step) {
                                int count_target = 0;
                                for (l = k; l < k + second_pass_step && l < shuffled.length(); l++) {
                                    if (fast_exact_is_equal(dataset->get(shuffled[l], tc), count_target)) {
                                        count_target++;
                                    }
                                }
                                if (l - k == second_pass_step && count_target > expected_count) {
                                    // Too many, need to remove some.
                                    ok = false;
                                    PLWARNING("In RepeatSplitter::build_ - The code reached hasn't been tested yet");
                                    // cout << "At l = " << l << ", need to remove " << - expected_count + count_target << " samples" << endl;
                                    for (int m = 0; m < - expected_count + count_target; m++) {
                                        bool can_swap = false;
                                        int to_swap = k - 1;
                                        // Find a sample to swap in the current window.
                                        while (!can_swap) {
                                            to_swap++;
                                            if (fast_exact_is_equal(dataset->get(shuffled[to_swap], tc), t)) {
                                                can_swap = true;
                                            }
                                        }
                                        // Find a sample to swap in the next samples.
                                        int next = k + first_pass_step - 1;
                                        can_swap = false;
                                        while (!can_swap) {
                                            next++;
                                            if (next >= shuffled.length()) {
                                                next = 0;
                                            }
                                            if (!fast_exact_is_equal(dataset->get(shuffled[next], tc), t)) {
                                                can_swap = true;
                                            }
                                        }
                                        // And swap baby!
                                        int tmp = shuffled[next];
                                        shuffled[next] = shuffled[to_swap];
                                        shuffled[to_swap] = tmp;
                                    }
                                }
                            }
                        }
                        it++;
                    }
                }
            }
            indices(i) << shuffled;
        }
    } else {
        indices = TMat<int>();
    }
    last_n = -1;
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void RepeatSplitter::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    Splitter::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    deepCopyField(to_repeat, copies);

}

//////////////
// getSplit //
//////////////
TVec<VMat> RepeatSplitter::getSplit(int k)
{
    int n_splits = this->nsplits();
    if (k >= n_splits) {
        PLERROR("In RepeatSplitter::getSplit: split asked is too high");
    }
    int child_splits = to_repeat->nsplits();
    int real_k = k % child_splits;
    if (shuffle && dataset) {
        int shuffle_indice = k / child_splits;
        if (shuffle_indice != last_n) {
            // We have to reshuffle the dataset, according to indices.
            VMat m = new SelectRowsVMatrix(dataset, indices(shuffle_indice));
            to_repeat->setDataSet(m);
            last_n = shuffle_indice;
        }
    }
    return to_repeat->getSplit(real_k);
}

///////////////////
// nSetsPerSplit //
///////////////////
int RepeatSplitter::nSetsPerSplit() const
{
    return to_repeat->nSetsPerSplit();
}

/////////////
// nsplits //
/////////////
int RepeatSplitter::nsplits() const
{
    return to_repeat->nsplits() * n;
}

////////////////
// setDataSet //
////////////////
void RepeatSplitter::setDataSet(VMat the_dataset) {
    inherited::setDataSet(the_dataset);
    to_repeat->setDataSet(the_dataset);
    build(); // necessary to recompute the indices.
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
