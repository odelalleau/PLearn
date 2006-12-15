// -*- C++ -*-

// GaussianContinuum.cc
//
// Copyright (C) 2004 Yoshua Bengio & Hugo Larochelle 
// 
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
   ******************************************************* */

// Authors: Yoshua Bengio & Martin Monperrus

/*! \file GaussianContinuum.cc */


#include "GaussianContinuum.h"
#include <plearn/vmat/LocalNeighborsDifferencesVMatrix.h>
#include <plearn/var/ProductVariable.h>
#include <plearn/var/PlusVariable.h>
#include <plearn/var/SoftplusVariable.h>
#include <plearn/var/VarRowsVariable.h>
#include <plearn/var/VarRowVariable.h>
#include <plearn/var/SourceVariable.h>
#include <plearn/var/Var_operators.h>
#include <plearn/vmat/ConcatColumnsVMatrix.h>
#include <plearn/math/random.h>
#include <plearn/var/SumOfVariable.h>
#include <plearn/var/TanhVariable.h>
#include <plearn/var/NllSemisphericalGaussianVariable.h>
#include <plearn/var/DiagonalizedFactorsProductVariable.h>
#include <plearn/math/random.h>
#include <plearn/math/plapack.h>
#include <plearn/var/ColumnSumVariable.h>
#include <plearn/vmat/VMat_basic_stats.h>
#include <plearn/vmat/ConcatRowsVMatrix.h>
#include <plearn/vmat/SubVMatrix.h>
#include <plearn/var/PDistributionVariable.h>
#include <plearn_learners/distributions/UniformDistribution.h>
#include <plearn_learners/distributions/GaussianDistribution.h>
#include <plearn/display/DisplayUtils.h>
#include <plearn/opt/GradientOptimizer.h>
#include <plearn/var/TransposeVariable.h>
#include <plearn/var/Var_utils.h>
#include <plearn/var/ConcatRowsVariable.h>
#include <plearn/var/RowSumVariable.h>
#include <plearn/var/NoBpropVariable.h>
#include <plearn/var/ReshapeVariable.h>
#include <plearn/var/SquareVariable.h>
#include <plearn/var/ExpVariable.h>
#include <plearn/io/load_and_save.h>
#include <plearn/vmat/VMat_computeNearestNeighbors.h>
#include <plearn/vmat/FractionSplitter.h>
#include <plearn/vmat/RepeatSplitter.h>

