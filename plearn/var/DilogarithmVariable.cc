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
   * $Id: DilogarithmVariable.cc,v 1.2 2003/12/08 03:46:31 yoshua Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "DilogarithmVariable.h"
#include "Var_utils.h"

namespace PLearn <%
using namespace std;


/** DilogarithmVariable **/

DilogarithmVariable::DilogarithmVariable(Variable* input) 
  :UnaryVariable(input, input->length(), input->width()) {}


PLEARN_IMPLEMENT_OBJECT(DilogarithmVariable, 
                        "This Var computes the dilogarithm function", 
                        "The dilogarithm function is useful to compute the primitive of the softplus.\n"
                        "  dilogarithm(x) = sum_{k=1}^\\infty x^k/k^2\n"
                        "so dilogarithm'(x) = -log(1-x), i.e. -dilogarithm'(-e^x)=log(1+e^x)=softplus(x)\n"
                        "and primitive(softplus)(x) = -dilogarithm(-e^x)\n");

void DilogarithmVariable::recomputeSize(int& l, int& w) const
{ l=input->length(); w=input->width(); }


void DilogarithmVariable::fprop()
{
  for(int i=0; i<nelems(); i++)
  {
    valuedata[i] = dilogarithm(input->valuedata[i]);
#ifdef BOUNDCHECK
    if (is_missing(valuedata[i]))
        PLWARNING("dilogarithm returned NaN");
#endif
  }
}


void DilogarithmVariable::bprop()
{
  for(int i=0; i<nelems(); i++)
    input->gradientdata[i] -= gradientdata[i] * log1p(-input->valuedata[i]);
}


void DilogarithmVariable::symbolicBprop()
{
  PLERROR("DilogarithmVariable::symbolicBprop() not implemented");
}



%> // end of namespace PLearn


