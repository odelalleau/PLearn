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
   * $Id: ProductTransposeVariable.cc,v 1.2 2003/01/08 21:32:57 ducharme Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "ProductTransposeVariable.h"
#include "Var_utils.h"

namespace PLearn <%
using namespace std;


/** ProductTransposeVariable **/

// Matrix product between matrix1 and transpose of matrix2

ProductTransposeVariable::ProductTransposeVariable(Variable* m1, Variable* m2)
  : BinaryVariable(m1, m2, m1->length(), m2->length())
{
  if (m1->width() != m2->width())
    PLERROR("In ProductVariable: the size of m1 and m2 are not compatible for a matrix product");
}


IMPLEMENT_NAME_AND_DEEPCOPY(ProductTransposeVariable);


void ProductTransposeVariable::recomputeSize(int& l, int& w) const
{ l=input1->length(); w=input2->width(); }


void ProductTransposeVariable::deepRead(istream& in, DeepReadMap& old2new)
{
  readHeader(in, "ProductTransposeVariable");
  inherited::deepRead(in, old2new);
  readFooter(in, "ProductTransposeVariable");
}


void ProductTransposeVariable::deepWrite(ostream& out, DeepWriteSet& already_saved) const
{
  writeHeader(out, "ProductTransposeVariable");
  inherited::deepWrite(out, already_saved);
  writeFooter(out, "ProductTransposeVariable");
}


void ProductTransposeVariable::fprop()
{
  // m[i,j] = sum_k input1[i,k] * input2[j,k]
  productTranspose(matValue, input1->matValue,input2->matValue);
}


void ProductTransposeVariable::bprop()
{
  // dC/dinput1[i,k] += sum_j dC/dm[i,j] input2[j,k]
  productAcc(input1->matGradient, matGradient,input2->matValue);
  // dC/dinput2[j,k] += sum_i dC/dm[i,j] itnput1[i,k]
  transposeProductAcc(input2->matGradient, matGradient,input1->matValue);
}


void ProductTransposeVariable::bbprop()
{
  if (input1->diaghessian.length()==0)
    input1->resizeDiagHessian();
  if (input2->diaghessian.length()==0)
    input2->resizeDiagHessian();
  // d^2C/dinput1[i,k]^2 += sum_j d^2C/dm[i,j]^2 input2[j,k]^2
  product2Acc(input1->matGradient, matGradient,input2->matValue);
  // d^2C/dinput2[j,k]^2 += sum_i d^C/dm[i,j]^2 input1[i,k]^2
  transposeProduct2Acc(input2->matGradient, matGradient,input1->matValue);
}


void ProductTransposeVariable::symbolicBprop()
{
  // dC/dinput1[i,k] += sum_j dC/dm[i,j] input2[j,k]
  input1->accg(product(g, input2));
  // dC/dinput2[j,k] += sum_i dC/dm[i,j] itnput1[i,k]
  input2->accg(transposeProduct(g,input1));
}


void ProductTransposeVariable::rfprop()
{
  if (rValue.length()==0)
    resizeRValue();
  productTranspose(matRValue, input1->matRValue,input2->matValue);
  productTransposeAcc(matRValue, input1->matValue,input2->matRValue);
}



%> // end of namespace PLearn


