// -*- C++ -*-

// AddBagInformationVMatrix.cc
//
// Copyright (C) 2007 Olivier Delalleau
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

/*! \file AddBagInformationVMatrix.cc */


#include "AddBagInformationVMatrix.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    AddBagInformationVMatrix,
    "Add bag information to an existing VMat.",
    "This VMatrix adds an extra target column to its source VMat, that\n"
    "contains bag information as follows:\n"
    "   - 1 = beginning of bag\n"
    "   - 2 = end of bag\n"
    "   - 0 = middle of bag\n"
    "   - 3 = single-row bag (both beginning and end)\n"
    "Bags are found by looking at a specific column of the source (given by\n"
    "the 'bag_info_column' option): a new bag is started when this column's\n"
    "value changes between two consecutive samples.\n"
);

//////////////////////////////
// AddBagInformationVMatrix //
//////////////////////////////
AddBagInformationVMatrix::AddBagInformationVMatrix():
    bag_info_idx(-1)
{
}

////////////////////
// declareOptions //
////////////////////
void AddBagInformationVMatrix::declareOptions(OptionList& ol)
{
    declareOption(ol, "bag_info_column",
                  &AddBagInformationVMatrix::bag_info_column,
                  OptionBase::buildoption,
        "The source's column that is used to find bags in the data. It can\n"
        "be either a number or a column's name.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void AddBagInformationVMatrix::build()
{
    // ### Nothing to add here, simply calls build_
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void AddBagInformationVMatrix::build_()
{
    if (source) {
        updateMtime(source);
        bag_info_idx = source->getFieldIndex(bag_info_column);
        sourcerow.resize(source->width());
        width_ = source->width() + 1;
        int st = source->targetsize();
        if (st >= 0)
            targetsize_ = st + 1;

        // Set field infos.
        Array<VMField> fields = source->getFieldInfos().copy();
        fields.append(VMField("bag_info"));
        setFieldInfos(fields);

        setMetaInfoFromSource();
    }
}

///////////////
// getNewRow //
///////////////
void AddBagInformationVMatrix::getNewRow(int i, const Vec& v) const
{
    bool is_beg = false;
    bool is_end = false;
    // Obtain current bag information.
    source->getRow(i, sourcerow);
    real cur = sourcerow[bag_info_idx];
    if (i == 0)
        is_beg = true;
    else {
        // Compare with previous sample.
        real prev = source->get(i - 1, bag_info_idx);
        if (!is_equal(cur, prev))
            is_beg = true;
    }
    if (i == length_ - 1)
        is_end = true;
    else {
        // Compare with next sample.
        real next = source->get(i + 1, bag_info_idx);
        if (!is_equal(cur, next))
            is_end = true;
    }
    real bag_info = is_beg ? is_end ? 3
                                    : 1
                           : is_end ? 2
                                    : 0;
    v.subVec(0, sourcerow.length()) << sourcerow;
    v.lastElement() = bag_info;
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void AddBagInformationVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
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
