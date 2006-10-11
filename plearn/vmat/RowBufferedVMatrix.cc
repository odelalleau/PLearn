// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2001 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2002 Pascal Vincent, Julien Keable, Xavier Saint-Mleux
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

#include "RowBufferedVMatrix.h"
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;

/** RowBufferedVMatrix **/

PLEARN_IMPLEMENT_ABSTRACT_OBJECT(
    RowBufferedVMatrix,
    "Base class for VMatrices keeping the last row(s) in a buffer for faster access.",
    ""
);

RowBufferedVMatrix::RowBufferedVMatrix(int the_length, int the_width,
                                       bool call_build_):
    inherited           (the_length, the_width, call_build_),
    current_row_index   (-1),
    current_row         (the_width),
    other_row_index     (-1),
    other_row           (the_width)
{
    // Never calling build_() since it is not overridden for this class.
}

RowBufferedVMatrix::RowBufferedVMatrix(bool call_build_):
    inherited        (call_build_),
    current_row_index(-1),
    other_row_index  (-1)
{
    // Never calling build_() since it is not overridden for this class.
}

void RowBufferedVMatrix::invalidateBuffer() const
{
    current_row_index = -1;
    other_row_index = -1;
}

real RowBufferedVMatrix::get(int i, int j) const
{
    if(current_row_index!=i)
    {
#ifdef BOUNDCHECK
        if (i < 0 || (i >= length() && length() >= 0))
            PLERROR("In RowBufferedVMatrix::get: row index (%d) outside valid range [%d,%d]", i, 0, length_-1);
        if (j < 0 || j >= width())
            PLERROR("In RowBufferedVMatrix::get: column index (%d) outside valid range [%d,%d]", j, 0, width_-1);
#endif
        current_row.resize(width_);
        getNewRow(i, current_row);
        current_row_index = i;
    }
    return current_row[j];
}


void RowBufferedVMatrix::getRow(int i, Vec v) const {
    if (current_row_index != i) {
#ifdef BOUNDCHECK
        if (i < 0 || (i >= length() && length() >= 0))
            PLERROR("In RowBufferedVMatrix::getRow: row index (%d) outside valid range [%d,%d]", i, 0, length_-1);
#endif
        current_row.resize(width_);
        getNewRow(i, current_row);
        current_row_index = i;
    }
    if (width_ > 0)
        v.copyFrom(current_row.data(), width_);
}


void RowBufferedVMatrix::getSubRow(int i, int j, Vec v) const
{
    if(current_row_index!=i)
    {
#ifdef BOUNDCHECK
        if ((i < 0 || i >= length()) && length() >= 0)
            PLERROR("In RowBufferedVMatrix::getSubRow: row index (%d) outside valid range [%d,%d]", i, 0, length_-1);
        if ((j < 0 || j >= width()) && width() >= 0)
            PLERROR("In RowBufferedVMatrix::getSubRow: column index (%d) outside valid range [%d,%d]", i, 0, width_-1);
#endif
        current_row.resize(width_);
        getNewRow(i,current_row);
        current_row_index = i;
    }
    if (v.length() > 0)
        v.copyFrom(current_row.data()+j, v.length());
}


real RowBufferedVMatrix::dot(int i1, int i2, int inputsize) const
{
#ifdef BOUNDCHECK
    if (i1 < 0 || (i1 >= length() && length() >= 0))
        PLERROR("In RowBufferedVMatrix::dot: first row index (%d) outside valid range [%d,%d]", i1, 0, length_-1);
    if (i2 < 0 || (i2 >= length() && length() >= 0))
        PLERROR("In RowBufferedVMatrix::dot: second row index (%d) outside valid range [%d,%d]", i2, 0, length_-1);
#endif
    int w = width_;
    current_row.resize(w);
    other_row.resize(w);

    if(i1==current_row_index)
    {
        if(i2==i1)
            return pownorm(current_row.subVec(0,inputsize));
        if(i2!=other_row_index)
        {
            getNewRow(i2,other_row);
            other_row_index = i2;
        }
    }
    else if(i1==other_row_index)
    {
        if(i2==i1)
            return pownorm(other_row.subVec(0,inputsize));
        if(i2!=current_row_index)
        {
            getNewRow(i2,current_row);
            current_row_index = i2;
        }
    }
    else // i1 not cached
    {
        if(i2==current_row_index)
        {
            getNewRow(i1,other_row);
            other_row_index = i1;
        }
        else if(i2==other_row_index)
        {
            getNewRow(i1,current_row);
            current_row_index = i1;
        }
        else // neither i1 nor i2 are cached
        {
            getNewRow(i1,current_row);
            getNewRow(i2,other_row);
            current_row_index = i1;
            other_row_index = i2;
        }
    }
    return PLearn::dot(current_row.subVec(0,inputsize), other_row.subVec(0,inputsize));
}

/////////
// dot //
/////////
real RowBufferedVMatrix::dot(int i, const Vec& v) const
{
    if(i!=current_row_index)
    {
#ifdef BOUNDCHECK
        if (i < 0 || i >= length())
            PLERROR("In RowBufferedVMatrix::dot: row index (%d) outside valid range [%d,%d]", i, 0, length_-1);
#endif
        current_row.resize(width_);
        getNewRow(i,current_row);
        current_row_index = i;
    }
    return PLearn::dot(current_row.subVec(0,v.length()),v);
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void RowBufferedVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies) {
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(current_row, copies);
    deepCopyField(other_row, copies);
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
