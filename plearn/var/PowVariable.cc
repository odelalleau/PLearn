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
   * $Id: PowVariable.cc,v 1.6 2004/04/27 15:59:16 morinf Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "PowVariable.h"
#include "Var_operators.h"
//#include "Var_utils.h"

namespace PLearn {
using namespace std;


/** PowVariable **/

PLEARN_IMPLEMENT_OBJECT(PowVariable,
                        "Elementwise pow (returns 0 wherever input is negative)",
                        "NO HELP");

PowVariable::PowVariable(Variable* input, real the_power)
  : inherited(input, input->length(), input->width()), power(the_power) 
{}

void
PowVariable::declareOptions(OptionList &ol)
{
    declareOption(ol, "power", &PowVariable::power, OptionBase::buildoption, "");
    inherited::declareOptions(ol);
}

void PowVariable::recomputeSize(int& l, int& w) const
{
    if (input) {
        l = input->length();
        w = input->width();
    } else
        l = w = 0;
}

void PowVariable::fprop()
{
  for(int i=0; i<nelems(); i++)
    valuedata[i] = mypow(input->valuedata[i],power);
}


void PowVariable::bprop()
{
  for(int i=0; i<nelems(); i++)
    input->gradientdata[i] += power*gradientdata[i]*mypow(input->valuedata[i],power-1.0);
}


void PowVariable::symbolicBprop()
{
  input->accg(g * (power*pow(input, power-1.)));
}



} // end of namespace PLearn


