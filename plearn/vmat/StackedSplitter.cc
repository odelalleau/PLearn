// -*- C++ -*-

// StackedSplitter.cc
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

/*! \file StackedSplitter.cc */


#include "NoSplitSplitter.h"
#include "StackedSplitter.h"

namespace PLearn {
using namespace std;

/////////////////////
// StackedSplitter //
/////////////////////
StackedSplitter::StackedSplitter()
    : last_k_init(-1)
{
}

PLEARN_IMPLEMENT_OBJECT(StackedSplitter,
                        "Applies additional splitters on the splits of a first splitter.",
                        "Each set of a split of the initial splitter is split again by another splitter,\n"
                        "which is different for each set. If no splitter is given for some set, then this\n"
                        "set remains unchanged.\n"
    );

////////////////////
// declareOptions //
////////////////////
void StackedSplitter::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    declareOption(ol, "initial_splitter", &StackedSplitter::initial_splitter, OptionBase::buildoption,
                  "The initial splitter to be used.");

    declareOption(ol, "top_splitters", &StackedSplitter::top_splitters, OptionBase::buildoption,
                  "The splitters applied on each set of the initial splitter. One must provide\n"
                  "initial_splitters->nSetsPerSplit() splitters (*0 means no splitter).");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void StackedSplitter::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void StackedSplitter::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation.
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
    // ### You should assume that the parent class' build_() has already been called.
    if (initial_splitter && top_splitters.isNotEmpty()) {
        if (initial_splitter->nSetsPerSplit() != top_splitters.length())
            PLERROR("In StackedSplitter::build_ - initial_splitter->nSetsPerSplit() != top_splitters.length()");
        // Replace each null splitter with a NoSplitSplitter
        for (int i = 0; i < top_splitters.length(); i++)
            if (!top_splitters[i])
                top_splitters[i] = new NoSplitSplitter();
        // Make sure all splitters have a consistent number of splits.
        int ns = top_splitters[0]->nsplits();
        for (int i = 1; i < top_splitters.length(); i++)
            if (top_splitters[i]->nsplits() != ns)
                PLERROR("In StackedSplitter::build_ - All splitters in 'top_splitters' must give the same number of splits");
    }
}

//////////////
// getSplit //
//////////////
TVec<VMat> StackedSplitter::getSplit(int k)
{
    TVec<VMat> result;
    int k_init = k / top_splitters[0]->nsplits();
    int k_top = k % top_splitters[0]->nsplits();
    if (k_init != last_k_init) {
        // We need to recompute the split given by the initial splitter.
        last_split_init = initial_splitter->getSplit(k_init);
        last_k_init = k_init;
        // Assign each set to its top splitter.
        for (int i = 0; i < top_splitters.length(); i++) {
            top_splitters[i]->setDataSet(last_split_init[i]);
        }
    }
    for (int i = 0; i < top_splitters.length(); i++) {
        result->append(top_splitters[i]->getSplit(k_top));
    }
    return result;
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void StackedSplitter::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("StackedSplitter::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

///////////////////
// nSetsPerSplit //
///////////////////
int StackedSplitter::nSetsPerSplit() const
{
    int count = 0;
    for (int i = 0; i < top_splitters.length(); i++) {
        count += top_splitters[i]->nSetsPerSplit();
    }
    return count;
}

/////////////
// nsplits //
/////////////
int StackedSplitter::nsplits() const
{
    return initial_splitter->nsplits() * top_splitters[0]->nsplits();
}

////////////////
// setDataSet //
////////////////
void StackedSplitter::setDataSet(VMat the_dataset) {
    initial_splitter->setDataSet(the_dataset);
    // Reset 'last_k_init' since the dataset has changed.
    last_k_init = -1;
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
