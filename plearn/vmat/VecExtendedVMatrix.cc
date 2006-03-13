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

#include "VecExtendedVMatrix.h"
#include <plearn/math/TMat.h>

namespace PLearn {
using namespace std;


/** VecExtendedVMatrix **/

PLEARN_IMPLEMENT_OBJECT(VecExtendedVMatrix,
                        "Extends the source by appending columns to its right",
                        "A VecExtendedVMatrix is similar to an"
                        " ExtendedVMatrix: it extends the\n"
                        "source VMat by appending COLUMNS to its right.  The"
                        " appended columns\n"
                        "are filled with a constant vector passed upon"
                        " construction.  For example,\n"
                        "if the vector [1,2,3] is passed at construction, then"
                        " every row of the\n"
                        "source VMat will be extended by 3 columns, containing"
                        " [1,2,3]\n"
                        "(constant)\n"
                        );

VecExtendedVMatrix::VecExtendedVMatrix(bool call_build_)
    : inherited(call_build_)
{
    // build_() won't do anything
}

VecExtendedVMatrix::VecExtendedVMatrix(VMat the_source, Vec extend_data,
                                       bool call_build_)
    : inherited(the_source,
                the_source->length(),
                the_source->width() + extend_data.length(),
                call_build_),
      extend_data_(extend_data)
{
    if( call_build_ )
        build_();
}

void VecExtendedVMatrix::build()
{
    inherited::build();
    build_();
}

void VecExtendedVMatrix::build_()
{
    if (source)
        fieldinfos = source->getFieldInfos();
}

void VecExtendedVMatrix::declareOptions(OptionList &ol)
{
    declareOption(ol, "underlying_", &VecExtendedVMatrix::source,
                  (OptionBase::learntoption | OptionBase::nosave),
                  "DEPRECATED - Use 'source' instead.");

    declareOption(ol, "extend_data_", &VecExtendedVMatrix::extend_data_,
                  OptionBase::buildoption, "");

    inherited::declareOptions(ol);
}

void VecExtendedVMatrix::getNewRow(int i, const Vec& v) const
{
#ifdef BOUNDCHECK
    if(i<0 || i>=length())
        PLERROR("In VecExtendedVMatrix::getNewRow OUT OF BOUNDS");
    if(v.length() != width())
        PLERROR("In VecExtendedVMatrix::getNewRow v.length() must be equal to the VMat's width");
#endif

    Vec subv = v.subVec(0, source->width());
    source->getRow(i,subv);
    copy(extend_data_.begin(), extend_data_.end(),
         v.begin() + source->width());
}

} // end of namespace PLearn


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
