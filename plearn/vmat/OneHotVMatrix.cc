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
   * $Id: OneHotVMatrix.cc,v 1.5 2004/06/29 19:54:43 tihocan Exp $
   ******************************************************* */

#include "OneHotVMatrix.h"

namespace PLearn {
using namespace std;


/** OneHotVMatrix **/

PLEARN_IMPLEMENT_OBJECT(OneHotVMatrix, "ONE LINE DESC", "NO HELP");

OneHotVMatrix::OneHotVMatrix()
    : nclasses(0), cold_value(0.0), hot_value(1.0)
{
}

OneHotVMatrix::OneHotVMatrix(VMat the_underlying_distr, int the_nclasses, real the_cold_value,
                             real the_hot_value)
  : inherited(the_underlying_distr->length(), the_underlying_distr->width()+the_nclasses-1),
    underlying_distr(the_underlying_distr), nclasses(the_nclasses),
    cold_value(the_cold_value), hot_value(the_hot_value)
{
}

void
OneHotVMatrix::build()
{
  inherited::build();
  build_();
}

void
OneHotVMatrix::build_()
{
}

void
OneHotVMatrix::declareOptions(OptionList &ol)
{
  declareOption(ol, "underlying_distr", &OneHotVMatrix::underlying_distr, OptionBase::buildoption, "");
  declareOption(ol, "nclasses", &OneHotVMatrix::nclasses, OptionBase::buildoption, "");
  declareOption(ol, "cold_value", &OneHotVMatrix::cold_value, OptionBase::buildoption, "");
  declareOption(ol, "hot_value", &OneHotVMatrix::hot_value, OptionBase::buildoption, "");
  inherited::declareOptions(ol);
}

void OneHotVMatrix::getNewRow(int i, Vec& samplevec) const
{
#ifdef BOUNDCHECK
  if(i<0 || i>=length())
    PLERROR("In OneHotVMatrix::getNewRow OUT OF BOUNDS");
  if(samplevec.length()!=width())
    PLERROR("In OneHotVMatrix::getNewRow samplevec.length() must be equal to the VMat's width");
#endif
  Vec input = samplevec.subVec(0,width()-nclasses);
  Vec target = samplevec.subVec(width()-nclasses,nclasses);
  underlying_distr->getSubRow(i,0,input);
  int classnum = int(underlying_distr->get(i,underlying_distr->width()-1));
  fill_one_hot(target,classnum,cold_value,hot_value);
}

real OneHotVMatrix::dot(int i1, int i2, int inputsize) const
{
  return underlying_distr->dot(i1,i2,inputsize);
}

real OneHotVMatrix::dot(int i, const Vec& v) const
{
  return underlying_distr->dot(i,v);
}

} // end of namespcae PLearn
