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

#include "DiagonalizedFactorsProductVariable.h"
#include "Var_utils.h"
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;


/** DiagonalizedFactorsProductVariable **/

PLEARN_IMPLEMENT_OBJECT(DiagonalizedFactorsProductVariable,
                        "Variable that represents the leftmatrix*diag(vector)*rightmatrix product",
                        "The three parents are respectively the left matrix U, the center vector d,\n"
                        "and the right matrix V. Options allow to transpose the matrices.\n"
                        "The output value has elements (i,j) equal to sum_k U_{ik} d_k V_{kj}\n"
    );

DiagonalizedFactorsProductVariable::DiagonalizedFactorsProductVariable(Var left_matrix, 
                                                                       Var center_diagonal, 
                                                                       Var right_matrix,
                                                                       bool transpose_left_, 
                                                                       bool transpose_right_)
    : inherited(left_matrix & center_diagonal & (VarArray)right_matrix,
                transpose_left_?left_matrix->width():left_matrix->length(), 
                transpose_right_?right_matrix->length():right_matrix->width()),
      transpose_left(transpose_left_), transpose_right(transpose_right_)
{
    build_();
}

void
DiagonalizedFactorsProductVariable::build()
{
    inherited::build();
    build_();
}

void
DiagonalizedFactorsProductVariable::build_()
{
    if (varray.size()) {
        int nl = transpose_left?leftMatrix()->length():leftMatrix()->width();
        int nr = transpose_right?rightMatrix()->width():rightMatrix()->length();
        int nc = centerDiagonal()->size();
        if (nl != nc || nr != nc)
            PLERROR("In DiagonalizedFactorsProductVariable: arguments have incompatible sizes!");
    }
}

void DiagonalizedFactorsProductVariable::recomputeSize(int& l, int& w) const
{
    if (varray.size()) {
        l = transpose_left?varray[0]->width():varray[0]->length();
        w = transpose_right?varray[2]->length():varray[2]->width();
    } else
        l = w = 0;
}

void DiagonalizedFactorsProductVariable::fprop()
{
    if (transpose_left)
    {
        if (transpose_right)
            diagonalizedFactorsTransposeProductTranspose(matValue,leftMatrix()->matValue,centerDiagonal()->value,rightMatrix()->matValue);
        else
            diagonalizedFactorsTransposeProduct(matValue,leftMatrix()->matValue,centerDiagonal()->value,rightMatrix()->matValue);
    } else {
        if (transpose_right)
            diagonalizedFactorsProductTranspose(matValue,leftMatrix()->matValue,centerDiagonal()->value,rightMatrix()->matValue);
        else
            diagonalizedFactorsProduct(matValue,leftMatrix()->matValue,centerDiagonal()->value,rightMatrix()->matValue);
    }
}


void DiagonalizedFactorsProductVariable::bprop()
{
    if (transpose_left)
    {
        if (transpose_right)
        {
            // SINCE res[i,j] = sum_k U[k,i] d[k] V[j,k] ==>
            // dC/dU[k,i] = d_k * sum_j dC/dres[i,j] V[j,k]
            // dC/dd[k] = sum_{ij} dC/dres[i,j] U[k,i] V[j,k]
            // dC/dV[j,k] = d_k * sum_i dC/dres[i,j] U[k,i]
            diagonalizedFactorsTransposeProductTransposeBprop(matGradient,leftMatrix()->matValue,
                                                              centerDiagonal()->value,rightMatrix()->matValue,
                                                              leftMatrix()->matGradient,
                                                              centerDiagonal()->gradient,rightMatrix()->matGradient); 
        }
        else
        {
            // SINCE res[i,j] = sum_k U[k,i] d[k] V[k,j] ==>
            // dC/dU[k,i] = d_k * sum_j dC/dres[i,j] V[k,j]
            // dC/dd[k] = sum_{ij} dC/dres[i,j] U[k,i] V[k,j]
            // dC/dV[k,j] = d_k sum_i dC/dres[i,j] U[k,i]
            diagonalizedFactorsTransposeProductBprop(matGradient,leftMatrix()->matValue,centerDiagonal()->value,
                                                     rightMatrix()->matValue, leftMatrix()->matGradient,
                                                     centerDiagonal()->gradient,rightMatrix()->matGradient); 
        }
    } 
    else 
    {
        if (transpose_right)
        {
            // SINCE res[i,j] = sum_k U[i,k] d[k] V[j,k] ==>
            // dC/dU[i,k] = sum_j dC/dres[i,j] d_k V[j,k]
            // dC/dd[k] = sum_{ij} dC/dres[i,j] U[i,k] V[j,k]
            // dC/dV[j,k] = sum_i dC/dres[i,j] d_k U[i,k]
            diagonalizedFactorsProductTransposeBprop(matGradient,leftMatrix()->matValue,centerDiagonal()->value,
                                                     rightMatrix()->matValue, leftMatrix()->matGradient,
                                                     centerDiagonal()->gradient,rightMatrix()->matGradient); 
        }
        else
        {    
            // SINCE res[i,j] = sum_k U[i,k] d[k] V[k,j] ==>
            // dC/dU[i,k] += sum_j dC/dres[i,j] d_k V[k,j]
            // dC/dd[k] += sum_{ij} dC/dres[i,j] U[i,k] V[k,j]
            // dC/dV[k,j] += d_k * sum_i U[i,k] dC/dres[i,j] 
            diagonalizedFactorsProductBprop(matGradient,leftMatrix()->matValue,centerDiagonal()->value,
                                            rightMatrix()->matValue,leftMatrix()->matGradient,
                                            centerDiagonal()->gradient,rightMatrix()->matGradient);
        }  
    }
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
