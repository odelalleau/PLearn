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
   * $Id: ConcatRowsVariable.cc,v 1.1 2002/10/23 23:32:34 dorionc Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "ConcatRowsVariable.h"

// From Old NaryVariable.cc: all includes are putted in every file.
// To be revised manually 
#include "NaryVariable.h"
#include "Var.h"
#include "TMat_maths.h"
#include "PLMPI.h"
#include "DisplayUtils.h"
#include "pl_erf.h"
#include "Var_utils.h"
namespace PLearn <%
using namespace std;



/** ConcatRowsVariable **/
ConcatRowsVariable::ConcatRowsVariable(const VarArray& vararray)
    :NaryVariable(vararray.nonNull(), vararray.sumOfLengths(), vararray.maxWidth())
{
  // all the variables must have the same width
  int w = varray[0]->width();
  for (int i=1; i<varray.size(); i++)
    if (w!=varray[i]->width())
      PLERROR("ConcatRowsVariable: all non-null variables must have the same width");
}


IMPLEMENT_NAME_AND_DEEPCOPY(ConcatRowsVariable);

void ConcatRowsVariable::recomputeSize(int& l, int& w) const
{ l=varray.sumOfLengths(); w=varray.maxWidth(); }


void ConcatRowsVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "ConcatRowsVariable");
  NaryVariable::deepRead(in, old2new);
  readFooter(in, "ConcatRowsVariable");
}


void ConcatRowsVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "ConcatRowsVariable");
  NaryVariable::deepWrite(out, already_saved);
  writeFooter(out, "ConcatRowsVariable");
}


void ConcatRowsVariable::fprop()
{
  int k=0;
  for (int n=0; n<varray.size(); n++) {
    Var vn = varray[n];
    for (int i=0; i<vn->nelems(); i++, k++)
      valuedata[k] = vn->valuedata[i];
  }
}


void ConcatRowsVariable::bprop()
{
  int k=0;
  for (int n=0; n<varray.size(); n++) {
    Var vn = varray[n];
    for (int i=0; i<vn->nelems(); i++, k++)
      vn->gradientdata[i] += gradientdata[k];
  }
}


void ConcatRowsVariable::symbolicBprop()
{
  int k=0;
  for (int n=0; n<varray.size(); n++) {
    Var vn = varray[n];
    vn->accg(new SubMatVariable(g, k, 0, vn->length(), width()));
    k += vn->length();
  }
}


void ConcatRowsVariable::rfprop()
{
  if (rValue.length()==0) resizeRValue();
  int k=0;
  for (int n=0; n<varray.size(); n++) {
    Var vn = varray[n];
    for (int i=0; i<vn->nelems(); i++, k++)
      rvaluedata[k] = vn->rvaluedata[i];
  }
}



%> // end of namespace PLearn


