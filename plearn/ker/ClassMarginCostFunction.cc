// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2001-2002 Nicolas Chapados, Ichiro Takeuchi, Jean-Sebastien Senecal
// Copyright (C) 2002 Xiangdong Wang, Christian Dorion

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
   * $Id: ClassMarginCostFunction.cc,v 1.4 2004/04/07 23:15:58 morinf Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "ClassMarginCostFunction.h"

namespace PLearn {
using namespace std;


PLEARN_IMPLEMENT_OBJECT(ClassMarginCostFunction, "ONE LINE DESCR", "NO HELP");

real ClassMarginCostFunction::evaluate(const Vec& output, const Vec& target) const
{
  real margin;
  if (output.length()==1)
  {
    real out = output[0];
    if (output_is_positive)
      out = 2*out-1;
    margin = binary_target_is_01 ? out*(target[0]-0.5)*4.0 : out*target[0];
  }
  else // we assume output gives a score for each class
  {
    int trueclass;
    if (target.length()==1)
    {
      trueclass = int(target[0]);
      if (!binary_target_is_01)
        trueclass = (trueclass+1)/2;
    }
    else
      trueclass = argmax(target);
    
    real trueclass_score = output[trueclass];
    output[trueclass] = -FLT_MAX;
    real otherclass_score = max(output);
    output[trueclass] = trueclass_score;
    margin = trueclass_score-otherclass_score;
  }
  return -margin;
}

void ClassMarginCostFunction::declareOptions(OptionList &ol)
{
    declareOption(ol, "binary_target_is_01", &ClassMarginCostFunction::binary_target_is_01, OptionBase::buildoption, "");
    declareOption(ol, "output_is_positive", &ClassMarginCostFunction::output_is_positive, OptionBase::buildoption, "");
    inherited::declareOptions(ol);
}



} // end of namespace PLearn

