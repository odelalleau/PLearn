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
   * $Id: AffineTransformVariable.cc,v 1.3 2003/08/13 08:13:17 plearner Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "AffineTransformVariable.h"
namespace PLearn <%
using namespace std;


PLEARN_IMPLEMENT_OBJECT(AffineTransformVariable, "ONE LINE DESCR", "NO HELP");

void AffineTransformVariable::recomputeSize(int& l, int& w) const
{ l=input1->isRowVec()?1:input2->width(); w=input1->isColumnVec()?1:input2->width(); }


void AffineTransformVariable::fprop()
  {
    value << input2->matValue.firstRow();
    Mat lintransform = input2->matValue.subMatRows(1,input2->length()-1);
    transposeProductAcc(value, lintransform, input1->value);
  }


void AffineTransformVariable::bprop()
  {
    Mat&  afftr = input2->matValue;
    int l = afftr.length();
    // Vec bias = afftr.firstRow();
    Mat lintr = afftr.subMatRows(1,l-1);

    Mat& afftr_g = input2->matGradient;
    Vec bias_g = afftr_g.firstRow();
    Mat lintr_g = afftr_g.subMatRows(1,l-1);

    bias_g += gradient;    
    if(!input1->dont_bprop_here)      
      productAcc(input1->gradient, lintr, gradient);
    externalProductAcc(lintr_g, input1->value, gradient);
  }


void AffineTransformVariable::symbolicBprop()
  {
   PLERROR("AffineTransformVariable::symbolicBprop() not implemented");
  }



%> // end of namespace PLearn


