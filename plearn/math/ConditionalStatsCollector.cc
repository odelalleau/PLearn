// -*- C++ -*-

// ConditionalStatsCollector.cc
//
// Copyright (C) 2003 Pascal Vincent 
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
   * $Id: ConditionalStatsCollector.cc,v 1.3 2004/02/18 01:19:46 plearner Exp $ 
   ******************************************************* */

// Authors: Pascal Vincent

/*! \file ConditionalStatsCollector.cc */


#include "ConditionalStatsCollector.h"

namespace PLearn <%
using namespace std;

ConditionalStatsCollector::ConditionalStatsCollector() 
  : inherited(),
    condvar(0) 
{}

  PLEARN_IMPLEMENT_OBJECT(ConditionalStatsCollector, "ONE LINE DESCRIPTION", "MULTI LINE\nHELP");

  void ConditionalStatsCollector::declareOptions(OptionList& ol)
  {
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify  
    // ### one of OptionBase::buildoption, OptionBase::learntoption or 
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    // ### ex:
    // declareOption(ol, "myoption", &ConditionalStatsCollector::myoption, OptionBase::buildoption,
    //               "Help text describing this option");
    // ...


    declareOption(ol, "condvar", &ConditionalStatsCollector::condvar, OptionBase::buildoption,
                  "index of conditioning variable \n");                  

    declareOption(ol, "ranges", &ConditionalStatsCollector::ranges, OptionBase::buildoption,
                  "ranges[k] must contain bin-mappings for variable k, \n"
                  "which maps it to an integer ( 0 to mappings[k].size()-1 ) \n");
                  

    declareOption(ol, "counts", &ConditionalStatsCollector::counts, OptionBase::learntoption,
                  "counts[k](i,j) is the number of times the variable k fell in \n"
                  "range i while variable condvar was in range j \n"
                  "counts[k] has one more row and column than there are mapping ranges:\n"
                  "the last ones counting MISSING_VALUE occurences.\n"
                  "Actually counts is the 'number of times' only when update is called \n"
                  "without a weight. Otherwise it's really the sum of the sample weights.");


    declareOption(ol, "sums", &ConditionalStatsCollector::sums, OptionBase::learntoption,
                  "sums[k](i,j) contains the sum of variable k's values that fell in range i while condvar was in range j \n"
                  "(unlike counts, these do not have an extra row and column for misisng value");

    declareOption(ol, "sums_condvar", &ConditionalStatsCollector::sums_condvar, OptionBase::learntoption,
                  "sums_condvar[k](i,j) contains the (possibly weighted) sum of variable condvar's values that fell in range i while variable k was in range j \n"
                  "(unlike counts, these do not have an extra row and column for misisng value)");

    declareOption(ol, "sumsquares", &ConditionalStatsCollector::sumsquares, OptionBase::learntoption,
                  "sumsquares[k](i,j) contains the (possibly weighted) sum of squares of variable k's values that fell in range i while condvar was in range j \n"
                  "(unlike counts, these do not have an extra row and column for misisng value)");

    declareOption(ol, "sumsquares_condvar", &ConditionalStatsCollector::sumsquares_condvar, OptionBase::learntoption,
                  "sumsquares_condvar[k](i,j) contains the (possibly weighted) sum of squares of condvar's values that fell in range i while variable k was in range j \n"
                  "(unlike counts, these do not have an extra row and column for misisng value)");

    declareOption(ol, "minima", &ConditionalStatsCollector::minima, OptionBase::learntoption,
                  "minima[k](i,j) contains the min of variable k's values that fell in range i while condvar was in range j \n"
                  "(unlike counts, these do not have an extra row and column for misisng value)");

    declareOption(ol, "minima_condvar", &ConditionalStatsCollector::minima_condvar, OptionBase::learntoption,
                  "minima_condvar[k](i,j) contains the min of variable condvar's values that fell in range i while variable k was in range j \n"
                  "(unlike counts, these do not have an extra row and column for misisng value)");

    declareOption(ol, "maxima", &ConditionalStatsCollector::maxima, OptionBase::learntoption,
                  "maxima[k](i,j) contains the max of variable k's values that fell in range i while condvar was in range j \n"
                  "(unlike counts, these do not have an extra row and column for misisng value)");

    declareOption(ol, "maxima_condvar", &ConditionalStatsCollector::maxima_condvar, OptionBase::learntoption,
                  "maxima_condvar[k](i,j) contains the max of variable condvar's values that fell in range i while variable k was in range j \n"
                  "(unlike counts, these do not have an extra row and column for misisng value)");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
  }

  void ConditionalStatsCollector::build_()
  {
    forget();
  }

  // ### Nothing to add here, simply calls build_
  void ConditionalStatsCollector::build()
  {
    inherited::build();
    build_();
  }

