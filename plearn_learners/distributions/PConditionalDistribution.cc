

// -*- C++ -*-

// PConditionalDistribution.cc
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

#include "PConditionalDistribution.h"

namespace PLearn <%
using namespace std;

PConditionalDistribution::PConditionalDistribution() 
  :inherited()
{
   
}

PLEARN_IMPLEMENT_OBJECT(PConditionalDistribution, "ONE LINE DESCR", "NO HELP");

string PConditionalDistribution::help()
{
  // ### Provide some useful description of what the class is ...
  return 
    "You must call setInput to set the condition before using the distribution"
    + optionHelp();
}


void PConditionalDistribution::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
}


void PConditionalDistribution::setInput(const Vec& input)
{ PLERROR("setInput must be implemented for this PConditionalDistribution"); }


void PConditionalDistribution::computeOutput(const Vec& input, Vec& output) const
{
  Vec x = input.subVec(0,input_part_size);
  int d=input.length()-input_part_size;
  Vec y = input.subVec(input_part_size,d);
  setInput(x);
  if (outputs_def=="l")
    output[0]=log_density(y);
  else if (outputs_def=="d")
    output[0]=density(y);
  else if (outputs_def=="c")
    output[0]=cdf(y);
  else if (outputs_def=="s")
    output[0]=survival_fn(y);
  else if (outputs_def=="e")
    expectation(output);
  else if (outputs_def=="v")
  {
    Mat covmat = output.toMat(d,d);
    variance(covmat);
  }
  else PLERROR("PConditionalDistribution: unknown setting of outputs_def = %s",outputs_def.c_str());
}

%> // end of namespace PLearn
