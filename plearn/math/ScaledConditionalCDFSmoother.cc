

// -*- C++ -*-

// ScaledConditionalCDFSmoother.cc
// 
// Copyright (C) *YEAR* *AUTHOR(S)* 
// ...
// Copyright (C) *YEAR* *AUTHOR(S)* 
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
   * $Id: ScaledConditionalCDFSmoother.cc,v 1.1 2002/12/02 22:11:00 zouave Exp $ 
   ******************************************************* */

/*! \file ScaledConditionalCDFSmoother.cc */

#include "ScaledConditionalCDFSmoother.h"

namespace PLearn <%
using namespace std;

ScaledConditionalCDFSmoother::ScaledConditionalCDFSmoother() 
  :ConditionalCDFSmoother() 
/* ### Initialise all fields to their default value */
  {
    // ...

    // ### You may or may not want to call build_() to finish building the object
    // build_();
  }

ScaledConditionalCDFSmoother::ScaledConditionalCDFSmoother(PP<HistogramDistribution>& prior_cdf)
  :ConditionalCDFSmoother(prior_cdf)
{}  

  IMPLEMENT_NAME_AND_DEEPCOPY(ScaledConditionalCDFSmoother);

  void ScaledConditionalCDFSmoother::declareOptions(OptionList& ol)
  {
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify  
    // ### one of OptionBase::buildoption, OptionBase::learntoption or 
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    // ### ex:
    // declareOption(ol, "myoption", &ScaledConditionalCDFSmoother::myoption, OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
  }

  string ScaledConditionalCDFSmoother::help() const
  {
    // ### Provide some useful description of what the class is ...
    return 
      "ScaledConditionalCDFSmoother implements a ..."
      + optionHelp();
  }

  void ScaledConditionalCDFSmoother::build_()
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
  void ScaledConditionalCDFSmoother::build()
  {
    inherited::build();
    build_();
  }


  void ScaledConditionalCDFSmoother::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
  {
    Object::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields 
    // ### that you wish to be deepCopied rather than 
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("ScaledConditionalCDFSmoother::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
  }


// To obtain each bin of the smoothed_function, scale multiplicatively each bin
// density of prior_cdf by a fixed factor per source_function bin, that
// makes it match the probability under the corresponding source_function
// bin.
real ScaledConditionalCDFSmoother::smooth(const Vec& source_function, Vec& smoothed_function, 
		      Vec bin_positions, Vec dest_bin_positions) const
{
  //assume source_function is a density fn.
  if(bin_positions.size() != source_function.size()+1)
    PLERROR("in ScaledConditionalCDFSmoother::smooth  you need to supply bin_positions");
  if(dest_bin_positions.size() == 0)
    PLERROR("in ScaledConditionalCDFSmoother::smooth  you need to supply dest_bin_positions");
  smoothed_function.resize(dest_bin_positions.size()-1);

  int j= 0;

  for(int i= 0; i < source_function.size(); ++i)
    {
      Vec v0(1), v1(1);
      v0[0]= bin_positions[i];
      v1[0]= bin_positions[i+1];
      real prior_prob= prior_cdf->survival_fn(v0) - prior_cdf->survival_fn(v1);
      real factor;

      if(0 < prior_prob)
	factor= source_function[i] / prior_prob;
      else
	factor= 1;

      cout << v0[0] << '-' << v1[0] << ":\t" << source_function[i] << '/' <<  prior_prob << '=' << factor << endl;

      while(j < smoothed_function.size() && dest_bin_positions[j+1] <= bin_positions[i+1])
	{
	  Vec v(1);
	  v[0]= (dest_bin_positions[j]+dest_bin_positions[j+1])/2;
	  smoothed_function[j]= factor * prior_cdf->density(v);
	  ++j;
	}
    }


}

%> // end of namespace PLearn
