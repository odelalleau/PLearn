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
 * $Id: NllSemisphericalGaussianVariable.cc,v 1.1 2004/08/06 14:34:16 larocheh Exp $
 * This file is part of the PLearn library.
 ******************************************************* */

#include <plearn/var/NllSemisphericalGaussianVariable.h>
#include <plearn/var/Var_operators.h>
#include <plearn/math/plapack.h>
//#include "Var_utils.h"

namespace PLearn {
  using namespace std;


  /** NllSemisphericalGaussianVariable **/

  PLEARN_IMPLEMENT_OBJECT(NllSemisphericalGaussianVariable,
                          "Computes the negative log-likelihood of a Gaussian on some data point, depending on the nearest neighbors.",
                          "The Gaussian also depends on the variance of the manifold directions, the variance of the noise directions,\n"
                          "the mean of the Gaussian and the tangent at the data point.\n");
  
  NllSemisphericalGaussianVariable::NllSemisphericalGaussianVariable(const VarArray& the_varray, real theepsilon) : inherited(the_varray,the_varray[4]->length(),1), 
                                                                                  n(varray[0]->width()), epsilon(theepsilon), n_dim(varray[0]->length()),
                                                                                  n_neighbors(varray[4]->length())
    {
      build_();
    }


  void
  NllSemisphericalGaussianVariable::build()
  {
    inherited::build();
    build_();
  }

  void
  NllSemisphericalGaussianVariable::build_()
  {
    
    // The VarArray constaints the following variables:
    //    - varray[0] = the tangent plane (n_dim x n)
    //    - varray[1] = mu(data_point) (n x 1)
    //    - varray[2] = sigma_manifold (1 x 1)
    //    - varray[3] = sigma_noise (1 x 1)
    //    - varray[4] = neighbor_distances (n_neighbors x n)
    //    - varray[5] = p_target (1 x 1)
    //    - varray[6] = p_neighbors (n_neighbors x 1)
 
    if(varray.length() != 7)
      PLERROR("In NllSemisphericalGaussianVariable constructor: varray is of length %d but should be of length %d", varray.length(), 7);
    
    if(varray[1]->length() != n || varray[1]->width() != 1) PLERROR("In NllSemisphericalGaussianVariable constructor: varray[1] is of size (%d,%d), but should be of size (%d,%d)",
                                                                    varray[1]->length(), varray[1]->width(),
                                                                        n_dim, 1);
    if(varray[2]->length() != 1 || varray[2]->width() != 1) PLERROR("In NllSemisphericalGaussianVariable constructor: varray[2] is of size (%d,%d), but should be of size (%d,%d)",
                                                                    varray[1]->length(), varray[1]->width(),
                                                                    1, 1);
    if(varray[3]->length() != 1 || varray[3]->width() != 1) PLERROR("In NllSemisphericalGaussianVariable constructor: varray[3] is of size (%d,%d), but should be of size (%d,%d)",
                                                                    varray[1]->length(), varray[1]->width(),
                                                                    1, 1);
    if(varray[4]->width() != n) PLERROR("In NllSemisphericalGaussianVariable constructor: varray[4] is of size (%d,%d), but should be of size (%d,%d)",
                                        varray[1]->length(), varray[1]->width(),
                                        n_neighbors, n);
    if(varray[5]->length() != 1 || varray[5]->width() != 1) PLERROR("In NllSemisphericalGaussianVariable constructor: varray[5] is of size (%d,%d), but should be of size (%d,%d)",
                                                                    varray[1]->length(), varray[1]->width(),
                                                                    1, 1);
    if(varray[6]->length() != n_neighbors || varray[6]->width() != 1) PLERROR("In NllSemisphericalGaussianVariable constructor: varray[6] is of size (%d,%d), but should be of size (%d,%d)",
                                                                              varray[1]->length(), varray[1]->width(), n_neighbors, 1);

    F = varray[0]->matValue;
    mu = varray[1]->value;
    sm = varray[2]->value;
    sn = varray[3]->value;
    diff_y_x = varray[4]->matValue;
    
    z.resize(n_neighbors,n);
    zm.resize(n_neighbors,n);
    zn.resize(n_neighbors,n);
    B.resize(n_dim,n);
    Ut.resize(n,n);
    V.resize(n_dim,n_dim);
    w.resize(n_neighbors,n_dim);

    p_target = varray[5]->value;
    p_neighbors = varray[6]->value;
  }


  void NllSemisphericalGaussianVariable::recomputeSize(int& len, int& wid) const
  {
    len = varray[4]->length();
    wid = 1;
  }

