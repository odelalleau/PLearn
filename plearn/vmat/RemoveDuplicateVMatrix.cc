// -*- C++ -*-

// RemoveDuplicateVMatrix.cc
//
// Copyright (C) 2004 Olivier Delalleau 
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
   * $Id: RemoveDuplicateVMatrix.cc,v 1.5 2005/02/04 15:08:50 tihocan Exp $ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file RemoveDuplicateVMatrix.cc */


#include <plearn/base/tostring.h>
#include <plearn/ker/DistanceKernel.h>
#include "RemoveDuplicateVMatrix.h"

namespace PLearn {
using namespace std;

//////////////////
// RemoveDuplicateVMatrix //
//////////////////
RemoveDuplicateVMatrix::RemoveDuplicateVMatrix()
: epsilon(1e-6),
  verbosity(1)
{
  // ...
  // ### You may or may not want to call build_() to finish building the object
  // build_();
}

PLEARN_IMPLEMENT_OBJECT(RemoveDuplicateVMatrix,
    "A VMatrix that removes any duplicated entry in its source VMat.",
    ""
);

////////////////////
// declareOptions //
////////////////////
void RemoveDuplicateVMatrix::declareOptions(OptionList& ol)
{
  // ### Declare all of this object's options here
  // ### For the "flags" of each option, you should typically specify  
  // ### one of OptionBase::buildoption, OptionBase::learntoption or 
  // ### OptionBase::tuningoption. Another possible flag to be combined with
  // ### is OptionBase::nosave

  declareOption(ol, "epsilon", &RemoveDuplicateVMatrix::epsilon, OptionBase::buildoption,
      "Two points will be considered equal iff their square distance is < epsilon.");

  declareOption(ol, "verbosity", &RemoveDuplicateVMatrix::verbosity, OptionBase::buildoption,
      "Controls the amount of output.");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);

  redeclareOption(ol, "indices", &RemoveDuplicateVMatrix::indices, OptionBase::nosave,
      "The indices will be computed at build time.");
}

///////////
// build //
///////////
void RemoveDuplicateVMatrix::build()
{
  // ### Nothing to add here, simply calls build_
  inherited::build();
  build_();
}

////////////
// build_ //
////////////
void RemoveDuplicateVMatrix::build_()
{
  if (source) {
    DistanceKernel dk;
    dk.pow_distance = true;
    if (verbosity >= 1)
      dk.report_progress = true;
    else
      dk.report_progress = false;
    dk.build();
    dk.setDataForKernelMatrix(source);
    int n = source.length();
    Mat distances(n,n);
    dk.computeGramMatrix(distances);
    TVec<bool> removed(n);
    removed.fill(false);
    for (int i = 0; i < n; i++) {
      if (!removed[i]) {
        for (int j = i + 1; j < n; j++) {
          if (!removed[j] && distances(i,j) < epsilon) {
            removed[j] = true;
          }
        }
      }
    }
    indices.resize(0);
    for (int i = 0; i < n; i++)
      if (!removed[i])
        indices.append(i);
    inherited::build();
  }
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void RemoveDuplicateVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("RemoveDuplicateVMatrix::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

} // end of namespace PLearn

