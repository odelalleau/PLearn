// -*- C++ -*-

// LimitedGaussianSmoother.cc
// 
// Copyright (C) 2002 Xavier Saint-Mleux
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
 * $Id$ 
 ******************************************************* */

/*! \file LimitedGaussianSmoother.cc */
#include "LimitedGaussianSmoother.h"
#include "pl_erf.h"

namespace PLearn {
using namespace std;

LimitedGaussianSmoother::LimitedGaussianSmoother() 
    :Smoother()
/* ### Initialise all fields to their default value */
{
    // ...

    // ### You may or may not want to call build_() to finish building the object
    // build_();
}

LimitedGaussianSmoother::LimitedGaussianSmoother(real window_size_wrt_sigma_, real sigma_bin_)
    :Smoother(), window_size_wrt_sigma(window_size_wrt_sigma_), sigma_bin(sigma_bin_)
{}

PLEARN_IMPLEMENT_OBJECT(LimitedGaussianSmoother, "ONE LINE DESCR", "NO HELP");

void LimitedGaussianSmoother::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify  
    // ### one of OptionBase::buildoption, OptionBase::learntoption or 
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    // ### ex:
    // declareOption(ol, "myoption", &LimitedGaussianSmoother::myoption, OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void LimitedGaussianSmoother::build_()
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
void LimitedGaussianSmoother::build()
{
    inherited::build();
    build_();
}


void LimitedGaussianSmoother::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields 
    // ### that you wish to be deepCopied rather than 
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("LimitedGaussianSmoother::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}


real LimitedGaussianSmoother::smooth(const Vec& source_function, Vec& smoothed_function, 
				     Vec bin_positions, Vec dest_bin_positions) const
{
    //parzen regressor?? kernel smoothing??
// smoothed_function[k] = sum_{j=max(0,k-window_size)}^{min(l-1,k+window_size)} w_{k,j} source_function[j]
//                        / sum_{j=max(0,k-window_size)}^{min(l-1,k+window_size)} w_{k,j} 
// with w_{k,j} = phi(bin_positions[j+1];mu_k,sigma_k)-phi(bin_positions[j];mu_k,sigma_k)
// where mu_k = 0.5*(bin_positions[k+1]+bin_positions[k]),
//       sigma_k = bin_positions[k+window_size]-bin_positions[k]
// where phi(x;mu,sigma) = cdf of normal(mu,sigma) at x,
// window_size = window_size_wrt_sigma * sigma_bin

// for dest_bin_positions != bin_positions: 2 methods:
// 1- trouver sigma_bin en fonction du voisinage
//    d'une position dest.  Somme ponderee avec 
//    gaussienne centree sur le pt. dest.
// 2- Un sigma_bin pour chaque bin_position (src).
//    Une gaussienne centree sur sur ch. pos src.
//    



    smoothed_function.resize(source_function.length());
    smoothed_function.fill(0.0);
    real window_size= window_size_wrt_sigma * sigma_bin;
    for(int i= 0; i < smoothed_function.length()-1; ++i)
    {
        int min_j= i-static_cast<int>(window_size), max_j= i+static_cast<int>(window_size);
        if(min_j < 0) min_j= 0;
        if(max_j > smoothed_function.length()) max_j= smoothed_function.length();
        real sum_weights= 0.0;
        real mu= 0.5*(bin_positions[i+1]+bin_positions[i]),
            sigma= bin_positions[max_j-1]-bin_positions[i];
        for(int j= min_j; j < max_j-1; ++j)
	{
            sum_weights+= gauss_cum(bin_positions[j+1], mu, sigma) -
                gauss_cum(bin_positions[j], mu, sigma);
	}
        for(int j= min_j; j < max_j-1; ++j)
            smoothed_function[i]+= ( gauss_cum(bin_positions[j+1], mu, sigma) 
                                     - gauss_cum(bin_positions[j], mu, sigma)
                ) 
                * source_function[j] / sum_weights;
    }

    return 0.0; //dummy - FIXME - xsm



/*
  if(bin_positions.length() != 0 && source_function.length() != bin_positions.length()-1)
  PLERROR("in LimitedGaussianSmoother::smooth  There must be one more bin_positions than the "
  "number of source_function points.");
  //if no bin_positions given, assume positions are 0, 1, 2, ..., n
  if(bin_positions.length() == 0)
  {
  int n= source_function.length()+1;
  bin_positions.resize(n);
  for(int i= 0; i < n; ++i)
  bin_positions[i]= i;
  }
  //if no dest_bin_positions given, assume same as bin_positions
  if(dest_bin_positions.length() == 0)
  dest_bin_positions= bin_positions;

  smoothed_function.resize(dest_bin_positions.length()-1);
  smoothed_function.fill(0.0);
  real window_size, mu, sigma, sum_weights;
  int n= smoothed_function.length();
  for(int i= 0; i < n; ++i)
  {
  sum_weights= 0.0;
  int nj= source_function.length();
  for(int j= 0; j < nj; ++j)
  {
  mu= 0.5*(bin_positions[i+1]+bin_positions[i]);
  sigma= bin_positions[i+1]-bin_positions[i];
  window_size= window_size_wrt_sigma * sigma;
  real p1= mu - 0.5*window_size,
  p2= mu + 0.5*window_size;

  Vec::iterator it = find_if(options.begin(), options.end(),
  bind2nd(mem_fun(&OptionBase::isOptionNamed), optionname));

	  

  }
      


  / *
  int min_j= i-static_cast<int>(window_size), max_j= i+static_cast<int>(window_size);
  if(min_j < 0) min_j= 0;
  if(max_j > smoothed_function.length()) max_j= smoothed_function.length();
  * /
  real sum_weights= 0.0;
  for(int j= min_j; j < max_j-1; ++j)
  {
  sum_weights+= gauss_cum(bin_positions[j+1], mu, sigma) -
  gauss_cum(bin_positions[j], mu, sigma);
  }
  for(int j= min_j; j < max_j-1; ++j)
  smoothed_function[i]+= ( gauss_cum(bin_positions[j+1], mu, sigma) 
  - gauss_cum(bin_positions[j], mu, sigma)
  ) 
  * source_function[j] / sum_weights;
  }
*/
}

} // end of namespace PLearn


/*
  Local Variables:
  mode:c++
  c-basic-offset:4
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:79
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=79 :
