
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
   * $Id: PCA.cc,v 1.9 2004/03/30 19:35:59 tihocan Exp $ 
   ******************************************************* */

/*! \file PCA.cc */
#include "PCA.h"
#include "VMat_maths.h"
//#include "TMat_maths.h"
#include "plapack.h"

namespace PLearn {
using namespace std;

PCA::PCA() 
  :ncomponents(2),
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
                "The number of principal components to keep (that's also the outputsize)");
  
  declareOption(ol, "sigmasq", &PCA::sigmasq, OptionBase::buildoption,
                "this gets added to the diagonal of the covariance matrix prior to eigen-decomposition");
  
  declareOption(ol, "normalize", &PCA::normalize, OptionBase::buildoption, 
                "if true, we divide by sqrt(eigenval) after projecting on the eigenvec.");
  
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

  void PCA::build_()
  {
  }

  // ### Nothing to add here, simply calls build_
  void PCA::build()
  {
    inherited::build();
    build_();
  }


  void PCA::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
  {
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(eigenvals, copies);
    deepCopyField(eigenvecs, copies);
  }


int PCA::outputsize() const
{
  return ncomponents;
}

void PCA::forget()
{
  stage = 0;
}
    
void PCA::train()
{
  static Mat covarmat;

  if(stage<1)
    {
      ProgressBar* pb = 0;
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
      if (pb)
        delete pb;
    }
}

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

void PCA::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                  const Vec& target, Vec& costs) const
{
  static Vec reconstructed_input;
  reconstruct(output, reconstructed_input);
  costs.resize(1);
  costs[0] = powdistance(input, reconstructed_input);
}                                

TVec<string> PCA::getTestCostNames() const
{
  return TVec<string>(1,"squared_reconstruction_error");
}

TVec<string> PCA::getTrainCostNames() const
{
  return TVec<string>();
}



} // end of namespace PLearn
