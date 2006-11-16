// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2001-2002 Nicolas Chapados, Ichiro Takeuchi, Jean-Sebastien Senecal
// Copyright (C) 2002 Xiangdong Wang, Christian Dorion

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
 * $Id: Min2Variable.cc 3994 2005-08-25 13:35:03Z chapados $
 * This file is part of the PLearn library.
 ******************************************************* */

#include "Min2Variable.h"
#include "Var_operators.h"

namespace PLearn {
using namespace std;


/** Min2Variable **/


PLEARN_IMPLEMENT_OBJECT(
    Min2Variable,
    "Elementwise minimum over two source variables",
    "This variable assumes that the source variables have the same dimensionality\n"
    "and it takes that dimensionality.  Its values are defined as the element-wise\n"
    "minimum between its source variables:\n"
    "\n"
    "  min(v1,v2)[i] = min(v1[i],v2[i])\n"
    "\n"
    "with same dimensions as the input vectors");

Min2Variable::Min2Variable(Variable* input1, Variable* input2)
    : inherited(input1, input2, input1->length(), input1->width())
{
    PLASSERT( input1 && input2 );
    build_();
}

void
Min2Variable::build()
{
    inherited::build();
    build_();
}

void
Min2Variable::build_()
{
    if (input1 && input2) {
        if (input1->length() != input2->length()  ||  input1->width() != input2->width())
            PLERROR("IN Min2Variable input1 and input2 must have the same size");
    }
}


void Min2Variable::recomputeSize(int& l, int& w) const
{
    if (input1) {
        l = input1->length();
        w = input1->width();
    } else
        l = w = 0;
}

void Min2Variable::fprop()
{
    PLASSERT( input1 && input2 );
    int n=input1->value.length();
    real* v1=input1->value.data();
    real* v2=input2->value.data();
    real* v=value.data();
    for (int i=0;i<n;i++)
        v[i] = std::min(v1[i],v2[i]);
}


void Min2Variable::bprop()
{
    PLASSERT( input1 && input2 );
    int n=input1->value.length();
    real* v1=input1->value.data();
    real* v2=input2->value.data();
    real* grad1=input1->gradient.data();
    real* grad2=input2->gradient.data();
    real* grad=gradient.data();
    for (int i=0;i<n;i++)
    {
        if (v1[i]<v2[i])
            grad1[i] += grad[i];
        if (v2[i]<v1[i])
            grad2[i] += grad[i];
    }
}


void Min2Variable::symbolicBprop()
{
    PLASSERT( input1 && input2 );
    input1->accg((input2<input1)*g);
    input2->accg((input1<input2)*g);
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
