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


/*! \file VVec.h */

#ifndef VVec_INC
#define VVec_INC

#include <plearn/base/Object.h>
#include "VMatrix.h"

namespace PLearn {

//! A VVec is a reference to a row or part of a row (a subrow) of a VMatrix

class VVec : public Object
{
    typedef Object inherited;

public:
    // We leave the actual representation choice to some
    // underlying virtual matrix. A VVec simply references a subRow of a VMatrix
    PP<VMatrix> data;
    int row_index;
    int col_index;
    int length_;

    VVec()
        :row_index(0), col_index(0), length_(0) {}

    VVec(const PP<VMatrix>& m, int i)
        :data(m), row_index(i), col_index(0), length_(m->width()) {}

    VVec(const PP<VMatrix>& m, int i, int j, int l)
        :data(m), row_index(i), col_index(j), length_(l) {}

    //! constructor from Vec
    //! Will build a MemoryVMatrix containing a view of v as its single row
    //! and have the VVec point to it. So data will be shared with v.
    VVec(const Vec& v);

    inline int length() const { return length_; }
    inline int size() const { return length_; }

    // to keep compatibility with most current code,
    // VVec's can be converted to Vec's
    inline void toVec(const Vec& v) const
    {
#ifdef BOUNDCHECK
        if(v.length()!=length_)
            PLERROR("In VVec::toVec length of Vec and VVec differ!");
#endif
        data->getSubRow(row_index,col_index,v);
    }

    //! copies v into into this VVec
    inline void copyFrom(const Vec& v) const
    {
#ifdef BOUNDCHECK
        if(v.length()!=length_)
            PLERROR("In VVec::copyFrom length of Vec and VVec differ!");
#endif
        data->putSubRow(row_index,col_index,v);
    }


    inline VVec subVec(int j, int len)
    { return VVec(data, row_index, col_index+j, len); }

    //! conversion to Vec
    operator Vec() const
    {
        Vec v(length_);
        data->getSubRow(row_index,col_index,v);
        return v;
    }

    virtual void newwrite(PStream& out) const
    {
        switch(out.outmode)
        {
        case PStream::raw_ascii:
        case PStream::pretty_ascii:
        {
            out << ((Vec)*this) << flush;
            break;
        }
        default:
            inherited::newwrite(out);
        }
    }

    PLEARN_DECLARE_OBJECT(VVec);
    static void declareOptions(OptionList &ol);

    virtual void build();
private:
    void build_();
};

inline void operator>>(const VVec& vv, const Vec& v)
{ vv.toVec(v); }

inline void operator<<(const VVec& vv, const Vec& v)
{ vv.copyFrom(v); }

inline void operator<<(const Vec& v, const VVec& vv)
{ vv.toVec(v); }

inline void operator>>(const Vec& v, const VVec& vv)
{ vv.copyFrom(v); }

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
