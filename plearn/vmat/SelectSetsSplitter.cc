// -*- C++ -*-

// SelectSetsSplitter.cc
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

/*! \file SelectSetsSplitter.cc */


#include "SelectSetsSplitter.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    SelectSetsSplitter,
    "Selects a subset of sets from an underlying splitter.",
    ""
);

SelectSetsSplitter::SelectSetsSplitter()
{}

////////////////////
// declareOptions //
////////////////////
void SelectSetsSplitter::declareOptions(OptionList& ol)
{
    declareOption(ol, "splitter", &SelectSetsSplitter::splitter,
                  OptionBase::buildoption,
        "The underlying splitter.");

    declareOption(ol, "indices", &SelectSetsSplitter::indices,
                  OptionBase::buildoption,
        "Indices of sets being selected.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void SelectSetsSplitter::build_()
{}

void SelectSetsSplitter::build()
{
    inherited::build();
    build_();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void SelectSetsSplitter::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(indices,  copies);
    deepCopyField(splitter, copies);
}

/////////////
// nsplits //
/////////////
int SelectSetsSplitter::nsplits() const
{
    return splitter->nsplits();
}

///////////////////
// nSetsPerSplit //
///////////////////
int SelectSetsSplitter::nSetsPerSplit() const
{
    return indices.length();
}

//////////////
// getSplit //
//////////////
TVec<VMat> SelectSetsSplitter::getSplit(int k)
{
    TVec<VMat> sub = splitter->getSplit(k);
    TVec<VMat> result;
    for (int i = 0; i < indices.length(); i++)
        result.append(sub[indices[i]]);
    return result;
}

////////////////
// setDataSet //
////////////////
void SelectSetsSplitter::setDataSet(VMat the_dataset)
{
    splitter->setDataSet(the_dataset);
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
