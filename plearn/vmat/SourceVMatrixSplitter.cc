// -*- C++ -*-

// SourceVMatrixSplitter.cc
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

/*! \file SourceVMatrixSplitter.cc */


#include "SourceVMatrixSplitter.h"

namespace PLearn {
using namespace std;

///////////////////////////
// SourceVMatrixSplitter //
///////////////////////////
SourceVMatrixSplitter::SourceVMatrixSplitter()
    : to_apply(0)
{
}

PLEARN_IMPLEMENT_OBJECT(SourceVMatrixSplitter,
                        "Returns the splits of an underlying splitter, seen by a SourceVMatrix.",
                        "Not all sets have to be modified: the 'sets_to_modify' option allows one\n"
                        "to choose on which sets we apply the SourceVMatrix, which will be deep-copied\n"
                        "on each set.\n"
                        "Note that the provided SourceVMatrix may be modified in the process.\n"
    );

////////////////////
// declareOptions //
////////////////////
void SourceVMatrixSplitter::declareOptions(OptionList& ol)
{
    declareOption(ol, "source_vm", &SourceVMatrixSplitter::source_vm, OptionBase::buildoption,
                  "The VMatrix to apply.");

    declareOption(ol, "source_splitter", &SourceVMatrixSplitter::source_splitter, OptionBase::buildoption,
                  "The underlying splitter.");

    declareOption(ol, "to_apply", &SourceVMatrixSplitter::to_apply, OptionBase::buildoption,
                  "Deprecated: the index of the returned set where we apply source_vm.");

    declareOption(ol, "sets_to_modify", &SourceVMatrixSplitter::sets_to_modify, OptionBase::buildoption,
                  "The indices of the returned sets on which we apply source_vm ([-1] means all sets).\n"
                  "This option will override 'to_apply' if it is also provided.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void SourceVMatrixSplitter::build_()
{
    if (to_apply != 0)
        // This deprecated option is being used.
        PLDEPRECATED("In SourceVMatrixSplitter::build_ - Use 'sets_to_modify' instead of the deprecated "
                     "'to_apply' option.");
}

///////////
// build //
///////////
void SourceVMatrixSplitter::build()
{
    inherited::build();
    build_();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void SourceVMatrixSplitter::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("SourceVMatrixSplitter::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

/////////////
// nsplits //
/////////////
int SourceVMatrixSplitter::nsplits() const
{
    return source_splitter->nsplits();
}

///////////////////
// nSetsPerSplit //
///////////////////
int SourceVMatrixSplitter::nSetsPerSplit() const
{
    return source_splitter->nSetsPerSplit();
}

//////////////
// getSplit //
//////////////
TVec<VMat> SourceVMatrixSplitter::getSplit(int k)
{
    TVec<VMat> result = source_splitter->getSplit(k);
    if (sets_to_modify.isEmpty()) {
        // Old code, to remove when the deprecated option 'to_apply' is removed.
        PLDEPRECATED("In SourceVMatrixSplitter::getSplit - Use 'sets_to_modify' instead of the deprecated "
                     "'to_apply' option.");
        source_vm->source = result[to_apply];
        source_vm->build();
        VMat the_vm = static_cast<SourceVMatrix*>(source_vm);
        result[to_apply] = the_vm;
    } else {
        TVec<int> sets_indices = sets_to_modify;
        if (sets_indices.length() == 1 && sets_indices[0] == -1) {
            // -1 means we apply it on all sets.
            sets_indices = TVec<int>(0, source_splitter->nSetsPerSplit() - 1, 1);
        }
        for (int i = 0; i < sets_indices.length(); i++) {
            int set = sets_indices[i];
            // Clear the source of 'source_vm' since we do not want to deep-copy it.
            source_vm->source = 0;
            // Copy 'source_vm' and fill 'result' accordingly.
            PP<SourceVMatrix> vm_copy = PLearn::deepCopy(source_vm);
            vm_copy->source = result[set];
            vm_copy->build();
            result[set] = (SourceVMatrix*) vm_copy;
        }
    }
    return result;
}

////////////////
// setDataSet //
////////////////
void SourceVMatrixSplitter::setDataSet(VMat the_dataset) {
    inherited::setDataSet(the_dataset);
    source_splitter->setDataSet(the_dataset);
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
