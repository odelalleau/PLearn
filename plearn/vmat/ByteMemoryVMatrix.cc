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

#include "ByteMemoryVMatrix.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(ByteMemoryVMatrix, "ONE_LINE_DESCR", "ONE LINE HELP");

/** ByteMemoryVMatrix **/

ByteMemoryVMatrix::ByteMemoryVMatrix()
    : data(0)
{
}

ByteMemoryVMatrix::ByteMemoryVMatrix(unsigned char* the_data,int the_length,int the_width, Vec the_scale)
    :inherited(the_length, the_width), data(the_data), scale(the_scale), offset_(the_scale.length())
{
    if (the_scale.length() != width_)
        PLERROR("ByteMemoryVMatrix: inconsistent arguments (scale(%d),n_col(%d))",
                the_scale.length(), width_);
}

ByteMemoryVMatrix::
ByteMemoryVMatrix(unsigned char* the_data,int the_length,int the_width, 
                  Vec the_scale, Vec the_offset)
    :VMatrix(the_length, the_width), data(the_data), 
     scale(the_scale), offset_(the_offset)
{
    if (the_scale.length() != width_ || the_offset.length()!=width_)
        PLERROR("ByteMemoryVMatrix: inconsistent arguments (scale(%d),offset(%d),n_col(%d))",
                the_scale.length(), the_offset.length(), width_);
}

ByteMemoryVMatrix::ByteMemoryVMatrix(unsigned char* the_data,int the_length,int the_width,
                                     double the_scaling_factor,double the_offset)
    : VMatrix(the_length, the_width), data(the_data), 
      scale(the_width, the_scaling_factor), offset_(the_width, the_offset)
{
}

real ByteMemoryVMatrix::get(int i, int j) const
{
    return ( data[i*width()+j] + offset_[j] ) * scale[j];
}

void ByteMemoryVMatrix::getSubRow(int i, int j, Vec samplevec) const
{
    unsigned char* p = &data[i*width_];
    real *v = samplevec.data();
    real *s = scale.data();
    real *o = offset_.data();
    for (int jj=0; jj<samplevec.length(); jj++)
        v[jj] = s[j+jj] * (p[j+jj] + o[j+jj]);
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
