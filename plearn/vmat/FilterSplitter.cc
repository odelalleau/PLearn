// -*- C++ -*-

// FilterSplitter.cc
//
// Copyright (C) 2004 Pierre-Jean L Heureux
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

// Authors: Pierre-Jean L Heureux

/*! \file FilterSplitter.cc */

#include <plearn/io/fileutils.h>   //!< For newFilename.
#include "FilterSplitter.h"
#include "FilteredVMatrix.h"

namespace PLearn {
using namespace std;

FilterSplitter::FilterSplitter()
    : report_progress(false)
{
}

PLEARN_IMPLEMENT_OBJECT(FilterSplitter,
                        "Each set of this Splitter is a FilteredVMatrix of the dataset.",
                        ""
    );

void FilterSplitter::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    declareOption(ol, "filters", &FilterSplitter::filters, OptionBase::buildoption,
                  "array of strings: The VPL code for each set that should produce a single scalar\n"
                  ", indicating whether we should keep the line (if the produced scalar is non zero)\n"
                  "or throw it away (if it's zero)");

    declareOption(ol, "report_progress", &FilterSplitter::report_progress, OptionBase::buildoption,
                  "Whether to report or not the progress in filtering.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void FilterSplitter::build_()
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
void FilterSplitter::build()
{
    inherited::build();
    build_();
}

void FilterSplitter::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    Splitter::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(filters, copies);
}

int FilterSplitter::nsplits() const
{
    // ### Return the number of available splits
    return 1;
}

int FilterSplitter::nSetsPerSplit() const
{
    // ### Return the number of sets per split
    return filters.length();
}

//////////////
// getSplit //
//////////////
TVec<VMat> FilterSplitter::getSplit(int k)
{
    if (k != 0) PLERROR("This splitter will only create a single split");
    // ### Build and return the kth split
    TVec<VMat> splitsets;
    for (int i=0; i<filters.length(); i++) {
        splitsets.append(new FilteredVMatrix(dataset, filters[i], newFilename("/tmp", "filtered_vmatrix_temp_dir_", true), report_progress));
    }
    return splitsets;
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
