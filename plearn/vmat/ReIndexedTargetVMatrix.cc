// -*- C++ -*-

// ReIndexedTargetVMatrix.cc
//
// Copyright (C) 2005 Hugo Larochelle
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

// Authors: Hugo Larochelle

/*! \file ReIndexedTargetVMatrix.cc */


#include "ReIndexedTargetVMatrix.h"
#include <plearn/dict/Dictionary.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    ReIndexedTargetVMatrix,
    "ONE LINE DESCRIPTION",
    "MULTI-LINE \nHELP"
    );

//////////////////
// ReIndexedTargetVMatrix //
//////////////////
ReIndexedTargetVMatrix::ReIndexedTargetVMatrix()
    : ignore_oov_tag(false)
{}

////////////////////
// declareOptions //
////////////////////
void ReIndexedTargetVMatrix::declareOptions(OptionList& ol)
{
    declareOption(ol, "ignore_oov_tag", &ReIndexedTargetVMatrix::ignore_oov_tag, OptionBase::buildoption,
                  "Indication that the OOV_TAG must be ignored in the possible values.\n");

    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void ReIndexedTargetVMatrix::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void ReIndexedTargetVMatrix::build_()
{
    setMetaInfoFromSource();
}

///////////////
// getNewRow //
///////////////
void ReIndexedTargetVMatrix::getNewRow(int i, const Vec& v) const
{
    source->getRow(i,v);
    for(t=inputsize_; t<targetsize_+inputsize_; t++)
    {
        source->getValues(i,t,values);

        oov_index = -1;
        if(ignore_oov_tag)
        {
            oov_index = source->getDictionary()->getId(OOV_TAG);
            oov_index = values.find(oov_index);
        }
        if(oov_index < 0) oov_index = values.length();

        index = values.find(v[t]);
        if(index < 0) PLERROR("In ReIndexedTargetVMatrix::getNewRow(): target (%d) isn't a possible value", v[t]);
        if(index == oov_index) PLERROR("In ReIndexedTargetVMatrix::getNewRow(): target (%d) is OOV_TAG, but OOV_TAG ignored", v[t]);
        v[t] = index<oov_index?index:index-1;
    }
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void ReIndexedTargetVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
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
