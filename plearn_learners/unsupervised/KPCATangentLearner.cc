// -*- C++ -*-

// KPCATangentLearner.cc
//
// Copyright (C) 2004 Martin Monperrus 
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
   * $Id: KPCATangentLearner.cc,v 1.1 2004/08/12 16:11:12 monperrm Exp $ 
   ******************************************************* */

// Authors: Martin Monperrus

/*! \file KPCATangentLearner.cc */

//
#include "KernelPCA.h"
#include "KPCATangentLearner.h"
#include "plearn/ker/GeodesicDistanceKernel.h"
#include "plearn/ker/AdditiveNormalizationKernel.h"
#include "plearn/ker/GaussianKernel.h"


namespace PLearn {
using namespace std;

KPCATangentLearner::KPCATangentLearner() : n_comp(2),sigma(1)
/* ### Initialize all fields to their default value here */
{
  // ...

  // ### You may or may not want to call build_() to finish building the object
  // build_();
}

PLEARN_IMPLEMENT_OBJECT(KPCATangentLearner, "Tangent learning based on KPCA Kernel", "MULTI-LINE \nHELP");

void KPCATangentLearner::declareOptions(OptionList& ol)
{
  // ### Declare all of this object's options here
  // ### For the "flags" of each option, you should typically specify  
  // ### one of OptionBase::buildoption, OptionBase::learntoption or 
  // ### OptionBase::tuningoption. Another possible flag to be combined with
  // ### is OptionBase::nosave
  
  
   declareOption(ol, "sigma", &KPCATangentLearner::sigma, OptionBase::buildoption,
                 "Sigma");
   declareOption(ol, "n_comp", &KPCATangentLearner::n_comp, OptionBase::buildoption,
                 "Number of Components");
   declareOption(ol, "KPCA", &KPCATangentLearner::KPCA, OptionBase::learntoption,
                 "");
  
  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void KPCATangentLearner::build_()
{
  // ### This method should do the real building of the object,
  // ### according to set 'options', in *any* situation. 
  // ### Typical situations include:
  // ###  - Initial building of an object from a few user-specified options
  // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
  // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
  // ### You should assume that the parent class' build_() has already been called.
}

// ### Nothing to add here, simply calls build_
void KPCATangentLearner::build()
{
  inherited::build();
  build_();
}


void KPCATangentLearner::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("KPCATangentLearner::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}


int KPCATangentLearner::outputsize() const
{
  // Compute and return the size of this learner's output (which typically
  // may depend on its inputsize(), targetsize() and set options).
  return inputsize()*n_comp;
}

void KPCATangentLearner::forget()
{
  //! (Re-)initialize the PLearner in its fresh state (that state may depend on the 'seed' option)
  //! And sets 'stage' back to 0   (this is the stage of a fresh learner!)
    /*!
      A typical forget() method should do the following:
         - initialize a random number generator with the seed option
         - initialize the learner's parameters, using this random generator
         - stage = 0
    */
}
    
void KPCATangentLearner::train()
{
    // The role of the train method is to bring the learner up to stage==nstages,
    // updating train_stats with training costs measured on-line in the process.
  KPCA.n_comp = n_comp;
  KPCA.kpca_kernel = new GaussianKernel(sigma);
  if (train_set)
    KPCA.setTrainingSet(train_set);
  KPCA.build();
  KPCA.train();
}


void KPCATangentLearner::computeOutput(const Vec& input, Vec& output) const
{
  PP<AdditiveNormalizationKernel> ank = dynamic_cast<AdditiveNormalizationKernel*>((Kernel*)KPCA.kernel);  
  PP<GaussianKernel> gk = dynamic_cast<GaussianKernel*>((Kernel*)ank->source_kernel);
  
  VMat trainset = ank->specify_dataset;
  int n_examples = trainset->length();
  Mat result(n_comp,inputsize());
  
  Vec dkdx(inputsize()); //dk/dx
  Vec temp(inputsize());
  Vec term2(inputsize());
  Vec sum(inputsize());
  Mat diK_dx(n_examples,inputsize());
  
  int i,j,nc;
  
  sum<<0;
  for(j=0;j<n_examples;++j) {
    trainset->getRow(j,temp);
    //real nt = norm(input-temp);
    // le noyau me renvoie ce que je veux mais les valeurs sont toutes petites: pb de sigma
    //cout<<gk->evaluate(temp,input)<<" "<<exp(-(nt*nt)/(sigma*sigma))<<endl;
    sum += gk->evaluate(temp,input)*(input-temp)/(sigma*sigma);
  }

  
  for(i=0;i<n_examples;++i)  {
    trainset->getRow(i,temp);
    term2 << gk->evaluate(temp,input)*(input-temp)/(sigma*sigma);      
    //cout<<term2<<endl;
    //cout<<sum;
    diK_dx(i) << (term2 - sum/n_examples); // on a le moins qui vient du moins de la dérivation de de exp(-)
    //diK_dx(i) << (sum/n_examples - term2); // exactement la formule de NIPS
    //cout<<diK_dx(i);
  }
  
  for(nc=0;nc<n_comp;++nc)
  {
    // compute the corresponding vector with the Nystrom formula
    // d ek / dx = 1/n sum_i dK/dX
    
    // initialisation
    temp<<(0);
    for(i=0;i<n_examples;++i)
    {
       temp += (KPCA.eigenvectors(nc,i) * diK_dx(i));
    }
    // on ne normalise pas car c'est la direction vecteur qui nous interesse et pas sa norme
    // en plus on normalise tout a 1 dans matlab pour eviter les erreurs numériques.
//     result(nc)<<(temp/iso_learner.eigenvalues[nc]);
        result(nc)<<(temp);
  }    
  //cout<<result; 
  // toVec: a mettre dans l'aide
  output << result.toVec();
  
}



void KPCATangentLearner::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                           const Vec& target, Vec& costs) const
{
// Compute the costs from *already* computed output. 
// ...
}                                

TVec<string> KPCATangentLearner::getTestCostNames() const
{
  // Return the names of the costs computed by computeCostsFromOutpus
  // (these may or may not be exactly the same as what's returned by getTrainCostNames).
  // ...
  return TVec<string>();
}
 
TVec<string> KPCATangentLearner::getTrainCostNames() const
{
  // Return the names of the objective costs that the train method computes and 
  // for which it updates the VecStatsCollector train_stats
  // (these may or may not be exactly the same as what's returned by getTestCostNames).
  // ...
  return TVec<string>();
}


} // end of namespace PLearn
