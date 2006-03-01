// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2004 Jasmin Lapalme

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

#include "InsertZerosVariable.h"
#include "Var_operators.h"

namespace PLearn {
using namespace std;


/** InsertZerosVariable **/

PLEARN_IMPLEMENT_OBJECT(InsertZerosVariable,
                        "Insert rows with zeros",
                        "NO HELP");

InsertZerosVariable::InsertZerosVariable(Variable* input, TVec<int> the_rows) 
    : inherited(input, input->length()+the_rows.length(), input->width()), rows(the_rows)
{}

void InsertZerosVariable::recomputeSize(int& l, int& w) const {
    if (input) {
        l = input->length()+rows.length();
        w = input->width();
    } else
        l = w = 0;
}

void InsertZerosVariable::build_()
{ 
    if(input)
    {
        sortElements(rows);
        // Check if all elements are unique
        for(int i=0; i<rows.length()-1; i++)
        {
            if(rows[i] == rows[i+1]) PLERROR("InsertZerosVariable::build_(): some elements in rows are the same");
            
            if(rows[i] > input->length()+i) PLERROR("InsertZerosVariable::build_(): cannot add row of zeros at positon %d", rows[i+1]);
        }
    }
}

void InsertZerosVariable::build()
{
    inherited::build();
    build_();
}

void InsertZerosVariable::fprop() {
    real* ptr;
    real* inputptr;
    int r=0;
    for(int i=0; i<matValue.length(); i++)
    {
        ptr = matValue[i];
        if(r<rows.length() && i==rows[r])
        {
            for(int j=0; j<matValue.width(); j++)         
                *ptr++ = 0;
            r++;
        }
        else
        {
            inputptr = input->matValue[i-r];
            for(int j=0; j<matValue.width(); j++)
                *ptr++ = *inputptr++;
        }
    }
}


void InsertZerosVariable::bprop() {
    real* gptr;
    real* ginputptr;
    int r=0;
    for(int i=0; i<matGradient.length(); i++)
    {
        if(r<rows.length() && i==rows[r])
            r++;
        else
        {
            gptr = matGradient[i];
            ginputptr = input->matGradient[i-r];
            for(int j=0; j<matGradient.width(); j++)
                *ginputptr++ += *gptr++;
        }
    }
}

void InsertZerosVariable::bbprop() {
    PLERROR("bbprop is not implemented for InsertZerosVariable");
}


void InsertZerosVariable::symbolicBprop() {
    PLERROR("symbolicBprop is not implemented for InsertZerosVariable");
}

void InsertZerosVariable::rfprop() {
    PLERROR("rfprop is not implemented for InsertZerosVariable");
}

void InsertZerosVariable::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(rows, copies);
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
