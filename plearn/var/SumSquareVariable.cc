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
   * $Id: SumSquareVariable.cc,v 1.2 2003/01/08 21:33:04 ducharme Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "SumSquareVariable.h"
#include "Var_utils.h"

namespace PLearn <%
using namespace std;


/** SumSquareVariable **/

SumSquareVariable::SumSquareVariable(Variable* input)
  :UnaryVariable(input, 1, 1) {}


IMPLEMENT_NAME_AND_DEEPCOPY(SumSquareVariable);

void SumSquareVariable::recomputeSize(int& l, int& w) const
{ l=1; w=1; }


void SumSquareVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "SumSquareVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "SumSquareVariable");
}


void SumSquareVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "SumSquareVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "SumSquareVariable");
}


void SumSquareVariable::fprop()
{
  int n=input->nelems();
  *valuedata= 0;
  for(int i=0; i<n; i++)
    *valuedata+= input->valuedata[i]*input->valuedata[i];
}


void SumSquareVariable::bprop()
{
  int n=input->nelems();
  for(int i=0; i<n; i++)
    input->gradientdata[i]+= 2.0 * input->valuedata[i] * *gradientdata;
}


void SumSquareVariable::symbolicBprop()
{
  input->accg(2.0 * (g*input));
}



%> // end of namespace PLearn


