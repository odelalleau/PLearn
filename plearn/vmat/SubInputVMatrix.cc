// -*- C++ -*-

// SubInputVMatrix.cc
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

/*! \file SubInputVMatrix.cc */

#include "SubInputVMatrix.h"

namespace PLearn {
using namespace std;

//////////////////
// SubInputVMatrix //
//////////////////
SubInputVMatrix::SubInputVMatrix()
    : inherited(),
      j_start(0),
      n_inputs(-1)
{
}

PLEARN_IMPLEMENT_OBJECT(SubInputVMatrix,
                        "A VMat that only takes part of the input of its source VMat.",
                        "This can be useful for instance to only take the first k components\n"
                        "after applying some dimensionality reduction method.\n"
                        "The other columns (target and weight) are copied from the source.\n"
    );

////////////////////
// declareOptions //
////////////////////
void SubInputVMatrix::declareOptions(OptionList& ol)
{
    declareOption(ol, "j_start", &SubInputVMatrix::j_start, OptionBase::buildoption,
                  "The column we start at.");

    declareOption(ol, "n_inputs", &SubInputVMatrix::n_inputs, OptionBase::buildoption,
                  "The number of inputs to keep (-1 means we keep them all, from j_start).");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void SubInputVMatrix::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void SubInputVMatrix::build_()
{
    if (source) {
        if (source->inputsize() < 0)
            PLERROR("In SubInputVMatrix::build_ - The source's inputsize must be set");
        if (n_inputs == -1) {
            // Default value: we keep all inputs.
            n_inputs = source->inputsize() - j_start;
        }
        if (n_inputs < 0 || n_inputs + j_start > source->inputsize()) {
            PLERROR("In SubInputVMatrix::build_ - Source VMatrix hasn't enough inputs");
        }
        int n_removed = source->inputsize() - n_inputs;
        inputsize_ = n_inputs;
        width_ = source->width() - n_removed;
        // Set field infos.
        Array<VMField>& source_infos = source->getFieldInfos();
        TVec<VMField> finfos(width_);
        finfos.subVec(0, inputsize_) << source_infos.subVec(0, inputsize_);
        finfos.subVec(inputsize_, finfos.length() - inputsize_)
            << source_infos.subVec(source->inputsize(), source_infos.length() - source->inputsize());
        setFieldInfos(finfos);
        // Set other meta information.
        setMetaInfoFromSource();
    }
}

///////////////
// getNewRow //
///////////////
void SubInputVMatrix::getNewRow(int i, const Vec& v) const
{
    // First fill the input part.
    source->getSubRow(i, j_start, v.subVec(0, n_inputs));
    // Then the rest (target + weight).
    source->getSubRow(
        i,
        source->inputsize(),
        v.subVec(n_inputs, v.length() - n_inputs));
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void SubInputVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
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
