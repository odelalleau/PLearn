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
   * $Id: ColumnSumVariable.cc,v 1.2 2003/01/08 21:32:14 ducharme Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "ColumnSumVariable.h"

namespace PLearn <%
using namespace std;


/** ColumnSumVariable **/

ColumnSumVariable::ColumnSumVariable(Variable* input)
  :UnaryVariable(input, 1, input->width()) {}


IMPLEMENT_NAME_AND_DEEPCOPY(ColumnSumVariable);

void ColumnSumVariable::recomputeSize(int& l, int& w) const
{ l=1; w=input->width(); }


void ColumnSumVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "ColumnSumVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "ColumnSumVariable");
}


void ColumnSumVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "ColumnSumVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "ColumnSumVariable");
}


void ColumnSumVariable::fprop()
{
  value.clear();
  int k=0;
  for(int i=0; i<input->length(); i++)
    for(int j=0; j<input->width(); j++, k++)
      valuedata[j] += input->valuedata[k];
}


void ColumnSumVariable::bprop()
{
  int k=0;
  for(int i=0; i<input->length(); i++)
    for(int j=0; j<input->width(); j++, k++)
      input->gradientdata[k] += gradientdata[j];
}


void ColumnSumVariable::symbolicBprop()
{
  input->accg(g);
}



%> // end of namespace PLearn


