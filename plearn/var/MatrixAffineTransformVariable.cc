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
   * $Id: MatrixAffineTransformVariable.cc,v 1.3 2004/02/20 21:11:50 chrish42 Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "MatrixAffineTransformVariable.h"
#include "SubMatVariable.h"
#include "ProductVariable.h"
#include "MatrixAffineTransformFeedbackVariable.h"

namespace PLearn {
using namespace std;


PLEARN_IMPLEMENT_OBJECT(MatrixAffineTransformVariable, "ONE LINE DESCR", "NO HELP");

void MatrixAffineTransformVariable::recomputeSize(int& l, int& w) const
{ l=input2->width(), w=input1->width(); }


void MatrixAffineTransformVariable::fprop()
{
    Mat lintransform = input2->matValue.subMatRows(1,input2->length()-1);
    for (int i = 0; i < length(); i++)
        for (int j = 0; j < width(); j++)
             matValue[i][j] = input2->matValue[0][i];
    transposeProductAcc(matValue,lintransform, input1->matValue);
}


void MatrixAffineTransformVariable::bprop()
{
    Mat&  afftr = input2->matValue;
    int l = afftr.length();
    Mat lintr = afftr.subMatRows(1,l-1);

    Mat& afftr_g = input2->matGradient;
    Vec bias_g = afftr_g.firstRow();
    Mat lintr_g = afftr_g.subMatRows(1,l-1);

    for (int i = 0; i < length(); i++)
        for (int j = 0; j < width(); j++)
            {
            bias_g[i] += matGradient[i][j];
            }
    if(!input1->dont_bprop_here)      
      productAcc(input1->matGradient, lintr, matGradient);
    productTransposeAcc(lintr_g, input1->matValue, matGradient);
}


void MatrixAffineTransformVariable::symbolicBprop()
{
    Var lintr = new SubMatVariable(input2,0,0,input2->length()-1,input2->width());
    //Var bias = new SubMatVariable(input2,length()-1,0,1,width());

    if(!input1->dont_bprop_here)
      input1->accg(new ProductVariable(lintr,g));
    input2->accg(new MatrixAffineTransformFeedbackVariable(input1,g));
}



} // end of namespace PLearn


