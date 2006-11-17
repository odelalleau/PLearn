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
 * $Id: BiasWeightAffineTransformVariable.cc 3994 2005-08-25 13:35:03Z chapados $
 * This file is part of the PLearn library.
 ******************************************************* */

#include "BiasWeightAffineTransformVariable.h"

namespace PLearn {
using namespace std;


PLEARN_IMPLEMENT_OBJECT(BiasWeightAffineTransformVariable,
                        "Affine transformation of a vector variable, from a weight and bias variable.",
                        "NO HELP");

BiasWeightAffineTransformVariable::BiasWeightAffineTransformVariable(
    const VarArray& the_varray,
    bool the_transpose_weights)
    : inherited(the_varray, 
                (the_varray[0]->size() == 1) ? 
                the_varray[2]->size() : 
                (the_varray[0]->isRowVec() ? 1 : the_varray[2]->size()),
                (the_varray[0]->size() == 1) ? 
                1 : (the_varray[0]->isRowVec() ? the_varray[2]->size() : 1)),
      transpose_weights(the_transpose_weights)
{
    build_();
}

void
BiasWeightAffineTransformVariable::build()
{
    inherited::build();
    build_();
}

void
BiasWeightAffineTransformVariable::build_()
{
    if(varray.length() != 3)
        PLERROR("In BiasWeightAffineTransformVariable():build_(): needs an input"
                " vector, a weight matrix and a bias vector");

    vec = varray[0];
    weights = varray[1];
    bias = varray[2];

    if (!vec->isVec())
        PLERROR("In BiasWeightAffineTransformVariable: expecting a vector Var"
            " for the input vector");
    if(vec->size() != (!transpose_weights ? weights->length() : weights->width()))
        PLERROR("In BiasWeightAffineTransformVariable: weight matrix has"
                " incompatible size");
}

void BiasWeightAffineTransformVariable::recomputeSize(int& l, int& w) const
{ 
    if (vec && weights) {
        l = (vec->isRowVec() && vec->size() != 1) ? 1 : bias->size();
        w = vec->isColumnVec() ? 1 : bias->size(); 
    } else
        l = w = 0;
}


void BiasWeightAffineTransformVariable::fprop()
{
    value << bias->value;

    // Seems contradictive, but we are considering x.T versus x.T',
    // not T.x versus T'.x, for consistency with AffineTransformVariable!
    if(transpose_weights)
        productAcc(value, weights->matValue, vec->value);
    else
        transposeProductAcc(value, weights->matValue, vec->value);
}


void BiasWeightAffineTransformVariable::bprop()
{    
    bias->gradient += gradient;    
    if(!vec->dont_bprop_here)
    {
        if(transpose_weights)
            transposeProductAcc(vec->gradient, weights->matValue, gradient);
        else
            productAcc(vec->gradient, weights->matValue, gradient);
    }

    if(transpose_weights)
        externalProductAcc(weights->matGradient, gradient, vec->value);
    else
        externalProductAcc(weights->matGradient, vec->value, gradient);
}


void BiasWeightAffineTransformVariable::symbolicBprop()
{
    PLERROR("BiasWeightAffineTransformVariable::symbolicBprop() not implemented");
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
