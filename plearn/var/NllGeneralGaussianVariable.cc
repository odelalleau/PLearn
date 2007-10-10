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

#include <plearn/var/NllGeneralGaussianVariable.h>
#include <plearn/var/Var_operators.h>
#include <plearn/math/plapack.h>

namespace PLearn {
using namespace std;

/** NllGeneralGaussianVariable **/

PLEARN_IMPLEMENT_OBJECT(NllGeneralGaussianVariable,
                        "Computes the NLL under a Gaussian distribution centered "
                        "around a data point.",
                        "This variable computes the negative log-likelihood "
                        "under a Gaussian distribution\n"
                        "centered near a data point. The likelihood is computed "
                        "for some given neighbors\n"
                        "of the data point. A set of bases defining the "
                        "principal components of the\n"
                        "covariance matrix, the difference mu between "
                        "the data point and the center of \n"
                        "the Gaussian and the noise variance in all directions "
                        "of the space must be\n"
                        "specified. Gradient is propagated in all these "
                        "parameters. Optionally, the \n"
                        "gradient for mu can be computed based on the likelihood "
                        "of less nearest neighbors.\n"
                        "It is assumed that this Gaussian is part of a mixture "
                        "model with L components.\n"
    );
  
NllGeneralGaussianVariable::NllGeneralGaussianVariable(const VarArray& the_varray, real thelogL, bool the_use_mu, int the_mu_nneighbors) 
    : inherited(the_varray,the_varray[3]->length(),1), 
      n(varray[3]->size()), 
      ncomponents(varray[0]->length()%varray[3]->size()),
      nneighbors(varray[4]->length()),
      log_L(thelogL),
      use_mu(the_use_mu),
      mu_nneighbors(the_mu_nneighbors)
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
    //    - varray[0] = the tangent plane (ncomponents x n sized vector)
    //    - varray[1] = mu(data_point) (n x 1)
    //    - varray[2] = sigma_noise (1 x 1)
    //    - varray[3] = input data point around which the Gaussian is centered
    //    - varray[4] = nearest neighbors (nneighbors x n)
     
    if(varray.length() != 5)
        PLERROR("In NllGeneralGaussianVariable::build_(): varray is of "
                "length %d but should be of length %d", varray.length(), 5);
    
    if(varray[1]->length() != n || varray[1]->width() != 1) 
        PLERROR("In NllGeneralGaussianVariable::build_(): varray[1] "
                "is of size (%d,%d), but should be of size (%d,%d)",
                varray[1]->length(), varray[1]->width(),
                ncomponents, 1);

    if(varray[2]->length() != 1 || varray[2]->width() != 1) 
        PLERROR("In NllGeneralGaussianVariable::build_(): varray[2] "
                "is of size (%d,%d), but should be of size (%d,%d)",
                varray[2]->length(), varray[2]->width(),
                1, 1);
    
    if(varray[3]->length() != n || varray[3]->width() != 1) 
        PLERROR("In NllGeneralGaussianVariable::build_(): varray[3] "
                "is of size (%d,%d), but should be of size (%d,%d)",
                varray[3]->length(), varray[3]->width(),
                n,1);

    if(varray[4]->width() != n) 
        PLERROR("In NllGeneralGaussianVariable::build_(): varray[4] "
                "is of size (%d,%d), but should be of size (%d,%d)",
                varray[3]->length(), varray[3]->width(),
                nneighbors, n);

    if(mu_nneighbors < 0) mu_nneighbors = nneighbors;
    if(mu_nneighbors > nneighbors)
        PLERROR("In NllGeneralGaussianVariable::build_(): mu_nneighbors "
            "cannot be > than number of provided neighbors");

    F = varray[0]->value.toMat(ncomponents,n);
    if(use_mu) mu = varray[1]->value;
    sn = varray[2]->value;
    input = varray[3]->value;
    neighbors = varray[4]->matValue;

    diff_neighbor_input.resize(n);
    z.resize(nneighbors,n);
    U.resize(ncomponents,n);
    Ut.resize(n,n);
    V.resize(ncomponents,ncomponents);
    inv_Sigma_F.resize(ncomponents,n);
    inv_Sigma_z.resize(nneighbors,n);
    temp_ncomp.resize(ncomponents);
}


