// -*- C++ -*-

// TemporalHorizonVMatrix.h
//
// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2003 Rejean Ducharme, Yoshua Bengio
// Copyright (C) 2003 Pascal Vincent
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



#ifndef TEMPORAL_HORIZON_VMATRIX
#define TEMPORAL_HORIZON_VMATRIX

#include "VMat.h"
#include "SourceVMatrix.h"

namespace PLearn {
using namespace std;

//!  This VMat delay the last targetsize entries of a source VMat
//!  by a certain horizon
class TemporalHorizonVMatrix: public SourceVMatrix
{
    typedef SourceVMatrix inherited;

protected:
    // DEPRECATED - Use inherited::source instead
    // VMat distr;  // the underlying VMat
    int horizon;  // the temporal value by which to delay the VMat
    int targetsize;  // the number of last entries to delay
    TVec<int> row_delay;  // delay for the row index

public:

    //! Also copies the original fieldinfos upon construction
    TemporalHorizonVMatrix(bool call_build_ = false);

    TemporalHorizonVMatrix(VMat the_source,
                           int the_horizon, int target_size,
                           bool call_build_ = false);

    PLEARN_DECLARE_OBJECT(TemporalHorizonVMatrix);

    static void declareOptions(OptionList &ol);

    virtual void build();
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    virtual real get(int i, int j) const;
    virtual void put(int i, int j, real value);
    virtual real getStringVal(int col, const string & str) const;
    virtual string getValString(int col, real val) const;
    virtual string getString(int row,int col) const;
    //! returns the whole string->value mapping
    virtual const map<string,real>& getStringToRealMapping(int col) const;
    virtual const map<real,string>& getRealToStringMapping(int col) const;

    virtual void reset_dimensions()
    {
        source->reset_dimensions();
        width_=source->width();
    }

    virtual real dot(int i1, int i2, int inputsize) const;
    virtual real dot(int i, const Vec& v) const;

private:
    void build_();
};

DECLARE_OBJECT_PTR(TemporalHorizonVMatrix);


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
