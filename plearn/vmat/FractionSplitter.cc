
// -*- C++ -*-

// FractionSplitter.cc
//
// Copyright (C) 2003  Pascal Vincent 
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
   * $Id: FractionSplitter.cc,v 1.1 2003/05/21 09:53:50 plearner Exp $ 
   ******************************************************* */

/*! \file FractionSplitter.cc */
#include "FractionSplitter.h"

namespace PLearn <%
using namespace std;

FractionSplitter::FractionSplitter() 
  :Splitter()
  /* ### Initialise all fields to their default value */
{
  // ...

  // ### You may or may not want to call build_() to finish building the object
  // build_();
}


IMPLEMENT_NAME_AND_DEEPCOPY(FractionSplitter);

void FractionSplitter::declareOptions(OptionList& ol)
{
  declareOption(ol, "fractions", &FractionSplitter::fractions, OptionBase::buildoption,
                "A list of vectors. Each vector represents a split. \n"
                "The elements of each vector, if less than 1, are fractions of the total number of elements in the data set\n"
                "If they are integers above one, they are an absolute number of elements\n"
                "Ex: [ [ .10 .20 ] ]  yields a single split with the first part being 10% of the data and the second the next 20% \n");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

string FractionSplitter::help()
{
  // ### Provide some useful description of what the class is ...
  return 
    "FractionSplitter is a splitter for which fractions (or number of samples) of the training set are specified explicitly";
}

void FractionSplitter::build_()
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
void FractionSplitter::build()
{
  inherited::build();
  build_();
}

void FractionSplitter::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  Splitter::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("FractionSplitter::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

int FractionSplitter::nsplits() const
{
  return fractions.length();
}

Array<VMat> FractionSplitter::getSplit(int k)
{
  Vec frac_k = fractions[k];
  int n = frac_k.length();
  Array<VMat> vms(n);
  int l = dataset.length();
  int start = 0;
  int end = 0;
  for(int i=0; i<n; i++)
    {
      real f = frac_k[i];
      if(f>1)
        end = start + int(f);
      else 
        end = start + int(f*l);
      if(end>l)
        end = l;
      vms[i] = dataset.subMatRows(start, end-start);
      start = end;
    }
  return vms;
}


%> // end of namespace PLearn
