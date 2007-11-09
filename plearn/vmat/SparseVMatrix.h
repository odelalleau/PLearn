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


/*! \file SparseVMatrix.h */

#ifndef SparseVMatrix_INC
#define SparseVMatrix_INC

#include "RowBufferedVMatrix.h"
#include "VMat.h"

namespace PLearn {
using namespace std;


/*!   Like MemoryVMatrix this class holds the data in memory.
  But it is designed to keep a compact representation of sparse matrices,
  keeping for each row only the position and values of the non-zero elements.
  The values are stored as floats regardless whether we ar in USEFLOAT or USEDOUBLE mode.
*/

class SparseVMatrixRow
{
public:
    int nelements; //!<  number of non zero elements in row
    int row_startpos; //!<  index of first element of this row in both the positions and the values arrays
    SparseVMatrixRow(): nelements(0), row_startpos(0) {}
};

class SparseVMatrix : public RowBufferedVMatrix
{
    typedef RowBufferedVMatrix inherited;

protected:
    int nelements; //!<  total number of non-zero elements in the VMatrix
    unsigned short* positions;
    float* values;

    SparseVMatrixRow* rows;

public:

    SparseVMatrix():
        nelements(0),
        positions(0),
        values(0),
        rows(0)
    {}

    //!  This builds a sparse representation in memory of the VMat m passed
    //!  as argument.  The original fieldinfos are copied as-is.
    SparseVMatrix(VMat m);

    //!  This reloads a previously saved sparse VMatrix
    SparseVMatrix(const string& filename);

    PLEARN_DECLARE_OBJECT(SparseVMatrix);

protected:

    static void declareOptions(OptionList &ol);
    virtual void getNewRow(int i, const Vec& v) const;

public:

    virtual void build();

    virtual real dot(int i1, int i2, int inputsize) const;
    virtual real dot(int i, const Vec& v) const;

    virtual void save(const PPath& filename) const
    { Object::save(filename); } //!<  calls write
    //virtual void write(ostream& out) const;
    //virtual void oldread(istream& in);
  	
    virtual ~SparseVMatrix();
private:
    void build_();
};

DECLARE_OBJECT_PTR(SparseVMatrix);

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
