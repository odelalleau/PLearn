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
   * $Id: MatrixSoftmaxVariable.cc,v 1.3 2003/08/13 08:13:17 plearner Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "MatrixSoftmaxVariable.h"

namespace PLearn <%
using namespace std;


/** MatrixSoftmaxVariable **/

MatrixSoftmaxVariable::MatrixSoftmaxVariable(Variable* input) 
  :UnaryVariable(input, input->length(), input->width()) {}


PLEARN_IMPLEMENT_OBJECT(MatrixSoftmaxVariable, "ONE LINE DESCR", "NO HELP");

void MatrixSoftmaxVariable::recomputeSize(int& l, int& w) const
{ l=input->length(); w=input->width(); }








void MatrixSoftmaxVariable::fprop()
{
  Vec column_max(width());
  columnMax(input->matValue, column_max);

  for(int j=0; j<input->width(); j++)
    {
    real s = 0;
    real curmax = column_max[j];
    for(int i=0; i<input->length(); i++)      
      s += (matValue[i][j] = safeexp(input->matValue[i][j]-curmax));
    if (s == 0) PLERROR("trying to divide by 0 in softmax");
    s = 1.0 / s;
    for(int i=0; i<input->length(); i++)
      matValue[i][j] *= s;
    }
}


void MatrixSoftmaxVariable::bprop()
{
  for(int i=0; i<input->width(); i++)
    for(int j=0; j<input->length(); j++)
      {
       real vali = matValue[j][i];
       for(int k=0; k<length(); k++)
         {
          if(k!=j)
            input->matGradient[j][i] -= matGradient[k][i]*vali*matValue[k][i];
          else
            input->matGradient[j][i] += matGradient[j][i]*vali*(1.-vali);
         }
       }
}


void MatrixSoftmaxVariable::bbprop()
{
  PLERROR("MatrixSofmaxVariable::bbprop() not implemented");
}


void MatrixSoftmaxVariable::symbolicBprop()
{
  PLERROR("MatrixSofmaxVariable::symbolicBprop() not implemented");
}


// R{ s_i = exp(x_i) / sum_j exp(x_j) }   = (s_i(1-s_i) - sum_{k!=i} s_i s_k) R(s_i) = s_i ((1-s_i) - sum_{k!=i} s_k) R(s_i)
void MatrixSoftmaxVariable::rfprop()
{
  PLERROR("SofmaxVariable::rfprop() not implemented");
}



%> // end of namespace PLearn