namespace PLearn {
using namespace std;

// les neurones de la couche cachée correspondent à des hyperplans
// la smartInitialization consiste a initialiser ces hyperplans passant
// des points du train_set pris aleatoirement
// comme ca, on est sur de bien quadriller l'espace des points.
// le c correspond a une sorte de contre weight decay
// plus c est grand plus on aura des poids grand et plus on a des neurones tranchés dans l'espace
Mat smartInitialization(VMat v, int n, real c, real regularization)
{
  int l = v->length();
  int w = v->width();
  
  Mat result(n,w);
  Mat temp(w,w);
  Vec b(w);
  b<<c;
  
  int i,j;

  for (i=0;i<n;++i)
  {
    temp.clear();
    for (j=0;j<w;++j)
    {
      v->getRow(uniform_multinomial_sample(l),temp(j));
    }
    // regularization pour eviter 1/ quand on a tire deux fois le meme indice  2/ quand les points sont trops proches
    regularizeMatrix(temp,regularization);
    result(i) << solveLinearSystem(temp, b);
  }
  return result;
}

GaussianContinuum::GaussianContinuum() 
/* ### Initialize all fields to their default value here */
  : weight_mu_and_tangent(0), include_current_point(false), random_walk_step_prop(1), use_noise(false),use_noise_direction(false), noise(-1), noise_type("uniform"), n_random_walk_step(0), n_random_walk_per_point(0),save_image_mat(false),walk_on_noise(true),min_sigma(0.00001), min_diff(0.01), min_p_x(0.001),print_parameters(false),sm_bigger_than_sn(true), n_neighbors(5), n_neighbors_density(-1), mu_n_neighbors(2), n_dim(1), compute_cost_every_n_epochs(5), variances_transfer_function("softplus"), validation_prop(0), architecture_type("single_neural_network"),
    n_hidden_units(-1), batch_size(1), norm_penalization(0), svd_threshold(1e-5)
    
{
}

PLEARN_IMPLEMENT_OBJECT(GaussianContinuum, "Learns a continuous (uncountable) Gaussian mixture with non-local parametrization",
                        "This learner implicitly estimates the density of the data through\n"
                        "a generalization of the Gaussian mixture model and of the TangentLearner\n"
                        "algorithm (see help on that class). The density is the fixed point of\n"
                        "a random walk {z_t} that follows the following transition probabilities:\n"
                        "   z_{t+1} sampled from a Gaussian associated with z_t, centered\n"
                        "   at z_t + mu(z_t), with covariance matrix S(z_t).\n"
                        "The semantic of that random walk is the following (and that is how\n"
                        "it will be estimated). Given a point z_t, the sample z_{t+1} represents\n"
                        "a 'near neighbor' of z_t. We assume that the density is smooth enough\n"
                        "that the cloud of 'near neighbors' around z_t can be modeled by a Gaussian.\n"
                        "The functions mu(.) and S(.) have globally estimated parameters (for example\n"
                        "using neural nets or linear functions of x, or linear functions of a basis).\n"
                        "Here we suppose that the eigenvalues of S(.) come from two groups:\n"
                        "the first group should correspond to locally estimated principal\n"
                        "directions of variations and there are no constraints on these eigenvalues\n"
                        "(except that they are positive), while the second group should correspond\n"
                        "to 'noise' directions, that have all the same value sigma2_noise\n"
                        "i.e. it is not necessary to explicitly model the directions of variations\n"
                        "(the eigenvectors) associated with the second group. In general we expect\n"
                        "sigma2_noise to be small compared to the first group eigenvalues, which\n"
                        "means that the Gaussians are flat in the corresponding directions, and\n"
                        "that the first group variations correspond to modeling a manifold near\n"
                        "which most of the data lie. Optionally, an embedding corresponding\n"
                        "to variations associated with the first group of eigenvalues can be learnt\n"
                        "by choosing for the architecture_type option a value of the form embedding_*.\n"
                        "Although the density is not available in closed form, it is easy (but maybe slow)\n"
                        "to sample from it: pick one of the training examples at random and then\n"
                        "follow the random walk (ideally, a long time). It is also possible in\n"
                        "principle to obtain a numerical estimate of the density at a point x,\n"
                        "by sampling enough random walk points around x.\n"
                        );

/* MATHEMATICAL DETAILS

* Fixed point of the random walk is the density:

  Let p(Z_t) represent the density of the t-th random walk sample Z_t (a r.v.).
  To obtain p(Z_{t+1}) we sample Z_t from p(Z_t) and then sample Z_{t+1}
  from p(Z_{t+1}|Z_t), using the Gaussian with mean z_t + mu(z_t) and 
  covariance matrix S(z_t). Thus p(Z_{t+1}=x) = \int_y p(Z_t=y) p(Z_{t+1}=x|Z_t=y) dy.
  Then at the fixed point we should have p(Z_t) = p(Z_{t+1)=p(X), i.e.
    p(x) = \int_y p(y) p(x|y) dy
  which has the same form as a Gaussian mixture, with p(x|y) Gaussian in x,
  and the sum replaced by an integral (i.e. there is an uncountable 'number'
  of Gaussian components, one at each position y in space). It is possible
  to achieve this only because each Gaussian component p(x|y) has mean and variance that
  depend on y and on global parameters theta, and those parameters are estimated
  from data everywhere, and might generalize to new places.

* How to estimate the density numerically:

  Although the density cannot be computed exactly, it can be estimated
  using a Gaussian mixture with a finite number of components. Suppose that
  we have sampled a set R of random samples on the above random walks
  (including also the training data, which we know to come from the
  true density). Then we obtain a Monte-Carlo approximation of
  the above equation as follows:
    p(x) ~=~ average_t p(x|x_t)

  where x_t is in R (i.e. sampled from the distribution p(y)).  This is
  simply a uniformly weighted Gaussian mixture centered on the data points
  and on the random walk points. If we want to get a more precise estimator
  of p(x), we should sample points more often around x, but then correct
  this bias, using importance sampling. A simple way to do this is to
  choose more points from to put in R in such a way as to give more
  preference to the near neighbors of x in R. Let q_x(x_t) be a discrete
  distribution over the elements of R which is non-zero everywhere but
  puts more weight on the neighbors of x. Then we create new samples,
  to be put in a set R', by performing random walks starting from 
  points of R with probability q_x(x_t). The resulting estimator
  would be
    p(x) ~=~ average_{x_t in R'} p(x|x_t) / (q_x(x_t) |R'|).

* How to estimate mu(x) and S(x)?

  We propose to estimate mu(x) and S(x) by minimizing the negative
  log-likelihood of the neighbors x_j of each training point x_i,
  according to the Gaussian with mean x_i + mu(x_i) and covariance
  matrix S(x_i), plus possibly some regularization term, such
  as weight decay on the parameters of the functions. In this 
  implementation training proceeds by stochastic gradient, visiting
  each example x_i (with all of its neighbors) and then making
  a parameter update.

* Parametrization of mu(x) and S(x):

  mu(x) is simply the output of a linear or neural-net function of x.
  S(x) is more difficult to parametrize. We consider two main solutions
  here: (1) semi-spherical (only two variances are considered: on the
  manifold and orthogonal to it), or (2) factor model with Cholesky
  decomposition for the manifold directions and a single shared variance
  for the directions orthogonal to the manifold. Note that we
  would prefer to parametrize S(x) in such a way as to make it
  easy to compute , v'S(x)^{-1}v for any vector v, and log(det(S(x))).

  Consider the derivative of NLL == -log(p(y)) wrt log(p(y|x)):
    d(-log(p(y)))/d(log(p(y|x))) = -p(y|x)p(x)/p(y) = -p(x|y).
  (this also corresponds to the 'posterior' factor in EM).

  The conditional log-likelihood  log(p(y|x)) for a neighbor y
  of an example x is written
    log(p(y|x)) = -0.5(y-x-mu(x))'S(x)^{-1}(y-x-mu(x)) - 0.5*log(det(S(x))) - (n/2)log(2pi).

  Hence dNLL/dtheta is obtained from
    0.5 p(x|y) (d((y-x-mu(x))'S(x)^{-1}(y-x-mu(x)))/dtheta + d(log(det(S(x))))/dtheta)       (1)
  which gives significant weight only to the near neighbors y of x.

  The gradient wrt mu(x) is in particular
    dNLL/dmu(x) = p(x|y) S(x)^{-1} (mu(x)+x-y).

* Semi-spherical covariance model:

  The idea of the semi-spherical model is that we assume that the neighbors difference
  vector y-x has two components: (a) one along the tangent plane of the manifold, spanned
  by a set vectors F_i(x), the rows of F(x) a matrix-valued unconstrained estimated function,
  and (b) one orthogonal to that tangent plane. We write z = y-x-mu(x) = z_m + z_n, with z_m the
  component on the manifold and z_n the noise component. Since we want z_n orthogonal
  to the tangent plane, we choose it such that F z_n = 0. Since z_m is in the span
  of the rows F_i of F, we can write it as a linear combination of these rows, with
  weights w_i. Let w=(w_1,...w_d), then z_m = F'w. To find w, it is enough to find
  the projection of y-x along the tangent plane, which corresponds to the shortest
  possible z_n. Minimizing the norm of z_n, equal to ||z-F'w||^2 yields the first order equation
      F(z-F'w) = 0
  i.e. the solution is
      w = (FF')^{-1} Fz.
  In practice, this will be done by using a singular value decomposition of F',
      F' = U D V'
  so w = V D^{-2} V' F z = V D^{-2} V' V D U' z = V D^{-1} U' z. Note that
  z_m' z_n = w'F (z - F'w) = 0 hence z_m orthogonal to z_n.

  By our model, the covariance matrix can be decomposed in two parts,
    S(x) = sigma2_manifold U U'  + sigma2_noise N N'
  where M=[U | N] is the matrix whose columns are eigenvectors of S(x), with U the e-vectors
  along the manifold directions and N the e-vectors along the noise directions.
  It is easy to show that one does not need to explicitly represent the
  noise eigenvectors N, because both the columns of U and the columns of N
  are also eigenvectors of the identity matrix. Hence
   S(x) = (sigma2_manifold - sigma2_noise) U U' + sigma2_noise I.
  with I the nxn identity matrix.
  This can be shown by re-writing  I = [U | N]' [U | N] and appriate algebra.

  It is also easy to show that S(x)^{-1} z = (1/sigma2_manifold) z_m + (1/sigma2_noise) z_n,
  that the quadratic form is 
     z' S(x)^{-1} z = (1/sigma2_manifold) ||z_m||^2 + (1/sigma2_noise) ||z_n||^2,          (2)
  and that 
     log(det(S(x))) = d log(sigma2_manifold) + (n-d) log(sigma2_noise).                    (3)

  How to show the above:
    @ We have defined S(x) = M diag(s) M' where s is a vector whose first d elements are sigma2_manifold
    and last n-d elements are sigma2_noise, and M=[U | N] are the eigenvectors, or
      S(x) = sum_{i=1}^d sigma2_manifold U_i U_i' + sum_{i=d+1}^n sigma2_noise N_i N_i'
    where U_i is a column of U and N_i a column of N. Hence
      S(x) = sigma2_manifold sum_{i=1}^d U_i U_i' - sigma2_noise sum_{i=1}^d U_i U_i'
             + sigma2_noise (sum_{i=1}^d U_i U_i' + sum_{i=d+1}^n  N_i N_i')
           = (sigma2_manifold - sigma2_noise) sum_{i=1}^d U_i U_i' + sigma2_noise I 
           = (sigma2_manifold - sigma2_noise) U U' + sigma2_noise I 
    since sum_{i=1}^n M_i M_i' = M M' = I (since M is full rank).

    @ S(x)^{-1} = M diag(s)^{-1} M' = (1/sigma2_manifold - 1/sigma2_noise) U U' + 1/sigma2_noise I 
    using the same argument as above but replacing all sigma2* by 1/sigma2*.

    @ Hence S(x)^{-1} z = S(x)^{-1} (z_m + z_n) 
                      = (1/sigma2_manifold - 1/sigma2_noise) z_m + 1/sigma2_noise (z_m + z_n)
                      = 1/sigma2_manifold z_m + 1/sigma2_noise z_n
    where on the second line we used the fact that U U' acts as the identity
    matrix for vectors spanned by the columns of U, which can be shown as follows.
    Let z_m = sum_i a_i U_i. Then U U' z_m = sum_i a_i U U' U_i = sum_i a_i U e_i = sum_i a_i U_i = z_m.

    @ Also, z' S(x)^{-1} z = (z_m + z_n) (1/sigma2_manifold z_m + 1/sigma2_noise z_n)
                         = 1/sigma2_manifold ||z_m||^2 + 1/sigma2_noise ||z_n||^2
    since by construction z_m . z_n = 0.

    @ Finally, log(det(S(x))) = sum_{i=1}^n log(s_i) 
                              = sum_{i=1}^d log(sigma2_manifold) + sum_{i=d+1}^n log(sigma2_noise)
                              = d log(sigma2_manifold) + (n-d) log(sigma2_noise).

                              
* Gradients on covariance for the semi-spherical model:

  We have already shown the gradient of NLL on mu(x) above. We need
  also here the gradient on sigma2_manifold, sigma2_noise, and F, all
  three of which are supposed to be functions of x (e.g. outputs of
  a neural network, so we need to provide the gradient on the output
  units of the neural network). Note that the sigma2's must be constrained
  to be positive (e.g. by squaring the output, using an exponential
  or softplus activation function).

    dNLL/dsigma2_manifold = 0.5 p(x|y) ( d/sigma2_manifold - ||z_m||^2/sigma2_manifold^2)

  N.B. this is the gradient on the variance, not on the standard deviation.

  Proof: Recall eq.(1) and let theta = dsigma2_manifold. Using eq.(2) we obtain
  for the first term in (1): 
    d/dsigma2_manifold (0.5/sigma2_manifold ||z_m||^2) = -0.5||z_m||^2/sigma2_manifold^2.
  Using (3) we obtain the second term 
    d/dsigma2_manifold (0.5 d log(sigma2_manifold)) = 0.5 d/sigma2_manifold.

  The same arguments yield the following for the gradient on sigma2_noise:

    dNLL/dsigma2_noise = 0.5 p(x|y) ( (n-d)/sigma2_noise - ||z_n||^2/sigma2_noise^2)


  Now let us consider the more difficult case of the theta = F_{ij} (i in {1..d}, j in {1..n}).
  The result is fortunately simple to write:

    dNLL/dF = p(x|y) (1/sigma2_manifold - 1/sigma2_noise) w z_n'

  Proof: First we see that the second term in eq.(1) does not depend on F because of eq.(3).
  For the first term of eq.(1), we obtain using (2)
    d(0.5 z'S(x)^{-1} z)/dF_{ij} 
      = d/dF_{ij} ((0.5/sigma2_manifold) ||z_m||^2 + (0.5/sigma2_noise) ||z_n||^2)
      = d/dF_{ij} ((0.5/sigma2_manifold) ||F'w||^2 + (0.5/sigma2_noise) ||z-F'w||^2)
      = (1/sigma2_manifold) (F'w)' d(F'w)/dF_{ij} + (1/sigma2_noise) z_n' d(z-F'w)/dF_{ij} 
      = (1/sigma2_manifold) (F'w)' d(F'w)/dF_{ij} - (1/sigma2_noise) z_n' d(F'w)/dF_{ij} (4)
  Note that w depends of F so we will have to compute two components:
    d(F'w)/dF_{ij} = w_i e_j + F' dw/dF_{ij}                                        (5)
  Now recall how w depends on F: w = (FF')^{-1} F z, and recall the identity 
  d(A^{-1})/dx = -A^{-1} dA/dx A^{-1} for square matrix A. Hence
    dw/dF_{ij} = - (FF')^{-1} d(FF')/dF_{ij} (FF')^{-1} F z + (FF')^{-1} dF/dF_{ij} z
               = - (FF')^{-1} ( F e_j e_i' + e_i e_j' F') w + (FF')^{-1} e_i e_j' z
  where we have replaced (FF')^{-1}Fz by w in the last factor of the first term, and
  where e_i is the d-vector with all 0's except a 1 at position i, and e_j is the n-vector
  with all 0's except a 1 at position j. It is easy to see that dF/dF_{ij} = e_i e_j'
  which is the matrix with all 0's except at position (i,j). Then 
    d(FF')/dF_{ij} = F (dF/dF_{ij})' + dF/dF_{ij} F' = F e_j e_i' + e_i e_j' F'.
  
  We are now ready to plug pop back and plug all these results together. First we plug
  the above in (5):
   d(F'w)/dF_{ij} = w_i e_j + F' (FF')^{-1} e_i e_j' z - (FF')^{-1} ( F e_j e_i' + e_i e_j' F') w
  then plug this back in (4) noting that FF' cancels with (FF')^{-1} everywhere in the sigma2_manifold term
   d(0.5 z'S(x)^{-1} z)/dF_{ij} =
     (1/sigma2_manifold)  (w'F w_i e_j + w'e_i e_j' z - w'(F e_j e_i' + e_i e_j' F') w
     - (1/sigma2_noise) w_i z_n'e_j
   using z_n'F' = 0, and z_n' (FF')^{-1} = z_n'VD^{-2}V'=0 since z_n is orthogonal to every column of V.
   Note: F'(FF')^{-1}F = UDV'(VD^{-2}V')VDU' = UU', and UU'z = UU'(z_m+z_n) = z_m to simplify last term.
   In the sigma2_manifold term let us use the facts that (a) each sub-term is a scalar, (b) tr(AB)=tr(BA),
   (c) scalar = scalar', and (e) e_i'A e_j = A_{ij} to write everything in matrix form:
       (1/sigma2_manifold)  (w'F e_j w_i  + w'e_i e_j' z - w'(F e_j e_i' + e_i e_j' F') w)
     = (1/sigma2_manifold)  (w'F e_j e_i' w + z'e_j e_i' w - w'F e_j e_i'w - z'UU'e_j e_i'w
     = (1/sigma2_manifold)  (e_i'ww'F e_j + e_i'wz'e_j - e_i'ww'Fe_j - e_i'w z_m' e_j
     = (1/sigma2_manifold)  (ww'F  + wz' - ww'F - w z_m')_{ij}
     = (1/sigma2_manifold)  (wz' - w z_m')_{ij}
     = (1/sigma2_manifold)  (w z_n')_{ij}
   Now let us do the sigma2_noise term:
       (1/sigma2_noise) w_i z_n'e_j = (1/sigma2_noise) e_i' w z_n'e_j = (1/sigma2_noise) (w z_n')_{ij}
   Putting the sigma2_manifold term and the sigma2_noise term together we obtain in matrix form
    d(0.5 z'S(x)^{-1} z)/dF = (1/sigma2_manifold) w z_n' - (1/sigma2_noise) w z_n'
   i.e. the final result
    d(0.5 z'S(x)^{-1} z)/dF = (1/sigma2_manifold - 1/sigma2_noise) w z_n'
   which gives (using dlog(det(S(x)))/dF = ), the claimed statement:
    dNLL/dF = p(x|y) (1/sigma2_manifold - 1/sigma2_noise) w z_n'

*/

void GaussianContinuum::declareOptions(OptionList& ol)
{
  // ### Declare all of this object's options here
  // ### For the "flags" of each option, you should typically specify  
  // ### one of OptionBase::buildoption, OptionBase::learntoption or 
  // ### OptionBase::tuningoption. Another possible flag to be combined with
  // ### is OptionBase::nosave

  declareOption(ol, "weight_mu_and_tangent", &GaussianContinuum::weight_mu_and_tangent, OptionBase::buildoption,
		"Weight of the cost on the scalar product between the manifold directions and mu.\n"
		);

  declareOption(ol, "include_current_point", &GaussianContinuum::include_current_point, OptionBase::buildoption,
		"Indication that the current point should be included in the nearest neighbors.\n"
		);

  declareOption(ol, "n_neighbors", &GaussianContinuum::n_neighbors, OptionBase::buildoption,
		"Number of nearest neighbors to consider for gradient descent.\n"
		);

  declareOption(ol, "n_neighbors_density", &GaussianContinuum::n_neighbors_density, OptionBase::buildoption,
		"Number of nearest neighbors to consider for p(x) density estimation.\n"
		);

  declareOption(ol, "mu_n_neighbors", &GaussianContinuum::mu_n_neighbors, OptionBase::buildoption,
		"Number of nearest neighbors to learn the mus (if < 0, mu_n_neighbors = n_neighbors).\n"
		);

  declareOption(ol, "n_dim", &GaussianContinuum::n_dim, OptionBase::buildoption,
		"Number of tangent vectors to predict.\n"
		);

  declareOption(ol, "compute_cost_every_n_epochs", &GaussianContinuum::compute_cost_every_n_epochs, OptionBase::buildoption,
		"Frequency of the computation of the cost on the training and validation set. \n"
		);

  declareOption(ol, "optimizer", &GaussianContinuum::optimizer, OptionBase::buildoption,
		"Optimizer that optimizes the cost function.\n"
		);
		  
  declareOption(ol, "variances_transfer_function", &GaussianContinuum::variances_transfer_function, 
                OptionBase::buildoption,
                "Type of output transfer function for predicted variances, to force them to be >0:\n"
                "  square : take the square\n"
                "  exp : apply the exponential\n"
                "  softplus : apply the function log(1+exp(.))\n"
                );
		  
  declareOption(ol, "architecture_type", &GaussianContinuum::architecture_type, OptionBase::buildoption,
		"For pre-defined tangent_predictor types: \n"
		"   single_neural_network : prediction = b + W*tanh(c + V*x), where W has n_hidden_units columns\n"
		"                          where the resulting vector is viewed as a n_dim by n matrix\n"
    "   embedding_neural_network: prediction[k,i] = d(e[k]/d(x[i), where e(x) is an ordinary neural\n"
    "                             network representing the embedding function (see output_type option)\n"
		"where (b,W,c,V) are parameters to be optimized.\n"
		);

  declareOption(ol, "n_hidden_units", &GaussianContinuum::n_hidden_units, OptionBase::buildoption,
		"Number of hidden units (if architecture_type is some kind of neural network)\n"
		);
/*
  declareOption(ol, "output_type", &GaussianContinuum::output_type, OptionBase::buildoption,
		"Default value (the only one considered if architecture_type != embedding_*) is\n"
    "   tangent_plane: output the predicted tangent plane.\n"
    "   embedding: output the embedding vector (only if architecture_type == embedding_*).\n"
    "   tangent_plane+embedding: output both (in this order).\n"
		);
*/
 
  declareOption(ol, "batch_size", &GaussianContinuum::batch_size, OptionBase::buildoption, 
                "    how many samples to use to estimate the average gradient before updating the weights\n"
                "    0 is equivalent to specifying training_set->length() \n");

  declareOption(ol, "svd_threshold", &GaussianContinuum::svd_threshold, OptionBase::buildoption,
		"Threshold to accept singular values of F in solving for linear combination weights on tangent subspace.\n"
		);

  declareOption(ol, "print_parameters", &GaussianContinuum::print_parameters, OptionBase::buildoption,
		"Indication that the parameters should be printed for the training set points.\n"
		);

   declareOption(ol, "sm_bigger_than_sn", &GaussianContinuum::sm_bigger_than_sn, OptionBase::buildoption,
		"Indication that sm should always be bigger than sn.\n"
		);

  declareOption(ol, "save_image_mat", &GaussianContinuum::save_image_mat, OptionBase::buildoption,
		"Indication that a matrix corresponding to the probabilities of the points on a 2d grid should be created.\n"
                );

  declareOption(ol, "walk_on_noise", &GaussianContinuum::walk_on_noise, OptionBase::buildoption,
		"Indication that the random walk should also consider the noise variation.\n"
                );

  declareOption(ol, "upper_y", &GaussianContinuum::upper_y, OptionBase::buildoption,
		"Upper bound on the y (second) coordinate.\n"
                );
  
  declareOption(ol, "upper_x", &GaussianContinuum::upper_x, OptionBase::buildoption,
		"Lower bound on the x (first) coordinate.\n"
                );

  declareOption(ol, "lower_y", &GaussianContinuum::lower_y, OptionBase::buildoption,
		"Lower bound on the y (second) coordinate.\n"
                );
  
  declareOption(ol, "lower_x", &GaussianContinuum::lower_x, OptionBase::buildoption,
		"Lower bound on the x (first) coordinate.\n"
                );

  declareOption(ol, "points_per_dim", &GaussianContinuum::points_per_dim, OptionBase::buildoption,
		"Number of points per dimension on the grid.\n"
                );

  declareOption(ol, "parameters", &GaussianContinuum::parameters, OptionBase::learntoption,
		"Parameters of the tangent_predictor function.\n"
		);

  declareOption(ol, "Bs", &GaussianContinuum::Bs, OptionBase::learntoption,
		"The B matrices for the training set.\n"
		);

  declareOption(ol, "Fs", &GaussianContinuum::Fs, OptionBase::learntoption,
		"The F (tangent planes) matrices for the training set.\n"
                );

  declareOption(ol, "mus", &GaussianContinuum::mus, OptionBase::learntoption,
		"The mu vertors for the training set.\n"
                );

  declareOption(ol, "sms", &GaussianContinuum::sms, OptionBase::learntoption,
		"The sm values for the training set.\n"
                );
  
  declareOption(ol, "sns", &GaussianContinuum::sns, OptionBase::learntoption,
		"The sn values for the training set.\n"
                );

  declareOption(ol, "min_sigma", &GaussianContinuum::min_sigma, OptionBase::buildoption,
		"The minimum value for sigma noise and manifold.\n"
                );

  declareOption(ol, "min_diff", &GaussianContinuum::min_diff, OptionBase::buildoption,
		"The minimum value for the difference between sigma manifold and noise.\n"
                );

  declareOption(ol, "min_p_x", &GaussianContinuum::min_p_x, OptionBase::buildoption,
		"The minimum value for p_x, for stability concerns when doing gradient descent.\n"
                );

  declareOption(ol, "n_random_walk_step", &GaussianContinuum::n_random_walk_step, OptionBase::buildoption,
		"The number of random walk step.\n"
                );

  declareOption(ol, "n_random_walk_per_point", &GaussianContinuum::n_random_walk_per_point, OptionBase::buildoption,
		"The number of random walks per training set point.\n"
                );

  declareOption(ol, "noise", &GaussianContinuum::noise, OptionBase::buildoption,
		"Noise parameter for the training data.\n"
                );

  declareOption(ol, "noise_type", &GaussianContinuum::noise_type, OptionBase::buildoption,
		"Type of the noise (\"uniform\" or \"gaussian\").\n"
                );

  declareOption(ol, "use_noise", &GaussianContinuum::use_noise, OptionBase::buildoption,
		"Indication that the training should be done using noise on training data.\n"
                );

  declareOption(ol, "use_noise_direction", &GaussianContinuum::use_noise_direction, OptionBase::buildoption,
		"Indication that the noise should be directed in the noise directions.\n"
                );

  declareOption(ol, "random_walk_step_prop", &GaussianContinuum::random_walk_step_prop, OptionBase::buildoption,
		"Proportion or confidence of the random walk steps.\n"
                );

  declareOption(ol, "validation_prop", &GaussianContinuum::validation_prop, OptionBase::buildoption,
		"Proportion of points for validation set (if uncorrect value, validtion_set == train_set).\n"
                );
  
  declareOption(ol, "reference_set", &GaussianContinuum::reference_set, OptionBase::learntoption,
		"Reference points for density computation.\n"
                );
  
  


  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void GaussianContinuum::build_()
{

  n = PLearner::inputsize_;

  if (n>0)
  {
    Var log_n_examples(1,1,"log(n_examples)");


    {
      if (n_hidden_units <= 0)
        PLERROR("GaussianContinuum::Number of hidden units should be positive, now %d\n",n_hidden_units);

      if(validation_prop <= 0 || validation_prop >= 1) valid_set = train_set;
      else
      {
        // Making FractionSplitter
        PP<FractionSplitter> fsplit = new FractionSplitter();
        TMat<pair<real,real> > splits(1,2); 
        splits(0,0).first = 0; splits(0,0).second = 1-validation_prop;
        splits(0,1).first = 1-validation_prop; splits(0,1).second = 1;
        fsplit->splits = splits;
        fsplit->build();
      
        // Making RepeatSplitter
        PP<RepeatSplitter> rsplit = new RepeatSplitter();
        rsplit->n = 1;
        rsplit->shuffle = true;
        rsplit->seed = 123456;
        rsplit->to_repeat = fsplit;
        rsplit->setDataSet(train_set);
        rsplit->build();

        TVec<VMat> vmat_splits = rsplit->getSplit();
        train_set = vmat_splits[0];
        valid_set = vmat_splits[1];
      
      }

      x = Var(n);
      c = Var(n_hidden_units,1,"c ");
      V = Var(n_hidden_units,n,"V ");               
      Var a = tanh(c + product(V,x));
      muV = Var(n,n_hidden_units,"muV "); 
      smV = Var(1,n_hidden_units,"smV ");  
      smb = Var(1,1,"smB ");
      snV = Var(1,n_hidden_units,"snV ");  
      snb = Var(1,1,"snB ");      
        

      if(architecture_type == "embedding_neural_network")
      {
        W = Var(n_dim,n_hidden_units,"W ");       
        tangent_plane = diagonalized_factors_product(W,1-a*a,V); 
        embedding = product(W,a);
      } 
      else if(architecture_type == "single_neural_network")
      {
        b = Var(n_dim*n,1,"b");
        W = Var(n_dim*n,n_hidden_units,"W ");
        tangent_plane = reshape(b + product(W,tanh(c + product(V,x))),n_dim,n);
      }
      else
        PLERROR("GaussianContinuum::build_, unknown architecture_type option %s",
                architecture_type.c_str());
     
      mu = product(muV,a); 
      min_sig = new SourceVariable(1,1);
      min_sig->value[0] = min_sigma;
      min_sig->setName("min_sig");
      min_d = new SourceVariable(1,1);
      min_d->value[0] = min_diff;
      min_d->setName("min_d");

      if(noise > 0)
      {
        if(noise_type == "uniform")
        {
          PP<UniformDistribution> temp = new UniformDistribution();
          Vec lower_noise(n);
          Vec upper_noise(n);
          for(int i=0; i<n; i++)
          {
            lower_noise[i] = -1*noise;
            upper_noise[i] = noise;
          }
          temp->min = lower_noise;
          temp->max = upper_noise;
          dist = temp;
        }
        else if(noise_type == "gaussian")
        {
          PP<GaussianDistribution> temp = new GaussianDistribution();
          Vec mu(n); mu.clear();
          Vec eig_values(n); 
          Mat eig_vectors(n,n); eig_vectors.clear();
          for(int i=0; i<n; i++)
          {
            eig_values[i] = noise; // maybe should be adjusted to the sigma noiseat the input
            eig_vectors(i,i) = 1.0;
          }
          temp->mu = mu;
          temp->eigenvalues = eig_values;
          temp->eigenvectors = eig_vectors;
          dist = temp;
        }
        else PLERROR("In GaussianContinuum::build_() : noise_type %c not defined",noise_type.c_str());
        noise_var = new PDistributionVariable(x,dist);
        if(use_noise_direction)
        {
          for(int k=0; k<n_dim; k++)
          {
            Var index_var = new SourceVariable(1,1);
            index_var->value[0] = k;
            Var f_k = new VarRowVariable(tangent_plane,index_var);
            noise_var = noise_var - product(f_k,noise_var)* transpose(f_k)/pownorm(f_k,2);
          }
        }
        noise_var = no_bprop(noise_var);
        noise_var->setName(noise_type);
      }
      else
      {
        noise_var = new SourceVariable(n,1);
        noise_var->setName("no noise");
        for(int i=0; i<n; i++)
          noise_var->value[i] = 0;
      }


      // Path for noisy mu
      Var a_noisy = tanh(c + product(V,x+noise_var));
      mu_noisy = product(muV,a_noisy); 

      if(sm_bigger_than_sn)
      {
        if(variances_transfer_function == "softplus") sn = softplus(snb + product(snV,a)) + min_sig;
        else if(variances_transfer_function == "square") sn = square(snb + product(snV,a)) + min_sig;
        else if(variances_transfer_function == "exp") sn = exp(snb + product(snV,a)) + min_sig;
        else PLERROR("In GaussianContinuum::build_ : unknown variances_transfer_function option %s ", variances_transfer_function.c_str());
        Var diff;
        
        if(variances_transfer_function == "softplus") diff = softplus(smb + product(smV,a)) + min_d;
        else if(variances_transfer_function == "square") diff = square(smb + product(smV,a)) + min_d;
        else if(variances_transfer_function == "exp") diff = exp(smb + product(smV,a)) + min_d;
        sm = sn + diff;
      }
      else
      {
        if(variances_transfer_function == "softplus"){
          sm = softplus(smb + product(smV,a)) + min_sig; 
          sn = softplus(snb + product(snV,a)) + min_sig;
        }
        else if(variances_transfer_function == "square"){
          sm = square(smb + product(smV,a)) + min_sig; 
          sn = square(snb + product(snV,a)) + min_sig;
        }
        else if(variances_transfer_function == "exp"){
          sm = exp(smb + product(smV,a)) + min_sig; 
          sn = exp(snb + product(snV,a)) + min_sig;
        }
        else PLERROR("In GaussianContinuum::build_ : unknown variances_transfer_function option %s ", variances_transfer_function.c_str());
      }
      
      mu_noisy->setName("mu_noisy ");
      tangent_plane->setName("tangent_plane ");
      mu->setName("mu ");
      sm->setName("sm ");
      sn->setName("sn ");
      a_noisy->setName("a_noisy ");
      a->setName("a ");
      if(architecture_type == "embedding_neural_network")
        embedding->setName("embedding ");
      x->setName("x ");

      if(architecture_type == "embedding_neural_network")
        predictor = Func(x, W & c & V & muV & smV & smb & snV & snb, tangent_plane & mu & sm & sn);
      if(architecture_type == "single_neural_network")
        predictor = Func(x, b & W & c & V & muV & smV & smb & snV & snb, tangent_plane & mu & sm & sn);
      /*
      if (output_type=="tangent_plane")
        output_f = Func(x, tangent_plane);
      else if (output_type=="embedding")
      {
        if(architecture_type == "single_neural_network")
          PLERROR("Cannot obtain embedding with single_neural_network architecture");
        output_f = Func(x, embedding);
      }
      else if (output_type=="tangent_plane+embedding")
      {
        if(architecture_type == "single_neural_network")
          PLERROR("Cannot obtain embedding with single_neural_network architecture");
        output_f = Func(x, tangent_plane & embedding);
      }
      else if(output_type == "tangent_plane_variance_normalized")
        output_f = Func(x,tangent_plane & sm);
      else if(output_type == "semispherical_gaussian_parameters")
        output_f = Func(x,tangent_plane & mu & sm & sn);
      */
      output_f_all = Func(x,tangent_plane & mu & sm & sn);
    }
    

    if (parameters.size()>0 && parameters.nelems() == predictor->parameters.nelems())
      predictor->parameters.copyValuesFrom(parameters);
    parameters.resize(predictor->parameters.size());
    for (int i=0;i<parameters.size();i++)
      parameters[i] = predictor->parameters[i];

    Var target_index = Var(1,1);
    target_index->setName("target_index");
    Var neighbor_indexes = Var(n_neighbors,1);
    neighbor_indexes->setName("neighbor_indexes");
    p_x = Var(train_set->length(),1);
    p_x->setName("p_x");
    p_target = new VarRowsVariable(p_x,target_index);
    p_target->setName("p_target");
    p_neighbors =new VarRowsVariable(p_x,neighbor_indexes);
    p_neighbors->setName("p_neighbors");

    tangent_targets = Var(n_neighbors,n);
    if(include_current_point)
    {
      Var temp = new SourceVariable(1,n);
      temp->value.fill(0);
      tangent_targets_and_point = vconcat(temp & tangent_targets);
      p_neighbors_and_point = vconcat(p_target & p_neighbors);
    }
    else
    {
      tangent_targets_and_point = tangent_targets;
      p_neighbors_and_point = p_neighbors;
    }
    
    if(mu_n_neighbors < 0 ) mu_n_neighbors = n_neighbors;

    // compute - log ( sum_{neighbors of x} P(neighbor|x) ) according to semi-spherical model
    Var nll = nll_semispherical_gaussian(tangent_plane, mu, sm, sn, tangent_targets_and_point, p_target, p_neighbors_and_point, noise_var, mu_noisy,
                                         use_noise, svd_threshold, min_p_x, mu_n_neighbors); // + log_n_examples;
    //nll_f = Func(tangent_plane & mu & sm & sn & tangent_targets, nll);
    Var knn = new SourceVariable(1,1);
    knn->setName("knn");
    knn->value[0] = n_neighbors + (include_current_point ? 1 : 0);

    if(weight_mu_and_tangent != 0)
    {
      sum_nll = new ColumnSumVariable(nll) / knn + weight_mu_and_tangent * ((Var) new RowSumVariable(square(product(no_bprop(tangent_plane),mu_noisy))));
    }
    else
      sum_nll = new ColumnSumVariable(nll) / knn;

    cost_of_one_example = Func(x & tangent_targets & target_index & neighbor_indexes, predictor->parameters, sum_nll);
    noisy_data = Func(x,x + noise_var);    // Func to verify what's the noisy data like (doesn't work so far, this problem will be investigated)
    //verify_gradient_func = Func(predictor->inputs & tangent_targets & target_index & neighbor_indexes, predictor->parameters & mu_noisy, sum_nll);  

    if(n_neighbors_density > train_set.length() || n_neighbors_density < 0) n_neighbors_density = train_set.length();

    best_validation_cost = REAL_MAX;

    train_nearest_neighbors.resize(train_set.length(), n_neighbors_density-1);
    validation_nearest_neighbors.resize(valid_set.length(), n_neighbors_density);

    t_row.resize(n);
    Ut_svd.resize(n,n);
    V_svd.resize(n_dim,n_dim);
    z.resize(n);
    zm.resize(n);
    zn.resize(n);
    x_minus_neighbor.resize(n);
    neighbor_row.resize(n);
    w.resize(n_dim);

    Bs.resize(train_set.length());
    Fs.resize(train_set.length());
    mus.resize(train_set.length(), n);
    sms.resize(train_set.length());
    sns.resize(train_set.length());
    
    reference_set = train_set;
  }

}

void GaussianContinuum::update_reference_set_parameters()
{
    // Compute Fs, Bs, mus, sms, sns
  Bs.resize(reference_set.length());
  Fs.resize(reference_set.length());
  mus.resize(reference_set.length(), n);
  sms.resize(reference_set.length());
  sns.resize(reference_set.length());
  
  for(int t=0; t<reference_set.length(); t++)
  {
    Fs[t].resize(tangent_plane.length(), tangent_plane.width());
    reference_set->getRow(t,t_row);
    predictor->fprop(t_row, Fs[t].toVec() & mus(t) & sms.subVec(t,1) & sns.subVec(t,1));
    
    // computing B

    static Mat F_copy;
    F_copy.resize(Fs[t].length(),Fs[t].width());
    F_copy << Fs[t];
    // N.B. this is the SVD of F'
    lapackSVD(F_copy, Ut_svd, S_svd, V_svd);
    Bs[t].resize(n_dim,reference_set.width());
    Bs[t].clear();
    for (int k=0;k<S_svd.length();k++)
    {
      real s_k = S_svd[k];
      if (s_k>svd_threshold) // ignore the components that have too small singular value (more robust solution)
      { 
        real coef = 1/s_k;
        for (int i=0;i<n_dim;i++)
        {
          real* Bi = Bs[t][i];
          for (int j=0;j<n;j++)
            Bi[j] += V_svd(i,k)*Ut_svd(k,j)*coef;
        }
      }
    }
    
  }

}

void GaussianContinuum::knn(const VMat& vm, const Vec& x, const int& k, TVec<int>& neighbors, bool sortk) const
{
  int n = vm->length();
  distances.resize(n,2);
  distances.column(1) << Vec(0, n-1, 1); 
  dk.setDataForKernelMatrix(vm);
  t_dist.resize(n);
  dk.evaluate_all_i_x(x, t_dist);
  distances.column(0) << t_dist;
  partialSortRows(distances, k, sortk);
  neighbors.resize(k);
  for (int i = 0; i < k; i++)
    neighbors[i] = int(distances(i,1));
}

void GaussianContinuum::make_random_walk()
{
  if(n_random_walk_step < 1) PLERROR("Number of step in random walk should be at least one");
  if(n_random_walk_per_point < 1) PLERROR("Number of random walk per training set point should be at least one");
  ith_step_generated_set.resize(n_random_walk_step);

  Mat generated_set(train_set.length()*n_random_walk_per_point,n);
  for(int t=0; t<train_set.length(); t++)
  {
    train_set->getRow(t,t_row);
    output_f_all(t_row);
      
    real this_sm = sm->value[0];
    real this_sn = sn->value[0];
    Vec this_mu(n); this_mu << mu->value;
    static Mat this_F(n_dim,n); this_F << tangent_plane->matValue;
      
    // N.B. this is the SVD of F'
    lapackSVD(this_F, Ut_svd, S_svd, V_svd);
      

    for(int rwp=0; rwp<n_random_walk_per_point; rwp++)
    {
      TVec<real> z_m(n_dim);
      TVec<real> z(n);
      for(int i=0; i<n_dim; i++)
        z_m[i] = normal_sample();
      for(int i=0; i<n; i++)
        z[i] = normal_sample();

      Vec new_point = generated_set(t*n_random_walk_per_point+rwp);
      for(int j=0; j<n; j++)
      {
        new_point[j] = 0;         
        for(int k=0; k<n_dim; k++)
          new_point[j] += Ut_svd(k,j)*z_m[k];
        new_point[j] *= sqrt(this_sm-this_sn);
        if(walk_on_noise)
          new_point[j] += z[j]*sqrt(this_sn);
      }
      new_point *= random_walk_step_prop;
      new_point += this_mu + t_row;
    }
  }

  // Test of generation of random points
  /*
  int n_test_gen_points = 3;
  int n_test_gen_generated = 30;

  Mat test_gen(n_test_gen_points*n_test_gen_generated,n);
  for(int p=0; p<n_test_gen_points; p++)
  {
    for(int t=0; t<n_test_gen_generated; t++)             
    {
      valid_set->getRow(p,t_row);
      output_f_all(t_row);
      
      real this_sm = sm->value[0];
      real this_sn = sn->value[0];
      Vec this_mu(n); this_mu << mu->value;
      static Mat this_F(n_dim,n); this_F << tangent_plane->matValue;
      
      // N.B. this is the SVD of F'
      lapackSVD(this_F, Ut_svd, S_svd, V_svd);      

      TVec<real> z_m(n_dim);
      TVec<real> z(n);
      for(int i=0; i<n_dim; i++)
        z_m[i] = normal_sample();
      for(int i=0; i<n; i++)
        z[i] = normal_sample();

      Vec new_point = test_gen(p*n_test_gen_generated+t);
      for(int j=0; j<n; j++)
      {
        new_point[j] = 0;         
        for(int k=0; k<n_dim; k++)
          new_point[j] += Ut_svd(k,j)*z_m[k];
        new_point[j] *= sqrt(this_sm-this_sn);
        if(walk_on_noise)
          new_point[j] += z[j]*sqrt(this_sn);
      }
      new_point += this_mu + t_row;
    }
  }
  
  PLearn::save("test_gen.psave",test_gen);
  */
  //PLearn::save("gen_points_0.psave",generated_set);
  ith_step_generated_set[0] = VMat(generated_set);
  
  for(int step=1; step<n_random_walk_step; step++)
  {
    Mat generated_set(ith_step_generated_set[step-1].length(),n);
    for(int t=0; t<ith_step_generated_set[step-1].length(); t++)
    {
      ith_step_generated_set[step-1]->getRow(t,t_row);
      output_f_all(t_row);
      
      real this_sm = sm->value[0];
      real this_sn = sn->value[0];
      Vec this_mu(n); this_mu << mu->value;
      static Mat this_F(n_dim,n); this_F << tangent_plane->matValue;
      
      // N.B. this is the SVD of F'
      lapackSVD(this_F, Ut_svd, S_svd, V_svd);
      
      TVec<real> z_m(n_dim);
      TVec<real> z(n);
      for(int i=0; i<n_dim; i++)
        z_m[i] = normal_sample();
      for(int i=0; i<n; i++)
        z[i] = normal_sample();
      
      Vec new_point = generated_set(t);
      for(int j=0; j<n; j++)
      {
        new_point[j] = 0;
        for(int k=0; k<n_dim; k++)
          if(S_svd[k] > svd_threshold)
            new_point[j] += Ut_svd(k,j)*z_m[k];
        new_point[j] *= sqrt(this_sm-this_sn);
        if(walk_on_noise)
          new_point[j] += z[j]*sqrt(this_sn);
      }
      new_point *= random_walk_step_prop;
      new_point += this_mu + t_row;
    
    }
    /*
    string path = " ";
    if(step == n_random_walk_step-1)
      path = "gen_points_last.psave";
    else
      path = "gen_points_" + tostring(step) + ".psave";
    
    PLearn::save(path,generated_set);
    */
    ith_step_generated_set[step] = VMat(generated_set);
  }

  reference_set = vconcat(train_set & ith_step_generated_set);

  // Single random walk
  /*
  Mat single_walk_set(100,n);
  train_set->getRow(train_set.length()-1,single_walk_set(0));
  for(int step=1; step<100; step++)
  {
    t_row << single_walk_set(step-1);
    output_f_all(t_row);
      
    real this_sm = sm->value[0];
    real this_sn = sn->value[0];
    Vec this_mu(n); this_mu << mu->value;
    static Mat this_F(n_dim,n); this_F << tangent_plane->matValue;
    
    // N.B. this is the SVD of F'
    lapackSVD(this_F, Ut_svd, S_svd, V_svd);
    
    TVec<real> z_m(n_dim);
    TVec<real> z(n);
    for(int i=0; i<n_dim; i++)
      z_m[i] = normal_sample();
    for(int i=0; i<n; i++)
      z[i] = normal_sample();
    
    Vec new_point = single_walk_set(step);
    for(int j=0; j<n; j++)
    {
      new_point[j] = 0;
      for(int k=0; k<n_dim; k++)
        if(S_svd[k] > svd_threshold)
          new_point[j] += Ut_svd(k,j)*z_m[k];
      new_point[j] *= sqrt(this_sm-this_sn);
      if(walk_on_noise)
        new_point[j] += z[j]*sqrt(this_sn);
    }
    new_point *= random_walk_step_prop;
    new_point += this_mu + t_row;
  }
  PLearn::save("image_single_rw.psave",single_walk_set);
  */
}


real GaussianContinuum::get_nll(VMat points, VMat image_points_vmat, int begin, int n_near_neigh)
{
  VMat reference_set = new SubVMatrix(points,begin,0,points.length()-begin,n);
  //Mat image(points_per_dim,points_per_dim); image.clear();
  image_nearest_neighbors.resize(image_points_vmat.length(),n_near_neigh);
  // Finding nearest neighbors

  for(int t=0; t<image_points_vmat.length(); t++)
  {
    image_points_vmat->getRow(t,t_row);
    TVec<int> nn = image_nearest_neighbors(t);
    computeNearestNeighbors(reference_set, t_row, nn);
  }

  real nll = 0;

  for(int t=0; t<image_points_vmat.length(); t++)
  {
    
    image_points_vmat->getRow(t,t_row);
    real this_p_x = 0;
    // fetching nearest neighbors for density estimation
    for(int neighbor=0; neighbor<n_near_neigh; neighbor++)
    {
      points->getRow(begin+image_nearest_neighbors(t,neighbor), neighbor_row);
      substract(t_row,neighbor_row,x_minus_neighbor);
      substract(x_minus_neighbor,mus(begin+image_nearest_neighbors(t,neighbor)),z);
      product(w, Bs[begin+image_nearest_neighbors(t,neighbor)], z);
      transposeProduct(zm, Fs[begin+image_nearest_neighbors(t,neighbor)], w);
      substract(z,zm,zn);
      this_p_x += exp(-0.5*(pownorm(zm,2)/sms[begin+image_nearest_neighbors(t,neighbor)] + pownorm(zn,2)/sns[begin+image_nearest_neighbors(t,neighbor)] 
                            + n_dim*log(sms[begin+image_nearest_neighbors(t,neighbor)]) + (n-n_dim)*log(sns[begin+image_nearest_neighbors(t,neighbor)])) - n/2.0 * Log2Pi);
    }
    
    this_p_x /= reference_set.length();
    nll -= log(this_p_x);
  }

  return nll/image_points_vmat.length();
}

void GaussianContinuum::get_image_matrix(VMat points, VMat image_points_vmat, int begin, string file_path, int n_near_neigh)
{
  VMat reference_set = new SubVMatrix(points,begin,0,points.length()-begin,n);
  cout << "Creating image matrix: " << file_path << endl;
  Mat image(points_per_dim,points_per_dim); image.clear();
  image_nearest_neighbors.resize(points_per_dim*points_per_dim,n_near_neigh);
  // Finding nearest neighbors

  for(int t=0; t<image_points_vmat.length(); t++)
  {
    image_points_vmat->getRow(t,t_row);
    TVec<int> nn = image_nearest_neighbors(t);
    computeNearestNeighbors(reference_set, t_row, nn);
  }

  for(int t=0; t<image_points_vmat.length(); t++)
  {
    
    image_points_vmat->getRow(t,t_row);
    real this_p_x = 0;
    // fetching nearest neighbors for density estimation
    for(int neighbor=0; neighbor<n_near_neigh; neighbor++)
    {
      points->getRow(begin+image_nearest_neighbors(t,neighbor), neighbor_row);
      substract(t_row,neighbor_row,x_minus_neighbor);
      substract(x_minus_neighbor,mus(begin+image_nearest_neighbors(t,neighbor)),z);
      product(w, Bs[begin+image_nearest_neighbors(t,neighbor)], z);
      transposeProduct(zm, Fs[begin+image_nearest_neighbors(t,neighbor)], w);
      substract(z,zm,zn);
      this_p_x += exp(-0.5*(pownorm(zm,2)/sms[begin+image_nearest_neighbors(t,neighbor)] + pownorm(zn,2)/sns[begin+image_nearest_neighbors(t,neighbor)] 
                            + n_dim*log(sms[begin+image_nearest_neighbors(t,neighbor)]) + (n-n_dim)*log(sns[begin+image_nearest_neighbors(t,neighbor)])) - n/2.0 * Log2Pi);
    }
    
    this_p_x /= reference_set.length();
    int y_coord = t/points_per_dim;
    int x_coord = t%points_per_dim;
    image(points_per_dim - y_coord - 1,x_coord) = this_p_x;
  }
  PLearn::save(file_path,image);
  
}



void GaussianContinuum::compute_train_and_validation_costs()
{
  update_reference_set_parameters();

  // estimate p(x) for the training set

  real nll_train = 0;

  for(int t=0; t<train_set.length(); t++)
  {

    train_set->getRow(t,t_row);
    p_x->value[t] = 0;
    // fetching nearest neighbors for density estimation
    for(int neighbor=0; neighbor<train_nearest_neighbors.width(); neighbor++)
    {
      train_set->getRow(train_nearest_neighbors(t,neighbor),neighbor_row);
      substract(t_row,neighbor_row,x_minus_neighbor);
      substract(x_minus_neighbor,mus(train_nearest_neighbors(t,neighbor)),z);
      product(w, Bs[train_nearest_neighbors(t,neighbor)], z);
      transposeProduct(zm, Fs[train_nearest_neighbors(t,neighbor)], w);
      substract(z,zm,zn);
      p_x->value[t] += exp(-0.5*(pownorm(zm,2)/sms[train_nearest_neighbors(t,neighbor)] + pownorm(zn,2)/sns[train_nearest_neighbors(t,neighbor)] 
                         + n_dim*log(sms[train_nearest_neighbors(t,neighbor)]) + (n-n_dim)*log(sns[train_nearest_neighbors(t,neighbor)])) - n/2.0 * Log2Pi);
    }
    p_x->value[t] /= train_set.length();
    nll_train -= log(p_x->value[t]);

    if(print_parameters)
    {
      output_f_all(t_row);
      cout << "data point = " << x->value << " parameters = " << tangent_plane->value << " " << mu->value << " " << sm->value << " " << sn->value << " p(x) = " << p_x->value[t] << endl;
    }
  }

  nll_train /= train_set.length();

  if(verbosity > 2) cout << "NLL train = " << nll_train << endl;

  // estimate p(x) for the validation set

  real nll_validation = 0;

  for(int t=0; t<valid_set.length(); t++)
  {

    valid_set->getRow(t,t_row);
    real this_p_x = 0;
    // fetching nearest neighbors for density estimation
    for(int neighbor=0; neighbor<n_neighbors_density; neighbor++)
    {
      train_set->getRow(validation_nearest_neighbors(t,neighbor), neighbor_row);
      substract(t_row,neighbor_row,x_minus_neighbor);
      substract(x_minus_neighbor,mus(validation_nearest_neighbors(t,neighbor)),z);
      product(w, Bs[validation_nearest_neighbors(t,neighbor)], z);
      transposeProduct(zm, Fs[validation_nearest_neighbors(t,neighbor)], w);
      substract(z,zm,zn);
      this_p_x += exp(-0.5*(pownorm(zm,2)/sms[validation_nearest_neighbors(t,neighbor)] + pownorm(zn,2)/sns[validation_nearest_neighbors(t,neighbor)] 
                         + n_dim*log(sms[validation_nearest_neighbors(t,neighbor)]) + (n-n_dim)*log(sns[validation_nearest_neighbors(t,neighbor)])) - n/2.0 * Log2Pi);
    }

    this_p_x /= train_set.length();  // When points will be added using a random walk, this will need to be changed (among other things...)
    nll_validation -= log(this_p_x);
  }

  nll_validation /= valid_set.length();

  if(verbosity > 2) cout << "NLL validation = " << nll_validation << endl;

}

// ### Nothing to add here, simply calls build_
void GaussianContinuum::build()
{
  inherited::build();
  build_();
}

extern void varDeepCopyField(Var& field, CopiesMap& copies);

void GaussianContinuum::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{  inherited::makeDeepCopyFromShallowCopy(copies);

  deepCopyField(cost_of_one_example, copies);
  deepCopyField(reference_set,copies);
  varDeepCopyField(x, copies);
  varDeepCopyField(noise_var, copies);  
  varDeepCopyField(b, copies);
  varDeepCopyField(W, copies);
  varDeepCopyField(c, copies);
  varDeepCopyField(V, copies);
  varDeepCopyField(tangent_targets, copies);
  varDeepCopyField(muV, copies);
  varDeepCopyField(smV, copies);
  varDeepCopyField(smb, copies);
  varDeepCopyField(snV, copies);
  varDeepCopyField(snb, copies);
  varDeepCopyField(mu, copies);
  varDeepCopyField(sm, copies);
  varDeepCopyField(sn, copies);
  varDeepCopyField(mu_noisy, copies);
  varDeepCopyField(tangent_plane, copies);
  varDeepCopyField(tangent_targets_and_point, copies);
  varDeepCopyField(sum_nll, copies);
  varDeepCopyField(min_sig, copies);
  varDeepCopyField(min_d, copies);
  varDeepCopyField(embedding, copies);

  deepCopyField(dist, copies);
  deepCopyField(ith_step_generated_set, copies);
  deepCopyField(train_nearest_neighbors, copies);
  deepCopyField(validation_nearest_neighbors, copies);
  deepCopyField(Bs, copies);
  deepCopyField(Fs, copies);
  deepCopyField(mus, copies);
  deepCopyField(sms, copies);
  deepCopyField(sns, copies);
  deepCopyField(Ut_svd, copies);
  deepCopyField(V_svd, copies);
  deepCopyField(S_svd, copies);
  deepCopyField(dk, copies);

  deepCopyField(parameters, copies);
  deepCopyField(optimizer, copies);
  deepCopyField(predictor, copies);
  deepCopyField(output_f, copies);
  deepCopyField(output_f_all, copies);
  deepCopyField(projection_error_f, copies);
  deepCopyField(noisy_data, copies);
}


int GaussianContinuum::outputsize() const
{
  return 1;
  /*
  if(output_type == "tangent_plane_variance_normalized")
    return output_f->outputsize-1;
  else
    return output_f->outputsize;
  */
}

void GaussianContinuum::forget()
{
  if (train_set) initializeParams();
  stage = 0;
}
    
void GaussianContinuum::train()
{

  // Creation of points for matlab image matrices

  if(save_image_mat)
  {
    if(n != 2) PLERROR("In GaussianContinuum::train(): Image matrix creation is only implemented for 2d problems");
    
    real step_x = (upper_x-lower_x)/(points_per_dim-1);
    real step_y = (upper_y-lower_y)/(points_per_dim-1);
    image_points_mat.resize(points_per_dim*points_per_dim,n);
    for(int i=0; i<points_per_dim; i++)
      for(int j=0; j<points_per_dim; j++)
      {
        image_points_mat(i*points_per_dim + j,0) = lower_x + j*step_x;
        image_points_mat(i*points_per_dim + j,1) = lower_y + i*step_y;
      }

    image_points_vmat = VMat(image_points_mat);
  }

  // find nearest neighbors...

  // ... on the training set
  
  for(int t=0; t<train_set.length(); t++)
  {
    train_set->getRow(t,t_row);
    TVec<int> nn = train_nearest_neighbors(t);
    computeNearestNeighbors(train_set, t_row, nn, t);
  }
  
  // ... on the validation set
  
  for(int t=0; t<valid_set.length(); t++)
  {
    valid_set->getRow(t,t_row);
    TVec<int> nn = validation_nearest_neighbors(t);
    computeNearestNeighbors(train_set, t_row, nn);
  }

  VMat train_set_with_targets;
  VMat targets_vmat;
  if (!cost_of_one_example)
    PLERROR("GaussianContinuum::train: build has not been run after setTrainingSet!");

  targets_vmat = local_neighbors_differences(train_set, n_neighbors, false, true);

  train_set_with_targets = hconcat(train_set, targets_vmat);
  train_set_with_targets->defineSizes(inputsize()+inputsize()*n_neighbors+1+n_neighbors,0);
  int l = train_set->length();  
  //log_n_examples->value[0] = log(real(l));
  int nsamples = batch_size>0 ? batch_size : l;

  Var totalcost = meanOf(train_set_with_targets, cost_of_one_example, nsamples);

  if(optimizer)
    {
      optimizer->setToOptimize(parameters, totalcost);  
      optimizer->build();
    }
  else PLERROR("GaussianContinuum::train can't train without setting an optimizer first!");
  
  // number of optimizer stages corresponding to one learner stage (one epoch)
  int optstage_per_lstage = l/nsamples;

  PP<ProgressBar> pb;
  if(report_progress>0)
    pb = new ProgressBar("Training GaussianContinuum from stage " + tostring(stage) + " to " + tostring(nstages), nstages-stage);

  t_row.resize(train_set.width());

  int initial_stage = stage;
  bool early_stop=false;
  while(stage<nstages && !early_stop)
    {
      optimizer->nstages = optstage_per_lstage;
      train_stats->forget();
      optimizer->early_stop = false;
      optimizer->optimizeN(*train_stats);
      train_stats->finalize();
      if(verbosity>2)
        cout << "Epoch " << stage << " train objective: " << train_stats->getMean() << endl;
      ++stage;
      if(pb)
        pb->update(stage-initial_stage);
      
      if(stage != 0 && stage%compute_cost_every_n_epochs == 0)
      {
        compute_train_and_validation_costs();
      }
    }
  if(verbosity>1)
    cout << "EPOCH " << stage << " train objective: " << train_stats->getMean() << endl;

  update_reference_set_parameters();

  cout << "best train: " << get_nll(train_set,train_set,0,n_neighbors_density) << endl;
  cout << "best validation: " << get_nll(train_set,valid_set,0,n_neighbors_density) << endl;

  // test computeOutput and Costs

  real nll_train = 0;
  Vec costs(1);
  Vec target;
  for(int i=0; i<train_set.length(); i++)
  {
    train_set->getRow(i,t_row);
    computeCostsOnly(t_row,target,costs);
    nll_train += costs[0];
  }
  nll_train /= train_set.length();
  cout << "nll_train: " << nll_train << endl;
  
  /*
  int n_test_gen_points = 3;
  int n_test_gen_generated = 30;
  Mat noisy_data_set(n_test_gen_points*n_test_gen_generated,n);
  
  for(int k=0; k<n_test_gen_points; k++)
  {
    for(int t=0; t<n_test_gen_generated; t++)
    {
      valid_set->getRow(k,t_row);
      Vec noisy_point = noisy_data_set(k*n_test_gen_generated+t);
      noisy_point << noisy_data(t_row);
    }
    PLearn::save("noisy_data.psave",noisy_data_set);
  }
  */
  
  if(n==2 && save_image_mat)
  {
    Mat test_set(valid_set.length(),valid_set.width());
    Mat m_dir(valid_set.length(),n);
    Mat n_dir(valid_set.length(),n);
    for(int t=0; t<valid_set.length(); t++)
    {
      valid_set->getRow(t,t_row);
      test_set(t) << t_row;
      output_f_all(t_row);
      Vec noise_direction = n_dir(t);
      noise_direction[0] = tangent_plane->value[1];
      noise_direction[1] = -1*tangent_plane->value[0];
      Vec manifold_direction = m_dir(t);
      manifold_direction << tangent_plane->value;
      noise_direction *= sqrt(sn->value[0])/norm(noise_direction,2);
      manifold_direction *= sqrt(sm->value[0])/norm(manifold_direction,2);
    }
    PLearn::save("test_set.psave",test_set);
    PLearn::save("m_dir.psave",m_dir);
    PLearn::save("n_dir.psave",n_dir);
  }
  

  if(n_random_walk_step > 0)
  {
    make_random_walk();
    update_reference_set_parameters();
  }
  
  if(save_image_mat)
  {
    cout << "Creating image matrix" << endl;
    get_image_matrix(train_set, image_points_vmat, 0,"image.psave", n_neighbors_density);

    image_prob_mat.resize(points_per_dim,points_per_dim);
    Mat image_points(points_per_dim*points_per_dim,2);
    Mat image_mu_vectors(points_per_dim*points_per_dim,2);
    //Mat image_sigma_vectors(points_per_dim*points_per_dim,2);
    for(int t=0; t<image_points_vmat.length(); t++)
    {
      image_points_vmat->getRow(t,t_row);
     
      output_f_all(t_row);

      image_points(t,0) = t_row[0];
      image_points(t,1) = t_row[1];
      
      image_mu_vectors(t) << mu->value;
    }
    PLearn::save("image_points.psave",image_points);
    PLearn::save("image_mu_vectors.psave",image_mu_vectors);

    if(n_random_walk_step > 0)
    {
      string path = "image_rw_" + tostring(0) + ".psave";

      get_image_matrix(reference_set, image_points_vmat, 0, path, n_neighbors_density*n_random_walk_per_point);
      
      for(int i=0; i<n_random_walk_step; i++)
      {
        if(i == n_random_walk_step - 1)
          path = "image_rw_last.psave";
        else
          path = "image_rw_" + tostring(i+1) + ".psave";

        get_image_matrix(reference_set, image_points_vmat, i*train_set.length()*n_random_walk_per_point+train_set.length(),path,n_neighbors_density*n_random_walk_per_point);
      }

      cout << "NLL random walk on train: " << get_nll(reference_set,train_set,(n_random_walk_step-1)*train_set.length()*n_random_walk_per_point+train_set.length(),n_neighbors_density*n_random_walk_per_point) << endl;
      cout << "NLL random walk on validation: " << get_nll(reference_set,valid_set,(n_random_walk_step-1)*train_set.length()*n_random_walk_per_point+train_set.length(),n_neighbors_density*n_random_walk_per_point) << endl;
    }
  }

}

void GaussianContinuum::initializeParams()
{
  if (seed_>=0)
    manual_seed(seed_);
  else
    PLearn::seed();

  if (architecture_type=="embedding_neural_network")
  {
    real delta = 1.0 / sqrt(real(inputsize()));
    fill_random_uniform(V->value, -delta, delta);
    delta = 1.0 / real(n_hidden_units);
    fill_random_uniform(W->matValue, -delta, delta);
    c->value.clear();
    fill_random_uniform(smV->matValue, -delta, delta);
    smb->value.clear();
    fill_random_uniform(smV->matValue, -delta, delta);
    snb->value.clear();
    fill_random_uniform(snV->matValue, -delta, delta);
    fill_random_uniform(muV->matValue, -delta, delta);
  }
  else if (architecture_type=="single_neural_network")
  {
    real delta = 1.0 / sqrt(real(inputsize()));
    fill_random_uniform(V->value, -delta, delta);
    delta = 1.0 / real(n_hidden_units);
    fill_random_uniform(W->matValue, -delta, delta);
    c->value.clear();
    fill_random_uniform(smV->matValue, -delta, delta);
    smb->value.clear();
    fill_random_uniform(smV->matValue, -delta, delta);
    snb->value.clear();
    fill_random_uniform(snV->matValue, -delta, delta);
    fill_random_uniform(muV->matValue, -delta, delta);
    b->value.clear();
  }
  else PLERROR("other types not handled yet!");
  
  for(int i=0; i<p_x.length(); i++)
    p_x->value[i] = 1.0/p_x.length();

  if(optimizer)
    optimizer->reset();
}


void GaussianContinuum::computeOutput(const Vec& input, Vec& output) const
{
  // compute density
  real ret = 0;

  // fetching nearest neighbors for density estimation
  knn(reference_set,input,n_neighbors_density,t_nn,bool(0));
  t_row << input;
  for(int neighbor=0; neighbor<t_nn.length(); neighbor++)
  {
    reference_set->getRow(t_nn[neighbor],neighbor_row);
    substract(t_row,neighbor_row,x_minus_neighbor);
    substract(x_minus_neighbor,mus(t_nn[neighbor]),z);
    product(w, Bs[t_nn[neighbor]], z);
    transposeProduct(zm, Fs[t_nn[neighbor]], w);
    substract(z,zm,zn);
    ret += exp(-0.5*(pownorm(zm,2)/sms[t_nn[neighbor]] + pownorm(zn,2)/sns[t_nn[neighbor]] 
                               + n_dim*log(sms[t_nn[neighbor]]) + (n-n_dim)*log(sns[t_nn[neighbor]])) - n/2.0 * Log2Pi);
  }
  ret /= reference_set.length();
  output[0] = ret;
  /*
  if(output_type == "tangent_plane_variance_normalized")
  {
    int nout = outputsize()+1;
    Vec temp_output(nout);
    temp_output << output_f(input);
    Mat F = temp_output.subVec(0,temp_output.length()-1).toMat(n_dim,n);
    if(n_dim*n != temp_output.length()-1) PLERROR("WHAT!!!");
    for(int i=0; i<F.length(); i++)
    {
      real norm = pownorm(F(i),1);
      F(i) *= sqrt(temp_output[temp_output.length()-1])/norm;
    }
    
    output.resize(temp_output.length()-1);
    output << temp_output.subVec(0,temp_output.length()-1);
  }
  else
  {
    int nout = outputsize();
    output.resize(nout);
    output << output_f(input);
  }
  */
}    

void GaussianContinuum::computeCostsFromOutputs(const Vec& input, const Vec& output, 
					     const Vec& target, Vec& costs) const
{
  costs[0] = -log(output[0]);
}                                

TVec<string> GaussianContinuum::getTestCostNames() const
{
  return getTrainCostNames();
}

TVec<string> GaussianContinuum::getTrainCostNames() const
{
  TVec<string> cost(1); cost[0] = "NLL";
  return cost;
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
