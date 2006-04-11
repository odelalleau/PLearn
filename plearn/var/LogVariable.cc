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

#include "LogVariable.h"
#include <plearn/display/DisplayUtils.h>
#include "Var_operators.h"

namespace PLearn {
using namespace std;


/** LogVariable **/


PLEARN_IMPLEMENT_OBJECT(
    LogVariable,
    "Takes the natural logarithm of its input variable.",
    ""
);

LogVariable::LogVariable(Variable* input)
    : inherited(input, input->length(), input->width())
{}

///////////////////
// recomputeSize //
///////////////////
void LogVariable::recomputeSize(int& l, int& w) const
{
    if (input) {
        l = input->length();
        w = input->width();
    } else
        l = w = 0;
}

///////////
// fprop //
///////////
void LogVariable::fprop()
{
    assert( input && nelems() == input->nelems() );
    for(int i=0; i<nelems(); i++)
    {
        valuedata[i] = safeflog(input->valuedata[i]);
#ifdef BOUNDCHECK
        if (!finite(valuedata[i]) && finite(input->valuedata[i]))
        {
            //PLWARNING("LogVariable::fprop qqchose va pas");
            cout << "inputdata[i]= " << input->valuedata[i] << endl;
            cout << "valuedata[i]= " << valuedata[i] << endl;
            displayVarGraph(this, true, 250);
            PLERROR("LogVariable::fprop qqchose va pas");
        }
#endif
    }
}

///////////
// bprop //
///////////
void LogVariable::bprop()
{
    assert( input && input->gradientdata == input->gradient.data() );
    for(int i=0; i<nelems(); i++)
        if (input->valuedata[i]>0)
            input->gradientdata[i] += gradientdata[i]/input->valuedata[i];
}

///////////////////
// symbolicBprop //
///////////////////
void LogVariable::symbolicBprop()
{
    input->accg(g / input);
}


////////////
// rfprop //
////////////
void LogVariable::rfprop()
// R{log(x)} = 1/x R(x)
{
    if (rValue.length()==0) resizeRValue();
    for(int i=0; i<nelems(); i++)
        rvaluedata[i] = input->rvaluedata[i] / input->valuedata[i];
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
