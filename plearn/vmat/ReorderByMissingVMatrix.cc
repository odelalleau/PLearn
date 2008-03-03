// -*- C++ -*-

// ReorderByMissingVMatrix.cc
//
// Copyright (C) 2005 Olivier Delalleau
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
   * $Id: .pyskeleton_header 544 2003-09-01 00:05:31Z plearner $
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file ReorderByMissingVMatrix.cc */


#include "ReorderByMissingVMatrix.h"
#include <vector>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    ReorderByMissingVMatrix,
    "Re-order samples in a source VMat by their missing attributes",

    "Each sample is associated with a binary string indicating which elements\n"
    "are missing or not.\n"
    "For instance, the string '1001' indicates that the second and third\n"
    "features are missing, while the first and fourth ones are present.\n"
    "Samples are then sorted according to the usual lexicographic order on\n"
    "these strings.\n"
);

/////////////////////////////
// ReorderByMissingVMatrix //
/////////////////////////////
ReorderByMissingVMatrix::ReorderByMissingVMatrix():
    verbosity(1)
{}

////////////////////
// declareOptions //
////////////////////
void ReorderByMissingVMatrix::declareOptions(OptionList& ol)
{

    // Build options.

    declareOption(ol, "verbosity", &ReorderByMissingVMatrix::verbosity,
                                   OptionBase::buildoption,
        "Control the amount of output.");

    // Learnt options.

    declareOption(ol, "missing_pattern_change",
                  &ReorderByMissingVMatrix::missing_pattern_change,
                  OptionBase::learntoption,
        "A vector whose i-th element is a boolean indicating whether the\n"
        "missing pattern has changed going from the (i-1)-th sample to the\n"
        "i-th sample (note: the first element is always true).");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);

    // Hide unused options.

    redeclareOption(ol, "indices", &ReorderByMissingVMatrix::indices,
                    OptionBase::nosave, "Not used.");

    redeclareOption(ol, "indices_vmat", &ReorderByMissingVMatrix::indices_vmat,
                    OptionBase::nosave, "Not used.");

}

///////////
// build //
///////////
void ReorderByMissingVMatrix::build()
{
    inherited::build();
    build_();
}

//! Simple class representing one sample (given by its index) with a string of
//! '0' and '1' where '0' represents a missing value and '1' a non-missing one.
struct IndexAndMissingFlags {
    int index;
    string missing_flags;
};

//! Comparison function used in sorting.
struct compareIndexAndMissingFlags {
    bool operator() (const IndexAndMissingFlags& x,
                     const IndexAndMissingFlags& y)
    {
        return (x.missing_flags < y.missing_flags);
    }
} compare_index_and_missing_flags;


////////////
// build_ //
////////////
void ReorderByMissingVMatrix::build_()
{
    updateMtime(indices_vmat);
    updateMtime(source);
    if (source) {
        // Construct a vector containing each sample index associated with its
        // missing flags.
        vector<IndexAndMissingFlags> vec;
        int n = source.length();
        int w = source.width();
        sourcerow.resize(w);
        string previous_flags;
        int n_flag_changes = 0;
        for (int i = 0; i < n; i++) {
            source->getRow(i, sourcerow);
            string missing_flags;
            for (int j = 0; j < w; j++)
                if (is_missing(sourcerow[j]))
                    missing_flags += '0';
                else
                    missing_flags += '1';
            IndexAndMissingFlags ex;
            ex.index = i;
            ex.missing_flags = missing_flags;
            vec.push_back(ex);
            if (!previous_flags.empty() && missing_flags != previous_flags)
                n_flag_changes++;
            previous_flags = missing_flags;
        }
        if (verbosity >= 1)
            pout << "Number of flag changes before sorting: "
                 << n_flag_changes << endl;
        // Sort this vector.
        sort(vec.begin(), vec.end(), compare_index_and_missing_flags);
        // Build the 'indices' vector.
        indices.resize(n);
        indices.resize(0);
        vector<IndexAndMissingFlags>::const_iterator it = vec.begin();
        previous_flags = "";
        n_flag_changes = 0;
        int index = 0;
        missing_pattern_change.resize(int(vec.size()));
        for (; it != vec.end(); it++) {
            indices.append(it->index);
            const string& missing_flags = it->missing_flags;
            if (missing_flags != previous_flags) {
                if (!previous_flags.empty())
                    n_flag_changes++;
                missing_pattern_change[index++] = true;
            } else
                missing_pattern_change[index++] = false;
            previous_flags = missing_flags;
        }
        if (verbosity >= 1)
            pout << "Number of flag changes after sorting: "
                 << n_flag_changes << endl;
        // Re-build the parent class according to the new 'indices' vector.
        inherited::build();
    }
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void ReorderByMissingVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(missing_pattern_change,  copies);
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
