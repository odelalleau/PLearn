// -*- C++ -*-

// RunObject.cc
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
   * $Id: RunObject.cc,v 1.2 2004/09/14 16:04:37 chrish42 Exp $ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file RunObject.cc */


#include "RunObject.h"

namespace PLearn {
using namespace std;

///////////////
// RunObject //
///////////////
RunObject::RunObject() 
{
}

PLEARN_IMPLEMENT_OBJECT(RunObject,
    "Allows to build a non-runnable object in a PLearn script.",
    "This Object implements a run() method so that it can be used in\n"
    "a PLearn script, in order to build another Object given by the\n"
    "'underlying_object' option without PLearn returning an error.\n"
);

void RunObject::declareOptions(OptionList& ol)
{
  declareOption(ol, "underlying_object", &RunObject::underlying_object, OptionBase::buildoption,
      "The underlying object to be built.");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void RunObject::build_()
{
}

void RunObject::build()
{
  inherited::build();
  build_();
}

void RunObject::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("RunObject::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

/////////
// run //
/////////
void RunObject::run() {
}

} // end of namespace PLearn
