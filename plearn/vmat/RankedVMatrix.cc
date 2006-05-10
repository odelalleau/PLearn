// -*- C++ -*-

// RankedVMatrix.cc
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

/*! \file RankedVMatrix.cc */


#include "RankedVMatrix.h"

namespace PLearn {
using namespace std;

///////////////////
// RankedVMatrix //
///////////////////
RankedVMatrix::RankedVMatrix()
{}

RankedVMatrix::RankedVMatrix(VMat the_source, PP<RankedVMatrix> the_reference)
    : reference(the_reference)
{
    source = the_source;
    build();
}

PLEARN_IMPLEMENT_OBJECT(RankedVMatrix,
                        "Replaces the target of a source VMat with its rank.",
                        "A 'reference' VMat, which is also a RankedVMatrix, can also be provided.\n"
                        "In this case, the target will be defined as follows from a target y:\n"
                        " 1. Find in the reference's source VMat the target closest to y\n"
                        " 2. Use the rank of the input corresponding to this target.\n"
                        " 3. If there is no such target, use the maximum rank + 1\n"
                        "If no reference is given, then the target is just the rank in the source\n"
                        "VMat's sorted targets, starting from 0.\n"
    );

////////////////////
// declareOptions //
////////////////////
void RankedVMatrix::declareOptions(OptionList& ol)
{
    declareOption(ol, "reference", &RankedVMatrix::reference, OptionBase::buildoption,
                  "An optional reference VMat used to define the targets.\n");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void RankedVMatrix::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void RankedVMatrix::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation.
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
    // ### You should assume that the parent class' build_() has already been called.
    if (source) {
        if (source->targetsize() != 1)
            PLERROR("In RankedVMatrix::build_ - The source VMat must have a targetsize equal to 1");
        // Get sorted target column.
        sorted_targets.resize(source->length(), 2);
        sorted_targets.column(0) << source.column(source->inputsize())->toMat();
        sorted_targets.column(1) << TVec<int>(0, source->length() - 1, 1);
        sortRows(sorted_targets);
        index_to_rank.resize(source->length());
        if (reference) {
            // We define the targets based on the reference rankings.
            // First get the sorted target column of the reference.
            Mat ref_sorted_targets = reference->getSortedTargets();
            // Now find the inverse mapping from index to rank.
            int ref_index = 0;
            int the_index;
            for (int i = 0; i < sorted_targets.length(); i++) {
                while (ref_index < ref_sorted_targets.length() &&
                       sorted_targets(i,0) > ref_sorted_targets(ref_index,0))
                    ref_index++;
                if (ref_index == 0)
                    // The first target higher or equal is the 0-th one.
                    the_index = 0;
                else if (ref_index == sorted_targets.length())
                    // There is no target higher or equal.
                    the_index = sorted_targets.length() - 1;
                else if (fast_exact_is_equal(sorted_targets(i,0),
                                             ref_sorted_targets(ref_index, 0)))
                    // We have an exact match.
                    the_index = ref_index;
                else {
                    // General case: we are in-between two targets. We choose the closest
                    // one.
                    if (fabs(sorted_targets(i,0) - ref_sorted_targets(ref_index,0)) <=
                        fabs(sorted_targets(i,0) - ref_sorted_targets(ref_index - 1 ,0)))
                        the_index = ref_index;
                    else
                        the_index = ref_index - 1;
                }
                index_to_rank[(int) sorted_targets(i,1)] = the_index;
            }
        } else {
            // Store the inverse mapping from index to rank.
            for (int i = 0; i < source->length(); i++)
                index_to_rank[(int) sorted_targets(i,1)] = i;
        }
        // Set VMat info.
        width_ = source->width() - source->targetsize() + 1;
        defineSizes(source->inputsize(), 1, source->weightsize());
        setMetaInfoFromSource();
    }
}

///////////////
// getNewRow //
///////////////
void RankedVMatrix::getNewRow(int i, const Vec& v) const
{
    source->getRow(i, v);
    // Replace the target with the rank.
    v[inputsize_] = index_to_rank[i];
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void RankedVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(index_to_rank, copies);
    deepCopyField(sorted_targets, copies);
    deepCopyField(reference, copies);
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