void NllGeneralGaussianVariable::recomputeSize(int& len, int& wid) const
{
    len = varray[4]->length();
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
    tr_inv_Sigma = 0;
    for(int j=0; j<nneighbors;j++)
    {
        zj = z(j);
        if(use_mu)
        {
            substract(neighbors(j),input,diff_neighbor_input); 
            substract(diff_neighbor_input,mu,zj); 
        }
        else
        {
            substract(neighbors(j),input,zj); 
        }
      
        mahal = -0.5*pownorm(zj)/sn[0];      
        norm_term = - n/2.0 * Log2Pi - 0.5*(n-ncomponents)*pl_log(sn[0]);

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
            multiplyAcc(inv_sigma_zj,uk,dotp*coef);
            mahal -= square(dotp)*0.5*coef;
            norm_term -= 0.5*pl_log(sm_svd[k]);
            if(j==0)
                tr_inv_Sigma += coef;
        }

        value[j] = -1*(norm_term + mahal);
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
            multiplyAcc(inv_sigma_fk,uk2,
                        (1.0/(sm_svd[k2]+sn[0]) - 1.0/sn[0])*dot(fk,uk2));
        }
    }
}

// grad_F += alpa ( M - v1 v2')
void NllGeneralGaussianVariable::bprop_to_bases(const Mat& R, const Mat& M, 
                                                const Vec& v1, 
                                                const Vec& v2, real alpha)
{
#ifdef BOUNDCHECK
    if (M.length() != R.length() || M.width() != R.width() 
        || v1.length()!=M.length() || M.width()!=v2.length() )
        PLERROR("NllGeneralGaussianVariable::bprop_to_bases(): incompatible "
                "arguments' sizes");
#endif

    const real* v_1=v1.data();
    const real* v_2=v2.data();
    for (int i=0;i<M.length();i++)
    {
        real* mi = M[i];
        real* ri = R[i];
        real v1i = v_1[i];
        for (int j=0;j<M.width();j++)
            ri[j] += alpha*(mi[j] - v1i * v_2[j]);
    }
}

void NllGeneralGaussianVariable::bprop()
{
    real coef = exp(-log_L);
    for(int neighbor=0; neighbor<nneighbors; neighbor++)
    {
        // dNLL/dF

        product(temp_ncomp,F,inv_Sigma_z(neighbor));
        bprop_to_bases(varray[0]->matGradient,inv_Sigma_F,
                         temp_ncomp,inv_Sigma_z(neighbor),
                         gradient[neighbor]*coef);

        if(use_mu && neighbor < mu_nneighbors)
        {
            // dNLL/dmu

            multiplyAcc(varray[1]->gradient, inv_Sigma_z(neighbor),
                        -1.0*gradient[neighbor] *coef) ;
        }

        // dNLL/dsn

        varray[2]->gradient[0] += gradient[neighbor]*coef* 
            0.5*(tr_inv_Sigma - pownorm(inv_Sigma_z(neighbor)));
      
    }
}


void NllGeneralGaussianVariable::symbolicBprop()
{
    PLERROR("In NllGeneralGaussianVariable::symbolicBprop(): Not implemented");
}

void NllGeneralGaussianVariable::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    NaryVariable::makeDeepCopyFromShallowCopy(copies);
    
    deepCopyField(input, copies);
    deepCopyField(neighbors, copies);
    deepCopyField(diff_neighbor_input, copies);
    deepCopyField(mu, copies);
    deepCopyField(sm_svd, copies);
    deepCopyField(sn, copies);
    deepCopyField(S, copies);
    deepCopyField(uk, copies);
    deepCopyField(fk, copies);
    deepCopyField(uk2, copies);
    deepCopyField(inv_sigma_zj, copies);
    deepCopyField(zj, copies);
    deepCopyField(inv_sigma_fk, copies);
    deepCopyField(temp_ncomp, copies);
    deepCopyField(F, copies);
    deepCopyField(F_copy, copies);
    deepCopyField(z, copies);
    deepCopyField(U, copies);
    deepCopyField(Ut, copies);
    deepCopyField(V, copies);
    deepCopyField(inv_Sigma_F, copies);
    deepCopyField(inv_Sigma_z, copies);
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
