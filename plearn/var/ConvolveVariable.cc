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
   * $Id: ConvolveVariable.cc,v 1.5 2004/04/27 16:02:26 morinf Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "ConvolveVariable.h"

namespace PLearn {
using namespace std;


/** ConvolveVariable **/

PLEARN_IMPLEMENT_OBJECT(ConvolveVariable,
                        "A convolve var; equals convolve(input, mask)",
                        "NO HELP");

ConvolveVariable::ConvolveVariable(Variable* input, Variable* mask)
  : inherited(input, mask, input->length()-mask->length()+1, input->width()-mask->width()+1)
{}


void ConvolveVariable::recomputeSize(int& l, int& w) const
{
    if (input1 && input2) {
        l = input1->length() - input2->length() + 1;
        w = input1->width() - input2->width() + 1;
    } else
        l = w = 0;
}


void ConvolveVariable::fprop()
{
  convolve(input1->matValue, input2->matValue, matValue);
}


void ConvolveVariable::bprop()
{
  for(int i=0; i<length(); i++) // size of matGradient
    for(int j=0; j<width(); j++)
      {
        real* input1valueptr = input1->matValue[i]+j;
        real* input2valueptr = input2->matValue.data();
        
        real thisgradient = matGradient(i,j);
        real* input1gradientptr = input1->matGradient[i]+j;
        real* input2gradientptr = input2->matGradient.data();
        
        for(int l=0; l<input2->length(); l++,
              input1valueptr += input1->matValue.mod(), input2valueptr += input2->matValue.mod(),
              input1gradientptr += input1->matGradient.mod(), input2gradientptr += input2->matGradient.mod())
          for(int c=0; c<input2->width(); c++)
            {
              input1gradientptr[c] += thisgradient * input2valueptr[c];
              input2gradientptr[c] += thisgradient * input1valueptr[c];
            }
      }
}


void ConvolveVariable::symbolicBprop()
{ PLERROR("ConvolveVariable::symbolicBprop() not yet implemented"); }



} // end of namespace PLearn


