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
   * $Id: TransposeProductVariable.cc,v 1.3 2003/08/13 08:13:17 plearner Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "TransposeProductVariable.h"
#include "Var_utils.h"

namespace PLearn <%
using namespace std;


/** TransposeProductVariable **/

// Matrix product between transpose of matrix1 and matrix2
TransposeProductVariable::TransposeProductVariable(Variable* m1, Variable* m2)
  : BinaryVariable(m1, m2, m1->width(), m2->width())
{
  if (m1->length() != m2->length())
    PLERROR("In ProductVariable: the size of m1 and m2 are not compatible for a matrix product");
}


PLEARN_IMPLEMENT_OBJECT(TransposeProductVariable, "ONE LINE DESCR", "NO HELP");

void TransposeProductVariable::recomputeSize(int& l, int& w) const
{ l=input1->width(); w=input2->width(); }








void TransposeProductVariable::fprop()
{
  // m[i,j] = sum_k input1[k,i] * input2[k,j]
  transposeProduct(matValue, input1->matValue,input2->matValue);
}


void TransposeProductVariable::bprop()
{
  // dC/dinput1[k,i] += sum_j input2[k,j] dC/dm[i,j] 
  productTransposeAcc(input1->matGradient, input2->matValue,matGradient);
  // dC/dinput2[k,j] += sum_i input1[k,i] dC/dm[i,j] 
  productAcc(input2->matGradient, input1->matValue,matGradient);
}


void TransposeProductVariable::bbprop()
{
  if (input1->diaghessian.length()==0)
    input1->resizeDiagHessian();
  if (input2->diaghessian.length()==0)
    input2->resizeDiagHessian();
  // d^2C/dinput1[k,i]^2 += sum_j input2[k,j]^2 dC/dm[i,j] 
  squareProductTransposeAcc(input1->matGradient, input2->matValue,matGradient);
  // d^2C/dinput2[k,j]^2 += sum_i input1[k,i]^2 dC/dm[i,j] 
  squareProductAcc(input2->matGradient, input1->matValue,matGradient);
}


void TransposeProductVariable::symbolicBprop()
{
  // dC/dinput1[k,i] += sum_j input2[k,j] dC/dm[i,j] 
  input1->accg(productTranspose(input2,g));
  // dC/dinput2[k,j] += sum_i input1[k,i] dC/dm[i,j] 
  input2->accg(product(input1, g));
}


void TransposeProductVariable::rfprop()
{
  if (rValue.length()==0) resizeRValue();
  // m[i,j] = sum_k input1[k,i] * input2[k,j]
  transposeProduct(matRValue, input1->matRValue,input2->matValue);
  transposeProductAcc(matRValue, input1->matValue,input2->matRValue);
}



%> // end of namespace PLearn


