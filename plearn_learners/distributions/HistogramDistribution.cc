// -*- C++ -*-

// HistogramDistribution.cc
// 
// Copyright (C) 2002 Yoshua Bengio, Pascal Vincent, Xavier Saint-Mleux
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
   * $Id: HistogramDistribution.cc,v 1.9 2003/08/13 08:13:46 plearner Exp $ 
   ******************************************************* */

/*! \file HistogramDistribution.cc */
#include "HistogramDistribution.h"
#include <algorithm>
#include <cmath>

namespace PLearn <%
using namespace std;

HistogramDistribution::HistogramDistribution() 
  :Distribution(), bin_positions(), bin_density(), survival_values(), the_binner(0),
   the_smoother(0), smooth_density(true)
  {
    // ### Possibly call setTestCostFunctions(...) to define the cost functions 
    // ### you are interested in (these are used by the default useAndCost() method,
    // ### which is called by the default test() method).
    // ### ex: 
    // setTestCostFunctions(squared_error());

    // ### You may also call setTestStatistics(...) if the Learner-default 'mean' and 'stderr' 
    // ### statistics are not appropriate...

    // ### You may or may not want to call build_() to finish building the object
    // build_();

    // ### You may want to set the Distribution::use_returns_what parameter
  }

HistogramDistribution::HistogramDistribution(VMat data, PP<Binner> the_binner_, 
					     PP<Smoother> the_smoother_, bool smooth_density_) 
  :Distribution(), bin_positions(data.length()+1), bin_density(data.length()), survival_values(data.length()),
   the_binner(the_binner_), the_smoother(the_smoother_), smooth_density(smooth_density_)
{
  train(data);
}

  PLEARN_IMPLEMENT_OBJECT(HistogramDistribution, "ONE LINE DESCR", "NO HELP");

  void HistogramDistribution::declareOptions(OptionList& ol)
  {
    declareOption(ol, "bin_positions", &HistogramDistribution::bin_positions, OptionBase::learntoption,
                  "The n+1 positions that define n bins. There is one more bin position "
		  "than number of bins, all the bins are supposed adjacent.");

    declareOption(ol, "bin_density", &HistogramDistribution::bin_density, OptionBase::learntoption,
                  "Density of the distribution for each bin.  The density is supposed "
		  "constant within each bin:\n"
		  "\tp(x) = bin_density[i] if bin_positions[i] < x <= bin_positions[i+1].");

    declareOption(ol, "survival_values", &HistogramDistribution::survival_values, OptionBase::learntoption,
                  "Redundant with density is the pre-computed survival function.");

    declareOption(ol, "the_binner", &HistogramDistribution::the_binner, OptionBase::buildoption,
                  "Used to do binning at training time.");

    declareOption(ol, "the_smoother", &HistogramDistribution::the_smoother, OptionBase::buildoption,
                  "Used to smooth learned density (or survival) at train time.");

    declareOption(ol, "smooth_density", &HistogramDistribution::smooth_density, OptionBase::buildoption,
                  "Indicates wether to smooth the density or the survival.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
  }

  string HistogramDistribution::help()
  {
    // ### Provide some useful description of what the class is ...
    return 
      "HistogramDistribution implements a ..."
      + optionHelp();
  }

  void HistogramDistribution::build_()
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
  void HistogramDistribution::build()
  {
    inherited::build();
    build_();
  }


  void HistogramDistribution::train(VMat training_set)
  { 

    /*
      - prend la distri empirique
      | trie les points
      | merge les bins (possiblement sous contraintes)
      |     - points de coupure predefinis (option include_cutpoints) ManualBinner
      |     - largeur des bins > a une valeur minimale 
      |     - bins contenir un minimum de points
      Binner
      
      Smoother
      (recalcule la densite)
      
      calculer survival_values
    */

    if(training_set->width() != inputsize()+targetsize())
      PLERROR("In HistogramDistribution::train(VMat training_set) training_set->width() != inputsize()+targetsize()");
    if(training_set->width() != 1)
      PLERROR("In HistogramDistribution::train(VMat training_set) training_set->width() must be 1 (column vec.)");
    if(the_binner == 0)
      PLERROR("In HistogramDistribution::train(VMat training_set) Can't train without a Binner.");

    setTrainingSet(training_set);

    Vec data(training_set.length());
    data << training_set.getColumn(training_set.width()-1);

    PP<RealMapping> binning= the_binner->getBinning(training_set);
    binning->setMappingForOther(0.0);
    binning->transform(data);


    bin_positions= binning->getCutPoints();
    bin_density.resize(bin_positions.length()-1);
    survival_values.resize(bin_positions.length()-1);

    for(int i= 0; i < data.length(); ++i)
      ++survival_values[static_cast<int>(data[i])];
    for(int i= survival_values.length()-2; i >= 0; --i)
      survival_values[i]+= survival_values[i+1];
    for(int i= survival_values.length()-1; i >= 0; --i)
      survival_values[i]/= survival_values[0];

    if(the_smoother)
      if(smooth_density)
	{
	  calc_density_from_survival();
	  Vec df(bin_density.length());
	  df << bin_density;
	  the_smoother->smooth(df, bin_density, bin_positions, bin_positions);
	  calc_survival_from_density();
	}
      else
	{
	  Vec sv(survival_values.length());
	  sv << survival_values;
	  the_smoother->smooth(sv, survival_values, bin_positions, bin_positions);
	  calc_density_from_survival();
	}
    else
      calc_density_from_survival();
  }

  void HistogramDistribution::use(const Vec& input, Vec& output)
  {
    if(input.size() != 1 || output.size() != 1)
      PLERROR("In HistogramDistribution::use  implemented only for reals; i.e. input.size()=output.size()=1.  "
	      "Got input.size()=%d and output.size()=%d", input.size(), output.size());
    // use_returns_what: 'l'->log_density, 'd' -> density, 'c' -> cdf, 's' -> survival_fn, 'e' -> expectation, 'v' -> variance
    if(use_returns_what == "l") output[0]= log_density(input);
    else if(use_returns_what == "d") output[0]= density(input);
    else if(use_returns_what == "c") output[0]= cdf(input);
    else if(use_returns_what == "s") output[0]= survival_fn(input);
    else if(use_returns_what == "e") output[0]= expectation();
    else if(use_returns_what == "v") output[0]= variance();
    else PLERROR("In HistogramDistribution::use  unknown value for use_returns_what= \"%s\"", use_returns_what.c_str());
  }

  void HistogramDistribution::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
  {
    Learner::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields 
    // ### that you wish to be deepCopied rather than 
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("HistogramDistribution::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
  }

double HistogramDistribution::log_density(const Vec& x) const
{ 
  return log(density(x));
}


double HistogramDistribution::density(const Vec& x) const
{  
  if(x.size() != 1)
    PLERROR("density implemented only for reals (vec size == 1).");

  return bin_density[find_bin(x[0])];
}


double HistogramDistribution::survival_fn(const Vec& x) const
{ 
  if(x.size() != 1)
    PLERROR("survival_fn implemented only for reals (vec size == 1).");

  int bin= find_bin(x[0]);
  if(bin < 0)
    if(x[0] < bin_positions[0])
      return 1.0;
    else
      return 0.0;
  
  if(x[0] < bin_positions[bin] && bin >= 1)
    return survival_values[bin-1] + (x[0] - bin_positions[bin-1]) * 
      (survival_values[bin] - survival_values[bin-1]) / (bin_positions[bin] - bin_positions[bin-1]);
  
  return survival_values[bin];
}

double HistogramDistribution::cdf(const Vec& x) const
{ 
  return 1.0-survival_fn(x);
}

double HistogramDistribution::expectation() const
{ 
  real sum= 0.0;
  for(int i= 0; i < bin_density.size(); ++i)
    sum+= bin_density[i] * (bin_positions[i+1]-bin_positions[i]) * (bin_positions[i]+bin_positions[i+1])/2;
    //    sum+= bin_density[i] * bin_positions[i+1];
  return sum; 
}

double HistogramDistribution::variance() const
{ 
  real sumsq= 0.0, sum= 0.0, s;
  int n= bin_density.size();
  for(int i= 0; i < n; ++i)
    {
      s= bin_density[i] * (bin_positions[i+1]-bin_positions[i]) * (bin_positions[i]+bin_positions[i+1])/2;
      sum+= s;
      sumsq+= s*s;
    }
  return abs(sumsq-(sum*sum)/n)/n;
}

double HistogramDistribution::prob_in_range(const Vec& x0, const Vec& x1) const
{
  return survival_fn(x0) - survival_fn(x1);
}


int HistogramDistribution::find_bin(real x) const
{
  int b= 0, e= bin_positions.length()-2, p= b+(e-b)/2;

  if(x < bin_positions[b] || x >= bin_positions[e+1])
    return -1;

  while(b < e)
    {
      if(bin_positions[p] == x)
	return p;
      if(bin_positions[p] > x)
	e= p-1;
      else
	b= p+1;
      p= b+(e-b)/2;
    }
  return p;
}

void HistogramDistribution::calc_density_from_survival()
{
  calc_density_from_survival(survival_values, bin_density, bin_positions);
  /*
  int n= bin_positions.length()-1;
  bin_density.resize(n);
  real sum= 0.0;
  for(int i= 0; i < n; ++i)
    if(bin_positions[i+1] != bin_positions[i])
      if(i == n-1)
	sum+= (bin_density[i]= survival_values[i] / (bin_positions[i+1]-bin_positions[i]));
      else
	sum+= (bin_density[i]= (survival_values[i] - survival_values[i+1]) / (bin_positions[i+1]-bin_positions[i]));
    else
      bin_density[i]= 0.0;
  */
}


void HistogramDistribution::calc_survival_from_density()
{
  calc_survival_from_density(bin_density, survival_values, bin_positions);
  /*
  int n= bin_positions.length()-1;
  survival_values.resize(n);
  real prec= 0.0;
  for(int i= n-1; i >= 0; --i)
    prec= survival_values[i]= bin_density[i]*(bin_positions[i+1]-bin_positions[i]) + prec;
  for(int i= 0; i < n; ++i)
    survival_values[i]/= prec;
  */
}

void HistogramDistribution::calc_density_from_survival(const Vec& survival, Vec& density_, const Vec& positions)
{
  int n= positions.length()-1;
  density_.resize(n);
  real sum= 0.0;
  for(int i= 0; i < n; ++i)
    if(positions[i+1] != positions[i])
      if(i == n-1)
	sum+= (density_[i]= survival[i] / (positions[i+1]-positions[i]));
      else
	sum+= (density_[i]= (survival[i] - survival[i+1]) / (positions[i+1]-positions[i]));
    else
      density_[i]= 0.0;
}

void HistogramDistribution::calc_survival_from_density(const Vec& density_, Vec& survival, const Vec& positions)
{
  int n= positions.length()-1;
  survival.resize(n);
  real prec= 0.0;
  for(int i= n-1; i >= 0; --i)
    prec= survival[i]= density_[i]*(positions[i+1]-positions[i]) + prec;
  for(int i= 0; i < n; ++i)
    survival[i]/= prec;
}


%> // end of namespace PLearn
