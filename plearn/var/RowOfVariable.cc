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
   * $Id: RowOfVariable.cc,v 1.1 2005/06/17 20:43:32 larocheh Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "RowOfVariable.h"
#include <plearn/sys/PLMPI.h>
#include <plearn/display/DisplayUtils.h>

namespace PLearn {
using namespace std;



/** RowOfVariable **/

PLEARN_IMPLEMENT_OBJECT(RowOfVariable,
                        "Variable that outputs the row at a certain index in a VMat",
                        "The index is the first element of the input variable.");

RowOfVariable::RowOfVariable(VMat the_distr, Var the_index)
  : inherited(the_index,the_distr->width(),1), distr(the_distr)
{
    build_();
}

void
RowOfVariable::build()
{
    inherited::build();
    build_();
}

void
RowOfVariable::build_()
{
}

void
RowOfVariable::declareOptions(OptionList &ol)
{
    declareOption(ol, "distr", &RowOfVariable::distr, OptionBase::buildoption, "VMat containing the rows to output");
    inherited::declareOptions(ol);
}


void RowOfVariable::recomputeSize(int& l, int& w) const
{
    if (input && distr) {
      l = distr->width();
      w = 1;
    } else
      l = w = 0;
}


void RowOfVariable::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  UnaryVariable::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(distr, copies);
}


void RowOfVariable::fprop()
{
  distr->getRow((int)input->value[0],value);
}


void RowOfVariable::bprop()
{ fbprop(); }


void RowOfVariable::fbprop()
{}


void RowOfVariable::symbolicBprop()
{PLERROR("RowOfVariable::symbolicBprop(): not implemented");}

void RowOfVariable::rfprop()
{PLERROR("RowOfVariable::symbolicBprop(): not implemented");}

} // end of namespace PLearn


