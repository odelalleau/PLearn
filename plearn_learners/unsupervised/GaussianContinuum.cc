// -*- C++ -*-

// GaussianContinuum.cc
//
// Copyright (C) 2004 Yoshua Bengio & Martin Monperrus 
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
   * $Id: GaussianContinuum.cc,v 1.2 2004/07/21 16:30:58 chrish42 Exp $
   ******************************************************* */

// Authors: Yoshua Bengio & Martin Monperrus

/*! \file GaussianContinuum.cc */


#include "GaussianContinuum.h"
#include <plearn/vmat/LocalNeighborsDifferencesVMatrix.h>
#include <plearn/var/ProductVariable.h>
#include <plearn/var/PlusVariable.h>
#include <plearn/var/Var_operators.h>
#include <plearn/vmat/ConcatColumnsVMatrix.h>
#include <plearn/math/random.h>
#include <plearn/var/SumOfVariable.h>
#include <plearn/var/TanhVariable.h>
#include <plearn/var/DiagonalizedFactorsProductVariable.h>
#include <plearn/math/random.h>
#include <plearn/math/plapack.h>

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
  : n_neighbors(5), n_dim(1), architecture_type("single_neural_network"), output_type("tangent_plane"),
    n_hidden_units(-1), batch_size(1), norm_penalization(0), svd_threshold(1e-5), 
    projection_error_regularization(0)
    
{
}