  void NllSemisphericalGaussianVariable::fprop()
  {
    // Let F the tangent plan matrix with rows f_i.
    //  We need to solve the system 
    //     F F' w_j = F z_j
    //  where z_j is the distance between the data point and the j_th neighbor, 
    //  to find the solution w_j of
    //    min_{w_j} || z_j - sum_i w_{ji} f_i ||^2
    //  for each j. Then sum over j the above square errors.
    //  Let F' = U D V' the SVD of F'. Then
    //    w_j = (F F')^{-1} F t_j = (V D U' U D V')^{-1} F t_j = V D^{-2} V' V D U' z_j
    //                                                         = V D^{-1} U' z_j 
    //                                                         = B z_j
    //

    // Compute w

    static Mat F_copy;
    F_copy.resize(F.length(),F.width());
    F_copy << F;
    // N.B. this is the SVD of F'
    lapackSVD(F_copy, Ut, S, V);
    B.clear();
    for (int k=0;k<S.length();k++)
    {
      real s_k = S[k];
      if (s_k>epsilon) // ignore the components that have too small singular value (more robust solution)
      { 
        real coef = 1/s_k;
        for (int i=0;i<n_dim;i++)
        {
          real* Bi = B[i];
          for (int j=0;j<n;j++)
            Bi[j] += V(i,k)*Ut(k,j)*coef;
        }
      }
    }

    //  now that we have B, we can compute the w's and the nll for every neighbors

    for(int j=0; j<n_neighbors;j++)
    {
      Vec zj = z(j);
      substract(diff_y_x(j),mu,zj); // z = y - x - mu(x)
      Vec zmj = zm(j);
      Vec znj = zn(j);
      Vec wj = w(j);
      product(wj, B, zj); // w = B * z = projection weights for neighbor j
      transposeProduct(zmj, F, wj); // F' w = z_m
      substract(zj,zmj,znj); // z_n = z - zm
      value[j] = 0.5*(pownorm(zmj,2)/sm[0] + pownorm(znj,2)/sn[0] + n_dim*log(sm[0]) + (n-n_dim)*log(sn[0])) + n/2.0 * Log2Pi; // This value is not really -log(p(y))
    }
     
    
  }


  void NllSemisphericalGaussianVariable::bprop()
  {
    // calcule dcost/F et incremente input1->matGadient avec cette valeur
    // keeping w fixed
    // 
    // IF use_subspace_distance
    //   dcost/dF = w (F'w - T'u)'
    //
    // ELSE
    //   dcost/dfw = 2 (fw - t_j)/||t_j||
    //   dfw/df_i = w_i 
    //  so 
    //   dcost/df_i = sum_j 2(fw - t_j) w_i/||t_j||
    //
    // IF norm_penalization>0
    //   add the following to the gradient of f_i:
    //     norm_penalization*2*(||f_i||^2 - 1)*f_i
    // N.B. WE CONSIDER THE input2 (t_j's) TO BE FIXED AND DO NOT 
    // COMPUTE THE GRADIENT WRT to input2. IF THE USE OF THIS
    // OBJECT CHANGES THIS MAY HAVE TO BE REVISED.
    //

    for(int neighbor=0; neighbor<n_neighbors; neighbor++)
    {

      // dNLL/dF

      for(int i=0; i<F.length(); i++)
        for(int j=0; j<F.width(); j++)
          varray[0]->matGradient(i,j) += gradient[neighbor]*exp(-1.0*value[neighbor])*p_target[0]/p_neighbors[neighbor] * (1/sm[0] - 1/sn[0]) * w(neighbor,i) * zn(neighbor,j);
      //varray[0]->matGradient(i,j) += gradient[neighbor]*p_target[0]/p_neighbors[neighbor] * (1/sm[0] - 1/sn[0]) * w(neighbor,i) * zn(neighbor,j);
      
      // dNLL/dmu

      for(int i=0; i<mu.length(); i++)
        varray[1]->gradient[i] -= gradient[neighbor]*exp(-1.0*value[neighbor])*p_target[0]/p_neighbors[neighbor] * ( 1/sm[0] * zm(neighbor,i) + 1/sn[0] * zn(neighbor,i));
      //varray[1]->gradient[i] -= gradient[neighbor]*p_target[0]/p_neighbors[neighbor] * ( 1/sm[0] * zm(neighbor,i) + 1/sn[0] * zn(neighbor,i));
      
      // dNLL/dsm

      varray[2]->gradient[0] += gradient[neighbor]*exp(-1.0*value[neighbor])*p_target[0]/p_neighbors[neighbor] * (0.5 * n_dim/sm[0] - pownorm(zm(neighbor),2)/(sm[0]*sm[0]));
      //varray[2]->gradient[0] += gradient[neighbor]*p_target[0]/p_neighbors[neighbor] * (0.5 * n_dim/sm[0] - 0.5*pownorm(zm(neighbor),2)/(sm[0]*sm[0]));
      
      // dNLL/dsn

      varray[3]->gradient[0] += gradient[neighbor]*exp(-1.0*value[neighbor])*p_target[0]/p_neighbors[neighbor] * (0.5 * (n-n_dim)/sn[0] - pownorm(zn(neighbor),2)/(sn[0]*sn[0]));
      //varray[3]->gradient[0] += gradient[neighbor]*p_target[0]/p_neighbors[neighbor] * (0.5 * (n-n_dim)/sn[0] - 0.5*pownorm(zn(neighbor),2)/(sn[0]*sn[0]));
    }

  }


  void NllSemisphericalGaussianVariable::symbolicBprop()
  {
    PLERROR("Not implemented");
  }

} // end of namespace PLearn


