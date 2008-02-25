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

#include "AffineTransformVariable.h"

namespace PLearn {
using namespace std;


PLEARN_IMPLEMENT_OBJECT(
        AffineTransformVariable,
        "Affine transformation of a vector variable.",
        "The first input is the vector variable.\n"
        "The second input is the matrix of biases (on the first row) and\n"
        "weights (in other rows).");

/////////////////////////////
// AffineTransformVariable //
/////////////////////////////
AffineTransformVariable::AffineTransformVariable(Variable* vec,
                                                 Variable* transformation,
                                                 bool call_build_):
    inherited(vec, transformation, 
            (vec->size() == 1) ? transformation->width()
                               : (vec->isRowVec() ? 1
                                                  : transformation->width()),
            (vec->size() == 1) ? 1
                               : (vec->isRowVec() ? transformation->width()
                                                  : 1),
            call_build_)
{
    if (call_build_)
        build_();
}

///////////
// build //
///////////
void AffineTransformVariable::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void AffineTransformVariable::build_()
{
    // input1 is vec from constructor
    if (input1 && !input1->isVec())
        PLERROR("In AffineTransformVariable: expecting a vector Var (row or column) as first argument");
}

///////////////////
// recomputeSize //
///////////////////
void AffineTransformVariable::recomputeSize(int& l, int& w) const
{ 
    if (input1 && input2) {
        l = input1->isRowVec() ? 1 : input2->width();
        w = input1->isColumnVec() ? 1 : input2->width(); 
    } else
        l = w = 0;
}


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
