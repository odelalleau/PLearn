// -*- C++ -*-

// TimesConstantScalarVariable2.cc
//
// Copyright (C) 2007 Pascal Vincent
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
 * $Id: TimesConstantScalarVariable2.cc 3994 2005-08-25 13:35:03Z chapados $
 * This file is part of the PLearn library.
 ******************************************************* */

#include "TimesConstantScalarVariable2.h"
#include <plearn/var/Var_operators.h>



namespace PLearn {
using namespace std;

/** TimesConstantScalarVariable2 **/

PLEARN_IMPLEMENT_OBJECT(TimesConstantScalarVariable2,
                        "Multiplies the first var V1 by a scalar second var V2. The second var is assumed constant, and no gradient is backpropagated to it.",
                        "NO HELP");

TimesConstantScalarVariable2::TimesConstantScalarVariable2(Variable* input1, Variable* input2)
    : inherited(input1, input2, input1->length(), input1->width())
{
    build_();
}

void
TimesConstantScalarVariable2::build()
{
    inherited::build();
    build_();
}

void
TimesConstantScalarVariable2::build_()
{
    if (input2 && !input2->isScalar())
        PLERROR("IN TimesConstantScalarVariable2: input2 is not a scalar");
}

void TimesConstantScalarVariable2::recomputeSize(int& l, int& w) const
{
    if (input1) {
        l = input1->length();
        w = input1->width();
    } else
        l = w = 0;
}

void TimesConstantScalarVariable2::fprop()
{
    real scal = input2->valuedata[0];
    for(int k=0; k<nelems(); k++)
        valuedata[k] = input1->valuedata[k] * scal;
}


void TimesConstantScalarVariable2::bprop()
{
    real scal = input2->valuedata[0];
    for(int k=0; k<nelems(); k++)
        input1->gradientdata[k] += scal*gradientdata[k];
}


void TimesConstantScalarVariable2::symbolicBprop()
{
    input1->accg(g*input2);
}


//R(x1x2)=R(x1)x2+x1R(x2)
void TimesConstantScalarVariable2::rfprop()
{
    if (rValue.length()==0) resizeRValue();
    real scal = input2->valuedata[0];
    real rscal = input2->rvaluedata[0];
    for(int k=0; k<nelems(); k++)
        rvaluedata[k] = input1->rvaluedata[k] * scal + input1->valuedata[k] * rscal;
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
