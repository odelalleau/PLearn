// -*- C++ -*-

// ExplicitSplitter.cc
// 
// Copyright (C) 2002 Pascal Vincent
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
   * $Id: ExplicitSplitter.cc,v 1.1 2002/11/30 04:27:33 plearner Exp $ 
   ******************************************************* */

/*! \file ExplicitSplitter.cc */
#include "ExplicitSplitter.h"
#include "getDataSet.h"

namespace PLearn <%
using namespace std;

ExplicitSplitter::ExplicitSplitter() 
  :Splitter()
{}


IMPLEMENT_NAME_AND_DEEPCOPY(ExplicitSplitter);

void ExplicitSplitter::declareOptions(OptionList& ol)
{
  declareOption(ol, "splitsets_specs", &ExplicitSplitter::splitsets_specs, OptionBase::buildoption,
                "This is an array of arrays of strings, indicating the split datasets with their string specification.\n"
                "If this is specified, while splitsets is left empty, splitsets will be built from those string specifications.");
  declareOption(ol, "splitsets", &ExplicitSplitter::splitsets, OptionBase::buildoption,
                "This is an array of arrays of VMat giving explicitly the datasets for each split. Either this OR splitsets_specs must be specified before a build()");
  inherited::declareOptions(ol);
}

string ExplicitSplitter::help() const
{
  return 
    "ExplicitSplitter allows you to define a 'splitter' by giving eplicitly the datasets for each split, as an array of array of strings or of VMatrices.\n"
    "ex: ExplicitSplitter( splitsets_specs = [ [ \"/home/db/mypb_train.amat\", \"/home/db/mypb_test.amat\" ] ] ) \n"
    "(This splitter in effect ignores the 'dataset' it is given with setDataSet) \n"
    + optionHelp();
}

void ExplicitSplitter::build_()
{
  if(!splitsets)
    {
      int m = splitsets_specs.size();
      splitsets.resize(m);
      for(int i=0; i<m; i++)
        {
          int n = splitsets_specs[i].size();
          splitsets[i].resize(n);
          for(int j=0; j<n; j++)
            splitsets[i][j] = PLearn::getDataSet(splitsets_specs[i][j]);
        }
    }
}

// ### Nothing to add here, simply calls build_
void ExplicitSplitter::build()
{
  inherited::build();
  build_();
}

void ExplicitSplitter::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  Splitter::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(splitsets_specs, copies);
  deepCopyField(splitsets, copies);
}

int ExplicitSplitter::nsplits() const
{
  return splitsets.size();
}

Array<VMat> ExplicitSplitter::getSplit(int k)
{
  return splitsets[k];
}


%> // end of namespace PLearn
