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
   * $Id: MinVariable.cc,v 1.1 2002/10/23 23:32:34 dorionc Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "MinVariable.h"

// From Old UnaryVariable.cc: all includes are putted in every file.
// To be revised manually 
#include "DisplayUtils.h"
#include "UnaryVariable.h"
#include "Var.h"
#include "TMat_maths.h"
#include "pl_erf.h"
#include "Var_utils.h"
namespace PLearn <%
using namespace std;


/** MinVariable **/

MinVariable::MinVariable(Variable* input)
  :UnaryVariable(input, 1, 1) {}


IMPLEMENT_NAME_AND_DEEPCOPY(MinVariable);

void MinVariable::recomputeSize(int& l, int& w) const
{ l=1; w=1; }


void MinVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "MinVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "MinVariable");
}


void MinVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "MinVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "MinVariable");
}


void MinVariable::fprop()
{
  real minval = input->valuedata[0];
  for(int i=1; i<input->nelems(); i++)
    {
      real val = input->valuedata[i];
      if(val<minval)
        minval = val;
    }
  valuedata[0] = minval;
}


void MinVariable::bprop()
{
  real minval = valuedata[0];
  for(int i=0; i<input->nelems(); i++)
    {
      if(input->valuedata[i]==minval)
        input->gradientdata[i] += gradientdata[0];
    }
}


void MinVariable::symbolicBprop()
{
  input->accg(new ElementAtPositionVariable(g, argmin(input), input->length(), input->width()));
}



%> // end of namespace PLearn


