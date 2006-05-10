// -*- C++ -*-

// ViewSplitterVMatrix.cc
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
 * $Id$
 ******************************************************* */

// Authors: Olivier Delalleau

/*! \file ViewSplitterVMatrix.cc */


#include "ViewSplitterVMatrix.h"
#include <plearn/vmat/Splitter.h>

namespace PLearn {
using namespace std;

/////////////////////////
// ViewSplitterVMatrix //
/////////////////////////
ViewSplitterVMatrix::ViewSplitterVMatrix()
    : set(0),
      split(0)
{
}

PLEARN_IMPLEMENT_OBJECT(ViewSplitterVMatrix,
                        "This VMatrix can be used to extract a Splitter's split and set.",
                        "This is especially useful to check the output of a Splitter on a given\n"
                        "source VMat.\n"
    );

////////////////////
// declareOptions //
////////////////////
void ViewSplitterVMatrix::declareOptions(OptionList& ol)
{
    declareOption(ol, "splitter", &ViewSplitterVMatrix::splitter, OptionBase::buildoption,
                  "The splitter applied on the source VMat.");

    declareOption(ol, "split", &ViewSplitterVMatrix::split, OptionBase::buildoption,
                  "The index of the wanted split.");

    declareOption(ol, "set", &ViewSplitterVMatrix::set, OptionBase::buildoption,
                  "The index of the wanted set.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void ViewSplitterVMatrix::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void ViewSplitterVMatrix::build_()
{
    splitter->setDataSet(source);
    sets = splitter->getSplit(split);
    setMetaInfoFrom(sets[set]);
}

///////////////
// getNewRow //
///////////////
void ViewSplitterVMatrix::getNewRow(int i, const Vec& v) const
{
    sets[set]->getRow(i, v);
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void ViewSplitterVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("ViewSplitterVMatrix::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
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
