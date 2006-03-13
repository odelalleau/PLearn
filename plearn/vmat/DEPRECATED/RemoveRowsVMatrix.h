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

#ifndef RemoveRowsVMatrix_INC
#define RemoveRowsVMatrix_INC

#include "VMat.h"

namespace PLearn {
using namespace std;
 
//!  sees an underlying VMat with the specified rows excluded
class RemoveRowsVMatrix: public VMatrix
{
    typedef VMatrix inherited;

protected:
    VMat distr;
    Vec indices;

    //!  returns the row number in distr corresponding to i in this VMat
    int getrownum(int i) const; 

public:
    virtual void reset_dimensions() { distr->reset_dimensions(); width_=distr->width(); }

    // ******************
    // *  Constructors  *
    // ******************
    RemoveRowsVMatrix() {}; //!<  default constructor (for automatic deserialization)

    //! Copy the original fieldinfos upon construction
    RemoveRowsVMatrix(VMat the_distr, Vec the_indices=Vec() );

    PLEARN_DECLARE_OBJECT(RemoveRowsVMatrix);
    static void declareOptions(OptionList &ol);

    virtual void build();

    //!  the given rownum of the underlying distr will also be excluded
    void remove(int rownum)
    { 
        indices.insertSorted(rownum,true); 
        length_--;
    }

    void unremove(int rownum)
    {
        indices.removeSorted(rownum);
        length_++;
    }

    virtual real get(int i, int j) const;
    virtual void getSubRow(int i, int j, Vec v) const;
    virtual real dot(int i1, int i2, int inputsize) const;
    virtual real dot(int i, const Vec& v) const;

private:
    void build_();
};

inline VMat removeRows(VMat d, Vec rownums)
{ return new RemoveRowsVMatrix(d,rownums); }

inline VMat removeRow(VMat d, int rownum)
{ return new RemoveRowsVMatrix(d,Vec(1,rownum)); }

DECLARE_OBJECT_PTR(RemoveRowsVMatrix);

} // end of namespcae PLearn
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
