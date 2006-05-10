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


#ifndef ThresholdVMatrix_INC
#define ThresholdVMatrix_INC

#include "SourceVMatrix.h"
#include "VMat.h"

namespace PLearn {
using namespace std;

/* A bit like OneHotVMatrix with a threshold level
 * instead.  The last column will become cold_value
 * if it was <= threshold, or hot_value if it was
 * > threshold.
 * N.B. the gt_threshold boolean value can be set
 * to true so that we get hot_value when
 * val > threshold, or false for val >= threshold.
 */
class ThresholdVMatrix: public SourceVMatrix
{
    typedef SourceVMatrix inherited;

protected:

    // DEPRECATED - Use inherited::source instead
    // VMat underlying_distr;
    real threshold;
    real cold_value;
    real hot_value;
    bool gt_threshold;

public:
    // ******************
    // *  Constructors  *
    // ******************

    //!  default constructor (for automatic deserialization)
    ThresholdVMatrix(bool call_build_ = false);

    ThresholdVMatrix(VMat the_source,
                     real threshold_,
                     real the_cold_value=0.0, real the_hot_value=1.0,
                     bool gt_threshold_ = true, bool call_build_ = false);

    virtual void getNewRow(int i, const Vec& v) const;
    virtual void reset_dimensions()
    {
        source->reset_dimensions();
        width_=source->width();
        length_=source->length();
    }
};

inline VMat thresholdVMat(VMat source, real threshold,
                          real cold_value=0.0, real hot_value=1.0,
                          bool gt_threshold = true, bool call_build_ = false)
{ return new ThresholdVMatrix(source, threshold, cold_value, hot_value,
                              gt_threshold, call_build_); }

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
