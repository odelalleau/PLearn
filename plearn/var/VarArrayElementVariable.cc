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
   * $Id: VarArrayElementVariable.cc,v 1.4 2004/02/20 21:11:54 chrish42 Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "VarArrayElementVariable.h"

namespace PLearn {
using namespace std;


/** VarArrayElementVariable **/
/* selects one element of a VarArray according to a Var index */

VarArrayElementVariable::
VarArrayElementVariable(VarArray& input1, const Var& input2)
  :NaryVariable(input1 & (VarArray)input2, input1[0]->length(), input1[0]->width())
{
  int l=input1[0]->length();
  int w=input1[0]->width();
  for (int i=1;i<input1.size();i++)
    if (input1[i]->length()!=l || input1[i]->width()!=w)
      PLERROR("VarArrayElementVairables expect all the elements of input1 array to have the same size");
  if (!input2->isScalar())
    PLERROR("VarArrayElementVariable expect an index Var (input2) of length 1");
}


PLEARN_IMPLEMENT_OBJECT(VarArrayElementVariable, "ONE LINE DESCR", "NO HELP");


void VarArrayElementVariable::recomputeSize(int& l, int& w) const
{ l=varray[0]->length(); w=varray[0]->width(); }


void VarArrayElementVariable::fprop()
{
  int index = (int)(varray.last()->valuedata[0]);
#ifdef BOUNDCHECK
  if (index<0 || index>=varray.size()-1)
    PLERROR("VarArrayElementVariable::fprop, out of bound access of array(%d) at %d",
          varray.size()-1,index);
#endif
  for (int i=0;i<nelems();i++)
    valuedata[i] = varray[index]->valuedata[i];
}


void VarArrayElementVariable::bprop()
{
  int index = (int)(varray.last()->valuedata[0]);
  for (int i=0;i<nelems();i++)
    varray[index]->gradientdata[i] += gradientdata[i];
}


void VarArrayElementVariable::symbolicBprop()
{
  int index = (int)(varray.last()->valuedata[0]);
  varray[index]->accg(g);
}



} // end of namespace PLearn


