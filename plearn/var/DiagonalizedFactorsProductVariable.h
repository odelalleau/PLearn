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
   * $Id: DiagonalizedFactorsProductVariable.h,v 1.1 2004/07/19 22:31:11 yoshua Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#ifndef DiagonalizedFactorsProductVariable_INC
#define DiagonalizedFactorsProductVariable_INC

#include "NaryVariable.h"

namespace PLearn {
using namespace std;


/*!   Variable that computes the matrix*diag(vector)*matrix product
      like the one found in singular value decomposition or eigen-decomposition.
      It has three parents: left_matrix U, center_diagonal_vector d, and right_matrix V.
      The output value has elements (i,j) equal to sum_k U_{ik} d_k V_{kj}.
      Optionally the right_matrix or the left_matrix is transposed before being applied.
*/
class DiagonalizedFactorsProductVariable: public NaryVariable
{
    typedef NaryVariable inherited;

public:
  bool transpose_left, transpose_right;

  //!  Default constructor for persistence
  DiagonalizedFactorsProductVariable() {}
  DiagonalizedFactorsProductVariable(Var left_matrix, Var center_diagonal, Var right_matrix,
                                     bool transpose_left=false, bool transpose_right=false);

  PLEARN_DECLARE_OBJECT(DiagonalizedFactorsProductVariable);

  virtual void build();

  virtual void recomputeSize(int& l, int& w) const;
  virtual void fprop();
  virtual void bprop();
  Var& leftMatrix() { return varray[0]; }
  Var& centerDiagonal() { return varray[1]; }
  Var& rightMatrix() { return varray[2]; }

protected:
    void build_();
};

DECLARE_OBJECT_PTR(DiagonalizedFactorsProductVariable);

inline Var diagonalized_factors_product(Var left_matrix, Var center_diagonal, Var right_matrix)
{ return new DiagonalizedFactorsProductVariable(left_matrix,center_diagonal,right_matrix); }

} // end of namespace PLearn

#endif 
