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
   * $Id: ProjectionErrorVariable.cc,v 1.1 2004/05/27 15:03:06 monperrm Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "ProjectionErrorVariable.h"
#include "Var_operators.h"
#include "plapack.h"
//#include "Var_utils.h"

namespace PLearn {
using namespace std;


/** ProjectionErrorVariable **/

PLEARN_IMPLEMENT_OBJECT(ProjectionErrorVariable,
                        "Computes the projection error of a set of vectors on a non-orthogonal basis.\n",
                        "The first input is a set of n_dim vectors (possibly seen as a single vector of their concatenation) f_i, each in R^n\n"
			"The second input is a set of T vectors (possibly seen as a single vector of their concatenation) t_j, each in R^n\n"
			"The output is the following:\n"
			"    sum_j min_w || t_j - sum_i w_i f_i ||^2\n"
			"where w is a local n_dim-vector that is optmized analytically.");

ProjectionErrorVariable::ProjectionErrorVariable(Variable* input1, Variable* input2, int n_, real epsilon_)
  : inherited(input1, input2, 1, 1), n(n_), epsilon(epsilon_)
{
    build_();
}

void
ProjectionErrorVariable::build()
{
    inherited::build();
    build_();
}

void
ProjectionErrorVariable::build_()
{
    if (input1 && input2) {
      if ((input1->length()==1 && input1->width()>1) || 
	  (input1->width()==1 && input1->length()>1))
      {
	if (n<0) PLERROR("ProjectionErrorVariable: Either the input should be matrices or n should be specified\n");
	n_dim = input1->size()/n;
	if (n_dim*n != input1->size())
	  PLERROR("ProjectErrorVariable: the first input size should be an integer multiple of n");
	input1->resize(n_dim,n);
      }
      else 
	n_dim = input1->length();
      if ((input2->length()==1 && input2->width()>1) || 
	  (input2->width()==1 && input2->length()>1))
      {
	if (n<0) PLERROR("ProjectionErrorVariable: Either the input should be matrices or n should be specified\n");
	T = input2->size()/n;
	if (T*n != input2->size())
	  PLERROR("ProjectErrorVariable: the second input size should be an integer multiple of n");
	input2->resize(T,n);
      }
      else 
	T = input2->length();

      if (n<0) n = input1->width();
      if (input2->width()!=n)
	PLERROR("ProjectErrorVariable: the two arguments have inconsistant sizes");
      if (n_dim>n)
	PLERROR("ProjectErrorVariable: n_dim should be less than data dimension n");
      V.resize(n_dim,n_dim);
      Ut.resize(n,n);
      B.resize(n_dim,n);
      VVt.resize(n_dim,n_dim);
      fw_minus_t.resize(T,n);
      w.resize(T,n_dim);
      fw.resize(n);
    }
}


void ProjectionErrorVariable::recomputeSize(int& len, int& wid) const
{
  len = 1;
  wid = 1;
}

void ProjectionErrorVariable::fprop()
{
  // Let F the input1 matrix with rows f_i.
  // We need to solve the system 
  //    F F' w_j = F t_j
  // for each t_j in order to find the solution w of
  //   min_{w_j} || t_j - sum_i w_{ji} f_i ||^2
  // for each j. Then sum over j the above square errors.
  // Let F' = U S V' the SVD of F'. Then
  //   w_j = (F F')^{-1} F t_j = (V S U' U S V')^{-1} F t_j = V S^{-2} V' F t_j.
  // Note that we can pre-compute
  //    B = V S^{-2} V' F 
  // and
  //    w_j = B t_j is our solution.
  //
  Mat F = input1->matValue;
  // N.B. this is the SVD of F'
  lapackSVD(F, Ut, S, V);
  VVt.clear();
  for (int i=0;i<n_dim;i++)
  {
    real s_i = S[i];
    if (s_i>epsilon) // ignore the components that have too small singular value (more robust solution)
    {
      Vec Vi = V(i);
      externalProductScaleAcc(VVt, Vi, Vi, 1.0 / (s_i * s_i));
    }
  }
  productAcc(B,VVt,F);
  //  now we have B, we can compute the w's and the cost
  real cost = 0;
  for(int j=0; j<T;j++)
  {
    Vec tj = input2->matValue(j);
    Vec wj = w(j);
    product(wj, B, tj); // w_j = B * t_j
    transposeProduct(fw, F, wj); // fw = sum_i w_ji f_i 
    Vec fw_minus_tj = fw_minus_t(j);
    substract(fw,tj,fw_minus_tj);
    cost += sumsquare(fw_minus_tj);
  }
  value[0] = cost;
}


void ProjectionErrorVariable::bprop()
{
  // calcule dcost/F et incremente input1->matGadient avec cette valeur
  // keeping w fixed
  // dcost/dfw = 2 (fw - t_j)
  // dfw/df_i = w_i 
  // so 
  //  dcost/df_i = sum_j 2(fw - t_j) w_i
  //
  // N.B. WE CONSIDER THE input2 (t_j's) TO BE FIXED AND DO NOT 
  // COMPUTE THE GRADIENT WRT to input2. IF THE USE OF THIS
  // OBJECT CHANGES THIS MAY HAVE TO BE REVISED.
  //
  for (int j=0;j<T;j++)
  {
    Vec fw_minus_tj = fw_minus_t(j); // n-vector
    Vec wj = w(j);
    for (int i=0;i<n_dim;i++)
    {
      Vec df_i = input1->matGradient(i); // n-vector
      multiplyAcc(df_i, fw_minus_tj, gradient[0] * wj[i]);
    }
  }
}


void ProjectionErrorVariable::symbolicBprop()
{
  PLERROR("Not implemented");
}



} // end of namespace PLearn


