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
   * $Id: MinusVariable.cc,v 1.1 2002/10/23 23:32:34 dorionc Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "MinusVariable.h"

// From Old BinaryVariable.cc: all includes are putted in every file.
// To be revised manually 
#include "BinaryVariable.h"
#include "Var.h"
#include "pl_erf.h"
#include "Var_utils.h"
namespace PLearn <%
using namespace std;


/** MinusVariable **/

MinusVariable::MinusVariable(Variable* input1, Variable* input2)
  : BinaryVariable(input1, input2, input1->length(), input1->width())
{
  if(input1->length()!=input2->length() || input1->width()!=input2->width())
    PLERROR("In MinusVariable: input1 and input2 must have exactly the same size");
}


IMPLEMENT_NAME_AND_DEEPCOPY(MinusVariable);


void MinusVariable::recomputeSize(int& l, int& w) const
{ l=input1->length(); w=input1->width(); }


void MinusVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "MinusVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "MinusVariable");
}


void MinusVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "MinusVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "MinusVariable");
}


void MinusVariable::fprop()
{
  for(int k=0; k<nelems(); k++)
    valuedata[k] = input1->valuedata[k]-input2->valuedata[k];
}


void MinusVariable::bprop()
{
  for(int k=0; k<nelems(); k++)
    {
      input1->gradientdata[k] += gradientdata[k];
      input2->gradientdata[k] -= gradientdata[k];
    }
}


void MinusVariable::bbprop()
{
  if (input1->diaghessian.length()==0)
    input1->resizeDiagHessian();
  if (input2->diaghessian.length()==0)
    input2->resizeDiagHessian();
  for(int k=0; k<nelems(); k++)
    {
      input1->diaghessiandata[k] += diaghessiandata[k];
      input2->diaghessiandata[k] -= diaghessiandata[k];
    }
}


void MinusVariable::symbolicBprop()
{
  input1->accg(g);
  input2->accg(-g);
}



%> // end of namespace PLearn


