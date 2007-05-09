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

#include "CompressedVMatrix.h"
#include <plearn/math/VecCompressor.h>

namespace PLearn {
using namespace std;


// ************************
// ** CompressedVMatrix **

PLEARN_IMPLEMENT_OBJECT(CompressedVMatrix, "ONE LINE DESCR", "ONE LINE HELP");

CompressedVMatrix::CompressedVMatrix()
    : data(0), rowstarts(0), dataend(0), curpos(0)
{
}

CompressedVMatrix::CompressedVMatrix(int the_max_length, int the_width, size_t memory_alloc)
{ init(the_max_length, the_width,
       memory_alloc!=0 ?memory_alloc :the_max_length*VecCompressor::worstCaseSize(the_width)); }

void CompressedVMatrix::init(int the_max_length, int the_width, size_t memory_alloc)
{
    length_ = 0;
    width_ = the_width;
    max_length = the_max_length;
    data = new signed char[memory_alloc];
    dataend = data+memory_alloc;
    curpos = data;
    rowstarts = new signed char*[max_length];
}

CompressedVMatrix::CompressedVMatrix(VMat m, size_t memory_alloc)
{
    if(memory_alloc==0)
        init(m.length(), m.width(), m.length()*VecCompressor::worstCaseSize(m.width()));
    Vec v(m.width());
    for(int i=0; i<m.length(); i++)
    {
        m->getRow(i,v);
        appendRow(v);
    }
}

void CompressedVMatrix::getNewRow(int i, const Vec& v) const
{
#ifdef BOUNDCHECK
    if(v.length() != width_)
        PLERROR("In CompressedVMatrix::getNewRow length of v and width of matrix do not match");
    if(i<0 || i>=length_)
        PLERROR("In CompressedVMatrix::getNewRow OUT OF BOUNDS row index");
#endif
    VecCompressor::uncompressVec(rowstarts[i],v);
}

void CompressedVMatrix::appendRow(Vec v)
{
    if(length_>=max_length)
        PLERROR("In CompressedVMatrix::appendRow, max_length exceeded");
    rowstarts[length_] = curpos;
    curpos = VecCompressor::compressVec(v,curpos);
    if(curpos>dataend)
        PLERROR("In CompressedVMatrix::appendRow not enough space reserved for data");
    ++length_;
}

void CompressedVMatrix::compacify()
{
    size_t datasize = curpos-data;
    signed char* old_data = data;
    signed char** old_rowstarts = rowstarts;
    data = new signed char[datasize];
    dataend = data+datasize;
    curpos = dataend;
    rowstarts = new signed char*[length_];
    memcpy(data, old_data, datasize);

    for(int i=0; i<length_; i++)
        rowstarts[i] = data + (old_rowstarts[i]-old_data);
    max_length = length_;
    delete[] old_data;
    delete[] old_rowstarts;
}

CompressedVMatrix::~CompressedVMatrix()
{
    if(data)
        delete[] data;
    if(rowstarts)
        delete[] rowstarts;
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
