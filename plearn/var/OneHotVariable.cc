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
   * $Id: OneHotVariable.cc,v 1.5 2004/04/27 16:03:35 morinf Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "OneHotVariable.h"

namespace PLearn {
using namespace std;


/** OneHotVariable **/

PLEARN_IMPLEMENT_OBJECT(OneHotVariable,
                        "Represents a vector of a given lenth, that has value 1 at the index "
                        "given by another variable and 0 everywhere else",
                        "NO HELP");

OneHotVariable::OneHotVariable(int thelength, Variable* hotindex, real the_coldvalue, real the_hotvalue)
    : inherited(hotindex,thelength,1), hotvalue(the_hotvalue), coldvalue(the_coldvalue), length_(thelength)
{
    build_();
}

void
OneHotVariable::build()
{
    inherited::build();
    build_();
}

void
OneHotVariable::build_()
{
    // input is hotindex from constructor
    if (input && !input->isScalar())
        PLERROR("InterValuesVariable OneHotVariable(int thelength, Variable* hotindex, real the_coldvalue, real the_hotvalue) hotindex must be scalar as it is supposed to be an integer index");
}

void
OneHotVariable::declareOptions(OptionList &ol)
{
    declareOption(ol, "hotvalue", &OneHotVariable::hotvalue, OptionBase::buildoption, "");
    declareOption(ol, "coldvalue", &OneHotVariable::coldvalue, OptionBase::buildoption, "");
    declareOption(ol, "length_", &OneHotVariable::length_, OptionBase::buildoption, "");
    inherited::declareOptions(ol);
}

void OneHotVariable::recomputeSize(int& l, int& w) const
{ l=length_; w=1; }


void OneHotVariable::fprop()
{
  int index = int(input->valuedata[0]);
  if (nelems()==1)
    value[0] = index==0 ? coldvalue : hotvalue;
  else
  {
    for(int i=0; i<nelems(); i++)
      valuedata[i] = coldvalue;
    value[index] = hotvalue;
  }
}


void OneHotVariable::bprop() {}


void OneHotVariable::symbolicBprop() {}


void OneHotVariable::rfprop() {
  if (rValue.length()==0) resizeRValue();
}



} // end of namespace PLearn


