

// -*- C++ -*-

// ConditionalCDFSmoother.cc
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
   * $Id: ConditionalCDFSmoother.cc,v 1.3 2003/08/13 08:13:17 plearner Exp $ 
   ******************************************************* */

/*! \file ConditionalCDFSmoother.cc */

#include "ConditionalCDFSmoother.h"

namespace PLearn <%
using namespace std;

ConditionalCDFSmoother::ConditionalCDFSmoother() 
  :Smoother() 
/* ### Initialise all fields to their default value */
  {
    // ...

    // ### You may or may not want to call build_() to finish building the object
    // build_();
  }

ConditionalCDFSmoother::ConditionalCDFSmoother(PP<HistogramDistribution>& prior_cdf_) 
  :Smoother(), prior_cdf(prior_cdf_)
{}


  PLEARN_IMPLEMENT_OBJECT(ConditionalCDFSmoother, "ONE LINE DESCR", "NO HELP");

  void ConditionalCDFSmoother::declareOptions(OptionList& ol)
  {
    declareOption(ol, "prior_cdf", &ConditionalCDFSmoother::prior_cdf, OptionBase::buildoption,
		  "Prior CDF used to smooth other functions");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
  }

  string ConditionalCDFSmoother::help()
  {
    // ### Provide some useful description of what the class is ...
    return 
      "ConditionalCDFSmoother implements a ..."
      + optionHelp();
  }

  void ConditionalCDFSmoother::build_()
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
  void ConditionalCDFSmoother::build()
  {
    inherited::build();
    build_();
  }


  void ConditionalCDFSmoother::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
  {
    Object::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields 
    // ### that you wish to be deepCopied rather than 
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("ConditionalCDFSmoother::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
  }


real ConditionalCDFSmoother::smooth(const Vec& source_function, Vec& smoothed_function, 
		      Vec bin_positions, Vec dest_bin_positions) const
{
  PLERROR("smooth not implemented for ConditionalCDFSmoother.");
  return 0.0;
}

%> // end of namespace PLearn
