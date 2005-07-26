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
   * $Id$
   * This file is part of the PLearn library.
   ******************************************************* */

#include "ConcatRowsVariable.h"
#include "SubMatVariable.h"

namespace PLearn {
using namespace std;



/** ConcatRowsVariable **/

PLEARN_IMPLEMENT_OBJECT(ConcatRowsVariable,
                        "Concatenation of the rows of several variables",
                        "");

ConcatRowsVariable::ConcatRowsVariable(const VarArray& vararray)
    : inherited(vararray.nonNull(), vararray.sumOfLengths(), vararray.maxWidth())
{
    build_();
}

void
ConcatRowsVariable::build()
{
    inherited::build();
    build_();
}

void
ConcatRowsVariable::build_()
{
    // all the variables must have the same width
    if (varray->length()) {
        int w = varray[0]->width();
        for (int i = 1; i < varray.size(); i++)
            if (w != varray[i]->width())
                PLERROR("ConcatRowsVariable: all non-null variables must have the same width");
    }
}


void ConcatRowsVariable::recomputeSize(int& l, int& w) const
{
    if (varray) {
        l = varray.sumOfLengths();
        w = varray.maxWidth(); // Note: actually, all vars have same width.
    } else
        l = w = 0;
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

} // end of namespace PLearn


