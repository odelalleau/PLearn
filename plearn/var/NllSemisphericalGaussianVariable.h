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


#ifndef NllSemisphericalGaussianVariable_INC
#define NllSemisphericalGaussianVariable_INC

#include <plearn/var/NaryVariable.h>

namespace PLearn {
using namespace std;

//! This class implements the negative log-likelihood cost of a Markov chain that
//! uses semispherical gaussian transition probabilities. The parameters of the
//! semispherical gaussians are a tangent plane, two variances,
//! one mean and the distance of the point with its nearest neighbors.
//! The two variances correspond to the shared variance of every manifold directions
//! and of every noise directions. 
//! This variable is used to do gradient descent on the parameters, but
//! not to estimate de likelihood of the Markov chain a some point, which is
//! more complex to estimate.

class NllSemisphericalGaussianVariable: public NaryVariable
{
  typedef NaryVariable inherited;
  
public:
  int n; // dimension of the vectors
  bool use_subspace_distance; // use subspace distance instead of distance to targets
  bool use_noise;          // Indication that noise on the data should be used to learn the mu parameter
  real epsilon; // cut-off of singular values to regularize linear system solution
  int n_dim; // nb of vectors in f
  int n_neighbors; // nb of neighbors
  Vec mu, sm, sn, S, noise, mu_noisy; 
  Mat F, diff_y_x, z, B, Ut, V, zn, zm, z_noisy, zn_noisy, zm_noisy;
  Vec p_neighbors, p_target;
  Mat w; // weights in the above minimization, in each row for each t_j

  //!  Default constructor for persistence
  NllSemisphericalGaussianVariable() {}
  NllSemisphericalGaussianVariable(const VarArray& the_varray, bool that_use_noise, real theepsilon);

  PLEARN_DECLARE_OBJECT(NllSemisphericalGaussianVariable);

  virtual void build();

  virtual void recomputeSize(int& l, int& w) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();

protected:
    void build_();
};

  DECLARE_OBJECT_PTR(NllSemisphericalGaussianVariable);

  inline Var nll_semispherical_gaussian(Var tangent_plane_var, Var mu_var, Var sm_var, Var sn_var, Var neighbors_dist_var, 
                                                 Var p_target_var, Var p_neighbors_var, Var noise, Var mu_noisy, bool use_noise=false, real epsilon=1e-6)
  {
    return new NllSemisphericalGaussianVariable(tangent_plane_var & mu_var & sm_var & sn_var & neighbors_dist_var & p_target_var & p_neighbors_var & noise & mu_noisy,use_noise, epsilon);
  }

                            
} // end of namespace PLearn

#endif 
