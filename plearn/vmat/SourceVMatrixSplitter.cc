// -*- C++ -*-

// SourceVMatrixSplitter.cc
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
   * $Id: SourceVMatrixSplitter.cc,v 1.3 2004/09/14 16:04:39 chrish42 Exp $ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file SourceVMatrixSplitter.cc */


#include "SourceVMatrixSplitter.h"

namespace PLearn {
using namespace std;

SourceVMatrixSplitter::SourceVMatrixSplitter() 
: to_apply(0)
{
}

PLEARN_IMPLEMENT_OBJECT(SourceVMatrixSplitter,
    "Returns the splits of an underlying splitter, seen by a SourceVMatrix.",
    ""
);

void SourceVMatrixSplitter::declareOptions(OptionList& ol)
{
  // ### Declare all of this object's options here
  // ### For the "flags" of each option, you should typically specify  
  // ### one of OptionBase::buildoption, OptionBase::learntoption or 
  // ### OptionBase::tuningoption. Another possible flag to be combined with
  // ### is OptionBase::nosave

  // ### ex:
  declareOption(ol, "source_vm", &SourceVMatrixSplitter::source_vm, OptionBase::buildoption,
      "The VMatrix to apply.");

  declareOption(ol, "source_splitter", &SourceVMatrixSplitter::source_splitter, OptionBase::buildoption,
      "The underlying splitter.");

  declareOption(ol, "to_apply", &SourceVMatrixSplitter::to_apply, OptionBase::buildoption,
      "The index of the returned split where we apply source_vm.");

  //               "Help text describing this option");
  // ...

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void SourceVMatrixSplitter::build_()
{
}

// ### Nothing to add here, simply calls build_
void SourceVMatrixSplitter::build()
{
  inherited::build();
  build_();
}

void SourceVMatrixSplitter::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  Splitter::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("SourceVMatrixSplitter::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

int SourceVMatrixSplitter::nsplits() const
{
  return source_splitter->nsplits();
}

int SourceVMatrixSplitter::nSetsPerSplit() const
{
  return source_splitter->nSetsPerSplit();
}

TVec<VMat> SourceVMatrixSplitter::getSplit(int k)
{
  TVec<VMat> result = source_splitter->getSplit(k);
  source_vm->source = result[to_apply];
  source_vm->build();
  VMat the_vm = static_cast<SourceVMatrix*>(source_vm);
  result[to_apply] = the_vm;
  return result;
}

////////////////
// setDataSet //
////////////////
void SourceVMatrixSplitter::setDataSet(VMat the_dataset) {
  inherited::setDataSet(the_dataset);
  source_splitter->setDataSet(the_dataset);
}

} // end of namespace PLearn

