// -*- C++ -*-

// ConcatSetsSplitter.cc
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

/*! \file ConcatSetsSplitter.cc */


#include "ConcatSetsSplitter.h"

namespace PLearn {
using namespace std;

////////////////////////
// ConcatSetsSplitter //
////////////////////////
ConcatSetsSplitter::ConcatSetsSplitter()
{
}

PLEARN_IMPLEMENT_OBJECT(ConcatSetsSplitter,
                        "Concatenates the sets of many splitters.",
                        "The concatenated splitters must have the same number of splits."
    );

////////////////////
// declareOptions //
////////////////////
void ConcatSetsSplitter::declareOptions(OptionList& ol)
{
    declareOption(ol, "splitters", &ConcatSetsSplitter::splitters, OptionBase::buildoption,
                  "The splitters whose sets we want to concatenate.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void ConcatSetsSplitter::build_()
{
    if (splitters.length() > 0) {
        int nsplits = splitters[0]->nsplits();
        for (int i = 1; i < splitters.length(); i++)
            if (splitters[i]->nsplits() != nsplits)
                PLERROR("In ConcatSetsSplitter::build_ - All concatenated splitters must return the same number of splits");
    }
}

///////////
// build //
///////////
void ConcatSetsSplitter::build()
{
    inherited::build();
    build_();
}

void ConcatSetsSplitter::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("ConcatSetsSplitter::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

/////////////
// nsplits //
/////////////
int ConcatSetsSplitter::nsplits() const
{
    return splitters[0]->nsplits();
}

///////////////////
// nSetsPerSplit //
///////////////////
int ConcatSetsSplitter::nSetsPerSplit() const
{
    int count = 0;
    for (int i = 0; i < splitters.length(); i++)
        count += splitters[i]->nSetsPerSplit();
    return count;
}

//////////////
// getSplit //
//////////////
TVec<VMat> ConcatSetsSplitter::getSplit(int k)
{
    TVec<VMat> sets;
    for (int i = 0; i < splitters.length(); i++) {
        sets.append(splitters[i]->getSplit(k));
    }
    return sets;
}

////////////////
// setDataSet //
////////////////
void ConcatSetsSplitter::setDataSet(VMat the_dataset) {
    inherited::setDataSet(the_dataset);
    for (int i = 0; i < splitters.length(); i++) {
        splitters[i]->setDataSet(the_dataset);
    }
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
