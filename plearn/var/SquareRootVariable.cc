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
   * $Id: SquareRootVariable.cc,v 1.3 2003/08/13 08:13:17 plearner Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "SquareRootVariable.h"

namespace PLearn <%
using namespace std;



/** SquareRootVariable **/

SquareRootVariable::SquareRootVariable(Variable* input)
  :UnaryVariable(input, input->length(), input->width()) {}


PLEARN_IMPLEMENT_OBJECT(SquareRootVariable, "ONE LINE DESCR", "NO HELP");

void SquareRootVariable::recomputeSize(int& l, int& w) const
{ l=input->length(); w=input->width(); }








void SquareRootVariable::fprop()
{
  int n=nelems();
  for(int i=0; i<n; i++)
    valuedata[i] = sqrt(input->valuedata[i]);
}


// dC/dx = dC/dy * 1/2 * 1/sqrt(x)
void SquareRootVariable::bprop()
{
  if (rValue.length()==0) resizeRValue();
  int n=nelems();
  for(int i=0; i<n; i++)
    {
      input->gradientdata[i] += 0.5/sqrt(input->valuedata[i]) * gradientdata[i];
      rvaluedata[i] = 2*input->valuedata[i]*input->rvaluedata[i];
    }
}
//!                          2                                -3
//! d2C/dx2 = d2C/dx2*(dy/dx)  + dC/dy * 1/2 *-1/2 * 1/sqrt(x)
//! Not verified yet: needs TimesScalarVariable's and DivVariable's fprop and bprop
// void SquareRootVariable::bbprop()
// {
//   if (input->diaghessian.length()==0)
//     input->resizeDiagHessian();
//   int n=nelems();
//   for(int i=0; i<n; i++)
//   {
//     real input_i = input->valuedata[i];
//     input->diaghessiandata[i] += 1/(4*input_i) * diaghessiandata[i]
//                                  + (-0.25) * pow( i/sqrt(input_i), 3 ) * gradientdata[i];
//   }
// }



%> // end of namespace PLearn


