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


/*! \file SubVMatrix.h */

#ifndef SubVMatrix_INC
#define SubVMatrix_INC

#include "VMat.h"
#include "SourceVMatrix.h"

namespace PLearn {
using namespace std;

class SubVMatrix: public SourceVMatrix
{

private:

    typedef SourceVMatrix inherited;

public:

    //! Build options.

    // DEPRECATED - Use inherited::source instead.
    // VMat parent;
    int istart;
    int jstart;
    real fistart;
    real flength;

    //! The appropriate VMFields of the parent VMat are copied upon
    //! construction
    SubVMatrix(bool call_build_ = false);

    SubVMatrix(VMat the_source,
               int the_istart, int the_jstart,
               int the_length, int the_width,
               bool call_build_ = true);

    SubVMatrix(VMat the_source,
               real the_fistart, int the_jstart,
               real the_flength, int the_width,
               bool call_build_ = true);

    virtual void getNewRow(int i, const Vec& v) const;
    virtual real get(int i, int j) const;
    virtual void getSubRow(int i, int j, Vec v) const;
    virtual real getStringVal(int col, const string & str) const;
    virtual string getValString(int col, real val) const;
    virtual string getString(int row,int col) const;
    virtual void getMat(int i, int j, Mat m) const;
    virtual void put(int i, int j, real value);
    virtual void putSubRow(int i, int j, Vec v);
    virtual void putMat(int i, int j, Mat m);
    virtual VMat subMat(int i, int j, int l, int w);

    virtual real dot(int i1, int i2, int inputsize) const;
    virtual real dot(int i, const Vec& v) const;

    virtual void reset_dimensions();

    //! Transforms a shallow copy into a deep copy.
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    PLEARN_DECLARE_OBJECT(SubVMatrix);
    virtual void build();

    //! Return the Dictionary object for a certain field, or a null pointer
    //! if there isn't one
    virtual PP<Dictionary>  getDictionary(int col) const;

    //! Gives the possible values for a certain field in the VMatrix
    virtual void getValues(int row, int col, Vec& values) const;

    //! Gives the possible values of a certain field (column) given the input
    virtual void getValues(const Vec& input, int col, Vec& values) const;

protected:

    static void declareOptions(OptionList &ol);

private:

    void build_();

};

DECLARE_OBJECT_PTR(SubVMatrix);

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
