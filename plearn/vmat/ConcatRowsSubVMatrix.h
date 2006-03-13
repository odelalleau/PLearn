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


/*! \file PLearnLibrary/PLearnCore/VMat.h */

#ifndef ConcatRowsSubVMatrix_INC
#define ConcatRowsSubVMatrix_INC

#include "VMat.h"
#include "SourceVMatrix.h"

namespace PLearn {
using namespace std;
 

/*!   This class concatenates several (virtual) subVMatrices of the same
  source VMatrix. For each sub-vmatrix block, the user
  specifies the starting row and the number of rows in the
  source VMatrix.
  The resulting vmatrix sees first all the rows of the
  first sub-vmatrix, then all the rows of the 2nd, etc...
*/
class ConcatRowsSubVMatrix: public SourceVMatrix
{
    typedef SourceVMatrix inherited;

protected:
    // DEPRECATED - Use inherited::source instead
    // VMat distr;
    TVec<int> start;
    TVec<int> len;

    //void check(); //!<  check ranges are compatible

    //!  returns the index of the correct sub-VMat in the array and the the row
    //!  number in this VMat that correspond to row i in the ConcatRowsVMat
    void getpositions(int i, int& whichvm, int& rowofvm) const; 

public:
    // ******************
    // *  Constructors  *
    // ******************

    //!  default constructor (for automatic deserialization)
    ConcatRowsSubVMatrix(bool call_build_ = false);

    //!  The field names of the parent VMat are copied upon construction
    ConcatRowsSubVMatrix(VMat the_source,
                         TVec<int>& the_start, TVec<int>& the_len,
                         bool call_build_ = true);

    ConcatRowsSubVMatrix(VMat the_source,
                         int start1, int len1,
                         int start2, int len2,
                         bool call_build_ = true);

    PLEARN_DECLARE_OBJECT(ConcatRowsSubVMatrix);
    static void declareOptions(OptionList &ol);

    virtual void build();

    virtual real get(int i, int j) const;
    virtual void getSubRow(int i, int j, Vec v) const;
    virtual void reset_dimensions()
    {
        source->reset_dimensions();
        width_=source->width();
        length_=0;
        for (int i=0;i<len.length();i++)
            length_ += len[i];
    }
    virtual real dot(int i1, int i2, int inputsize) const;
    virtual real dot(int i, const Vec& v) const;
private:
    void build_();
};

DECLARE_OBJECT_PTR(ConcatRowsSubVMatrix);

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
