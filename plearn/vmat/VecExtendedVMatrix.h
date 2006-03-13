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

#ifndef VecExtendedVMatrix_INC
#define VecExtendedVMatrix_INC

#include "SourceVMatrix.h"
#include "VMat.h"

namespace PLearn {
using namespace std;


/*!  A VecExtendedVMatrix is similar to an ExtendedVMatrix: it extends the
  source VMat by appending COLUMNS to its right.  The appended columns
  are filled with a constant vector passed upon construction.  For example,
  if the vector [1,2,3] is passed at construction, then every row of the
  source VMat will be extended by 3 columns, containing [1,2,3]
  (constant).
*/

class VecExtendedVMatrix : public SourceVMatrix
{
    typedef SourceVMatrix inherited;

public:
    // ******************
    // *  Constructors  *
    // ******************

    //!  default constructor (for automatic deserialization)
    VecExtendedVMatrix(bool call_build_=false);

    //! The fieldinfos of the source are copied, the extension fieldinfos
    //! are left empty (fill them yourself)
    VecExtendedVMatrix(VMat the_source, Vec extend_data,
                       bool call_build_=true);

    PLEARN_DECLARE_OBJECT(VecExtendedVMatrix);

protected:

    static void declareOptions(OptionList &ol);
    virtual void getNewRow(int i, const Vec& v) const;

public:

    virtual void build();

    virtual void reset_dimensions() {
        source->reset_dimensions();
        width_ = source.width() + extend_data_.length();
        length_ = source.length();
    }

protected:

    // DEPRECATED - Use inherited::source instead
    // VMat underlying_;
    Vec extend_data_;

private:
    void build_();
};

DECLARE_OBJECT_PTR(VecExtendedVMatrix);

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
