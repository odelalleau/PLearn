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


/*! \file OneHotVMatrix.h */

#ifndef OneHotVMatrix_INC
#define OneHotVMatrix_INC

#include "SourceVMatrix.h"
#include "VMat.h"

namespace PLearn {
using namespace std;

class OneHotVMatrix: public SourceVMatrix
{
    typedef SourceVMatrix inherited;

public:

    int nclasses;
    real cold_value;
    real hot_value;
    int index;

public:

    // ******************
    // *  Constructors  *
    // ******************

    //!  default constructor (for automatic deserialization)
    OneHotVMatrix(bool call_build_ = false);

    //!  (see special case when nclasses==1 desribed above)
    OneHotVMatrix(VMat the_source, int the_nclasses,
                  real the_cold_value=0.0, real the_host_value=1.0,
                  int the_index=-1, bool call_build_ = false);

    PLEARN_DECLARE_OBJECT(OneHotVMatrix);

protected:

    virtual void getNewRow(int i, const Vec& samplevec) const;
    static void declareOptions(OptionList &ol);

    //! If 'nclasses' is greater than zero, do nothing.
    //! Otherwise, obtain 'nclasses' by finding the maximum value of the
    //! source's 'index' column. Then update the width of this VMat
    //! accordingly.
    void updateNClassesAndWidth();

public:

    virtual void build();

    virtual void reset_dimensions()
    {
        source->reset_dimensions();
        width_ = source->width() + nclasses - 1;
        length_ = source->length();
    }
    virtual real dot(int i1, int i2, int inputsize) const;
    virtual real dot(int i, const Vec& v) const;
private:
    void build_();
};

inline VMat onehot(VMat the_source, int nclasses,
                   real cold_value=0.0, real hot_value=1.0, int index=-1,
                   bool call_build_=false)
{ return new OneHotVMatrix(the_source, nclasses, cold_value, hot_value, index,
                           call_build_); }

DECLARE_OBJECT_PTR(OneHotVMatrix);

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