void ConditionalStatsCollector::forget()
{
  counts.resize(0);
  sums.resize(0);
  sumsquares.resize(0);
  minima.resize(0);
  maxima.resize(0);
  sums_condvar.resize(0);
  sumsquares_condvar.resize(0);
  minima_condvar.resize(0);
  maxima_condvar.resize(0);
}

void ConditionalStatsCollector::setBinMappingsAndCondvar(const TVec<RealMapping>& the_ranges, int the_condvar) 
{ 
  ranges = the_ranges;
  condvar = the_condvar;
  forget();
}

int ConditionalStatsCollector::findrange(int varindex, real val) const
{
  RealMapping& r = ranges[varindex];
  int pos = -1;
  if(is_missing(val))
    pos = r.length();
  else
    {
      pos = (int) r.map(val);
      /*
      if(pos==-1)
        {
          real minimum = r.begin()->first.low;
          real maximum = (--r.end())->first.high;

          PLWARNING("In ConditionalStatsCollector::findrange(%d, %.18g) value of variable not in mapping (min=%.18g, max=%.18g)",varindex,val,minimum,maximum);
          cerr << r << endl;

          if(val>maximum && val-maximum<1e-6)
            pos = r.length()-1;
          else if(val<minimum && minimum-val<1e-6)
            pos = 0;
        }
      */
    }
  return pos;
}
  
void ConditionalStatsCollector::update(const Vec& v, real weight)
{
  int nvars = ranges.length();
  if(v.length()!=nvars)
    PLERROR("IN ConditionalStatsCollectos::update length of update vector and nvars differ!");

  if(counts.length()!=nvars)
    {
      counts.resize(nvars);
      sums.resize(nvars);
      sums_condvar.resize(nvars);
      sumsquares.resize(nvars);
      sumsquares_condvar.resize(nvars);
      minima.resize(nvars);
      minima_condvar.resize(nvars);
      maxima.resize(nvars);
      maxima_condvar.resize(nvars);
      int nranges_condvar = ranges[condvar].length();
      for(int k=0; k<nvars; k++)
        {        
          int nranges_k = ranges[k].length();
          counts[k].resize(nranges_k+1, nranges_condvar+1);
          counts[k].fill(0);
          sums[k].resize(nranges_k, nranges_condvar);
          sums[k].fill(0);
          sums_condvar[k].resize(nranges_condvar, nranges_k);
          sums_condvar[k].fill(0);
          sumsquares[k].resize(nranges_k, nranges_condvar);
          sumsquares[k].fill(0);
          sumsquares_condvar[k].resize(nranges_condvar, nranges_k);
          sumsquares_condvar[k].fill(0);
          minima[k].resize(nranges_k, nranges_condvar);
          minima[k].fill(FLT_MAX);
          minima_condvar[k].resize(nranges_condvar, nranges_k);
          minima_condvar[k].fill(FLT_MAX);
          maxima[k].resize(nranges_k, nranges_condvar);
          maxima[k].fill(-FLT_MAX);
          maxima_condvar[k].resize(nranges_condvar, nranges_k);
          maxima_condvar[k].fill(-FLT_MAX);
        }
    }

  real condvar_val = v[condvar];
  int j = findrange(condvar, condvar_val);
  if(j==-1)
    PLWARNING("In ConditionalStatsCollector::update value of conditioning var in none of the ranges");
  for(int k=0; k<nvars; k++)
  {
    real val = v[k];
    int i = findrange(k, val);
    if(i==-1)
      {
        PLWARNING("In ConditionalStatsCollector::update value of variable #%d in none of the ranges",k);
      }

    counts[k](i,j)+=weight;
    if(!is_missing(val))
      {
        sums[k](i,j) += weight*val;
        sumsquares[k](i,j) += weight*square(val);
        if(val<minima[k](i,j))
          minima[k](i,j) = val;
        if(val>maxima[k](i,j))
          maxima[k](i,j) = val;
      }

    if(!is_missing(condvar_val))
      {
        sums_condvar[k](j,i) += weight*condvar_val;
        sumsquares_condvar[k](j,i) += weight*square(condvar_val);
        if(condvar_val<minima_condvar[k](j,i))
          minima_condvar[k](j,i) = condvar_val;
        if(condvar_val>maxima_condvar[k](j,i))
          maxima_condvar[k](j,i) = condvar_val;
      }
  }
}

  void ConditionalStatsCollector::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
  {
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(ranges, copies); 
    deepCopyField(counts, copies);
    deepCopyField(sums, copies); 
    deepCopyField(sumsquares, copies);
    deepCopyField(minima, copies);
    deepCopyField(maxima, copies);
    deepCopyField(sums_condvar, copies); 
    deepCopyField(sumsquares_condvar, copies);
    deepCopyField(minima_condvar, copies);
    deepCopyField(maxima_condvar, copies);
  }

%> // end of namespace PLearn
