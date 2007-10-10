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


#ifndef NllGeneralGaussianVariable_INC
#define NllGeneralGaussianVariable_INC

#include <plearn/var/NaryVariable.h>

namespace PLearn {
using namespace std;

class NllGeneralGaussianVariable: public NaryVariable
{
    typedef NaryVariable inherited;
  
protected: 
    //! Dimensionality of the input vectors
    int n; 
    //! Number of components (i.e. number of vectors in f)
    int ncomponents; 
    //! Number of nearest neighbors
    int nneighbors;
    //! Trace of the inverse of the covariance matrix
    real tr_inv_Sigma;

    //! Temporary storage variables
    Vec input, diff_neighbor_input, mu, sm_svd, sn, S;
    Vec uk, fk, uk2, inv_sigma_zj, zj, inv_sigma_fk;
    Vec temp_ncomp;
    Mat neighbors, F,F_copy, z, U, Ut, V, inv_Sigma_F, inv_Sigma_z;

public:
    //! Log of number of components L
    real log_L; 
    //! Indication that a parameter corresponding to the difference
    //! between the Gaussian center and the input data point position
    //! should be used.
    bool use_mu;
    //! Number of nearest neighbors to learn mu,
    //! which must be < then nneighbors
    int mu_nneighbors; 

   
    //!  Default constructor for persistence
    NllGeneralGaussianVariable() {}
    NllGeneralGaussianVariable(const VarArray& the_varray, real thelogL, 
                               bool use_mu, int mu_nneighbors);

    PLEARN_DECLARE_OBJECT(NllGeneralGaussianVariable);

    virtual void build();

    virtual void recomputeSize(int& l, int& w) const;
    virtual void fprop();
    virtual void bprop();
    virtual void symbolicBprop();
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    void build_();

private:

    void bprop_to_bases(const Mat& R, const Mat& M, const Vec& v1, 
                        const Vec& v2,real alpha);

};

DECLARE_OBJECT_PTR(NllGeneralGaussianVariable);

inline Var nll_general_gaussian(Var tangent_plane_var, Var mu_var, Var sn_var, 
                                Var neighbors_var, 
                                real log_L, bool use_mu, int mu_nneighbors)
{
    return new NllGeneralGaussianVariable(tangent_plane_var & mu_var & sn_var & neighbors_var,log_L, use_mu, mu_nneighbors);
}

                            
} // end of namespace PLearn

#endif 


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
