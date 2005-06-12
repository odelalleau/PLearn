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
 * $Id: NllGeneralGaussianVariable.cc,v 1.2 2005/06/12 19:49:21 larocheh Exp $
 * This file is part of the PLearn library.
 ******************************************************* */

#include <plearn/var/NllGeneralGaussianVariable.h>
#include <plearn/var/Var_operators.h>
#include <plearn/math/plapack.h>
//#include "Var_utils.h"

namespace PLearn {
  using namespace std;

  // R += alpa ( M - v1 v2')
  template<class T>
void my_weird_product(const TMat<T>& R, const TMat<T>& M, const TVec<T>& v1, const TVec<T>& v2,T alpha)
{
#ifdef BOUNDCHECK
  if (M.length() != R.length() || M.width() != R.width() || v1.length()!=M.length() || M.width()!=v2.length() )
    PLERROR("my_weird_product: incompatible arguments' sizes");
#endif
  const T* v_1=v1.data();
  const T* v_2=v2.data();
  for (int i=0;i<M.length();i++)
  {
    T* mi = M[i];
    T* ri = R[i];
    T v1i = v_1[i];
    for (int j=0;j<M.width();j++)
      ri[j] += alpha*(mi[j] - v1i * v_2[j]);
  }
}

  /** NllGeneralGaussianVariable **/

  PLEARN_IMPLEMENT_OBJECT(NllGeneralGaussianVariable,
                          "to do.",
                          " I said TO DO.\n");
  
  NllGeneralGaussianVariable::NllGeneralGaussianVariable(const VarArray& the_varray, real thelogL, int the_mu_nneighbors) 
    : inherited(the_varray,the_varray[3]->length(),1), 
      n(varray[0]->width()), log_L(thelogL),ncomponents(varray[0]->length()),
      nneighbors(varray[3]->length()), mu_nneighbors(the_mu_nneighbors)
    {
      build_();
    }


  void
  NllGeneralGaussianVariable::build()
  {
    inherited::build();
    build_();
  }

  void
  NllGeneralGaussianVariable::build_()
  {
    
    // The VarArray constaints the following variables:
    //    - varray[0] = the tangent plane (ncomponents x n)
    //    - varray[1] = mu(data_point) (n x 1)
    //    - varray[2] = sigma_noise (1 x 1)
    //    - varray[3] = neighbor_distances (nneighbors x n)
     
    if(varray.length() != 4 && varray.length() != 6)
      PLERROR("In NllGeneralGaussianVariable constructor: varray is of length %d but should be of length %d", varray.length(), 4);
    
    if(varray[1]->length() != n || varray[1]->width() != 1) PLERROR("In NllGeneralGaussianVariable constructor: varray[1] is of size (%d,%d), but should be of size (%d,%d)",
                                                                    varray[1]->length(), varray[1]->width(),
                                                                        ncomponents, 1);
    if(varray[2]->length() != 1 || varray[2]->width() != 1) PLERROR("In NllGeneralGaussianVariable constructor: varray[2] is of size (%d,%d), but should be of size (%d,%d)",
                                                                    varray[2]->length(), varray[2]->width(),
                                                                    1, 1);
    if(varray[3]->width() != n) PLERROR("In NllGeneralGaussianVariable constructor: varray[3] is of size (%d,%d), but should be of size (%d,%d)",
                                        varray[3]->length(), varray[3]->width(),
                                        nneighbors, n);

    if(mu_nneighbors < 0) mu_nneighbors = nneighbors;
    use_noise = (varray.length() == 6);

    F = varray[0]->matValue;
    mu = varray[1]->value;
    sn = varray[2]->value;
    diff_y_x = varray[3]->matValue;
    if(use_noise)
    {
      noise_var = varray[4]->value;
      mu_noisy = varray[5]->value;
    }
    z.resize(nneighbors,n);
    U.resize(ncomponents,n);
    Ut.resize(n,n);
    V.resize(ncomponents,ncomponents);
    inv_Sigma_F.resize(ncomponents,n);
    inv_Sigma_z.resize(nneighbors,n);
    if(use_noise) 
    {
      inv_Sigma_z_noisy.resize(nneighbors,n);
      zj_noisy.resize(n);
    } 
    temp_ncomp.resize(ncomponents);
  }


  void NllGeneralGaussianVariable::recomputeSize(int& len, int& wid) const
  {
    len = varray[3]->length();
    wid = 1;
  }

