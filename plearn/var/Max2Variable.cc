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
   * $Id: Max2Variable.cc,v 1.4 2004/02/17 21:05:19 tihocan Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "Max2Variable.h"
#include "Var_operators.h"
//#include "Var_utils.h"

namespace PLearn <%
using namespace std;


/** Max2Variable **/

Max2Variable::Max2Variable(Variable* input1, Variable* input2)
  :BinaryVariable(input1, input2, input1->length(), input1->width())
{
  if (input1->length() != input2->length()  ||  input1->width() != input2->width())
    PLERROR("IN Max2Variable input1 and input2 must have the same size");
}


PLEARN_IMPLEMENT_OBJECT(Max2Variable, "ONE LINE DESCR", "NO HELP");


void Max2Variable::recomputeSize(int& l, int& w) const
{ l=input1->length(); w=input1->width(); }









void Max2Variable::fprop()
{
  int n=input1->value.length();
  real* v1=input1->value.data();
  real* v2=input2->value.data();
  real* v=value.data();
  for (int i=0;i<n;i++)
    v[i] = std::max(v1[i],v2[i]);
}


void Max2Variable::bprop()
{
  int n=input1->value.length();
  real* v1=input1->value.data();
  real* v2=input2->value.data();
  real* grad1=input1->gradient.data();
  real* grad2=input2->gradient.data();
  real* grad=gradient.data();
  for (int i=0;i<n;i++)
  {
    if (v2[i]<v1[i])
      grad1[i] += grad[i];
    if (v1[i]<v2[i])
      grad2[i] += grad[i];
  }
}


void Max2Variable::symbolicBprop()
{
  input1->accg((input2<input1)*g);
  input2->accg((input1<input2)*g);
}



%> // end of namespace PLearn


