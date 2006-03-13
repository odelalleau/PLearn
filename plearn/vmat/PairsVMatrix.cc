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

#include "PairsVMatrix.h"

namespace PLearn {
using namespace std;


/** PairsVMatrix **/

PLEARN_IMPLEMENT_OBJECT(PairsVMatrix, "ONE LINE DESC", "NO HELP");

PairsVMatrix::PairsVMatrix()
{
}

PairsVMatrix::PairsVMatrix(Mat the_data1, Mat the_data2)
    : inherited(data1.width()+data2.width(), data1.length()*data2.length()), 
      data1(the_data1), data2(the_data2)
{
}

void
PairsVMatrix::build()
{
    inherited::build();
    build_();
}

void
PairsVMatrix::build_()
{
}

void
PairsVMatrix::declareOptions(OptionList &ol)
{
    declareOption(ol, "data1", &PairsVMatrix::data1, OptionBase::buildoption, "");
    declareOption(ol, "data2", &PairsVMatrix::data2, OptionBase::buildoption, "");
    inherited::declareOptions(ol);
}

void PairsVMatrix::getNewRow(int ij, const Vec& samplevec) const
{
    //ij = ij%length_;
    ij %= length_;
    real* data = samplevec.data();
    real* data_i = data1[ij/data2.length()];
    real* data_j = data2[ij%data2.length()];
    int kk=0;
    for (int k=0;k<data1.width();k++)
        data[kk++] = data_i[k];
    for (int k=0;k<data2.width();k++)
        data[kk++] = data_j[k];
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
