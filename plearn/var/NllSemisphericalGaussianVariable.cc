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
 * $Id: NllSemisphericalGaussianVariable.cc,v 1.3 2004/08/24 21:15:43 larocheh Exp $
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
                          " This class implements the negative log-likelihood cost of a Markov chain that\n"
                          " uses semispherical gaussian transition probabilities. The parameters of the\n"
                          " semispherical gaussians are a tangent plane, two variances,\n"
                          " one mean and the distance of the point with its nearest neighbors.\n"
                          " The two variances correspond to the shared variance of every manifold directions\n"
                          " and of every noise directions. \n"
                          " This variable is used to do gradient descent on the parameters, but\n"
                          " not to estimate de likelihood of the Markov chain a some point, which is\n"
                          " more complex to estimate.\n");
  
  NllSemisphericalGaussianVariable::NllSemisphericalGaussianVariable(const VarArray& the_varray, bool that_use_noise, real theepsilon, real themin_p_x, int the_mu_n_neighbors) : inherited(the_varray,the_varray[4]->length(),1), 
                                                                                                                                                                                  n(varray[0]->width()), use_noise(that_use_noise),epsilon(theepsilon), min_p_x(themin_p_x), n_dim(varray[0]->length()),
                                                                                                                                                                                  n_neighbors(varray[4]->length()), mu_n_neighbors(the_mu_n_neighbors)
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
    //    - varray[7] = noisy x (n x 1)
 
    if(varray.length() != 9)
      PLERROR("In NllSemisphericalGaussianVariable constructor: varray is of length %d but should be of length %d", varray.length(), 7);
    
    if(varray[1]->length() != n || varray[1]->width() != 1) PLERROR("In NllSemisphericalGaussianVariable constructor: varray[1] is of size (%d,%d), but should be of size (%d,%d)",
                                                                    varray[1]->length(), varray[1]->width(),
                                                                        n_dim, 1);
    if(varray[2]->length() != 1 || varray[2]->width() != 1) PLERROR("In NllSemisphericalGaussianVariable constructor: varray[2] is of size (%d,%d), but should be of size (%d,%d)",
                                                                    varray[2]->length(), varray[2]->width(),
                                                                    1, 1);
    if(varray[3]->length() != 1 || varray[3]->width() != 1) PLERROR("In NllSemisphericalGaussianVariable constructor: varray[3] is of size (%d,%d), but should be of size (%d,%d)",
                                                                    varray[3]->length(), varray[3]->width(),
                                                                    1, 1);
    if(varray[4]->width() != n) PLERROR("In NllSemisphericalGaussianVariable constructor: varray[4] is of size (%d,%d), but should be of size (%d,%d)",
                                        varray[4]->length(), varray[4]->width(),
                                        n_neighbors, n);
    if(varray[5]->length() != 1 || varray[5]->width() != 1) PLERROR("In NllSemisphericalGaussianVariable constructor: varray[5] is of size (%d,%d), but should be of size (%d,%d)",
                                                                    varray[5]->length(), varray[5]->width(),
                                                                    1, 1);
    if(varray[6]->length() != n_neighbors || varray[6]->width() != 1) PLERROR("In NllSemisphericalGaussianVariable constructor: varray[6] is of size (%d,%d), but should be of size (%d,%d)",
                                                                              varray[6]->length(), varray[6]->width(), n_neighbors, 1);
    if(varray[7]->length() != n || varray[7]->width() != 1) PLERROR("In NllSemisphericalGaussianVariable constructor: varray[7] is of size (%d,%d), but should be of size (%d,%d)",
                                                                              varray[7]->length(), varray[7]->width(), n, 1);
    if(varray[8]->length() != n || varray[8]->width() != 1) PLERROR("In NllSemisphericalGaussianVariable constructor: varray[8] is of size (%d,%d), but should be of size (%d,%d)",
                                                                              varray[8]->length(), varray[8]->width(), n, 1);

    if(mu_n_neighbors < 0)
      mu_n_neighbors = n_neighbors;

    F = varray[0]->matValue;
    mu = varray[1]->value;
    sm = varray[2]->value;
    sn = varray[3]->value;
    diff_y_x = varray[4]->matValue;
    
    z.resize(n_neighbors,n);
    zm.resize(n_neighbors,n);
    zn.resize(n_neighbors,n);
    z_noisy.resize(n_neighbors,n);
    zm_noisy.resize(n_neighbors,n);
    zn_noisy.resize(n_neighbors,n);
    B.resize(n_dim,n);
    Ut.resize(n,n);
    V.resize(n_dim,n_dim);
    w.resize(n_neighbors,n_dim);

    p_target = varray[5]->value;
    p_neighbors = varray[6]->value;
    noise = varray[7]->value;
    mu_noisy = varray[8]->value;
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
    /*
    Vec mean_diff(n); mean_diff.clear();
    for(int j=0; j<n_neighbors;j++)
    {
      mean_diff += diff_y_x(j);
    }
    
    mean_diff /= n_neighbors;
    */
    for(int j=0; j<n_neighbors;j++)
    {
      Vec zj = z(j);
      //substract(diff_y_x(j),mean_diff,zj); // z = y - x - mean_diff
      substract(diff_y_x(j),mu,zj); // z = y - x - mu(x)
      Vec zmj = zm(j);
      Vec znj = zn(j);
      Vec wj = w(j);
      product(wj, B, zj); // w = B * z = projection weights for neighbor j
      transposeProduct(zmj, F, wj); // F' w = z_m
      substract(zj,zmj,znj); // z_n = z - zm
      value[j] = 0.5*(pownorm(zmj,2)/sm[0] + pownorm(znj,2)/sn[0] + n_dim*log(sm[0]) + (n-n_dim)*log(sn[0])) + n/2.0 * Log2Pi; // This value is not really -log(p(y))
    }
     
    // and we can make the noisy zm and zn

    for(int j=0; j<n_neighbors;j++)
    {
      Vec zj_noisy = z_noisy(j);
      Vec diff_noisy(n);
      substract(diff_y_x(j),noise,diff_noisy); 
      substract(diff_noisy,mu_noisy,zj_noisy); // z = y - x - mu(x)
      Vec zmj_noisy = zm_noisy(j);
      Vec znj_noisy = zn_noisy(j);
      Vec wj_noisy(n_dim);
      product(wj_noisy, B, zj_noisy); // w = B * z = projection weights for neighbor j
      transposeProduct(zmj_noisy, F, wj_noisy); // F' w = z_m
      substract(zj_noisy,zmj_noisy,znj_noisy); // z_n = z - zm
    }
     
    
  }


  void NllSemisphericalGaussianVariable::bprop()
  {
    

    for(int neighbor=0; neighbor<n_neighbors; neighbor++)
    {

      // dNLL/dF

      for(int i=0; i<F.length(); i++)
        for(int j=0; j<F.width(); j++)
          varray[0]->matGradient(i,j) += gradient[neighbor]*exp(-1.0*value[neighbor])*(p_target[0]+min_p_x)/(p_neighbors[neighbor]+min_p_x) * (1/sm[0] - 1/sn[0]) * w(neighbor,i) * zn(neighbor,j);
     
      if(neighbor < mu_n_neighbors)
      {
        // dNLL/dmu
        if(!use_noise)
        {
          for(int i=0; i<mu.length(); i++)
            varray[1]->gradient[i] -= gradient[neighbor]*exp(-1.0*value[neighbor])*(p_target[0]+min_p_x)/(p_neighbors[neighbor]+min_p_x) * ( 1/sm[0] * zm(neighbor,i) + 1/sn[0] * zn(neighbor,i));
        }
        else
        {
          // dNLL/dmu with noisy data
      
          for(int i=0; i<mu_noisy.length(); i++)
            varray[8]->gradient[i] -= gradient[neighbor]*exp(-1.0*value[neighbor])*(p_target[0]+min_p_x)/(p_neighbors[neighbor]+min_p_x) * ( 1/sm[0] * zm_noisy(neighbor,i) + 1/sn[0] * zn_noisy(neighbor,i));
        }
      }

      // dNLL/dsm

      varray[2]->gradient[0] += gradient[neighbor]*exp(-1.0*value[neighbor])*(p_target[0]+min_p_x)/(p_neighbors[neighbor]+min_p_x) * (0.5 * n_dim/sm[0] - pownorm(zm(neighbor),2)/(sm[0]*sm[0]));
      
      // dNLL/dsn

      varray[3]->gradient[0] += gradient[neighbor]*exp(-1.0*value[neighbor])*(p_target[0]+min_p_x)/(p_neighbors[neighbor]+min_p_x) * (0.5 * (n-n_dim)/sn[0] - pownorm(zn(neighbor),2)/(sn[0]*sn[0]));
      
      
    }

  }


  void NllSemisphericalGaussianVariable::symbolicBprop()
  {
    PLERROR("Not implemented");
  }

} // end of namespace PLearn


