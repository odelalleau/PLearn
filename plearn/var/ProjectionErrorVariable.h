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
   * $Id: ProjectionErrorVariable.h,v 1.9 2004/08/09 23:49:49 yoshua Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#ifndef ProjectionErrorVariable_INC
#define ProjectionErrorVariable_INC

#include "BinaryVariable.h"

namespace PLearn {
using namespace std;

//! The first input is a set of n_dim vectors (possibly seen as a single vector of their concatenation) f_i, each in R^n
//! The second input is a set of T vectors (possibly seen as a single vector of their concatenation) t_j, each in R^n
//! The output is the following:
//!    sum_j min_{w_j} || t_j - sum_i w_{ji} f_i ||^2
//! where row w_j of w is optmized analytically and separately for each j.
// N.B. WE CONSIDER THE input2 (t_j's) TO BE FIXED AND DO NOT 
// COMPUTE THE GRADIENT WRT to input2. IF THE USE OF THIS
// OBJECT CHANGES THIS MAY HAVE TO BE REVISED.
class ProjectionErrorVariable: public BinaryVariable
{
  typedef BinaryVariable inherited;
  
public:
  int n; // dimension of the vectors
  bool use_subspace_distance; // use subspace distance instead of distance to targets
  bool normalize_by_neighbor_distance; // normalize projection error by neighbor distance
  real norm_penalization; // penalize sum_i (||f_i||^2 - 1)^2
  real epsilon; // cut-off of singular values to regularize linear system solution
  real regularization; // add to the diagonal of the system matrix for regularization
  int n_dim; // nb of vectors in f
  int T; // nb of vectors in t
  Vec S, fw, norm_err, ww, uu, wwuu, rhs, Tu, one_over_norm_T;
  Mat F, TT, dF, Ut, V, B, VVt, A, A11, A12, A21, A22, wwuuM, FT, FT1, FT2;
  Mat fw_minus_t;
  Mat w; // weights in the above minimization, in each row for each t_j


  //!  Default constructor for persistence
  ProjectionErrorVariable() {}
  ProjectionErrorVariable(Variable* input1, Variable* input2, int n=-1, bool normalize_by_neighbor_distance = true, bool use_subspace_distance=false, real norm_penalization=1.0, real epsilon=1e-6, real regularization=0);

  PLEARN_DECLARE_OBJECT(ProjectionErrorVariable);

  virtual void build();

  virtual void recomputeSize(int& l, int& w) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();

protected:
    void build_();
};

DECLARE_OBJECT_PTR(ProjectionErrorVariable);

inline Var projection_error(Var f, Var t, real norm_penalization=0, int n=-1, 
                            bool normalize_by_neighbor_distance=true,
                            bool use_subspace_distance=false, real epsilon=0, real regularization=0,
                            bool ordered_vectors=true)
  {
      return new ProjectionErrorVariable(f, t, n, normalize_by_neighbor_distance,
                                         use_subspace_distance, norm_penalization, epsilon, 
                                         regularization, ordered_vectors);
  }

                            
} // end of namespace PLearn

#endif 
