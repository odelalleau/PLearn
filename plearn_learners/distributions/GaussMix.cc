// -*- C++ -*-

// GaussMix.cc
// 
// Copyright (C) 2003 Julien Keable
// Copyright (C) 2004 Université de Montréal
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
 * $Id: GaussMix.cc,v 1.20 2004/05/18 16:12:17 tihocan Exp $ 
 ******************************************************* */

/*! \file GaussMix.cc */
#include "ConcatColumnsVMatrix.h"
#include "GaussMix.h"
#include "pl_erf.h"   //!< For gauss_density().
#include "plapack.h"
#include "random.h"
#include "SubVMatrix.h"
#include "VMat_maths.h"

namespace PLearn {
using namespace std;

//////////////
// GaussMix //
//////////////
GaussMix::GaussMix() 
: PDistribution(),
  global_lambda0(false),   // TODO Declare as option.
  alpha_min(1e-5),
  epsilon(1e-6),
  kmeans_iterations(3),
  L(1),
  sigma_min(1e-5),
  type("unknown")
{
//  forget(); TODO Call forget ?
  nstages = 10;
}


PLEARN_IMPLEMENT_OBJECT(GaussMix, "Gaussian Mixture, either set non-parametrically or trained by EM", 
                        "GaussMix implements a mixture of gaussians, for which\n"
                        "L : number of gaussians\n"
                        "D : number of dimensions in the feature space\n"
                        "2 parameters are common to all 4 types :\n"
                        "alpha : the ponderation factor of that gaussian\n"
                        "mu : its center\n"
                        "There are 4 possible parametrization types.:\n"
                        " - spherical : gaussians have covar matrix = diag(sigma). parameter used : sigma.\n"
                        " - diagonal : gaussians have covar matrix = diag(sigma_i). parameters used : diags.\n"
                        " - general : gaussians have an unconstrained covariance matrix. User provides the K<=D greatest eigenvalues\n"
                        "          (through parameter lambda) and their corresponding eigenvectors (through matrix V)\n"
                        "          of the covariance matrix. For the remaining D-K eigenvectors, a user-given eigenvalue (through sigma) is assumed\n"
                        " - factor : as in the general case, the gaussians are defined with K<=D vectors (through KxD matrix 'V'), but these need not be orthogonal/orthonormal.\n"
                        "         The covariance matrix used will be V(t)V + psi with psi a D-vector (through parameter diags).\n");

////////////////////
// declareOptions //
////////////////////
void GaussMix::declareOptions(OptionList& ol)
{

  // Build options.

  declareOption(ol,"L", &GaussMix::L, OptionBase::buildoption,
                "Number of gaussians in the mixture.");
  
  declareOption(ol,"type", &GaussMix::type, OptionBase::buildoption,
                "A string :  'unknown', 'spherical', 'diagonal', 'general', 'factor',\n"
                "This is the type of Covariance matrix for each Gaussian.\n"
                "   - unknown   : ? \n"
                "   - spherical : spherical covariance matrix sigma * I\n"
                "   - diagonal  : diagonal covariance matrix\n"
                "   - general   : ? \n"
                "   - factor    : represented by Ks[i] principal components\n");
                // TODO Finish this help.

  declareOption(ol,"alpha_min", &GaussMix::alpha_min, OptionBase::buildoption,
                "The minimum weight for each Gaussian.");
  
  declareOption(ol,"sigma_min", &GaussMix::sigma_min, OptionBase::buildoption,
                "The minimum standard deviation allowed.");
  
  declareOption(ol,"epsilon", &GaussMix::epsilon, OptionBase::buildoption,
                "A small number to check for near-zero probabilities.");
  
  declareOption(ol,"kmeans_iterations", &GaussMix::kmeans_iterations, OptionBase::buildoption,
                "The maximum number of iterations performed in the initial K-means algorithm.");
  
  // Learnt options.

  declareOption(ol, "alpha", &GaussMix::alpha, OptionBase::learntoption,
                "Coefficients of each gaussian.\n"
                "They sum to 1 and are positive: they can be interpreted as prior P(gaussian i).\n");

  declareOption(ol, "covariance", &GaussMix::covariance, OptionBase::learntoption,
                "The covariance matrix of each Gaussian.");

  declareOption(ol,"D", &GaussMix::D, OptionBase::learntoption,
                "Number of dimensions in input space.");
    
  declareOption(ol,"diags", &GaussMix::diags, OptionBase::learntoption,
                "The element (i,j) is the stddev of Gaussian j on the i-th dimension.");
    
  declareOption(ol, "mu", &GaussMix::mu, OptionBase::learntoption,
                "These are the centers of each Gaussian, stored in rows.");

  declareOption(ol, "nsamples", &GaussMix::nsamples, OptionBase::learntoption,
                "The number of samples in the training set.");

  declareOption(ol, "posteriors", &GaussMix::posteriors, OptionBase::learntoption,   // TODO Remove from options (too much space on disk).
                "The posterior probabilities P(j | x_i), where j is the index of a Gaussian,\n"
                "and i is the index of a sample.");

  declareOption(ol, "sigma", &GaussMix::sigma, OptionBase::learntoption,
                "This is one way to represent the covariance or part of it, depending on type:\n"
                " - spherical : the sigma for each spherical gaussian, in all directions\n"
                " - diagonal : not used"\
                "General : for the l-th gaussian, the eigenvalue (a.k.a lambda0) used for all D-Ks[l] dimensions");
  //TODO Redo the help on sigma.

  // TODO What to do with these options.
  
  /*
  declareOption(ol, "diags", &GaussMix::diags, OptionBase::buildoption,
                "diagonal : a L x D matrix where row 'l' is the diagonal of the covariance matrix of gaussian l\n"\
                "factor : a L x D matrix where row 'l' is psi (the output noise) ** note that after calling build(), the rows of diags are inverted (diags(i,j)=1/diags(i,j)"); */ // TODO Put back some of those comments.

  declareOption(ol, "lambda", &GaussMix::lambda, OptionBase::buildoption,
                "General : The concatenation of all vectors of length K[l] containing the eigenvalues of the l-th gaussian.");

  declareOption(ol, "V", &GaussMix::V, OptionBase::buildoption,
                "The vertical concatenation of all the Ks[i] x D matrix, (each contains the Ks[i] vectors that define gaussian i.)\n "\
                "General: the rows of one Ks[i] x D matrix are the principal eigenvectors of the covariance matrix of the i-th gaussian\n"\
                "Factor: the Ks[i] x D matrix is the factor loading matrix (this matrix corresponds to big-lambda *transposed* in the factor analysis litterature).\n");

  declareOption(ol, "V_idx", &GaussMix::V_idx, OptionBase::buildoption,
                "Used for general and factore gaussians : A vector of size L. V_idx[l] is the row index of the first vector of gaussian 'l' in the matrix 'V' (also used to index vector 'lambda')");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

///////////
// build //
///////////
void GaussMix::build()
{
  inherited::build();
  build_();
}

////////////
// build_ //
////////////
void GaussMix::build_()
{

  // TODO Below is old stuff.

  /*
  
  if(type=="Unknown")
    return;

  if(alpha.length()!=L)
    PLERROR("You provided %i mixture coefficients. need %i",alpha.length(),L);
  if(mu.length()!=L)
    PLERROR("You provided %i rows in matrix mu. need %i",mu.length(),L);

  int sum_Ks_square=0;
  if(type=="General" || type=="Factor")
  { */
    /* There are two possible cases here :
       
       1 : the mixture has been set by loading from a file (or setOptions), the Ks array is not valid yet, and we must build it. In that case, we can use 
       V.length() to know how much total vectors we have.

       2 : the mixture was set with SetGaussianXXX, Ks is already set and V.length() is probably larger (because we overallocate for speed concerns)
       than total number of vectors 

       If Ks[0] = 0, we assume case 1.
    */
       
/*
    int sum_Ks=0;
    Ks.resize(L);
        
    if(Ks[0]==0)
    {
      int l;
      for(l=0;l<L-1;l++)
      {
        sum_Ks+=Ks[l]=V_idx[l+1]-V_idx[l];
        sum_Ks_square += Ks[l]*Ks[l];
      }
      sum_Ks+=Ks[l]=V.length()-V_idx[L-1];
      sum_Ks_square += Ks[l]*Ks[l];
    }
    else
      for(int l=0;l<L;l++)
      {
        sum_Ks+=Ks[l];
        sum_Ks_square += Ks[l]*Ks[l];
      }

    if(sigma.size()==0)
    {
      sigma.resize(L);
      sigma.fill(0.0);
    }

    V.resize(sum_Ks,D);
    lambda.resize(sum_Ks);
  }

  log_coef.resize(L);
  mu.resize(L,D);
  tmpvec.resize(D);
  tmpvec2.resize(D);

  if(type=="Spherical")
    for(int l=0;l<L;l++)
      log_coef[l]=log(1/sqrt(pow(2*3.14159,D) * pow(sigma[l],D)));

  else if(type=="Diagonal")
    for(int l=0;l<L;l++)
      log_coef[l]=log(1/sqrt(pow(2*3.14159,D) * product(diags(l))));

  else if(type=="General")
    for(int l=0;l<L;l++)
    {
      // compute determinant
      double log_det = log(product(lambda.subVec(V_idx[l],Ks[l])));
      
      if(D-Ks[l]>0)
      {
        if(sigma[l]==0)
          PLERROR("lambda_0 for gaussian #%i is 0!",l);
        log_det+=log(sigma[l]*(D-Ks[l]));
      }
      log_coef[l] = -0.5*( D*log(2*3.141549) + log_det );
    }
  
  else if(type=="Factor")
  {
    inv_ivtdv.resize(sum_Ks_square);
    inv_ivtdv_idx.resize(L);

    for(int l=0;l<L;l++)
    {
      // diags will contain the diagonal of psi^-1
      diags(l)<<inverted(diags(l));
      if(l>0)
        inv_ivtdv_idx[l]=inv_ivtdv_idx[l-1]+Ks[l]*Ks[l];
      int K=Ks[l];
      if(l>0)
        inv_ivtdv_idx[l] = inv_ivtdv_idx[l-1] + K*K;
      precomputeFAStuff(V.subMatRows(V_idx[l],Ks[l]), diags(l), log_coef[l], inv_ivtdv.subVec(inv_ivtdv_idx[l], K*K));
    }
    
  } */
}

////////////////////////////////
// computeMeansAndCovariances //
////////////////////////////////
void GaussMix::computeMeansAndCovariances() {
  VMat weighted_train_set;
  Vec sum_columns(L);
  columnSum(posteriors, sum_columns);
  for (int j = 0; j < L; j++) {
    // Build the weighted dataset.
    if (sum_columns[j] < epsilon) {
      PLWARNING("In GaussMix::computeMeansAndCovariances - A posterior is almost zero");
    }
    VMat weights(posteriors.column(j));
    weighted_train_set = new ConcatColumnsVMatrix(
        new SubVMatrix(train_set, 0, 0, nsamples, D), weights);
    weighted_train_set->defineSizes(D, 0, 1);
/*
  // mu_j <- (sum_i x_i P(j | x_i)) / (sum_i P(j | x_i))
  static Vec sample;
  static Vec normalization;
  static real t;
  sample.resize(D);
  normalization.resize(L);
  mu.fill(0);
  normalization.fill(0);
  for (int i = 0; i < nsamples; i++) {
    train_set->getSubRow(i, 0, sample);
    for (int j = 0; j < L; j++) {
      t = posteriors(i,j);
      mu(j) += sample * t;
      normalization[j] += t;
    }
  }
  for (int j = 0; j < L; j++) {
    mu(j) /= normalization[j];
  } */ // TODO Get rid of this.
    if (type == "spherical") {
      Vec variance(D);
      Vec center = mu(j);
      computeInputMeanAndVariance(weighted_train_set, center, variance);
      sigma[j] = sqrt(mean(variance));   // TODO See if harmonic mean is needed ?
#ifdef BOUNDCHECK
      if (isnan(sigma[j])) {
        PLWARNING("In GaussMix::computeMeansAndCovariances - A standard deviation is nan");
      }
#endif
    } else if (type == "diagonal") {
      Vec variance(D);
      Vec center = mu(j);
      computeInputMeanAndVariance(weighted_train_set, center, variance);
      for (int i = 0; i < D; i++) {
        diags(i,j) = sqrt(variance[i]);
      }
    } else {
      PLERROR("In GaussMix::computeMeansAndCovariances - Not implemented for this type of Gaussian");
    }
    // TODO Implement them all.
  }
}

/////////////////////
// computeLikehood //
/////////////////////
real GaussMix::computeLikehood(Vec& x, int j) {
  if (type == "spherical") {
    real p = 1.0;
    if (sigma[j] < sigma_min) {
      return 0;
    }
    for (int k = 0; k < D; k++) {
      p *= gauss_density(x[k], mu(j, k), sigma[j]);
#ifdef BOUNDCHECK
      if (isnan(p)) {
        PLWARNING("In GaussMix::computeLikehood - Density is nan");
      }
#endif
    }
    return p;
  } else if (type == "diagonal") {
    real p = 1.0;
    real sig;
    for (int k = 0; k < D; k++) {
      sig = diags(k,j);
      if (sig < sigma_min) {
        return 0;
      } else {
        p *= gauss_density(x[k], mu(j, k), sig);
      }
    }
    return p;
  } else {
    PLERROR("In GaussMix::computeLikehood - Not implemented for this type of Gaussian");
  }
  return 0;
}

///////////////////////
// computePosteriors //
///////////////////////
void GaussMix::computePosteriors() {
  static Vec sample;
  static Vec likehood;
  sample.resize(D);
  likehood.resize(L);
  real sum_likehood;
  for (int i = 0; i < nsamples; i++) {
    train_set->getSubRow(i, 0, sample);
    sum_likehood = 0;
    // First we need to compute the likehood P(x_i | j).
    for (int j = 0; j < L; j++) {
      likehood[j] = computeLikehood(sample, j) * alpha[j];
#ifdef BOUNDCHECK
      if (isnan(likehood[j])) {
        PLWARNING("In GaussMix::computePosteriors - computeLikehood returned nan");
      }
#endif
      sum_likehood += likehood[j];
    }
#ifdef BOUNDCHECK
    if (sum_likehood < epsilon) {
      // x_i is far from each Gaussian, and thus sum_likehood is null
      // because of numerical approximations. We find the closest
      // Gaussian, and say P(j | x_i) = delta_{j is the closest Gaussian}
      PLWARNING("In GaussMix::computePosteriors - A point has near zero density");
    }
#endif
    for (int j = 0; j < L; j++) {
      // Compute the posterior P(j | x_i) = P(x_i | j) * alpha_i / (sum_i ")
      posteriors(i, j) = likehood[j] / sum_likehood;
    }
  }
}

////////////////////
// computeWeights //
////////////////////
bool GaussMix::computeWeights() {
  bool replaced_gaussian = false;
  alpha.fill(0);
  for (int i = 0; i < nsamples; i++) {
    for (int j = 0; j < L; j++) {
      alpha[j] += posteriors(i,j);
    }
  }
  alpha /= real(nsamples);
  for (int j = 0; j < L && !replaced_gaussian; j++) {
    if (alpha[j] < alpha_min) {
      // alpha[j] is too small! We need to remove this Gaussian from the
      // mixture, and find a new (better) one.
      replaceGaussian(j);
      replaced_gaussian = true;
    }
  }
  return replaced_gaussian;
}

////////////
// forget //
////////////
void GaussMix::forget()
{
  stage = 0;
  // Free memory.
  mu = Mat();
  posteriors = Mat();
  alpha = Vec();
  sigma = Vec();
  diags = Mat();

  // Below is old stuff. See what to do with this !
  /*
  type="Unknown";
  L=D=0;
  avg_K=0;
  n_principal_components = 0;
  EM_lambda0 = 0;

  initArrays(); */
}

//////////////
// generate //
//////////////
void GaussMix::generate(Vec& x) const
{
  generateFromGaussian(x, -1);
}

//////////////////////////
// generateFromGaussian //
//////////////////////////
void GaussMix::generateFromGaussian(Vec& x, int given_gaussian) const {
  int j;    // The index of the Gaussian to use.
  if (given_gaussian < 0)
    j = multinomial_sample(alpha);
  else
    j = given_gaussian % alpha.length();
  x.resize(D);
  if (type == "spherical") {
    for (int k = 0; k < D; k++) {
      x[k] = gaussian_mu_sigma(mu(j, k), sigma[j]);
    }
  } else if (type == "diagonal") {
    for (int k = 0; k < D; k++) {
      x[k] = gaussian_mu_sigma(mu(j, k), diags(k,j));
    }
//    generateDiagonal(x,given_gaussian);
  } else if(type[0]=='G') {
//    generateGeneral(x,given_gaussian);
  } else if(type[0]=='F') {
//    generateFactor(x,given_gaussian);
  } else if (type == "unknown") {
    PLERROR("In GaussMix::generate - You didn't specify the mixture type");
  } else {
    PLERROR("In GaussMix::generate - Unknown mixture type");
  }
}

////////////
// kmeans //
////////////
void GaussMix::kmeans(VMat samples, int nclust, TVec<int> & clust_idx, Mat & clust, int maxit)
// TODO Put it into the PLearner framework.
{
  int nsamples = samples.length();
  Mat newclust(nclust,samples->inputsize());
  clust.resize(nclust,samples->inputsize());
  clust_idx.resize(nsamples);

  Vec input(samples->inputsize());
  Vec target(samples->targetsize());
  real weight;
    
  Vec samples_per_cluster(nclust);
  TVec<int> old_clust_idx(nsamples);
  bool ok=false;

  // build a nclust-long vector of samples indexes to initialize clusters centers
  Vec start_idx(nclust,-1.0);
  int val;
  for(int i=0;i<nclust;i++)
  {
    bool ok=false;
    while(!ok)
    {
      
      ok=true;
      val = rand() % nsamples;
      for(int j=0;j<nclust && start_idx[j]!=-1.0;j++)
        if(start_idx[j]==val)
        {
          ok=false;
          break;
        }
    }
    start_idx[i]=val;
    samples->getExample(val,input,target,weight);    
    clust(i)<<input;
  }

  while(!ok && maxit--)
  {
    newclust.clear();
    samples_per_cluster.clear();
    old_clust_idx<<clust_idx;
    for(int i=0;i<nsamples;i++)
    {
      samples->getExample(i,input,target,weight);
      real dist,bestdist=1E300;
      int bestclust=0;
      for(int j=0;j<nclust;j++)
        if((dist=pownorm(clust(j)-input)) < bestdist)
        {
          bestdist=dist;
          bestclust=j;
        }
      clust_idx[i]=bestclust;
      samples_per_cluster[bestclust]++;
      newclust(bestclust)+=input;
    }
    for(int i=0; i<nclust; i++)
      if (samples_per_cluster[i]>0)
        newclust(i)/=samples_per_cluster[i];
    clust << newclust;
    ok=true;
    for(int i=0;i<nsamples;i++)
      if(old_clust_idx[i]!=clust_idx[i])
      {
        ok=false;
        break;
      }
    
  }
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void GaussMix::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
  PLERROR("GaussMix::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

/////////////////////
// replaceGaussian //
/////////////////////
void GaussMix::replaceGaussian(int j) {
  // Find the Gaussian with highest weight.
  int high = 0;
  for (int k = 1; k < L; k++) {
    if (alpha[k] > alpha[high]) {
      high = k;
    }
  }
  // Generate the new center from this Gaussian.
  Vec new_center = mu(j);
  generateFromGaussian(new_center, high);
  // Copy the covariance.
  if (type == "spherical") {
    sigma[j] = sigma[high];
  } else if (type == "diagonal") {
    diags.column(j) << diags.column(high);
  } else {
    PLERROR("In GaussMix::replaceGaussian - Not implemented for this type");
  }
  // Arbitrarily takes half of the weight of this Gaussian.
  alpha[j] = alpha[high] / 2.0;
  alpha[high] = alpha[j];
}

////////////////////
// resetGenerator //
////////////////////
void GaussMix::resetGenerator(long g_seed) const
{ 
  manual_seed(g_seed);  
}

///////////
// train //
///////////
void GaussMix::train()
{
  // TODO Actually, we should compute the posteriors because saving them
  // could take too much room.
  // Check we actually need to train.
  if (stage >= nstages) {
    PLWARNING("In GaussMix::train - The learner is already trained");
    return;
  }

  // Check there is a training set.
  if (!train_set) {
    PLERROR("In GaussMix::train - No training set specified");
  }

  // Make sure everything is the right size.
  nsamples = train_set->length();
  D = train_set->inputsize(); // TODO Check this is the right thing.
  mu.resize(L,D);
  posteriors.resize(nsamples, L);
  alpha.resize(L);
  nsamples = train_set->length();
  // Those are not used for every type:
  sigma.resize(0);
  diags.resize(0,0);
  if (type == "unknown") {
  } else if (type == "diagonal") {
    diags.resize(D,L);
  } else if (type == "factor") {
  } else if (type == "general") {
  } else if (type == "spherical") {
    sigma.resize(L);
  } else {
    PLERROR("In GaussMix::train - Unknown value for the 'type' option");
  }

  if (stage == 0) {
    // Perform K-means to initialize the centers of the mixture.
    TVec<int> clust_idx;  // Store the cluster index for each sample.
    kmeans(train_set, L, clust_idx, mu, kmeans_iterations);
    posteriors.fill(0);
    for (int i = 0; i < nsamples; i++) {
      // Initially, P(j | x_i) = 0 if x_i is not in the j-th cluster,
      // and 1 otherwise.
      posteriors(i, clust_idx[i]) = 1;
    }
    computeWeights();
    computeMeansAndCovariances();
  }

  Vec sample(D);
  bool replaced_gaussian = false;
  while (stage < nstages) {
    do {
      computePosteriors();
      replaced_gaussian = computeWeights();
    } while (replaced_gaussian);
    computeMeansAndCovariances();
    stage++;
  }

  // Was: void GaussMix::EM(VMat samples, real relativ_change_stop_value)
  /*


     faire K-means

     initialiser selon les resultats et selon le type

     boucle sur les stages (comme dans autres Learners)
        
        boucle sur les donnees
          pour calculer les posterieurs

        boucle sur les composantes
          faire updateWSamples selon le type


  */

  /*
  VMat samples = train_set;
  real relativ_change_stop_value=MISSING_VALUE; // what? should be a field;
  if(type=="Factor")
  {
    EMFactorAnalyser(samples,relativ_change_stop_value);
    return;
  }

  bool samples_have_weights = samples->weightsize()>0;

  int nsamples=samples.length();
  Vec wvec(nsamples);
  TVec<int> clust_idx(nsamples);
  Vec input(samples->inputsize());
  Vec target(samples->targetsize());
  real weight;

  Mat mu;
  Vec alphas(L);
  Mat weights(nsamples, L);

  kmeans(samples, L, clust_idx, mu, 2); // do 2 kmeans iterations, should be enough

  setMixtureTypeGeneral(L,2,D);

  real sum=0;

  // alpha[l] <- relative frequency of cluster j
  for(int i=0;i<nsamples;i++)
  {
    samples->getExample(i,input,target,weight);
    alphas[clust_idx[i]]+=weight;
    sum+=weight;
  }

  cout<<PLearn::sum(alphas)<<" "<<sum<<endl;
  // normalize alphas;
  alphas/=sum;


  // weights are initialized with 1 for the kmeans' cluster they're in, otherwise 0
  for(int i=0;i<nsamples;i++)
  {
    samples->getExample(i,input,target,weight);
    for(int j=0;j<L;j++)
      weights(i,j) = (clust_idx[i] == j? weight * 1.0 : 0.0);
  }
  
  for(int i=0;i<L;i++)
  {
    VMat weighted_samples(ConcatColumnsVMatrix(samples->inputsize()==samples->width()?samples:
                                               samples.subMatColumns(0,samples->inputsize()), 
                                               weights.column(i)));
    weighted_samples->defineSizes(samples->inputsize(),0,1);
    setGaussianGeneralWSamples(i, alphas[i],EM_lambda0,n_principal_components,weighted_samples);
  }

  // build();
  itn count=0; // what?
  while(count--)
  {
    cout<<"count:"<<count<<" nll:"<<NLL(samples)<<endl;//<<"mus:"<<endl<<mu<<endl<<"alphas:"<<endl<<alpha<<endl;
    // update alphas
    sum=0;
    if(samples_have_weights)
    {
      sum=0;
      for(int i=0; i<L; i++)
      {
        samples->getColumn(samples.width()-1,wvec);
        for(int j=0;j<nsamples;j++)
          wvec[j] *= weights(j,i);
        sum+=alphas[i] = mean(wvec);
      }
      alphas/=sum;
    }
    else
      for(int i=0; i<L; i++)
        sum+=alphas[i] = mean(weights.column(i));
    cout<<alphas<<endl;
    // update weights
    for(int i=0; i<nsamples; i++)
    {
      samples->getExample(i,input,target,weight);
      Vec Pxn_alpha(L);
      sum=0;
      for(int j=0;j<L;j++)
      {
        sum+=Pxn_alpha[j] =  exp(logDensityGeneral(input, j)); // * alpha[j]
//        cout<<i<<" "<<j<<" "<<weights(i,j)<<" "<<exp(logDensityGeneral(samples(i), j))<<endl;
      }
      for(int j=0;j<L;j++)
        {
          if(sum)
            weights(i,j) = Pxn_alpha[j] / sum;
          else
            weights(i,j) = 0;
//          cout<<i<<" "<<j<<" "<<weights(i,j)<<endl;
        }
    }
    
    for(int i=0;i<L;i++)
    {
      if(samples_have_weights)
      {
        samples->getColumn(samples.width()-1,wvec);
        for(int j=0;j<nsamples;j++)
          wvec[j] *= weights(j,i);
        VMat weighted_samples(ConcatColumnsVMatrix(samples.subMatColumns(0,samples->inputsize()), wvec.toMat(nsamples,1)));
        weighted_samples->defineSizes(samples->inputsize(),0,1);
        setGaussianGeneralWSamples(i, alphas[i],EM_lambda0,n_principal_components,weighted_samples);
      }
      else
      {
        VMat weighted_samples(ConcatColumnsVMatrix(samples, weights.column(i)));
        weighted_samples->defineSizes(samples.width(),0,1);
        setGaussianGeneralWSamples(i, alphas[i],EM_lambda0,n_principal_components,weighted_samples);
      }
    }
    
    build();
    
   Vec mu1(2);
   for(int y=0;y<50;y++)
     for(int z=0;z<50;z++)
//       for(int y=0;y<50;y++)
       {
         //  generate(mu1);
         mu1[0]=(float)y*(3.0/5.0);
         mu1[1]=(float)z*(3.0/5.0);
//         mu1[2]=(float)z/5-5;
         
       den(y*50+z+(count)*2500,0)=mu1[0];
       den(y*50+z+(count)*2500,1)=mu1[1];

       den(y*50+z+(count)*2500,2)=density(mu1);
       den(y*50+z+(count)*2500,3)=count;
       }
  }
  VMat out(den);
  out->savePMAT("out.pmat");
*/
}

/*********************** METHODS BELOW ARE OLD (USELESS ?) STUFF **************************/

/*
void GaussMix::setMixtureTypeSpherical(int _L, int _D)
{
  L=_L;
  D=_D;
  avg_K=0;
  type="Spherical";
  initArrays();
}

void GaussMix::setMixtureTypeDiagonal(int _L, int _D)
{
  L=_L;
  D=_D;
  avg_K=0;
  type="Diagonal";
  initArrays();
}

void GaussMix::setMixtureTypeGeneral(int _L, int _avg_K, int _D)
{
  L=_L;
  D=_D;
  avg_K=_avg_K;
  type="General";
  initArrays();
}

void GaussMix::setMixtureTypeFactor(int _L, int _avg_K, int _D)
{
  L=_L;
  D=_D;
  avg_K=_avg_K;
  type="Factor";
  initArrays();
}

// spherical
void GaussMix::setGaussianSpherical(int l, real _alpha, Vec _mu, real _sigma)
{
  if(type!="Spherical")
    PLERROR("GaussMix::setGaussian : type is not Spherical");

  alpha[l]=_alpha;
  mu(l)<<_mu;
  sigma[l]=_sigma;
}

// diagonal
void GaussMix::setGaussianDiagonal(int l, real _alpha, Vec _mu, Vec diag)
{
  if(type!="Diagonal")
    PLERROR("GaussMix::setGaussian : type is not Diagonal");

  alpha[l]=_alpha;
  mu(l)<<_mu;
  diags(l)<<diag;
}

//general
void GaussMix::setGaussianGeneral(int l, real _alpha, Vec _mu, Vec _lambda, Mat _V, real lambda0 )
{
  int num_lambda = _lambda.length();

  if(num_lambda==0)
    PLERROR("GaussMix::setGaussian : need at least 1 eigenvector");
  if(type!="General")
    PLERROR("GaussMix::setGaussian : type is not General");

  alpha[l]=_alpha;
  mu(l)<<_mu;
  sigma[l]=lambda0;

  if(Ks[l]==0)
  {
    // things to do only on the first time this gaussian is set
    Ks[l]=num_lambda;
    V_idx[l]=(l==0)?0:V_idx[l-1]+Ks[l];

    if(V.length()< V_idx[l] + Ks[l])
    {
      int newl = MAX(V_idx[l] + Ks[l], (int)ceil((double)V.length()*1.5));
      V.resize(V_idx[l] + Ks[l],V.width(), newl);
      lambda.resize(newl);
    }
  }
  else 
  {
    if(Ks[l]!=num_lambda)
      PLERROR("GaussMix::setGaussian : for the same gaussian, the number of vectors (length of the 'eigenvecs' matrix) that "\
              "define a gaussian must not change between calls. ");
  }

  for(int i=0;i<_lambda.length();i++)
    _lambda[i] = MAX(lambda0,_lambda[i]);

  lambda.subVec(V_idx[l],Ks[l])<<_lambda;
  
  V.subMatRows(V_idx[l],Ks[l])<<_V;

}

//factor
void GaussMix::setGaussianFactor(int l, real _alpha, Vec _mu, Mat _V, Vec diag )
{
  if(_V.length()==0)
    PLERROR("GaussMix::setGaussian : need at least 1 eigenvector");
  if(type!="Factor")
    PLERROR("GaussMix::setGaussian : type is not General");
  alpha[l]=_alpha;
  mu(l)<<_mu;
  diags(l) << diag;

  // things to do only on the first time

  if(Ks[l]==0)
  {
    Ks[l]=_V.length();
    V_idx[l]=(l==0)?0:V_idx[l-1]+Ks[l];

    if(V.length()< V_idx[l] + Ks[l])
    {
      int newl = MAX(V_idx[l] + Ks[l],(int)ceil((double)V.length()*1.5));
      V.resize(V_idx[l] + Ks[l], V.width(), newl);
    }
 }
  else if(Ks[l]!=_V.length())
    PLERROR("GaussMix::setGaussian : for the same gaussian, the number of vectors (length of the 'eigenvecs' matrix) that "\
            "define a gaussian must not change between calls. ");
  
  V.subMatRows(V_idx[l],Ks[l])<<_V;
  
}

void GaussMix::setGaussianSphericalWSamples(int l, real _alpha, VMat samples)
{
  Vec wmeans(D);
  Vec wvar(D);
  computeInputMeanAndVariance(samples, wmeans, wvar);
  setGaussianSpherical(l, _alpha, wmeans, mean(wvar));
}

void GaussMix::setGaussianDiagonalWSamples(int l, real _alpha, VMat samples)
{
  Vec wmeans(D);
  Vec wvar(D);
  computeInputMeanAndVariance(samples, wmeans,wvar);
  setGaussianDiagonal(l, _alpha, wmeans, wvar);
}


// 
void GaussMix::setGaussianGeneralWSamples(int l, real _alpha, real _sigma, int ncomponents, VMat samples)
{
  if(ncomponents==0)
    PLERROR("With general gaussians, you must set n_principal_components to a number greater than 0.");

  static Vec wmeans;
  wmeans.resize(D);
  static Mat wcovar;
  wcovar.resize(D,D);
  Mat eig_vectors(ncomponents,D);
  Vec eig_values(ncomponents);

  computeInputMeanAndCovar(samples, wmeans,wcovar);
  if (global_lambda0)
  {
    eigenVecOfSymmMat(wcovar, ncomponents,  eig_values, eig_vectors); 
    setGaussianGeneral(l, _alpha, wmeans, eig_values, eig_vectors, _sigma );
  }
  else
  {
    eigenVecOfSymmMat(wcovar, ncomponents+1,  eig_values, eig_vectors); 
    setGaussianGeneral(l, _alpha, wmeans, eig_values.subVec(0,ncomponents), eig_vectors.subMatRows(0,ncomponents), eig_values[ncomponents] );
  }
}
*/

/*
  Use EM to fit a factor analyser on a group of (possibly) weighted samples.

  For now, initialisation is done by setting the factor analyser to have a
  factor loading matrix (V, in this code) composed of the N greatest
  eigenvectors of the weighted covariance matrix multiplied by their
  eigenvalue, while the psi (noise on each of the output dimensions) is set
  to be I * EM_lambda0
  
  The Factor Analysis model is as follows:
    x = Vz + mu + psi, with

    V, a DxK matrix (factor loading matrix)
    z, a K vector (values of the hidden factors)
    mu, a D vector (mean of x)
    psi, a D vector (random noise added on each output dimension).

  The EM steps for a single factor analyzer are (taken from : The EM
  algorithm for mixtures of Factor Analyzer, 1996, Ghahramani & Hinton) :

  E-step : compute E(z|x_i) and E(zz'|x_i) for each sample x_i given V and psi
  M-step : Vnew = ( sum_i {(x_i-mu)E(z|x_i)'} ) ( sum_l E(zz'|x_l) )^-1
           psi_new = 1/n diag{ sum_i{(x_i-mu)(x_i-mu)'} - Vnew E(z|x_i) (x_i-mu)'}

  with E(z|x) = B(x-mu), where B = V'(psi + V V')^{-1}
  and  E(zz'|x) = Var(z|x) + E(z|x)E(z|x)'  = I - BV + B(x-mu)(x-mu)'B'  ( dimension is k x k )

  Using the matrix inversion lemma, we can reexpress B as

  B = V' (psi^-1 - psi^-1 * V * (I + V'*psi^-1*V)^-1 * V' * psi^-1)

  Here we do ONE STEP of EM update, using the weighted samples
  (presumably because of this Gaussian is part of a mixture, and
  the weights are its posteriors).
*/
/*
void GaussMix::setGaussianFactorWSamples(int l, real _alpha, VMat samples)
{
  //bool samples_have_weights = samples->weightsize()>0;

  //int nsamples=samples.length();
  //Vec wvec(nsamples);
  //Vec input(samples->inputsize());
  //Vec target(samples->targetsize());
  //real weight;

  static Mat eig_vectors;
  eig_vectors.resize(n_principal_components+1,D);
  static Vec eig_values;
  eig_values.resize(n_principal_components+1);

  static Mat Vnew;
  Vnew.resize(n_principal_components,D);
  

  // initialize factors using principal eigenvectors * sqrt(eigenvalues)
  // and the next eigenvalue as initial "psi".
  static Vec wmeans;
  wmeans.resize(D);
  static Mat wcovar;
  wcovar.resize(D,D);

  computeInputMeanAndCovar(samples, wmeans,wcovar);
  eigenVecOfSymmMat(wcovar, n_principal_components+1,  eig_values, eig_vectors); 
  real initial_psi = eig_values[n_principal_components];
  real * data = eig_vectors.data();

  // multiply row[i] of eig_vectors by eig_values[i]
  for(int i=0;i<n_principal_components;i++)
  {
    real stddev=sqrt(eig_values[i]-initial_psi);
    for(int j=0;j<D;j++)
      (*data++)*=stddev;
  }

  // constant noise is initial_psi
  setGaussianFactor(l, _alpha, wmeans,  
                    eig_vectors.subMatRows(0,n_principal_components), Vec(D,initial_psi));

  if(l>0)
    inv_ivtdv_idx[l] = inv_ivtdv_idx[l-1] + Ks[l]*Ks[l];
  precomputeFAStuff(V.subMatRows(V_idx[l],Ks[l]), diags(l), log_coef[l], inv_ivtdv.subVec(inv_ivtdv_idx[l], Ks[l]*Ks[l]));

  // do just ONE EM step, assuming this is called in an outer EM loop

  // compute B
  // compute E(z|x_i)
  // compute E(zz'|x_i)
  // compute Vnew
  //   compute sum_i{(x_i-mu) E(z|x_i)') and sum_i E(zz'|x_i)
  // compute psi_new
    
}

// this function computes (I + V(t) * psi^-1 * V) ^ -1 and logcoef
void GaussMix::precomputeFAStuff(Mat V, Vec diag, real &log_coef, Vec inv_ivtdv)
{
  // compute I + V(t) * psi^-1 * V
  int K = V.length();
  Mat i_plus_vt_ipsi_v( K, K, 0.0 );

  for(int i=0;i<K;i++)
  {
    Vec vi(V(i));
    Vec mati(i_plus_vt_ipsi_v(i));
    for(int j=0;j<K;j++)
    {
      Vec vj(V(j));
      for(int k=0;k < D;k++)

        mati[j]+=vi[k] * diag[k] * vj[k];
    }
  }
  addToDiagonal(i_plus_vt_ipsi_v,1.0);
      
  real determinant = det(i_plus_vt_ipsi_v);
      
  // see logDensityFactor for explanations on formulas
  log_coef = -0.5*(D*log(2*3.14159) + log(determinant) + log(product(inverted(diag))));
      
  // compute (I + V(t) * psi * V)^-1
      
  Mat eigenvec(K,K);
  Vec eigenval(K);
     
  eigenVecOfSymmMat(i_plus_vt_ipsi_v, K, eigenval, eigenvec);
  eigenvec=transpose(eigenvec);
  Mat inv(K,K);
  for(int i=0;i<K;i++)
  {
    Vec vi(eigenvec(i));
    Vec mati(inv(i));
    for(int j=0;j<K;j++)
    {
      Vec vj(eigenvec(j));
      for(int k=0;k < K;k++)
        mati[j]+=vi[k] / eigenval[k] * vj[k];
    }
  }

  inv_ivtdv << inv.toVec();
}


void GaussMix::initArrays()
{
  alpha.resize(L);
  log_coef.resize(L);
  sigma.resize(L);
  diags.resize(L,D);
  mu.resize(L,D);
    
  if(type=="General" || type=="Factor")
  {
    Ks.resize(L);
    Ks.fill(0);
    V_idx.resize(L);
    V.resize(L*avg_K,D);
    lambda.resize(L*avg_K,D);
  }
  else
  {
    Ks.resize(0);
    V_idx.resize(0);
    V.resize(0,0);
    lambda.resize(0,0);
  }
    
  if(type=="Factor")
  {
    inv_ivtdv_idx.resize(L);
    inv_ivtdv.resize(L*avg_K*avg_K);
    tmpvec.resize(D);
    tmpvec2.resize(D);
  }
  else 
  {
    tmpvec.resize(0);
    tmpvec2.resize(0);
  }

}


void GaussMix::EMFactorAnalyser(VMat samples, real relativ_change_stop_value)
{
*/
/*
  // start by initializing the factor loading matrices and psi
  
// % X - data matrix
// % M - number of mixtures (default 1)
// % K - number of factors in each mixture (default 2)
// % cyc - maximum number of cycles of EM (default 100)
// % tol - termination tolerance (prop change in likelihood) (default 0.0001)
// %
// % Lh - factor loadings 
// % Ph - diagonal uniquenesses matrix
// % Mu - mean vectors
// % Pi - priors
// % LL - log likelihood curve

    Vec meanvec(samples->inputsize());
    Vec vovarmat(samples->inputsize(),samples->inputsize());

    computeInputMeanAndCovar(samples, meanvec, covarmat);

  real scale=pow(product(diag(cX)), 1.0/D);
  
  V.resize(n_principal_components * L , D);
  for
fill_random_normal(Vconst Mat& dest, real mean=0, real sdev=1);  

Lh=randn(D*M,K)*sqrt(scale/K);
  Ph=diag(cX)+tiny;
  Pi=ones(M,1)/M;
  Mu=randn(M,D)*sqrtm(cX)+ones(M,1)*mX;
  oldMu=Mu;
  I=eye(K);

  lik=0;
  LL=[];

  H=zeros(N,M); 	% E(w|x) 
  EZ=zeros(N*M,K);
  EZZ=zeros(K*M,K);
  XX=zeros(D*M,D);
  s=zeros(M,1);
  const=(2*pi)^(-D/2);

}

*/

/*
void GaussMix::generateSpherical(Vec &x, int given_gaussian) const
{
  int l=0;
  if (given_gaussian<0)
    l = multinomial_sample(alpha);
  else if (given_gaussian<alpha.length())
    l = given_gaussian;
  else
    l = given_gaussian % alpha.length();

  fill_random_normal(x);
  tmpvec.fill(sqrt(sigma[l]));
  x*=tmpvec;
  x += mu(l);
}

void GaussMix::generateDiagonal(Vec &x, int given_gaussian) const
{
  int l=0;
  if (given_gaussian<0)
    l = multinomial_sample(alpha);
  else if (given_gaussian<alpha.length())
    l = given_gaussian;
  else
    l = given_gaussian % alpha.length();

  Vec lambda(diags(l));
  fill_random_normal(x);
  for(int i=0;i<D;i++)
    x[i]*=sqrt(lambda[i]);
  x += mu(l);
}

void GaussMix::generateGeneral(Vec &x, int given_gaussian) const
{
  int l=0;
  if (given_gaussian<0)
    l = multinomial_sample(alpha);
  else if (given_gaussian<alpha.length())
    l = given_gaussian;
  else
    l = given_gaussian % alpha.length();

  x=0;
  
  // the covariance matrix of the general gaussian type can be expressed as :
  // C = sum{i=1,K} [ lambda_i Vi Vi(t) ] + sum{i=K+1,D} [ lamda_0 Vi Vi(t) ]
  // Where Vi is the i-th eigenvector
  // this can also be reformulated as : 
  // C2 = sum{i=1,K} [ (lamda_i-lambda_0) Vi Vi(t) ] + diag(lambda_0) * I 
  // C2 = A + B
  // provided lambda_0 is smaller than all lambda_i's, C and C2 are equal 
  // since C2 and C are diagonalizable and share the same eigenvectors/values
  // Thus, to sample from a gaussian with covar. matrix C , we add two samples from a gaussian with covar. A and one with covar. B

  Vec norm(Ks[l]);
  fill_random_normal(norm);

  for(int i=0;i<Ks[l];i++)
  {
    Vec vi = V(V_idx[l]+i);
    real val = sqrt(lambda[V_idx[l]+i] - sigma[l]) * norm[i];
    for(int j=0;j<D;j++)
      x[j]+= vi[j] * val;
  }
  if(D-Ks[l]==0)return;
  // now add sample from N( 0,diag(lambda0) )
  norm.resize(D);
  fill_random_normal(norm);
  x += norm*sqrt(sigma[l]); // sigma[l] has lambda_0[l]
  x += mu(l);
}

void GaussMix::generateFactor(Vec &x, int given_gaussian) const
{
  int l=0;
  if (given_gaussian<0)
    l = multinomial_sample(alpha);
  else if (given_gaussian<alpha.length())
    l = given_gaussian;
  else
    l = given_gaussian % alpha.length();

  x=0;
  Vec norm(Ks[l]);
  fill_random_normal(norm);
  
  for(int i=0;i<Ks[l];i++)
  {
    Vec vi = V(V_idx[l]+i);
    for(int j=0;j<D;j++)
      x[j]+= vi[j] * norm[i];
  }
  norm.resize(D);
  fill_random_normal(norm);
  for(int i=0;i<D;i++)
    // diags(l) is inverse of noise vector gaussian l
    x[i] += norm[i]/diags(l)[i];
  x += mu(l);
}

*/

double GaussMix::log_density(const Vec& x) const
{ 
  /*
  if(type[0]=='S')
    return logDensitySpherical(x);
  else if(type[0]=='D')
    return logDensityDiagonal(x);
  else if(type[0]=='G')
    return logDensityGeneral(x);
  else if(type[0]=='F')
    return logDensityFactor(x);
  else if(type=="Unknown")
    PLERROR("You forgot to specify mixture type (Spherical, Diagonal, General, Factor).");
  else PLERROR("unknown mixtrure type");
  */
  return 0.0;
}

/*
double GaussMix::logDensitySpherical(const Vec& x, int num) const
{
  Vec x_minus_mu(x.length()), tmp(x.length());

  int begin= (num==-1?0:num);
  int end= (num==-1?L-1:num);

  Vec logs(end-begin+1);
  int idx=0;

  for(int l=begin;l<=end;l++)
  {
    logs[idx]=0;
    x_minus_mu =x-mu(l);
    logs[idx] += log(alpha[l]) + log_coef[l];
    transposeProduct(tmp, diagonalmatrix(Vec(D,1/sigma[l])), x_minus_mu); 
    // exponential part in multivariate normal density equation
    logs[idx++]+= -0.5 * dot(tmp, x_minus_mu);
  }
  
  return logadd(logs);
}
 
double GaussMix::logDensityDiagonal(const Vec& x, int num) const
{
  Vec x_minus_mu(x.length()), tmp(x.length()),tmp2(x.length());

  int begin= (num==-1?0:num);
  int end= (num==-1?L-1:num);

  Vec logs(end-begin+1);
  int idx=0;

  for(int l=begin;l<=end;l++)
  {
    logs[idx]=0;
    x_minus_mu =x-mu(l);
    logs[idx] += log(alpha[l]) + log_coef[l];
    tmp2<<diags(l);
    invertElements(tmp2);
    transposeProduct(tmp, diagonalmatrix(tmp2), x_minus_mu); 
    // exponential part in multivariate normal density equation
    logs[idx++]+= -0.5 * dot(tmp, x_minus_mu);
  }
 
  return logadd(logs);
}

// return log(p(x)) =  logadd {1..l} ( log(alpha[l]) + log_coeff[l] - 0.5 + q[l] )
// with q[l] = -0.5 * (x-mu[l])' V'.inv(D).V (x-mu[l]) = (V'(x-mu))'.inv(D).(V'(x-mu))
//     The expression q = (V'(x-mu))'.inv(D).(V'(x-mu)) can be understood as:
//        a) projecting vector x-mu on the orthonormal basis V, 
//           i.e. obtaining a transformed x that we shall call y:  y = V'(x-mu)
//           (y corresponds to x, expressed in the coordinate system V)
//           y_i = V'_i.(x-mu)
//
//        b) computing the squared norm of y , after first rescaling each coordinate by a factor 1/sqrt(lambda_i)
//           (i.e. differences in the directions with large lambda_i are given less importance)
//           Giving  q = sum_i[ 1/lambda_i  y_i^2]
//
//     If we only keep the first k eigenvalues, and replace the following d-k ones by the same value gamma
//     i.e.  lambda_k+1 = ... = lambda_d = gamma
//    
//     Then q can be expressed as:
//       q = \sum_{i=1}^k [ 1/lambda_i y_i^2 ]   +   1/gamma \sum_{i=k+1}^d [ y_i^2 ]
//
//     But, as y is just x expressed in another orthonormal basis, we have |y|^2 = |x-mu|^2
//     ( proof: |y|^2 = |V'(x-mu)|^2 = (V'(x-mu))'.(V'(x-mu)) = (x-mu)'.V.V'.(x-mu) = (x-mu)'(x-mu) = |x-mu|^2 )
//    
//     Thus, we know  \sum_{i=1}^d [ y_i^2 ] = |x-mu|^2
//     Thus \sum_{i=k+1}^d [ y_i^2 ] = |x-mu|^2 - \sum_{i=1}^k [ y_i^2 ]
//
//     Consequently: 
//       q = \sum_{i=1}^k [ 1/lambda_i y_i^2 ]   +  1/gamma ( |x-mu|^2 - \sum_{i=1}^k [ y_i^2 ] )
//
//       q = \sum_{i=1}^k [ (1/lambda_i - 1/gamma) y_i^2 ]  +  1/gamma  |x-mu|^2
//
//       q = \sum_{i=1}^k [ (1/lambda_i - 1/gamma) (V'_i.(x-mu))^2 ]  +  1/gamma  |x-mu|^2
//
//       This gives the efficient algorithm implemented below
double GaussMix::logDensityGeneral(const Vec& x, int num) const
{
  Vec x_minus_mu(x.length());

  int begin= (num==-1?0:num);
  int end= (num==-1?L-1:num);

  Vec logs(end-begin+1);
  int idx=0;
  for(int l=begin;l<=end;l++)
  {
    bool galette = Ks[l]<D;
    logs[idx]=log(alpha[l]);
    logs[idx]+= log_coef[l];
    x_minus_mu << x-mu(l);

    // compute q
    Mat subV = V.subMatRows(V_idx[l],Ks[l]);

    real* ptr_lambda = lambda.data() + V_idx[l];

    if(galette)
    {
      real* px = x.data();
      real* pmu = mu(l).data();
      
      real sqnorm_xmu = 0;
      for(int i=0; i<D; i++)
      {
        real val = *px++ - *pmu++;
        sqnorm_xmu += val * val;
      }
      
      logs[idx] -= 0.5/sigma[l] * sqnorm_xmu;
      for(int i=0; i<Ks[l]; i++)
        logs[idx] -= 0.5* (1.0 / *ptr_lambda++ - 1.0 / sigma[l]) *square(dot(subV(i),x_minus_mu));

    }
    else
      for(int i=0; i<Ks[l]; i++)
        logs[idx] -= 0.5* (1.0 / *ptr_lambda++) *square(dot(subV(i),x_minus_mu));

    
    idx++;
  }
  return logadd(logs);
}
 
double GaussMix::logDensityFactor(const Vec& x, int num) const
{
  Vec x_minus_mu(x.length());

  int begin= (num==-1?0:num);
  int end= (num==-1?L-1:num);

  Vec logs(end-begin+1);
  int idx=0;
  for(int l=begin;l<=end;l++)
  {

    logs[idx]=log(alpha[l]);
    logs[idx]+= log_coef[l];
    x_minus_mu << x-mu(l);
    
    int K=Ks[l];

    // The covariance matrix of a factor analyzer is (VV' + psi). Using the matrix inversion lemma, its inverse can be reexpressed as
    // inv_cov=(psi^-1 - psi^-1 * V * (I + V'*psi^-1*V) ^-1 * V' * psi^-1)             (precomupted)
    // here we compute log(p(x))= logcoef - 0.5 * ( (x-mu)' * inv_cov * (x-mu))
    //                                             
    //                                             
    // (see Frey, B.1999, Factor Analysis using batch online EM) : formula (9)
    
    real q=0; // will hold the factor (x-mu)'*inv_cov*(x-mu)
    Vec diag = diags(l);
    for(int i=0;i<D; i++)
      q+=x_minus_mu[i]*x_minus_mu[i]*diag[i];
    
    tmpvec.clear();
    tmpvec2.clear();
    
    Mat subv = V.subMatRows(V_idx[l],K);

    // tmpvec <- lambda(t) * psi^-1) * x

    for(int i=0;i<Ks[l]; i++)
    {
      Vec vrow=subv(i);
      for(int j=0;j<D;j++)
        tmpvec[i] += vrow[j] * diag[j] * x_minus_mu[j];
    }

    //tmpvec2 <- inv_ivtdv * tmpvec

    for(int i=0;i<Ks[l]; i++)
    {
      Vec row = inv_ivtdv.subVec( inv_ivtdv_idx[l] + i *K,K );
      for(int j=0;j<K;j++)
        tmpvec2[i] += row[j]*tmpvec[j];
    }

    //tmpvec <- lambda* tmpvec2
    tmpvec.clear();
    for(int j=0; j<K; j++)
    {
      real val = tmpvec2[j];
      Vec vrow=subv(j);
      for(int i=0; i<D; i++)
      {
        tmpvec[i]+= vrow[i] * val;
      }
    }
    
    //q -= x(t) * psi^-1 * tmpvec
    for(int i=0; i<D; i++)
      q-= tmpvec[i] * diag[i] * x_minus_mu[i];
    
    logs[idx] -= 0.5*q;
    idx++;
  }
  return logadd(logs);
}
*/

double GaussMix::survival_fn(const Vec& x) const
{ 
   PLERROR("survival_fn not implemented for GaussMix"); return 0.0; 
}

double GaussMix::cdf(const Vec& x) const
{ 
  PLERROR("cdf not implemented for GaussMix"); return 0.0; 
}

Vec GaussMix::expectation() const
{ 
  PLERROR("expectation not implemented for GaussMix"); return Vec(); 
}

Mat GaussMix::variance() const
{ 
  PLERROR("variance not implemented for GaussMix"); return Mat(); 
}


// for now, this function does not take sample weights into account

/*
real GaussMix::NLL(VMat dataset)
{
  real nll=0;
  Vec input(dataset->inputsize());
  Vec target(dataset->targetsize());
  real weight;
  for(int i=0;i<dataset.length();i++)
  {
    dataset->getExample(i,input,target,weight);
    nll += log_density(input);
  }
  return -nll;
}
*/

} // end of namespace PLearn

