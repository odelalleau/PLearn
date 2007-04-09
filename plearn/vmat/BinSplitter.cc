// -*- C++ -*-

// BinSplitter.cc
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

/*! \file BinSplitter.cc */


#include "BinSplitter.h"
#include <plearn/vmat/SelectRowsVMatrix.h>

namespace PLearn {
using namespace std;

/////////////////
// BinSplitter //
/////////////////
BinSplitter::BinSplitter()
    : column(0),
      column_spec("")
{
}

PLEARN_IMPLEMENT_OBJECT(BinSplitter,
                        "Split a dataset into bins given by a Binner object.",
                        ""
    );

////////////////////
// declareOptions //
////////////////////
void BinSplitter::declareOptions(OptionList& ol)
{
    declareOption(ol, "binner", &BinSplitter::binner, OptionBase::buildoption,
                  "The binner used to create the bins.");

    declareOption(ol, "column", &BinSplitter::column, OptionBase::buildoption,
                  "The column used to defined the bins.");

    declareOption(ol, "column_spec", &BinSplitter::column_spec, OptionBase::buildoption,
                  "Alternatively, this option can be used to override the 'column' option:\n"
                  " - 'first_input' : the first input column\n"
                  " - 'last_input'  : the last input column\n"
                  " - 'first_target': the first target column\n"
                  " - 'last_target' : the last target column");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void BinSplitter::build_()
{
}

///////////
// build //
///////////
void BinSplitter::build()
{
    inherited::build();
    build_();
}

void BinSplitter::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("BinSplitter::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

/////////////
// nsplits //
/////////////
int BinSplitter::nsplits() const
{
    return 1;
}

///////////////////
// nSetsPerSplit //
///////////////////
int BinSplitter::nSetsPerSplit() const
{
    return binner->nBins();
}

//////////////
// getSplit //
//////////////
TVec<VMat> BinSplitter::getSplit(int k)
{
    TVec<VMat> sets(nSetsPerSplit());
    for (int i = 0; i < sets.length(); i++) {
        sets[i] = new SelectRowsVMatrix(dataset, bins[i]);
    }
    return sets;
}

////////////////
// setDataSet //
////////////////
void BinSplitter::setDataSet(VMat the_dataset) {
    inherited::setDataSet(the_dataset);
    int col = column;
    if (column_spec != "") {
        if (column_spec == "first_input")
            col = 0;
        else if (column_spec == "last_input")
            col = the_dataset->inputsize() - 1;
        else if (column_spec == "first_target")
            col = the_dataset->inputsize();
        else if (column_spec == "last_target")
            col = the_dataset->inputsize() + the_dataset->targetsize() - 1;
        else
            PLERROR("In BinSplitter::setDataSet - Wrong value for 'column_spec'");
        if (col < 0)
            PLERROR("In BinSplitter::setDataSet - Found a negative column, check the dataset sizes are correctly defined");
    }
    else if (column >= the_dataset->width())
        PLERROR("In BinSplitter::setDataSet - The column number is higher than available");
    Vec col_vec = the_dataset.column(col)->toMat().toVec();
    if (!binner)
        PLERROR("In BinSplitter::setDataSet - You must provide a binner first.");
    bins = binner->getBins(col_vec);
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