PLEARN_IMPLEMENT_OBJECT(GaussianContinuum, 
                        "Learn a continuous (uncountable) Gaussian mixture with non-local parametrization"
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

    dNLL/dsigma2_manifold = p(x|y) (0.5 d/sigma2_manifold - ||z_m||^2/sigma2_manifold^2)

  N.B. this is the gradient on the variance, not on the standard deviation.

  Proof: Recall eq.(1) and let theta = dsigma2_manifold. Using eq.(2) we obtain
  for the first term in (1): 
    d/dsigma2_manifold (0.5/sigma2_manifold ||z_m||^2) = -||z_m||^2/sigma2_manifold^2.
  Using (3) we obtain the second term 
    d/dsigma2_manifold (0.5 d log(sigma2_manifold)) = 0.5 d/sigma2_manifold.

  The same arguments yield the following for the gradient on sigma2_noise:

    dNLL/dsigma2_noise = p(x|y) (0.5 (n-d)/sigma2_noise - ||z_n||^2/sigma2_noise^2)


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

  declareOption(ol, "n_neighbors", &GaussianContinuum::n_neighbors, OptionBase::buildoption,
		"Number of nearest neighbors to consider.\n"
		);

  declareOption(ol, "n_dim", &GaussianContinuum::n_dim, OptionBase::buildoption,
		"Number of tangent vectors to predict.\n"
		);

  declareOption(ol, "optimizer", &GaussianContinuum::optimizer, OptionBase::buildoption,
		"Optimizer that optimizes the cost function Number of tangent vectors to predict.\n"
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
		"   multi_neural_network : prediction[j] = b[j] + W[j]*tanh(c[j] + V[j]*x), where W[j] has n_hidden_units columns\n"
		"                          where there is a separate set of parameters for each of n_dim tangent vectors to predict.\n"
		"   single_neural_network : prediction = b + W*tanh(c + V*x), where W has n_hidden_units columns\n"
		"                          where the resulting vector is viewed as a n_dim by n matrix\n"
		"   linear :         prediction = b + W*x\n"
    "   embedding_neural_network: prediction[k,i] = d(e[k]/d(x[i), where e(x) is an ordinary neural\n"
    "                             network representing the embedding function (see output_type option)\n"
    "   embedding_quadratic: prediction[k,i] = d(e_k/d(x_i) = A_k x + b_k, where e_k(x) is a quadratic\n"
    "                        form in x, i.e. e_k = x' A_k x + b_k' x\n"
		"   (empty string):  specify explicitly the function with tangent_predictor option\n"
		"where (b,W,c,V) are parameters to be optimized.\n"
		);

  declareOption(ol, "n_hidden_units", &GaussianContinuum::n_hidden_units, OptionBase::buildoption,
		"Number of hidden units (if architecture_type is some kidn of neural network)\n"
		);

  declareOption(ol, "output_type", &GaussianContinuum::output_type, OptionBase::buildoption,
		"Default value (the only one considered if architecture_type != embedding_*) is\n"
    "   tangent_plane: output the predicted tangent plane.\n"
    "   embedding: output the embedding vector (only if architecture_type == embedding_*).\n"
    "   tangent_plane+embedding: output both (in this order).\n"
		);

 
  declareOption(ol, "batch_size", &GaussianContinuum::batch_size, OptionBase::buildoption, 
                "    how many samples to use to estimate the average gradient before updating the weights\n"
                "    0 is equivalent to specifying training_set->length() \n");

  declareOption(ol, "norm_penalization", &GaussianContinuum::norm_penalization, OptionBase::buildoption,
		"Factor that multiplies an extra penalization of the norm of f_i so that ||f_i|| be close to 1.\n"
    "The penalty is norm_penalization*sum_i (1 - ||f_i||^2)^2.\n"                
		);

  declareOption(ol, "svd_threshold", &GaussianContinuum::svd_threshold, OptionBase::buildoption,
		"Threshold to accept singular values of F in solving for linear combination weights on tangent subspace.\n"
		);

  declareOption(ol, "projection_error_regularization", &GaussianContinuum::projection_error_regularization, OptionBase::buildoption,
		"Term added to the linear system matrix involved in fitting subspaces in the projection error computation.\n"
		);

  declareOption(ol, "parameters", &GaussianContinuum::parameters, OptionBase::learntoption,
		"Parameters of the tangent_predictor function.\n"
		);

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void GaussianContinuum::build_()
{

  int n = PLearner::inputsize_;
  
  if (n>0)
  {
    if (architecture_type == "multi_neural_network")
      {
        if (n_hidden_units <= 0)
          PLERROR("GaussianContinuum::Number of hidden units should be positive, now %d\n",n_hidden_units);
      }
    if (architecture_type == "single_neural_network")
      {
        if (n_hidden_units <= 0)
          PLERROR("GaussianContinuum::Number of hidden units should be positive, now %d\n",n_hidden_units);
        Var x(n);
        b = Var(n_dim*n,1,"b");
        W = Var(n_dim*n,n_hidden_units,"W");
        c = Var(n_hidden_units,1,"c");
        V = Var(n_hidden_units,n,"V");
        tangent_predictor = Func(x, b & W & c & V, b + product(W,tanh(c + product(V,x))));
      }
    else if (architecture_type == "linear")
      {
        Var x(n);
        b = Var(n_dim*n,1,"b");
        W = Var(n_dim*n,n,"W");
        tangent_predictor = Func(x, b & W, b + product(W,x));
      }
    else if (architecture_type == "embedding_neural_network")
      {
        if (n_hidden_units <= 0)
          PLERROR("GaussianContinuum::Number of hidden units should be positive, now %d\n",n_hidden_units);
        Var x(n);
        W = Var(n_dim,n_hidden_units,"W");
        c = Var(n_hidden_units,1,"c");
        V = Var(n_hidden_units,n,"V");
        Var a = tanh(c + product(V,x));
        Var tangent_plane = diagonalized_factors_product(W,1-a*a,V); 
        tangent_predictor = Func(x, W & c & V, tangent_plane);
        embedding = product(W,a);
        if (output_type=="tangent_plane")
          output_f = tangent_predictor;
        else if (output_type=="embedding")
          output_f = Func(x, embedding);
        else if (output_type=="tangent_plane+embedding")
          output_f = Func(x, tangent_plane & embedding);
      }
    else if (architecture_type == "embedding_quadratic")
      {
        Var x(n);
        b = Var(n_dim,n,"b");
        W = Var(n_dim*n,n,"W");
        Var Wx = product(W,x);
        Var tangent_plane = Wx + b;
        tangent_predictor = Func(x, W & b, tangent_plane);
        embedding = product(new PlusVariable(b,Wx),x);
        if (output_type=="tangent_plane")
          output_f = tangent_predictor;
        else if (output_type=="embedding")
          output_f = Func(x, embedding);
        else if (output_type=="tangent_plane+embedding")
          output_f = Func(x, tangent_plane & embedding);
      }
    else if (architecture_type != "")
      PLERROR("GaussianContinuum::build, unknown architecture_type option %s (should be 'neural_network', 'linear', or empty string '')\n",
              architecture_type.c_str());

    if (parameters.size()>0 && parameters.nelems() == tangent_predictor->parameters.nelems())
      tangent_predictor->parameters.copyValuesFrom(parameters);
    else
      {
        parameters.resize(tangent_predictor->parameters.size());
        for (int i=0;i<parameters.size();i++)
          parameters[i] = tangent_predictor->parameters[i];
      }
    
    if (training_targets=="local_evectors")
      tangent_targets = Var(n_dim,n);
    else if (training_targets=="local_neighbors")
      tangent_targets = Var(n_neighbors,n);
    else PLERROR("GaussianContinuum::build, option training_targets is %s, should be 'local_evectors' or 'local_neighbors'.",
                 training_targets.c_str());

    Var proj_err = projection_error(tangent_predictor->outputs[0], tangent_targets, norm_penalization, n, 
                                    normalize_by_neighbor_distance, use_subspace_distance, svd_threshold, 
                                    projection_error_regularization);
    projection_error_f = Func(tangent_predictor->outputs[0] & tangent_targets, proj_err);
    cost_of_one_example = Func(tangent_predictor->inputs & tangent_targets, tangent_predictor->parameters, proj_err);

  }
}

// ### Nothing to add here, simply calls build_
void GaussianContinuum::build()
{
  inherited::build();
  build_();
}

extern void varDeepCopyField(Var& field, CopiesMap& copies);

void GaussianContinuum::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{  inherited::makeDeepCopyFromShallowCopy(copies);

  deepCopyField(cost_of_one_example, copies);
  varDeepCopyField(b, copies);
  varDeepCopyField(W, copies);
  varDeepCopyField(c, copies);
  varDeepCopyField(V, copies);
  varDeepCopyField(tangent_targets, copies);
  deepCopyField(parameters, copies);
  deepCopyField(optimizer, copies);
  deepCopyField(tangent_predictor, copies);
}


int GaussianContinuum::outputsize() const
{
  return output_f->outputsize;
}

void GaussianContinuum::forget()
{
  if (train_set) initializeParams();
  stage = 0;
}
    
void GaussianContinuum::train()
{

  VMat train_set_with_targets;
  VMat targets_vmat;
  if (!cost_of_one_example)
    PLERROR("GaussianContinuum::train: build has not been run after setTrainingSet!");

  if (training_targets == "local_evectors")
  {
    //targets_vmat = new LocalPCAVMatrix(train_set, n_neighbors, n_dim);
    PLERROR("local_evectors not yet implemented");
  }
  else if (training_targets == "local_neighbors")
  {

    targets_vmat = local_neighbors_differences(train_set, n_neighbors);
  }
  else PLERROR("GaussianContinuum::train, unknown training_targets option %s (should be 'local_evectors' or 'local_neighbors')\n",
	       training_targets.c_str());
  
  train_set_with_targets = hconcat(train_set, targets_vmat);
  train_set_with_targets->defineSizes(inputsize(),inputsize()*n_neighbors,0);
  int l = train_set->length();  
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

  ProgressBar* pb = 0;
  if(report_progress>0)
    pb = new ProgressBar("Training GaussianContinuum from stage " + tostring(stage) + " to " + tostring(nstages), nstages-stage);

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
    }
  if(verbosity>1)
    cout << "EPOCH " << stage << " train objective: " << train_stats->getMean() << endl;

  if(pb)
    delete pb;
}

