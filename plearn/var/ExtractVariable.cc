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
   * $Id: ExtractVariable.cc,v 1.2 2005/01/11 20:02:34 tihocan Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "ExtendedVariable.h"
#include "ExtractVariable.h"
//#include "Var_utils.h"

namespace PLearn {
using namespace std;


/** ExtractVariable **/

PLEARN_IMPLEMENT_OBJECT(ExtractVariable,
                        "Variable extracted from a vector variable.",
                        "NO HELP");

ExtractVariable::ExtractVariable(Variable* v, int the_offset, int the_length, int the_width)
: inherited(v, the_length, the_width),
  offset_(the_offset),
  length_(the_length),
  width_(the_width)
{
    build_();
}

void ExtractVariable::build()
{
    inherited::build();
    build_();
}

void ExtractVariable::build_()
{
    if (input) {
        // input is v from constructor
        if(offset_ < 0)
            PLERROR("In ExtractVariable: requested matrix is out of bounds");
    }
}

void ExtractVariable::recomputeSize(int& l, int& w) const
{ l=length_; w=width_; }

void ExtractVariable::fprop()
{
    real* inputdata = input->valuedata+offset_;
    int cnt = min(input->length()-offset_,length()*width());
    for(int i=0; i<cnt; i++)
    {
        valuedata[i] = inputdata[i];
    }
}


void ExtractVariable::bprop()
{
    real* inputgradient = input->gradientdata+offset_;
    int cnt = min(input->length()-offset_,length()*width());
    for(int i=0; i<cnt; i++)
    {
        inputgradient[i] += gradientdata[i];
    }

}


void ExtractVariable::bbprop()
{
     PLERROR("In SVDVariable::bbprop: feature not implemented");
}

void ExtractVariable::symbolicBprop()
{
    PLERROR("In SVDVariable::symbolicBprop: feature not implemented");
}


void ExtractVariable::rfprop()
{
    PLERROR("In SVDVariable::rfprop: feature not implemented");
}

} // end of namespace PLearn

