// -*- C++ -*-

// ManifoldParzen2.cc
// 
// Copyright (C) 2003 Pascal Vincent, Julien Keable
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


/*! \file ManifoldParzen2.cc */
#include "ManifoldParzen2.h"

#include <plearn/math/plapack.h>
#include <plearn/base/general.h>
#include <plearn/math/TMat.h>
#include <plearn/math/TMat_maths.h>
#include <plearn/math/BottomNI.h>

namespace PLearn {

PLEARN_IMPLEMENT_OBJECT(ManifoldParzen2,
    "ManifoldParzen implements a manifold Parzen.",
    ""
);

/////////////////////
// ManifoldParzen2 //
/////////////////////
ManifoldParzen2::ManifoldParzen2()
: nneighbors(4),
  ncomponents(1),
  use_last_eigenval(true),
  scale_factor(1)
{
  nstages = 1;
}

ManifoldParzen2::ManifoldParzen2(int the_nneighbors, int the_ncomponents, bool use_last_eigenvalue, real the_scale_factor)
  : nneighbors(the_nneighbors),ncomponents(the_ncomponents),use_last_eigenval(true),scale_factor(the_scale_factor)
{
}

// ### Nothing to add here, simply calls build_
void ManifoldParzen2::build()
{
  inherited::build();
  build_();
}

// TODO Hide the options from GaussMix that are overwritten.

void ManifoldParzen2::build_()
{}

void ManifoldParzen2::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  PLearner::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("ManifoldParzen2::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

// This is an efficient version of the most basic nearest neighbor search, using a Mat and euclidean distance
void computeNearestNeighbors(Mat dataset, Vec x, Mat& neighbors, int ignore_row=-1)
{
  int K = neighbors.length(); // how many neighbors do we want?
  BottomNI<real> neighbs(K);
  for(int i=0; i<dataset.length(); i++)
    if(i!=ignore_row)
      neighbs.update(powdistance(dataset(i),x), i);
  neighbs.sort();
  TVec< pair<real,int> > indices = neighbs.getBottomN();
  int nonzero=0;
  for(int k=0; k<K; k++)
  {
    if(indices[k].first>0)
      nonzero++;
    neighbors(k) << dataset(indices[k].second);
  }
  if(nonzero==0)
    PLERROR("All neighbors had 0 distance. Use more neighbors. (There were %i other patterns with same values)",neighbs.nZeros());
}

// computes the first components.length() principal components of the rows of datset.
// Result will be put in the rows of components (which is expected to have the appropriate size).
// Computed components do not have a norm of 1, but rather a norm corresponding to the eigenvalues
// (indicating how important the component is...)
void computePrincipalComponents(Mat dataset, Vec& eig_values, Mat& eig_vectors)
{
#ifdef BOUNDCHECK
  if(eig_vectors.width()!=dataset.width())
    PLERROR("In computePrincipalComponents eig_vectors and dataset must have same width");    
  if(eig_values.length() != eig_vectors.length())
    PLERROR("In computePrincipalComponents eig_values vec and eig_vectors mat must have same length");
#endif

  static Mat covar;
  int ncomp = eig_values.length(); // number of components we want
  covar.resize(dataset.width(), dataset.width());
  transposeProduct(covar, dataset,dataset);
  eigenVecOfSymmMat(covar, ncomp,  eig_values, eig_vectors); 
  for (int i=0;i<eig_values.length();i++)
    if (eig_values[i]<0)
      eig_values[i] = 0;
}

void computeLocalPrincipalComponents(Mat& dataset, int which_pattern, Mat& delta_neighbors, Vec& eig_values, Mat& eig_vectors, Vec& mean)
{
  Vec center = dataset(which_pattern);
  if (center.hasMissing())
    PLERROR("dataset row %d has missing values!", which_pattern);
  computeNearestNeighbors(dataset, center, delta_neighbors, which_pattern);
  mean.resize(delta_neighbors.width());
  columnMean(delta_neighbors, mean);
  delta_neighbors -= mean;
  computePrincipalComponents(delta_neighbors, eig_values, eig_vectors);
}

void ManifoldParzen2::train()
{
  Mat trainset(train_set);
  int l = train_set.length();
  int w = train_set.width();
  
  type = "general";
  L = l;
  D = ncomponents;
  GaussMix::build();
  resizeStuffBeforeTraining();
//  setMixtureTypeGeneral(l, ncomponents, w); // TODO Remove this line when it works.

  // storage for neighbors
  Mat delta_neighbors(nneighbors, w);
  Vec eigvals(ncomponents+1);
  Mat components_eigenvecs(ncomponents+1,w);
  for(int i=0; i<l; i++)
  {
    if(i%100==0)
      cerr << "[SEQUENTIAL TRAIN: processing pattern #" << i << "/" << l << "]\n";
      
    // center is sample
    mu(i) << trainset(i);

    Vec center;
    computeLocalPrincipalComponents(trainset, i, delta_neighbors, eigvals, components_eigenvecs, center);

    eigvals *= scale_factor;

//    cout<<delta_neighbors<<endl;
    
    real d=0;
    for(int k=0;k<delta_neighbors.length();k++)
      d+=dist(delta_neighbors(k),Vec(D,0.0),2);
    d/=delta_neighbors.length();

    // find out eigenvalue (a.k.a lambda0) that will be used for all D-K directions
    real lambda0;
    if(use_last_eigenval)
    {
      // take last (smallest) eigenvalue as a variance in the non-principal directions
      // (but if it is 0 because of linear dependencies in the data, take the
      // last, i.e. smallest, non-zero eigenvalue).
      int last=ncomponents;
      lambda0 = eigvals[last];
      while (lambda0==0 && last>0)
        lambda0 = eigvals[--last];
      // the sigma-square for all remaining dimensions
      if (lambda0 == 0)
        PLERROR("All (%i) principal components have zero variance!?",eigvals.length());
    }
    else lambda0 = global_lambda0;

    alpha[i] = 1.0 / l;
    n_eigen = eigvals.length() - 1;
    GaussMix::build();
    resizeStuffBeforeTraining();
    mu(i) << center;
    eigenvalues(i) << eigvals;
    eigenvalues(i, n_eigen_computed - 1) = lambda0;
    eigenvectors[i] << components_eigenvecs;
//    setGaussianGeneral(i, 1.0/l, center, eigvals.subVec(0,eigvals.length()-1), components_eigenvecs.subMatRows(0,eigvals.length()-1), lambda0);
  }
  stage = 1;
  build();
}

} // end of namespace PLearn
