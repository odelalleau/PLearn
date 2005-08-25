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

#include "SVDVariable.h"
#include "Var_operators.h"
#include <plearn/math/plapack.h>

namespace PLearn {
using namespace std;


/** SVDVariable **/

PLEARN_IMPLEMENT_OBJECT(SVDVariable, "A SVD decomposition variable.", 
                        "This variable does SVD decomposition of the input matrix variable A=U.D.Vt\n"
                        "The resulting U,D,Vt of the decomposition are stored in a vector as folows:\n"
                        "   - the first MxM elements of the vector are the elements of U\n"
                        "   - the following NxN elements are the elemets of Vt\n"
                        "   - the remaining min(M,N) elements are the elemets of D\n");

SVDVariable::SVDVariable(Variable* input)
    : inherited(input, input->length()*input->length()+input->width()*input->width()+min(input->length(),input->width()),1), 
      m(input->length()),
      n(input->width())
{
    input_copy.resize(input->length(), input->width());
}

void SVDVariable::recomputeSize(int& l, int& w) const
{
    if (input) {
        l = m*m+n*n+min(m,n);
        w = 1;
    } else
        l = w = 0;
}


void SVDVariable::fprop()
{
    input_copy << input->matValue ;

    U = value->subVec(0,m*m)->toMat(m,m);
    Vt = value->subVec(m*m,n*n)->toMat(n,n);
    S = value->subVec(m*m+n*n,min(m,n));

    SVD(input_copy,U,S,Vt);
}


void SVDVariable::bprop() {}

void SVDVariable::symbolicBprop()
{
    PLERROR("In SVDVariable::symbolicBprop: feature not implemented");
}


void SVDVariable::rfprop()
{
    PLERROR("In SVDVariable::rfprop: feature not implemented");
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
