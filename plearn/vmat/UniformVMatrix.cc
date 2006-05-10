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

#include "UniformVMatrix.h"
#include <plearn/math/random.h>

namespace PLearn {
using namespace std;


/** Uniform VMatrix **/

PLEARN_IMPLEMENT_OBJECT(UniformVMatrix, "ONE LINE DESC", "NO HELP");

UniformVMatrix::UniformVMatrix()
    : minval(0), maxval(1)
{
}

UniformVMatrix::UniformVMatrix(real the_minval, real the_maxval, int the_width)
    : inherited(-1,the_width),minval(the_minval), maxval(the_maxval)
{}

void
UniformVMatrix::build()
{
    inherited::build();
    build_();
}

void
UniformVMatrix::build_()
{
}

void
UniformVMatrix::declareOptions(OptionList &ol)
{
    declareOption(ol, "minval", &UniformVMatrix::minval, OptionBase::buildoption, "");
    declareOption(ol, "maxval", &UniformVMatrix::maxval, OptionBase::buildoption, "");
    inherited::declareOptions(ol);
}

real UniformVMatrix::get(int i, int j) const
{
    double scale = maxval-minval;
    return uniform_sample()*scale+minval;
}

void UniformVMatrix::getSubRow(int i, int j, Vec v) const
{
    double scale = maxval-minval;
    for(int k=0; k<v.length(); k++)
        v[k] = uniform_sample()*scale+minval;
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
