// -*- C++ -*-

// RemoveDuplicateVMatrix.cc
//
// Copyright (C) 2004 Olivier Delalleau
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

// Authors: Olivier Delalleau

/*! \file RemoveDuplicateVMatrix.cc */


#include "RemoveDuplicateVMatrix.h"
#include <plearn/base/tostring.h>
#include <plearn/ker/DistanceKernel.h>

namespace PLearn {
using namespace std;

////////////////////////////
// RemoveDuplicateVMatrix //
////////////////////////////
RemoveDuplicateVMatrix::RemoveDuplicateVMatrix()
    : epsilon(1e-6),
      max_source_length(10000),
      only_input(false),
      verbosity(1)
{
    // ...
    // ### You may or may not want to call build_() to finish building the object
    // build_();
}

PLEARN_IMPLEMENT_OBJECT(RemoveDuplicateVMatrix,
                        "A VMatrix that removes any duplicated entry in its source VMat.",
                        ""
    );

////////////////////
// declareOptions //
////////////////////
void RemoveDuplicateVMatrix::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    declareOption(ol, "epsilon", &RemoveDuplicateVMatrix::epsilon, OptionBase::buildoption,
                  "Two points will be considered equal iff their square distance is < epsilon.\n"
                  "If epsilon is set to 0, a more accurate check is performed and only samples\n"
                  "which are perfectly equal are removed.\n");

    declareOption(ol, "only_input", &RemoveDuplicateVMatrix::only_input, OptionBase::buildoption,
                  "If set to 1, only the input part will be considered when computing the inter-points\n"
                  "distance. If set to 0, the whole row of the matrix is considered.\n");

    declareOption(ol, "max_source_length", &RemoveDuplicateVMatrix::max_source_length, OptionBase::buildoption,
                  "If the source's length is higher than this value, the whole Gram matrix will\n"
                  "not be stored in memory (which will be slightly slower).\n");

    declareOption(ol, "verbosity", &RemoveDuplicateVMatrix::verbosity, OptionBase::buildoption,
                  "Controls the amount of output.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);

    redeclareOption(ol, "indices", &RemoveDuplicateVMatrix::indices, OptionBase::nosave,
                    "The indices will be computed at build time.");
    redeclareOption(ol, "indices_vmat", &RemoveDuplicateVMatrix::indices_vmat, OptionBase::nosave,
                    "Unused.");
}

///////////
// build //
///////////
void RemoveDuplicateVMatrix::build()
{
    // ### Nothing to add here, simply calls build_
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void RemoveDuplicateVMatrix::build_()
{
    updateMtime(indices_vmat);
    updateMtime(source);

    if (source) {
        DistanceKernel dk;
        dk.pow_distance = true;
        if (verbosity >= 1)
            dk.report_progress = true;
        else
            dk.report_progress = false;
        dk.build();
        int old_is = source->inputsize();
        int old_ts = source->targetsize();
        int old_ws = source->weightsize();
        if (!only_input)
            source->defineSizes(source->width(), 0, 0);
        dk.setDataForKernelMatrix(source);
        int n = source.length();
        bool compute_gram = (n <= max_source_length);
        Mat distances;
        if (compute_gram) {
            if (n > 10000 && verbosity >= 2)
                PLWARNING("In RemoveDuplicateVMatrix::build_ - Computing a large Gram "
                          "matrix (%d x %d), there may not be enough memory available", n, n);
            distances.resize(n, n);
            dk.computeGramMatrix(distances);
        }
        if (!only_input)
            source->defineSizes(old_is, old_ts, old_ws);
        TVec<bool> removed(n);
        removed.fill(false);
        Vec row_i, row_j;
        if (fast_exact_is_equal(epsilon, 0) || !compute_gram) {
            int w = only_input ? source->inputsize() : source->width();
            row_i.resize(w);
            row_j.resize(w);
        }
        real delta = epsilon > 0 ? epsilon : 1e-4;
        int count = 0;
        PP<ProgressBar> pb;
        bool report_progress = (!compute_gram && verbosity >= 1);
        int iterate = 0;
        if (report_progress)
            pb = new ProgressBar("Looking for duplicated entries", (n * (n - 1)) / 2);
        for (int i = 0; i < n; i++) {
            if (!removed[i]) {
                if (!compute_gram)
                    source->getSubRow(i, 0, row_i);
                for (int j = i + 1; j < n; j++) {
                    if (!removed[j]) {
                        bool equal;
                        if (compute_gram)
                            equal = (distances(i,j) < delta);
                        else {
                            source->getSubRow(j, 0, row_j);
                            equal = (dk.evaluate(row_i, row_j) < delta);
                        }
                        if (equal && fast_exact_is_equal(epsilon, 0)) {
                            // More accurate check.
                            if (compute_gram) {
                                source->getSubRow(i, 0, row_i);
                                source->getSubRow(j, 0, row_j);
                            }
                            real* data_i = row_i->data();
                            real* data_j = row_j->data();
                            int w = row_i->length();
                            for (int k = 0; k < w; k++, data_i++, data_j++)
                                if (!fast_exact_is_equal(*data_i, *data_j))
                                    equal = false;
                        }
                        if (equal) {
                            if (verbosity >= 5)
                                pout << "Removed sample "           << j
                                     << " (duplicated with sample " << i << ")" << endl;
                            removed[j] = true;
                            count++;
                        }
                    }
                }
            }
            iterate += (n - i);
            if (report_progress)
                pb->update(iterate);
        }
        indices.resize(0);
        for (int i = 0; i < n; i++)
            if (!removed[i])
                indices.append(i);
        inherited::build();
        if (verbosity >= 2)
            if (count > 0)
                pout << "Removed a total of " << count << " duplicated samples (new length: "
                     << length() << ")" << endl;
            else
                pout << "No duplicated samples found." << endl;
    }
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void RemoveDuplicateVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("RemoveDuplicateVMatrix::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
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
