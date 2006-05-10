// -*- C++ -*-

// TransposeVMatrix.cc
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

/*! \file TransposeVMatrix.cc */


#include "TransposeVMatrix.h"

namespace PLearn {
using namespace std;

//////////////////////
// TransposeVMatrix //
//////////////////////
TransposeVMatrix::TransposeVMatrix()
{}

TransposeVMatrix::TransposeVMatrix(const VMat& the_source)
{
    source = the_source;
    build();
}

PLEARN_IMPLEMENT_OBJECT(TransposeVMatrix,
                        "A VMatrix that sees the transpose of another VMatrix.",
                        ""
    );

////////////////////
// declareOptions //
////////////////////
void TransposeVMatrix::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    // ### ex:
    // declareOption(ol, "myoption", &TransposeVMatrix::myoption, OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void TransposeVMatrix::build()
{
    // ### Nothing to add here, simply calls build_
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void TransposeVMatrix::build_()
{
    if (source) {
        width_ = source->length();
        length_ = source->width();
        if (inputsize_ < 0)
            inputsize_ = width_;
        if (targetsize_ < 0)
            targetsize_ = 0;
        if (weightsize_ < 0)
            weightsize_ = 0;
        setMetaInfoFromSource();
    }
}

///////////////
// getNewRow //
///////////////
void TransposeVMatrix::getNewRow(int i, const Vec& v) const
{
    source->getColumn(i, v);
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void TransposeVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
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
