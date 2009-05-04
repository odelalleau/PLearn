// -*- C++ -*-

// OneVsAllVMatrix.cc
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
 * $Id: .pyskeleton_header,v 1.1 2003/09/01 00:05:31 plearner Exp $
 ******************************************************* */

// Authors: Hugo Larochelle

/*! \file OneVsAllVMatrix.cc */


#include "OneVsAllVMatrix.h"

namespace PLearn {
using namespace std;

//////////////////
// OneVsAllVMatrix //
//////////////////
OneVsAllVMatrix::OneVsAllVMatrix()
    : inherited(),
      target_class(0)
{

}

OneVsAllVMatrix::OneVsAllVMatrix(VMat the_source, int the_target_class,
                                 bool inverse_target)
    : inherited(the_source),
      target_class(the_target_class),
      inverse_target(inverse_target)
{
    build();
}


PLEARN_IMPLEMENT_OBJECT(OneVsAllVMatrix,
                        "Changes the target so that it is 1 if the target is equal to target_class, 0 if not",
                        ""
    );

////////////////////
// declareOptions //
////////////////////
void OneVsAllVMatrix::declareOptions(OptionList& ol)
{
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void OneVsAllVMatrix::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void OneVsAllVMatrix::build_()
{
    if(source)
    {
        if(source->targetsize() != 1)
            PLERROR("OneVsAllVMatrix::build_(): targetsize_ should be 1");

        updateMtime(source);
        setMetaInfoFromSource();

        PLCHECK(width_ == inputsize_+targetsize_+weightsize_+extrasize_);
        sourcerow.resize(source->width());
    }
}

///////////////
// getNewRow //
///////////////
void OneVsAllVMatrix::getNewRow(int i, const Vec& v) const
{
    source->getRow(i,v);
    bool t = int(v[inputsize_]) == target_class;
    v[inputsize_] = inverse_target? !t : t;
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void OneVsAllVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    //PLERROR("OneVsAllVMatrix::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
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
