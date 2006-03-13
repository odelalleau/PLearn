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

#ifndef ConcatColumnsVMatrix_INC
#define ConcatColumnsVMatrix_INC

#include "RowBufferedVMatrix.h"
#include "VMat.h"

namespace PLearn {
using namespace std;


class ConcatColumnsVMatrix: public RowBufferedVMatrix
{
    typedef RowBufferedVMatrix inherited;

protected:
    // Array<VMat> array;
    TVec<VMat> sources;

public:

    bool no_duplicate_fieldnames;

    //! The lists of VMFields are appended upon construction.  The case where
    //! some VMat may have some fields and others not is handled properly.
    ConcatColumnsVMatrix(TVec<VMat> the_sources = TVec<VMat>());
    ConcatColumnsVMatrix(VMat d1, VMat d2);

protected:

    static void declareOptions(OptionList &ol);
    virtual void getNewRow(int i, const Vec& samplevec) const; 

public:

    virtual real getStringVal(int col, const string & str) const;
    //! returns the whole string->value mapping
    const map<string,real>& getStringMapping(int col) const;
    virtual string getValString(int col, real val) const;
    virtual string getString(int row,int col) const;
    virtual void reset_dimensions()
    { PLERROR("ConcatColumnsVMatrix::reset_dimensions() not implemented"); }

    virtual real dot(int i1, int i2, int inputsize) const;
    virtual real dot(int i, const Vec& v) const;

    PLEARN_DECLARE_OBJECT(ConcatColumnsVMatrix);
    virtual void build();

    //! Transforms a shallow copy into a deep copy.
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

private:
    void build_();

    //! Return the Dictionary object for a certain field, or a null pointer
    //! if there isn't one
    virtual PP<Dictionary> getDictionary(int col) const;

    //! Returns the possible values for a certain field in the VMatrix
    virtual Vec getValues(int row, int col) const;

    //! Returns the possible values of a certain field (column) given the input 
    virtual Vec getValues(const Vec& input, int col) const;

};

DECLARE_OBJECT_PTR(ConcatColumnsVMatrix);

inline VMat hconcat(VMat d1, VMat d2)
{ return new ConcatColumnsVMatrix(d1,d2); }

inline VMat hconcat(TVec<VMat> ds)
{ return new ConcatColumnsVMatrix(ds); }


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
