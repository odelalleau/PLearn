
// -*- C++ -*-

// ClassifierFromDensity.cc
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
   * $Id: ClassifierFromDensity.cc,v 1.1 2003/06/06 05:23:52 plearner Exp $ 
   ******************************************************* */

/*! \file ClassifierFromDensity.cc */
#include "ClassifierFromDensity.h"
#include "VMat_maths.h"

namespace PLearn <%
using namespace std;

ClassifierFromDensity::ClassifierFromDensity() 
  :nclasses(-1)
  {
    // build_();
  }

  PLEARN_IMPLEMENT_OBJECT_METHODS(ClassifierFromDensity, "ClassifierFromDensity", PLearner);

  void ClassifierFromDensity::declareOptions(OptionList& ol)
  {
    declareOption(ol, "nclasses", &ClassifierFromDensity::nclasses, OptionBase::buildoption,
                  "The number of classes");
    declareOption(ol, "estimators", &ClassifierFromDensity::estimators, OptionBase::buildoption,
                  "The array of density estimators, one for each class. \n"
                  "You may also specify just one that will be replicated as many times as there are classes.");
    declareOption(ol, "log_priors", &ClassifierFromDensity::log_priors, OptionBase::buildoption,
                  "The log of the class prior probabilities");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
  }

  string ClassifierFromDensity::help()
  {
    return "ClassifierFromDensity allowd to build a classifier\n"
      "by building one density estimator for each class, \n"
      "and using Bayes rule to combine them. \n";
  }

  void ClassifierFromDensity::build_()
  {
  if(estimators.size()==1)
    {
      estimators.resize(nclasses);
      for(int i=1; i<nclasses; i++)
        estimators[i] = PLearn::deepCopy(estimators[0]);
    }
  else if(estimators.size()!=nclasses)
    PLERROR("In ClassifierFromDensity: specified %d estimators but there are %d classes",estimators.size(), nclasses);
  }

  // ### Nothing to add here, simply calls build_
  void ClassifierFromDensity::build()
  {
    inherited::build();
    build_();
  }


  void ClassifierFromDensity::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
  {
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields 
    // ### that you wish to be deepCopied rather than 
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("ClassifierFromDensity::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
  }


int ClassifierFromDensity::outputsize() const
{
  return nclasses;
}

void ClassifierFromDensity::forget()
{
  for(int c=0; c<estimators.length(); c++)
    estimators[c]->forget();
}
    
void ClassifierFromDensity::train()
{
  if(targetsize()!=1)
    PLERROR("In ClassifierFromDensity: expecting a targetsize of 1 (class index between 0 and nclasses-1), not %d !!",targetsize());

  if(!train_stats)  // make a default stats collector, in case there's none
    train_stats = new VecStatsCollector();

  if(nstages<stage) // asking to revert to a previous stage!
    forget();  // reset the learner to stage=0

  if(stage==0)
    {
      log_priors.resize(nclasses);
      
      map<real, TVec<int> > indices = indicesOfOccurencesInColumn(train_set, inputsize());
      
      for(int c=0; c<nclasses; c++)
        log_priors[c] = log(real(indices[real(c)].length())) - log(real(train_set.length())); // how many do we have?

      string expd = getExperimentDirectory();
      
      for(int c=0; c<nclasses; c++)
        {
          if(verbosity>=1)
            cerr << ">>> Training class " << c;
          VMat set_c = train_set.rows(indices[c]).subMatColumns(0,inputsize());
          if(expd!="")
            estimators[c]->setExperimentDirectory(expd+"Class"+tostring(c));
          if(verbosity>=1)
            cerr << " ( " << set_c.length() << " samples)" << endl;
          estimators[c]->setTrainingSet(set_c);
          estimators[c]->train();
        }
      stage = 1; // trained!
    }
}

void ClassifierFromDensity::computeOutput(const Vec& input, Vec& output) const
{
  output.resize(nclasses);

  real logprob;
  Vec logprobvec(1,&logprob);
  double log_of_sumprob = 0.;

  for(int c=0; c<nclasses; c++)
    {
      estimators[c]->computeOutput(input, logprobvec);
      double logprob_c = logprob + log_priors[c]; // multiply p by the prior
      output[c] = logprob_c;
      if(c==0)
        log_of_sumprob = logprob_c;
      else
        log_of_sumprob = logadd(log_of_sumprob, logprob_c);
    }      

  // cerr << "unnormalized logprob: " << output << endl;
  // cerr << "log of sumprob: " << log_of_sumprob << endl;
      
  output -= log_of_sumprob; // divide by the sum
      
  // make it probabilities rather than log probabilities...
  exp(output, output);
      
  // cout << "output: " << output << endl;
  // cout << "argmax: " << argmax(output) << endl;
}

void ClassifierFromDensity::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                           const Vec& target, Vec& costs) const
{
  static CostFunc cl_er;
  static CostFunc cond_p;
  if(!cl_er)
    cl_er = class_error();
  if(!cond_p)
    cond_p = condprob_cost();

  costs.resize(2);
  costs[0] = cl_er->evaluate(output, target);
  costs[1] = cond_p->evaluate(output, target);
}                                

TVec<string> ClassifierFromDensity::getTestCostNames() const
{
  TVec<string> cnames(2);
  cnames[0] = "class_error";
  cnames[1] = "condprob_cost";
  return cnames;
}

TVec<string> ClassifierFromDensity::getTrainCostNames() const
{
  return TVec<string>();
}



%> // end of namespace PLearn
