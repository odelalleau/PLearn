// -*- C++ -*-

// PutSubVMatrix.cc
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

/*! \file PutSubVMatrix.cc */


#include "PutSubVMatrix.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    PutSubVMatrix,
    "Replace a sub-matrix of its source by another VMat.",
    "Keep in mind that field names and string mappings of the imputed submat\n"
    "will be ignored."
);

//////////////////
// PutSubVMatrix //
//////////////////
PutSubVMatrix::PutSubVMatrix():
    istart(0),
    jstart(0)
{}

////////////////////
// declareOptions //
////////////////////
void PutSubVMatrix::declareOptions(OptionList& ol)
{
    declareOption(ol, "submat", &PutSubVMatrix::submat,
                  OptionBase::buildoption,
        "The data matrix to put in the source.");

    declareOption(ol, "istart", &PutSubVMatrix::istart,
                  OptionBase::buildoption,
        "Row index where 'submat' will be put.");

    declareOption(ol, "jstart", &PutSubVMatrix::jstart,
                  OptionBase::buildoption,
        "Column index where 'submat' will be put.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void PutSubVMatrix::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void PutSubVMatrix::build_()
{
    if (!source)
        return;

    if (submat) {
        updateMtime(submat);
        PLCHECK( istart + submat.length() <= source->length() );
        PLCHECK( jstart + submat.width() <= source->width() );
        subrow.resize(submat->width());
    }

    setMetaInfoFromSource();
}

///////////////
// getNewRow //
///////////////
void PutSubVMatrix::getNewRow(int i, const Vec& v) const
{
    source->getRow(i, v);
    if (submat && submat->length() > 0 && subrow.length() > 0) {
        if (i >= istart && i < istart + submat->length()) {
            submat->getRow(i - istart, subrow);
            for (int j = 0; j < subrow.length(); j++)
                v[jstart + j] = subrow[j];
        }
    }
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void PutSubVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("PutSubVMatrix::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
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
