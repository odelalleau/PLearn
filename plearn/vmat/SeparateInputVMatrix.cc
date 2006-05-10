// -*- C++ -*-

// SeparateInputVMatrix.cc
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
 * $Id$
 ******************************************************* */

// Authors: Hugo Larochelle

/*! \file SeparateInputVMatrix.cc */


#include "SeparateInputVMatrix.h"

namespace PLearn {
using namespace std;

//////////////////
// SeparateInputVMatrix //
//////////////////
SeparateInputVMatrix::SeparateInputVMatrix()
    : nsep(1)
{

    // build_();
}

PLEARN_IMPLEMENT_OBJECT(SeparateInputVMatrix,
                        "Separates the input in nsep parts and  distributes them on different rows.",
                        "Also copies target and weight parts for each of these rows."
    );

SeparateInputVMatrix::SeparateInputVMatrix(VMat the_source, int the_nsep)
    : inherited(the_source), nsep(the_nsep)
{
    build_();
}


////////////////////
// declareOptions //
////////////////////
void SeparateInputVMatrix::declareOptions(OptionList& ol)
{
    declareOption(ol, "nsep", &SeparateInputVMatrix::nsep, OptionBase::buildoption,
                  "Number of separations of the input. The input size has to be\n"
                  "a multiple of that value.");

    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void SeparateInputVMatrix::build()
{
    // ### Nothing to add here, simply calls build_
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void SeparateInputVMatrix::build_()
{
    if (source) {
        if(source->inputsize() % nsep != 0)
            PLERROR("In SeparateInputVMatrix::build_(): inputsize=%d of source vmat should be a multiple of nsep=%d",source->inputsize(),nsep);
        inputsize_ = source->inputsize()/nsep;
        targetsize_ = source->targetsize();
        weightsize_ = source->weightsize();
        //fieldinfos = source->fieldinfos;
        length_ = source.length() * nsep;
        width_ = inputsize_+targetsize_+weightsize_;
        sourcerow.resize(source->width());
    }
}

///////////////
// getNewRow //
///////////////
void SeparateInputVMatrix::getNewRow(int i, const Vec& v) const
{
    source->getRow(i/nsep,sourcerow);
    v.subVec(0,inputsize_) << sourcerow.subVec(i%nsep * inputsize_, inputsize_);
    v.subVec(inputsize_,targetsize_+weightsize_) <<
        sourcerow.subVec(source->inputsize(),targetsize_+weightsize_);
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void SeparateInputVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    //PLERROR("SeparateInputVMatrix::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
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
