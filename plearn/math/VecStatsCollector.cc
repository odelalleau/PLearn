// -*- C++ -*-
// VecStatsCollector.cc
// 
// Copyright (C) 2002 Pascal Vincent
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
   * $Id: VecStatsCollector.cc,v 1.3 2003/04/06 23:22:38 plearner Exp $ 
   ******************************************************* */

/*! \file VecStatsCollector.cc */
#include "VecStatsCollector.h"
#include "TMat_maths.h"

namespace PLearn <%
using namespace std;

VecStatsCollector::VecStatsCollector() 
  :maxnvalues(0), compute_covariance(false)
  {}

IMPLEMENT_NAME_AND_DEEPCOPY(VecStatsCollector);

void VecStatsCollector::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify  
    // ### one of OptionBase::buildoption, OptionBase::learntoption or 
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

  declareOption(ol, "maxnvalues", &VecStatsCollector::maxnvalues, OptionBase::buildoption,
                "maximum number of different values to keep track of for each element");
  declareOption(ol, "compute_covariance", &VecStatsCollector::compute_covariance, OptionBase::buildoption,
                "should we compute and keep X'.X ?");

  declareOption(ol, "stats", &VecStatsCollector::stats, OptionBase::learntoption,
                "the stats for each element");
  declareOption(ol, "cov", &VecStatsCollector::cov, OptionBase::learntoption,
                "the uncentered covariance matrix (mean not subtracted): X'.X");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
  }

string VecStatsCollector::help() const
{
    // ### Provide some useful description of what the class is ...
    return 
      "VecStatsCollector allows to collect statistics on a series of vectors.\n"
      "Individual vectors x are presented by calling update(x), and this class will\n"
      "collect both individual statistics for each element (as a Vec<StatsCollector>)\n"
      "as well as (optionally) compute the covariance matrix."
      + optionHelp();
}

void VecStatsCollector::update(const Vec& x)
{
  int n = x.size();
  if(stats.size()==0)
    {
      stats.resize(n);
      for(int k=0; k<n; k++)
        {
          stats[k].maxnvalues = maxnvalues;
          stats[k].forget();
        }
      if(compute_covariance)
        {
          cov.resize(n,n);
          cov.fill(0);
        }      
    }

  if(stats.size()!=n)
    PLERROR("In VecStatsCollector: problem, called update with vector of length %d, while size of stats (and most likeley previously seen vector) is %d", n, stats.size());

  // this speeds things up a bit
  //bool has_missing=false;

  for(int k=0; k<n; k++)
  {
    stats[k].update(x[k]);
/*    if(is_missing(x[k]))
      x[k]=0;//has_missing=true;*/
  }
       
/*  if(compute_covariance)
    if(has_missing)
    {
      for(int i=0;i<n;i++)
        for(int j=0;j<n;j++)
          if(!is_missing(x[i]) && !is_missing(x[j]))
            cov(i,j)+=x[i]*x[j];
    }
    else*/ 
  externalProductAcc(cov, x, x);
}

void VecStatsCollector::forget()
{
  stats.resize(0);
  cov.resize(0,0);
}

void VecStatsCollector::finalize()
{
  int n = stats.size();
  for(int i=0; i<n; i++)
    stats[i].finalize();
}

//! returns the empirical mean (sample average) vec
Vec VecStatsCollector::getMean() const
{
  int n = stats.size();
  Vec res(n);
  for(int k=0; k<n; k++)
    res[k] = stats[k].mean();
  return res;
}

//! returns the empirical variance vec
Vec VecStatsCollector::getVariance() const
{
  int n = stats.size();
  Vec res(n);
  for(int k=0; k<n; k++)
    res[k] = stats[k].variance();
  return res;
}

//! returns the empirical standard deviation vec
Vec VecStatsCollector::getStdDev() const
{
  int n = stats.size();
  Vec res(n);
  for(int k=0; k<n; k++)
    res[k] = stats[k].stddev();
  return res;
}

//! returns the empirical standard deviation vec
Vec VecStatsCollector::getStdError() const
{
  int n = stats.size();
  Vec res(n);
  for(int k=0; k<n; k++)
    res[k] = stats[k].stderror();
  return res;
}

//! returns centered covariance matrix (mean subtracted)
Mat VecStatsCollector::getCovariance() const
{
  Vec meanvec = getMean();
  Mat covarmat = cov / real(stats[0].n());
  externalProductScaleAcc(covarmat,meanvec,meanvec,real(-1.));
  return covarmat;
}
  
//! returns correlation matrix
Mat VecStatsCollector::getCorrelation() const
{  
  Mat norm(cov.width(),cov.width());
  externalProduct(norm,getStdDev(),getStdDev());
  return getCovariance()/norm;
}

void VecStatsCollector::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  Object::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(stats, copies);
}

%> // end of namespace PLearn
