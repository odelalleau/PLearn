// -*- C++ -*-

// PCA.cc
//
// Copyright (C) 2003  Pascal Vincent 
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
   * $Id: PCA.cc,v 1.10 2004/06/29 13:28:11 tihocan Exp $ 
   ******************************************************* */

/*! \file PCA.cc */
#include "CenteredVMatrix.h"
#include "PCA.h"
#include "plapack.h"
#include "random.h"     //!< For fill_random_normal.
#include "VMat_maths.h"

namespace PLearn {
using namespace std;

PCA::PCA() 
: algo("classical"),
  ncomponents(2),
  sigmasq(0),
  normalize(true)
{
}

PLEARN_IMPLEMENT_OBJECT(PCA, 
                        "Performs a Principal Component Analysis preprocessing (projecting on the principal directions)", 
                        "This learner finds the empirical covariance matrix of the input part of the training data"
                        "and learns to project its input vectors along the principal eigenvectors"
                        "of that matrix, optionally scaling by the inverse of the square root"
                        "of the eigenvalues (to obtained 'sphered', i.e. Normal(0,I) data)");

void PCA::declareOptions(OptionList& ol)
{
  declareOption(ol, "ncomponents", &PCA::ncomponents, OptionBase::buildoption,
                "The number of principal components to keep (that's also the outputsize).");
  
  declareOption(ol, "sigmasq", &PCA::sigmasq, OptionBase::buildoption,
                "This gets added to the diagonal of the covariance matrix prior to eigen-decomposition.");
  
  declareOption(ol, "normalize", &PCA::normalize, OptionBase::buildoption, 
                "If true, we divide by sqrt(eigenval) after projecting on the eigenvec.");
  
  declareOption(ol, "algo", &PCA::algo, OptionBase::buildoption,
      "The algorithm used to perform the Principal Component Analysis:\n"
      " - 'classical' : compute the eigenvectors of the covariance matrix\n"
      " - 'em'        : EM algorithm from \"EM algorithms for PCA and SPCA\" by S. Roweis");
  
  // saved options
  declareOption(ol, "mu", &PCA::mu, OptionBase::learntoption,
                "The (weighted) mean of the samples");
  declareOption(ol, "eigenvals", &PCA::eigenvals, OptionBase::learntoption,
                "The ncomponents eigenvalues corresponding to the principal directions kept");
  declareOption(ol, "eigenvecs", &PCA::eigenvecs, OptionBase::learntoption,
                "A ncomponents x inputsize matrix containing the principal eigenvectors");
  
  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

///////////
// build //
///////////
void PCA::build()
{
  inherited::build();
  build_();
}

////////////
// build_ //
////////////
void PCA::build_()
{
}

/////////////////////////////
// computeCostsFromOutputs //
/////////////////////////////
void PCA::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                  const Vec& target, Vec& costs) const
{
  static Vec reconstructed_input;
  reconstruct(output, reconstructed_input);
  costs.resize(1);
  costs[0] = powdistance(input, reconstructed_input);
}                                

///////////////////
// computeOutput //
///////////////////
void PCA::computeOutput(const Vec& input, Vec& output) const
{
  static Vec x;
  x.resize(input.length());
  x << input;
  x -= mu;
  output.resize(ncomponents);

  if(normalize)
  {
    for(int i=0; i<ncomponents; i++)
      output[i] = dot(x,eigenvecs(i)) / sqrt(eigenvals[i]);
  }
  else
  {
    for(int i=0; i<ncomponents; i++)
      output[i] = dot(x,eigenvecs(i));
  }
}    

////////////
// forget //
////////////
void PCA::forget()
{
  stage = 0;
}
    
//////////////////////
// getTestCostNames //
//////////////////////
TVec<string> PCA::getTestCostNames() const
{
  return TVec<string>(1,"squared_reconstruction_error");
}

///////////////////////
// getTrainCostNames //
///////////////////////
TVec<string> PCA::getTrainCostNames() const
{
  return TVec<string>();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void PCA::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(mu, copies);
  deepCopyField(eigenvals, copies);
  deepCopyField(eigenvecs, copies);
}


////////////////
// outputsize //
////////////////
int PCA::outputsize() const
{
  return ncomponents;
}

///////////
// train //
///////////
void PCA::train()
{
  static Mat covarmat;
  if(stage<1)
  {
    ProgressBar* pb = 0;
    if (algo == "classical") {
      if (report_progress) {
        pb = new ProgressBar("Training PCA", 2);
      }
      computeInputMeanAndCovar(train_set, mu, covarmat);
      if (pb)
        pb->update(1);
      eigenVecOfSymmMat(covarmat, ncomponents, eigenvals, eigenvecs);      
      if (pb)
        pb->update(2);
      stage = 1;
    } else if (algo == "em") {
      int n = train_set->length();
      int p = train_set->inputsize();
      int k = ncomponents;
      // Fill the matrix C with random data.
      Mat C(k,p);
      fill_random_normal(C);
      // Center the data.
      VMat centered_data = new CenteredVMatrix(train_set);
  //      mu = toreal(centered_data->getOption("mu")); // TODO Use getOption here.
      Vec sample_mean = static_cast<CenteredVMatrix*>((VMatrix*) centered_data)->getMu();
      mu.resize(sample_mean.length());
      mu << sample_mean;
      Mat Y = centered_data.toMat();
      Mat X(n,k);
      Mat tmp_k_k(k,k);
      Mat tmp_k_k_2(k,k);
      Mat tmp_p_k(p,k);
      Mat tmp_k_n(k,n);
      // Iterate through EM.
      if (report_progress)
        pb = new ProgressBar("Training EM PCA", nstages - stage);
      int init_stage = stage;
      while (stage < nstages) {
        // E-step: X <- Y C (C C')^-1
        productTranspose(tmp_k_k, C, C);
        matInvert(tmp_k_k, tmp_k_k_2);
        transposeProduct(tmp_p_k, C, tmp_k_k_2);
        product(X, Y, tmp_p_k);
        // M-step: C <- (X' X)^-1 X' Y
        transposeProduct(tmp_k_k, X, X);
        matInvert(tmp_k_k, tmp_k_k_2);
        productTranspose(tmp_k_n, tmp_k_k_2, X);
        product(C, tmp_k_n, Y);
        stage++;
        if (report_progress)
          pb->update(stage - init_stage);
      }
      // Compute the orthonormal projection matrix.
      int n_base = GramSchmidtOrthogonalization(C);
      if (n_base != k) {
        PLWARNING("In PCA::train - The rows of C are not linearly independent");
      }
      // Compute the projected data.
      productTranspose(X, Y, C);
      // And do a PCA to get the eigenvectors and eigenvalues.
      PCA true_pca;
      VMat proj_data(X);
      true_pca.ncomponents = k;
      true_pca.normalize = 0;
      true_pca.setTrainingSet(proj_data);
      true_pca.train();
      // Transform back eigenvectors to input space.
      eigenvecs.resize(k, p);
      product(eigenvecs, true_pca.eigenvecs, C);
      eigenvals.resize(k);
      eigenvals << true_pca.eigenvals;
    } else {
      PLERROR("In PCA::train - Unknown value for 'algo'");
    }
    if (pb)
      delete pb;
  } else {
    PLWARNING("In PCA::train - The learner has already been train, skipping training");
  }
}


/////////////////
// reconstruct //
/////////////////
void PCA::reconstruct(const Vec& output, Vec& input) const
{
  input.resize(mu.length());
  input << mu;

  int n = output.length();
  if(normalize)
    {
      for(int i=0; i<n; i++)
        multiplyAcc(input, eigenvecs(i), output[i]*sqrt(eigenvals[i]));
    }
  else
    {
      for(int i=0; i<n; i++)
        multiplyAcc(input, eigenvecs(i), output[i]);
    }
}

} // end of namespace PLearn