void GaussianContinuum::initializeParams()
{
  if (seed_>=0)
    manual_seed(seed_);
  else
    PLearn::seed();

  if (architecture_type=="single_neural_network")
  {
    if (smart_initialization)
    {
      V->matValue<<smartInitialization(train_set,n_hidden_units,smart_initialization,initialization_regularization);
      W->value<<(1/real(n_hidden_units));
      b->matValue.clear();
      c->matValue.clear();
    }
    else
    {
      real delta = 1.0 / sqrt(real(inputsize()));
      fill_random_uniform(V->value, -delta, delta);
      delta = 1.0 / real(n_hidden_units);
      fill_random_uniform(W->matValue, -delta, delta);
      b->matValue.clear();
      c->matValue.clear();
    }
  }
  else if (architecture_type=="linear")
  {
    real delta = 1.0 / sqrt(real(inputsize()));
    b->matValue.clear();
    fill_random_uniform(W->matValue, -delta, delta);
  }
  else if (architecture_type=="embedding_neural_network")
  {
    real delta = 1.0 / sqrt(real(inputsize()));
    fill_random_uniform(V->value, -delta, delta);
    delta = 1.0 / real(n_hidden_units);
    fill_random_uniform(W->matValue, -delta, delta);
    c->value.clear();
  }
  else if (architecture_type=="embedding_quadratic")
  {
    real delta = 1.0 / sqrt(real(inputsize()));
    fill_random_uniform(W->matValue, -delta, delta);
    b->value.clear();
  }
  else PLERROR("other types not handled yet!");
  // Reset optimizer
  if(optimizer)
    optimizer->reset();
}


void GaussianContinuum::computeOutput(const Vec& input, Vec& output) const
{
  int nout = outputsize();
  output.resize(nout);
  output << output_f(input);
}    

void GaussianContinuum::computeCostsFromOutputs(const Vec& input, const Vec& output, 
					     const Vec& target, Vec& costs) const
{
  PLERROR("GaussianContinuum::computeCostsFromOutputs not defined for this learner");
}                                

TVec<string> GaussianContinuum::getTestCostNames() const
{
  return getTrainCostNames();
}

TVec<string> GaussianContinuum::getTrainCostNames() const
{
  TVec<string> cost(1); cost[0] = "projection_error";
  return cost;
}



} // end of namespace PLearn
