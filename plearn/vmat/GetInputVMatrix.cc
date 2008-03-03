// -*- C++ -*-

// GetInputVMatrix.cc
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

/*! \file GetInputVMatrix.cc */


#include "GetInputVMatrix.h"

namespace PLearn {
using namespace std;

/////////////////////
// GetInputVMatrix //
/////////////////////
GetInputVMatrix::GetInputVMatrix()
{
}

GetInputVMatrix::GetInputVMatrix(VMat the_source) {
    this->source = the_source;
    build_();
}

PLEARN_IMPLEMENT_OBJECT(GetInputVMatrix,
                        "This VMatrix only sees the input part of its source VMatrix.",
                        ""
    );

////////////////////
// declareOptions //
////////////////////
void GetInputVMatrix::declareOptions(OptionList& ol)
{
    // ### ex:
    // declareOption(ol, "myoption", &GetInputVMatrix::myoption, OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void GetInputVMatrix::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void GetInputVMatrix::build_()
{
    weightsize_ = 0;
    if (source) {
        updateMtime(source);
        if(targetsize_ < 0) targetsize_ = 0;
        if(inputsize_ < 0) inputsize_ = source->inputsize();

        if (inputsize_ < 0) {
            PLERROR("In GetInputVMatrix::build_ - The source VMatrix doesn't have an inputsize defined");
        }
        indices.resize(source->inputsize());
        for (int i = 0; i < source->inputsize(); i++) {
            indices[i] = i;
        }

        // We need to rebuild the SelectColumnsVMatrix.
        inherited::build();
        if(inputsize_+targetsize_ != width_) PLERROR("In GetInputVMatrix:build_() : inputsize_ + targetsize_ != width_");
    }
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void GetInputVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
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
