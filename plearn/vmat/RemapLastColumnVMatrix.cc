// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2001 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2002 Pascal Vincent, Julien Keable, Xavier Saint-Mleux
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
   * $Id: RemapLastColumnVMatrix.cc,v 1.5 2004/06/29 19:54:43 tihocan Exp $
   ******************************************************* */

#include "RemapLastColumnVMatrix.h"

namespace PLearn {
using namespace std;


/** RemapLastColumnVMatrix **/

PLEARN_IMPLEMENT_OBJECT(RemapLastColumnVMatrix, "ONE LINE DESC", "NO HELP");

RemapLastColumnVMatrix::RemapLastColumnVMatrix()
    : if_equals_val(0), then_val(0), else_val(0)
{
}

RemapLastColumnVMatrix::RemapLastColumnVMatrix(VMat the_underlying_distr, Mat the_mapping)
  : inherited (the_underlying_distr->length(), the_underlying_distr->width()+the_mapping.width()-2),
    underlying_distr(the_underlying_distr), mapping(the_mapping)
{
}

RemapLastColumnVMatrix::RemapLastColumnVMatrix(VMat the_underlying_distr, real if_equals_value, real then_value, real else_value)
  : inherited(the_underlying_distr->length(), the_underlying_distr->width()),
    underlying_distr(the_underlying_distr), if_equals_val(if_equals_value),
    then_val(then_value), else_val(else_value)
{
}

void
RemapLastColumnVMatrix::build()
{
    inherited::build();
    build_();
}

void
RemapLastColumnVMatrix::build_()
{
}

void
RemapLastColumnVMatrix::declareOptions(OptionList &ol)
{
    declareOption(ol, "underlying_distr", &RemapLastColumnVMatrix::underlying_distr, OptionBase::buildoption, "");
    declareOption(ol, "mapping", &RemapLastColumnVMatrix::mapping, OptionBase::buildoption, "");
    declareOption(ol, "if_equals_val", &RemapLastColumnVMatrix::if_equals_val, OptionBase::buildoption, "");
    declareOption(ol, "then_val", &RemapLastColumnVMatrix::then_val, OptionBase::buildoption, "");
    declareOption(ol, "else_val", &RemapLastColumnVMatrix::else_val, OptionBase::buildoption, "");
    inherited::declareOptions(ol);
}

void RemapLastColumnVMatrix::getNewRow(int i, Vec& samplevec) const
{
#ifdef BOUNDCHECK
  if(i<0 || i>=length())
    PLERROR("In RemapLastColumnVMatrix::getNewRow OUT OF BOUNDS");
  if(samplevec.length()!=width())
    PLERROR("In RemapLastColumnVMatrix::getNewRow samplevec.length() must be equal to the VMat's width");
#endif
  if(mapping.isEmpty()) // use if-then-else mapping
  {
    underlying_distr->getRow(i,samplevec);
    real& lastelem = samplevec.lastElement();
    if(lastelem==if_equals_val)
      lastelem = then_val;
    else
      lastelem = else_val;
  }
  else // use mapping matrix
  {
    int underlying_width = underlying_distr->width();
    int replacement_width = mapping.width()-1;
    underlying_distr->getRow(i,samplevec.subVec(0,underlying_width));
    real val = samplevec[underlying_width-1];
    int k;
    for(k=0; k<mapping.length(); k++)
    {
      if(mapping(k,0)==val)
      {
        samplevec.subVec(underlying_width-1,replacement_width) << mapping(k).subVec(1,replacement_width);
        break;
      }
    }
    if(k>=mapping.length())
      PLERROR("In RemapLastColumnVMatrix::getRow there is a value in the last column that does not have any defined mapping");
  }
}

} // end of namespcae PLearn
