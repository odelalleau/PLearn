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
   * $Id: ErfVariable.cc,v 1.2 2003/01/08 21:32:22 ducharme Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "ErfVariable.h"
#include "pl_erf.h"
#include "Var_utils.h"

namespace PLearn <%
using namespace std;


/** ErfVariable **/

ErfVariable::ErfVariable(Variable* input) 
  :UnaryVariable(input, input->length(), input->width()) {}


IMPLEMENT_NAME_AND_DEEPCOPY(ErfVariable);

void ErfVariable::recomputeSize(int& l, int& w) const
{ l=input->length(); w=input->width(); }


void ErfVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "ErfVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "ErfVariable");
}


void ErfVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "ErfVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "ErfVariable");
}


void ErfVariable::fprop()
{
  for(int i=0; i<nelems(); i++)
    valuedata[i] = pl_erf(input->valuedata[i]);
}


void ErfVariable::bprop()
{
  real cst = 2.0/sqrt(Pi);
  for(int i=0; i<nelems(); i++)
    input->gradientdata[i] += gradientdata[i] * cst*exp(input->valuedata[i]*input->valuedata[i]);
}


void ErfVariable::symbolicBprop()
{
  Var v(this);
  input->accg(g * 2.0/sqrt(Pi)*exp(square(input)));
}



%> // end of namespace PLearn


