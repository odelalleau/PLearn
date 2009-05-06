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


/*! \file PLearn/plearn/vmat/ShiftAndRescaleVMatrix.h */

#ifndef ShiftAndRescaleVMatrix_INC
#define ShiftAndRescaleVMatrix_INC

#include "VMat.h"
#include "SourceVMatrix.h"

namespace PLearn {
using namespace std;

/*!   VMatrix that can be used to rescale and shift each feature of the
  source: x'_i = a_i*(x_i+b_i)
  This can be used to normalize the inputs of a distribution
  (see the NormalizeInputDistr function in PLearn.h)
*/
class ShiftAndRescaleVMatrix: public SourceVMatrix
{
    typedef SourceVMatrix inherited;

public:
    //!  x'_i = (x_i+shift_i)*scale_i

    // DEPRECATED - use inherited::source instead
    // VMat vm;
    Vec shift;
    Vec scale;
    Vec min_max;

    real shared_shift;
    real shared_scale;

    //! find shift and scale automatically?
    bool automatic;

    //! when automatic, use the n_train first examples to estimate shift and
    //! scale
    int n_train;
    int n_inputs; // when automatic,
    bool negate_shift;
    bool no_scale;
    TVec<string> fields;
    int verbosity;

    //! For all constructors, the original VMFields are copied upon construction
    ShiftAndRescaleVMatrix(bool call_build_=false);

    ShiftAndRescaleVMatrix(VMat the_source, bool call_build_=true);

    ShiftAndRescaleVMatrix(VMat the_source, Vec the_shift, Vec the_scale,
                           bool call_build_=true);

    ShiftAndRescaleVMatrix(VMat the_source, int the_n_inputs,
                           bool call_build_=true);

    ShiftAndRescaleVMatrix(VMat the_source,
                           int the_n_inputs,
                           int the_n_train,
                           bool the_ignore_missing = false,
                           bool the_verbosity = 1,
                           bool call_build_=true);

    PLEARN_DECLARE_OBJECT(ShiftAndRescaleVMatrix);
    
    //! Transform a shallow copy into a deep copy.
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    virtual void getNewRow(int i, const Vec& v) const;

    virtual void reset_dimensions()
    {
        if (width_>0 && width_!=source->width())
            PLERROR("ShiftAndRescaleVMatrix: can't change width");
        inherited::reset_dimensions();
    }

private:
    //! This does the actual building.
    void build_();

protected:
    //! Declares this class' options
    static void declareOptions(OptionList& ol);

public:
    // simply calls inherited::build() then build_()
    virtual void build();

};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(ShiftAndRescaleVMatrix);

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
