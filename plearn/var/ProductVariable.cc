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

#include "ProductTransposeVariable.h"
#include "ProductVariable.h"
#include "TransposeProductVariable.h"
//#include "Var_utils.h"

namespace PLearn {
using namespace std;


/** ProductVariable **/

// Matrix product

PLEARN_IMPLEMENT_OBJECT(
        ProductVariable,
       "Matrix product.",
       "");

ProductVariable::ProductVariable(Variable* m1, Variable* m2)
    : inherited(m1, m2, m1->length(), m2->width())
{
    build_();
}

void
ProductVariable::build()
{
    inherited::build();
    build_();
}

void
ProductVariable::build_()
{
    if (input1 && input2) {
        // input1 and input2 are (respectively) m1 and m2 from constructor
        if (input1->width() != input2->length())
            PLERROR("In ProductVariable: the size of m1 and m2 are not compatible for a matrix product");
    }
}


void ProductVariable::recomputeSize(int& l, int& w) const
{
    if (input1 && input2) {
        l = input1->length();
        w = input2->width();
    } else
        l = w = 0;
}

void ProductVariable::fprop()
{
    // m[i,j] = sum_k input1[i,k] * input2[k,j]
    product(matValue, input1->matValue, input2->matValue);
}


void ProductVariable::bprop()
{
    // dC/dinput1[i,k] = sum_j dC/dm[i,j] input2[k,j]
    productTransposeAcc(input1->matGradient, matGradient, input2->matValue);
    // dC/dinput2[k,j] += sum_i dC/dm[i,j] input1[i,k]
    transposeProductAcc(input2->matGradient, input1->matValue, matGradient);
}


void ProductVariable::bbprop()
{
    if (input1->diaghessian.length()==0)
        input1->resizeDiagHessian();
    if (input2->diaghessian.length()==0)
        input2->resizeDiagHessian();
    // d^2C/dinput1[i,k]^2 = sum_j d^2C/dm[i,j]^2 input2[k,j]*input2[k,j]
    product2TransposeAcc(input1->matGradient, matGradient, input2->matValue);
    // dC/dinput2[k,j] += sum_i d^2C/dm[i,j]^2 input1[i,k]*input1[i,k]
    transposeProduct2Acc(input2->matGradient, input1->matValue, matGradient);
}


void ProductVariable::symbolicBprop()
{
    // dC/dinput1[i,k] = sum_j dC/dm[i,j] input2[k,j]
    input1->accg(productTranspose(g, input2));
    // dC/dinput2[k,j] += sum_i dC/dm[i,j] input1[i,k]
    input2->accg(transposeProduct(input1, g));
}


//R(x1x2)=R(x1)x2+x1R(x2)
void ProductVariable::rfprop()
{
    if (rValue.length()==0)
        resizeRValue();
    product(matRValue, input1->matValue, input2->matRValue);
    productAcc(matRValue,input1->matRValue, input2->matValue);
}



} // end of namespace PLearn


/*
  Local Variables:
  mode:c++
  c-basic-offset:4
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:79
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=79 :
