// -*- C++ -*-

// TemporalHorizonVMatrix.cc
//
// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2003 Rejean Ducharme, Yoshua Bengio
// Copyright (C) 2003 Pascal Vincent
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
   * $Id: TemporalHorizonVMatrix.cc,v 1.4 2004/04/05 23:07:07 morinf Exp $
   ******************************************************* */

#include "TemporalHorizonVMatrix.h"

namespace PLearn {
using namespace std;

/** TemporalHorizonVMatrix **/

PLEARN_IMPLEMENT_OBJECT(TemporalHorizonVMatrix, "ONE LINE DESCR",
                        "    VMat class that delay the last entries of an underlying VMat by a certain horizon.\n");

TemporalHorizonVMatrix::TemporalHorizonVMatrix(VMat the_distr, int the_horizon, int target_size)
  : inherited(the_distr.length()-the_horizon, the_distr->width()),
    distr(the_distr), horizon(the_horizon), targetsize(target_size)
{
  fieldinfos = distr->fieldinfos;
  row_delay.resize(width());
  for (int i=0; i<width(); i++)
    row_delay[i] = i<width()-targetsize ? 0 : horizon;

  defineSizes(distr->inputsize(), distr->targetsize(), distr->weightsize());
}

real TemporalHorizonVMatrix::get(int i, int j) const
{ return distr->get(i+row_delay[j], j); }

void TemporalHorizonVMatrix::put(int i, int j, real value)
{ distr->put(i+row_delay[j], j, value); }

real TemporalHorizonVMatrix::dot(int i1, int i2, int inputsize) const
{
  real res = 0.;
  for(int k=0; k<inputsize; k++)
    res += distr->get(i1+row_delay[k],k)*distr->get(i2+row_delay[k],k);
  return res;
}

real TemporalHorizonVMatrix::dot(int i, const Vec& v) const
{
  real res = 0.;
  for(int k=0; k<v.length(); k++)
    res += distr->get(i+row_delay[k],k)*v[k];
  return res;
}

real TemporalHorizonVMatrix::getStringVal(int col, const string & str) const
{ return distr->getStringVal(col, str); }

string TemporalHorizonVMatrix::getValString(int col, real val) const
{ return distr->getValString(col,val); }

string TemporalHorizonVMatrix::getString(int row, int col) const
{ return distr->getString(row+row_delay[col],col); }

const map<string,real>& TemporalHorizonVMatrix::getStringToRealMapping(int col) const
{ return distr->getStringToRealMapping(col);}

const map<real,string>& TemporalHorizonVMatrix::getRealToStringMapping(int col) const
{ return distr->getRealToStringMapping(col);}

void TemporalHorizonVMatrix::declareOptions(OptionList &ol)
{
    declareOption(ol, "distr", &TemporalHorizonVMatrix::distr, OptionBase::buildoption,
        "    The matrix viewed by the TemporalHorizonVMatrix");
    declareOption(ol, "horizon", &TemporalHorizonVMatrix::horizon, OptionBase::buildoption, 
        "    The temporal value by which to delay the VMat");
    declareOption(ol, "targetsize", &TemporalHorizonVMatrix::targetsize, OptionBase::buildoption, 
        "    The number of last entries to delay");
    inherited::declareOptions(ol);
}

void TemporalHorizonVMatrix::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(distr, copies);
}

///////////
// build //
///////////
void TemporalHorizonVMatrix::build()
{
  inherited::build();
  build_();
}

////////////
// build_ //
////////////
void TemporalHorizonVMatrix::build_()
{
  if (distr) {
    length_ = distr->length()-horizon;
    width_ = distr->width();
    fieldinfos = distr->fieldinfos;

    row_delay.resize(width());
    for (int i=0; i<width(); i++)
        row_delay[i] = i<width()-targetsize ? 0 : horizon;

    defineSizes(distr->inputsize(), distr->targetsize(), distr->weightsize());
  }
}

} // end of namespace PLearn
