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
   * $Id: SubsampleVariable.cc,v 1.3 2003/08/13 08:13:17 plearner Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "SubsampleVariable.h"

namespace PLearn <%
using namespace std;


/** SubsampleVariable **/

SubsampleVariable::SubsampleVariable(Variable* input, int the_subsamplefactor) 
  :UnaryVariable(input, input->length()/the_subsamplefactor, input->width()/the_subsamplefactor), 
   subsamplefactor(the_subsamplefactor) 
{
  if(input->length()%the_subsamplefactor!=0 || input->width()%the_subsamplefactor!=0)
    PLERROR("In SubsampleVariable constructor: Dimensions of input are not dividable by subsamplefactor");
}


PLEARN_IMPLEMENT_OBJECT(SubsampleVariable, "ONE LINE DESCR", "NO HELP");

void SubsampleVariable::recomputeSize(int& l, int& w) const
{ l=input->length()/subsamplefactor; w=input->width()/subsamplefactor; }









void SubsampleVariable::fprop()
{
  subsample(input->matValue, subsamplefactor, matValue);
}


void SubsampleVariable::bprop()
{
  int norm = subsamplefactor * subsamplefactor;
  for(int i=0; i<length(); i++)
    for(int j=0; j<width(); j++)
      {
        real* inputgradientptr = input->matGradient[subsamplefactor*i]+subsamplefactor*j;
        real thisgradient = matGradient(i,j);
        for(int l=0; l<subsamplefactor; l++, inputgradientptr += input->matGradient.mod())
          for(int c=0; c<subsamplefactor; c++)
            {
              inputgradientptr[c] = thisgradient/norm;
            }
      }
}


void SubsampleVariable::symbolicBprop()
{ PLERROR("SubsampleVariable::symbolicBprop() not yet implemented"); }



%> // end of namespace PLearn