  void NllGeneralGaussianVariable::fprop()
  {
    F_copy.resize(F.length(),F.width());
    sm_svd.resize(ncomponents);
    // N.B. this is the SVD of F'
    F_copy << F;
    lapackSVD(F_copy, Ut, S, V,'A',1.5);
    for (int k=0;k<ncomponents;k++)
    {
      sm_svd[k] = mypow(S[k],2);
      U(k) << Ut(k);
    }  

    real mahal = 0;
    real norm_term = 0;
    real dotp = 0;
    real coef = 0;
    inv_Sigma_z.clear();
    if(use_noise) inv_Sigma_z_noisy.clear();
    tr_inv_Sigma = 0;
    for(int j=0; j<nneighbors;j++)
    {
      zj = z(j);
      substract(diff_y_x(j),mu,zj); // z = y - x - mu(x)
      
      mahal = -0.5*pownorm(zj)/sn[0];      
      norm_term = - n/2.0 * Log2Pi - 0.5*(n-ncomponents)*log(sn[0]);

      inv_sigma_zj = inv_Sigma_z(j);
      inv_sigma_zj << zj; 
      inv_sigma_zj /= sn[0];

      if(j==0)
        tr_inv_Sigma = n/sn[0];

      for(int k=0; k<ncomponents; k++)
      { 
        uk = U(k);
        dotp = dot(zj,uk);
        coef = (1.0/(sm_svd[k]+sn[0]) - 1.0/sn[0]);
        //inv_sigma_zj += dotp*coef*uk;
        multiplyAcc(inv_sigma_zj,uk,dotp*coef);
        mahal -= square(dotp)*0.5*coef;
        norm_term -= 0.5*log(sm_svd[k]);
        if(j==0)
          tr_inv_Sigma += coef;//*pownorm(uk,2);
      }      

      value[j] = -1*(norm_term + mahal);

      if(use_noise)
      {
        substract(zj,noise_var,zj_noisy);
        
        inv_sigma_zj_noisy = inv_Sigma_z_noisy(j);
        inv_sigma_zj_noisy << zj_noisy; 
        inv_sigma_zj_noisy /= sn[0];

        for(int k=0; k<ncomponents; k++)
        { 
          uk = U(k);
          dotp = dot(zj_noisy,uk);
          coef = (1.0/(sm_svd[k]+sn[0]) - 1.0/sn[0]);
          multiplyAcc(inv_sigma_zj_noisy,uk,dotp*coef);
        }      
      }
    }

    inv_Sigma_F.clear();
    for(int k=0; k<ncomponents; k++)
    { 
      fk = F(k);
      inv_sigma_fk = inv_Sigma_F(k);
      inv_sigma_fk << fk;
      inv_sigma_fk /= sn[0];
      for(int k2=0; k2<ncomponents;k2++)
      {
        uk2 = U(k2);
        //inv_sigma_fk += (1.0/(sm_svd[k2]+sn[0]) - 1.0/sn[0])*dot(fk,uk2)*uk2; 
        multiplyAcc(inv_sigma_fk,uk2,(1.0/(sm_svd[k2]+sn[0]) - 1.0/sn[0])*dot(fk,uk2));
      }
    }
  }


  void NllGeneralGaussianVariable::bprop()
  {
    real coef = exp(-log_L);
    for(int neighbor=0; neighbor<nneighbors; neighbor++)
    {
      // dNLL/dF

      product(temp_ncomp,F,inv_Sigma_z(neighbor));
      my_weird_product(varray[0]->matGradient,inv_Sigma_F,temp_ncomp,inv_Sigma_z(neighbor),gradient[neighbor]*coef);

      if(neighbor < mu_nneighbors)
      {
        // dNLL/dmu

        multiplyAcc(varray[1]->gradient, inv_Sigma_z(neighbor),-1.0*gradient[neighbor] *coef) ;
        
        if(use_noise)
          multiplyAcc(varray[5]->gradient, inv_Sigma_z_noisy(neighbor),-1.0*gradient[neighbor] *coef) ;
      }

      // dNLL/dsn

      varray[2]->gradient[0] += gradient[neighbor]*coef* 0.5*(tr_inv_Sigma - pownorm(inv_Sigma_z(neighbor)));
      
    }
  }


  void NllGeneralGaussianVariable::symbolicBprop()
  {
    PLERROR("Not implemented");
  }

} // end of namespace PLearn


