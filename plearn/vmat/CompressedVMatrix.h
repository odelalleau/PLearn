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


/*! \file CompressedVMatrix.h */

#ifndef CompressedVMatrix_INC
#define CompressedVMatrix_INC

#include "RowBufferedVMatrix.h"
#include "VMat.h"

namespace PLearn {
using namespace std;

/*!   Like MemoryVMatrix this class holds the data in memory.
  But it is designed to keep a compact representation.
  Each row is compressed/decompressed through the methods of VecCompressor
  (it's the same encoding of row vectors that is used in the DiskVMatrix)
  This encodes only non zero values, using one byte for small integers,
  and 4-byte floating points for all other values.
  This representation should be reasonably good for both sparse matrices,
  and matrices containing categorical data (represented by small integers)
  possibly with one hot representations.
*/
class CompressedVMatrix: public RowBufferedVMatrix
{
    typedef RowBufferedVMatrix inherited;

protected:
    int max_length; //!<  maximum number of rows that can be appended to the initially empty matrix

    signed char* data;
    signed char** rowstarts;
    signed char* dataend; //!<  data+memory_alloc

    //!  next writing position in data chunk
    signed char* curpos;

/*!       This initializes the matrix with a length of 0 (but up to the_max_length rows can be appended)
  A memory_alloc of length*(2 + 4*width + width/128) should be enough in every case
  (and much less is needed for matrices containing a lot of 0 or small integers)
*/
    void init(int the_max_length, int the_width, size_t memory_alloc);

public:
    // ******************
    // *  Constructors  *
    // ******************
    CompressedVMatrix(); //!<  default constructor (for automatic deserialization)

/*!       This initializes the matrix with a length of 0 (but up to the_max_length rows can be appended)
  If no memory_alloc value is given, a sufficient default value will be used initially.
  You can always call reallocateCompactMemory() later to use the minimum memory
*/
    CompressedVMatrix(int the_max_length, int the_width, size_t memory_alloc=0);

/*!       This initializes a matrix from the data of another.
  If no memory_alloc value is given, a sufficient default value will be used initially.
  You can always call reallocateCompactMemory() later to use the minimum memory
*/
    CompressedVMatrix(VMat v, size_t memory_alloc=0);

    PLEARN_DECLARE_OBJECT(CompressedVMatrix);

protected:

    virtual void getNewRow(int i, const Vec& v) const;

public :

    virtual ~CompressedVMatrix();
    virtual void appendRow(Vec v);

    //!  returns the number of bytes allocated for the data
    size_t memoryAllocated() { return dataend-data; }

    //!  returns the memory actually used for the data
    size_t memoryUsed() { return curpos-data; }

    //!  This will reallocate the data chunk to be exactly of the needed size, so that no memory will be wasted.
    virtual void compacify();
};

DECLARE_OBJECT_PTR(CompressedVMatrix);

} // end of namespace PLearn
#endif


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
