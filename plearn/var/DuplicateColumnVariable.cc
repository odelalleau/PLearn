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
   * $Id: DuplicateColumnVariable.cc,v 1.6 2004/04/27 16:03:35 morinf Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "DuplicateColumnVariable.h"
#include "RowSumVariable.h"
//#include "Var_utils.h"

namespace PLearn {
using namespace std;


/** DuplicateColumnVariable **/

PLEARN_IMPLEMENT_OBJECT(DuplicateColumnVariable,
                        "ONE LINE DESCR",
                        "NO HELP");

DuplicateColumnVariable::DuplicateColumnVariable(Variable* input, int thewidth)
  : inherited(input, input->length(), thewidth), width_(thewidth)
{
    build_();
}

void
DuplicateColumnVariable::build()
{
    inherited::build();
    build_();
}

void
DuplicateColumnVariable::build_()
{
    if (input && !input->isColumnVec())
        PLERROR("In DuplicateColumnVariable input is not a column");
}

void
DuplicateColumnVariable::declareOptions(OptionList &ol)
{
    declareOption(ol, "width_", &DuplicateColumnVariable::width_, OptionBase::buildoption, "");
    inherited::declareOptions(ol);
}

void DuplicateColumnVariable::recomputeSize(int& l, int& w) const
{
    if (input)
        l = input->length();
    else
        l = 0;
    w = width_;
}

void DuplicateColumnVariable::fprop()
{
  int k=0;
  for(int i=0; i<length(); i++)
    for(int j=0; j<width(); j++, k++)
      valuedata[k] = input->valuedata[i];
}


void DuplicateColumnVariable::bprop()
{
  int k=0;
  for(int i=0; i<length(); i++)
    for(int j=0; j<width(); j++, k++)
      input->gradientdata[i] += gradientdata[k];
}


void DuplicateColumnVariable::symbolicBprop()
{
  input->accg(rowSum(g));
}


} // end of namespace PLearn
