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

#include "RangeVMatrix.h"

namespace PLearn {
using namespace std;

/** Range VMatrix **/

PLEARN_IMPLEMENT_OBJECT(RangeVMatrix, "ONE LINE DESC", "NO HELP");

RangeVMatrix::RangeVMatrix()
    : start(0), end(0), step(1)
{
}

RangeVMatrix::RangeVMatrix(real the_start, real the_end, real the_step)
    : start(the_start), end(the_end), step(the_step)
{
    build();
}

void
RangeVMatrix::build()
{
    inherited::build();
    build_();
}

void
RangeVMatrix::build_()
{
    width_=1;
    length_ = (int)((end-start)/step);
    if (length_*step==(end-start))
        length_++;
}

void
RangeVMatrix::declareOptions(OptionList &ol)
{
    declareOption(ol, "start", &RangeVMatrix::start, OptionBase::buildoption, "");
    declareOption(ol, "end", &RangeVMatrix::end, OptionBase::buildoption, "");
    declareOption(ol, "step", &RangeVMatrix::step, OptionBase::buildoption, "");
    inherited::declareOptions(ol);
}

real RangeVMatrix::get(int i, int j) const
{
#ifdef BOUNDCHECK
    if(j!=0 || i<0 || i>=length())
        PLERROR("In RangeVMatrix::get OUT OF BOUNDS");
#endif
    return start+i*step;
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
