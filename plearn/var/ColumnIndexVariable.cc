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
 * $Id$
 * This file is part of the PLearn library.
 ******************************************************* */

#include "ColumnIndexVariable.h"
#include "IndexAtPositionVariable.h"
namespace PLearn {
using namespace std;



/** ColumnIndexVariable **/

PLEARN_IMPLEMENT_OBJECT(
    ColumnIndexVariable,
    "Return a row vector with the elements indexed in each column.",
    "The first input is a matrix of size NxM.\n"
    "The second input is a vector of size M, with elements in {0, ..., N-1}.\n"
    "The result is a row vector of size M, where the i-th element is equal\n"
    "to input1(input2[i], i)."
);

ColumnIndexVariable::ColumnIndexVariable(Variable *input1, Variable *input2)
    : inherited(input1, input2, 1, input1->width())
{
    build_();
}

///////////
// build //
///////////
void ColumnIndexVariable::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void ColumnIndexVariable::build_()
{
    if (input2 && !input2->isVec())
        PLERROR("In ColumnIndexVariable::build_ - input2 must be a vector "
                "variable representing the indices of input1");
    if (input1 && input2 && input1->width() != input2->size())
        PLERROR("In ColumnIndexVariable::build_ - input1's width (%d) "
                "should be equal to input2's size (%d)",
                input1->width(), input2->size());
}

///////////////////
// recomputeSize //
///////////////////
void ColumnIndexVariable::recomputeSize(int& l, int& w) const
{
    l = 1;
    if (input1)
        w = input1->width();
    else
        w = 0;
}

///////////
// fprop //
///////////
void ColumnIndexVariable::fprop()
{
    for (int i=0; i<input2->size(); i++)
    {
        int num = (int)input2->valuedata[i];
        valuedata[i] = input1->valuedata[num*input1->width()+i];
    }
}


void ColumnIndexVariable::bprop()
{
    for (int i=0; i<input2->size(); i++)
    {
        int num = (int) input2->valuedata[i];
        input1->gradientdata[num*input1->width()+i] += gradientdata[i];
    }
}


void ColumnIndexVariable::symbolicBprop()
{
    input1->accg(new IndexAtPositionVariable(g,input2,input1->length(),input1->width()));
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
