// -*- C++ -*-

// NearestNeighborPredictionCost.cc
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
   * $Id: NearestNeighborPredictionCost.cc,v 1.1 2004/08/12 16:11:59 monperrm Exp $ 
   ******************************************************* */

// Authors: Martin Monperrus

/*! \file NearestNeighborPredictionCost.cc */


#include "NearestNeighborPredictionCost.h"
#include "plearn/var/ProjectionErrorVariable.h"
#include "plearn/vmat/LocalNeighborsDifferencesVMatrix.h"
#include "plearn/var/Func.h"
#include "plearn/vmat/AutoVMatrix.h"
#include "plearn/vmat/MemoryVMatrix.h"

namespace PLearn {
//using namespace std;
using namespace PLearn;


NearestNeighborPredictionCost::NearestNeighborPredictionCost() : test_set(AutoVMatrix("/u/monperrm/data/amat/gauss2D_200_0p001_1.amat"))
  /* ### Initialize all fields to their default value */
{
  // ...

  // ### You may or may not want to call build_() to finish building the object
  // build_();
}

PLEARN_IMPLEMENT_OBJECT(NearestNeighborPredictionCost, "ONE LINE DESCRIPTION", "MULTI LINE\nHELP");

void NearestNeighborPredictionCost::declareOptions(OptionList& ol)
{
  // ### Declare all of this object's options here
  // ### For the "flags" of each option, you should typically specify  
  // ### one of OptionBase::buildoption, OptionBase::learntoption or 
  // ### OptionBase::tuningoption. Another possible flag to be combined with
  // ### is OptionBase::nosave

  declareOption(ol, "knn", &NearestNeighborPredictionCost::knn, OptionBase::buildoption,
                 "Help text describing this option");
  declareOption(ol, "test_set", &NearestNeighborPredictionCost::test_set, OptionBase::buildoption,
                 "Help text describing this option");
  declareOption(ol, "learner_spec", &NearestNeighborPredictionCost::learner_spec, OptionBase::buildoption,
                 "Help text describing this option");
  
// ### ex:
  // declareOption(ol, "myoption", &NearestNeighborPredictionCost::myoption, OptionBase::buildoption,
  //               "Help text describing this option");
  // ...

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void NearestNeighborPredictionCost::build_()
{
  // ### This method should do the real building of the object,
  // ### according to set 'options', in *any* situation. 
  // ### Typical situations include:
  // ###  - Initial building of an object from a few user-specified options
  // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
  // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
  // ### You should assume that the parent class' build_() has already been called.
  
  PLearn::load(learner_spec,learner);
  learner->report_progress = false;
  cost.resize(knn);
  cost<<0;
  computed_outputs = new MemoryVMatrix(test_set->length(),learner->outputsize());
  learner->use(test_set,computed_outputs);

}

void NearestNeighborPredictionCost::run()
{

  VMat targets_vmat;
  int l = test_set->length();
  int n = test_set->width();
  int n_dim = learner->outputsize() / test_set->width();
  Var targets = Var(1,n);
  Var prediction = Var(n_dim,n);
  Var proj_err = projection_error(prediction, targets, 0, n);
  Func projection_error_f =  Func(prediction & targets, proj_err);
  Vec temp(n_dim*n);
  Vec temp2(n);

  for (int j=0; j<knn; ++j)
    {
      targets_vmat = local_neighbors_differences(test_set, j+1, true);
      for (int i=0;i<l;++i)
        {
          computed_outputs->getRow(i,temp);
          targets_vmat->getRow(i,temp2);
          //cout<<temp2<<"/ "<<temp<<" "<<dot(temp,temp2)<<endl;
          //cout<<projection_error_f(temp,temp2)<<endl;
          cost[j]+=projection_error_f(temp,temp2);
        }
    }
  cost/=l;
  cout<<"Cost = "<<cost<<endl;
  cout<<min(cost)<<endl;
//   printf("%.12f",cost);

}


// ### Nothing to add here, simply calls build_
void NearestNeighborPredictionCost::build()
{
  inherited::build();
  build_();
}

void NearestNeighborPredictionCost::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("NearestNeighborPredictionCost::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

} // end of namespace PLearn
