// -*- C++ -*-

// RepeatSplitter.cc
//
// Copyright (C) 2003 Olivier Delalleau 
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
   * $Id: RepeatSplitter.cc,v 1.2 2003/11/27 20:12:48 tihocan Exp $ 
   ******************************************************* */

/*! \file RepeatSplitter.cc */

#include "random.h"
#include "RepeatSplitter.h"
#include "SelectRowsVMatrix.h"

namespace PLearn <%
using namespace std;

////////////////////
// RepeatSplitter //
////////////////////
RepeatSplitter::RepeatSplitter() 
  :Splitter(),
  last_n(-1),
  n(0),
  shuffle(0)
  /* ### Initialise all fields to their default value */
{
}

PLEARN_IMPLEMENT_OBJECT(RepeatSplitter, "ONE LINE DESCR", "NO HELP");

////////////////////
// declareOptions //
////////////////////
void RepeatSplitter::declareOptions(OptionList& ol)
{
  declareOption(ol, "n", &RepeatSplitter::n, OptionBase::buildoption,
                 "How many times we want to repeat it");

  declareOption(ol, "shuffle", &RepeatSplitter::shuffle, OptionBase::buildoption,
                 "If set to 1, the dataset will be shuffled differently at each repetition");

  declareOption(ol, "to_repeat", &RepeatSplitter::to_repeat, OptionBase::buildoption,
                 "The splitter we want to repeat");

  inherited::declareOptions(ol);
}

///////////
// build //
///////////
void RepeatSplitter::build()
{
  inherited::build();
  build_();
}

////////////
// build_ //
////////////
void RepeatSplitter::build_()
{
  if (shuffle && dataset) {
    // Prepare the shuffled indices.
    int n_splits = nsplits();
    indices.resize(n_splits, dataset.length());
    TVec<int> shuffled;
    for (int i = 0; i < n_splits; i++) {
      shuffled = TVec<int>(0, dataset.length()-1, 1);
      shuffleElements(shuffled);
      indices(i) << shuffled;
    }
  } else {
    indices.resize(0,0);
  }
  last_n = -1;
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void RepeatSplitter::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  Splitter::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);

  deepCopyField(to_repeat, copies);

}

//////////////
// getSplit //
//////////////
TVec<VMat> RepeatSplitter::getSplit(int k)
{
  int n_splits = this->nsplits();
  if (k >= n_splits) {
    PLERROR("In RepeatSplitter::getSplit: split asked is too high");
  }
  int child_splits = to_repeat->nsplits();
  int real_k = k % child_splits;
  if (shuffle && dataset) {
    int shuffle_indice = k / child_splits;
    if (shuffle_indice != last_n) {
      // We have to reshuffle the dataset, according to indices.
      VMat m = new SelectRowsVMatrix(dataset, indices(shuffle_indice));
      to_repeat->setDataSet(m);
      last_n = shuffle_indice;
    }
  } 
  return to_repeat->getSplit(real_k);
}

///////////////////
// nSetsPerSplit //
///////////////////
int RepeatSplitter::nSetsPerSplit() const
{
  return to_repeat->nSetsPerSplit();
}

/////////////
// nsplits //
/////////////
int RepeatSplitter::nsplits() const
{
  return to_repeat->nsplits() * n;
}

////////////////
// setDataSet //
////////////////
void RepeatSplitter::setDataSet(VMat the_dataset) {
  inherited::setDataSet(the_dataset);
  to_repeat->setDataSet(the_dataset);
  build(); // necessary to recompute the indices.
}

%> // end of namespace PLearn
