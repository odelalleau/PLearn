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
   * $Id: MemoryVMatrix.cc,v 1.8 2004/03/17 15:06:48 tihocan Exp $
   ******************************************************* */

#include "MemoryVMatrix.h"

namespace PLearn {
using namespace std;



/** MemoryVMatrix **/

PLEARN_IMPLEMENT_OBJECT(MemoryVMatrix, "ONE LINE DESCR", "NO HELP");

MemoryVMatrix::MemoryVMatrix() : data(Mat())
{}

MemoryVMatrix::MemoryVMatrix(const Mat& the_data)
  :VMatrix(the_data.length(), the_data.width()), data(the_data)
{
  defineSizes(the_data.width(), 0, 0);
}

void MemoryVMatrix::declareOptions(OptionList& ol)
{
  declareOption(ol, "data", &MemoryVMatrix::data, OptionBase::buildoption, "The underlying matrix. \n");
  inherited::declareOptions(ol);
}

void MemoryVMatrix::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  deepCopyField(data, copies);
}

////////////
// build_ //
////////////
void MemoryVMatrix::build_()
{
  if (this->length() >= 0 && this->length() != data.length()) {
    // New length specified.
    data.resize(this->length(), data.width());
  }
  if (this->width() >= 0 && this->width() != data.width()) {
    // New width specified.
    data.resize(data.length(), this->width());
  }
  if (this->length() < 0 && data.length() >= 0) {
    // Take the length from the data matrix.
    this->length_ = data.length();
  }
  if (this->width() < 0 && data.width() >= 0) {
    // Take the width from the data matrix.
    this->width_ = data.width();
  }
}

void MemoryVMatrix::build()
{
  inherited::build();
  build_();
}

real MemoryVMatrix::get(int i, int j) const
{ return data(i,j); }

void MemoryVMatrix::put(int i, int j, real value)
{ data(i,j) = value; }

void MemoryVMatrix::getColumn(int i, Vec v) const
{ v << data.column(i); }

void MemoryVMatrix::getSubRow(int i, int j, Vec v) const
{ v << data(i).subVec(j,v.length()); }

void MemoryVMatrix::getRow(int i, Vec v) const
{ v << data(i); }

void MemoryVMatrix::getMat(int i, int j, Mat m) const
{ m << data.subMat(i,j,m.length(),m.width()); }

void MemoryVMatrix::putSubRow(int i, int j, Vec v)
{ data.subMat(i,j,1,v.length()) << v; }

void MemoryVMatrix::fill(real value)
{ data.fill(value); }

void MemoryVMatrix::putRow(int i, Vec v)
{ data(i) << v; }

void MemoryVMatrix::putMat(int i, int j, Mat m)
{ data.subMat(i,j,m.length(),m.width()) << m; }

void MemoryVMatrix::appendRow(Vec v)
{ 
  data.appendRow(v); 
  length_++;
}

void MemoryVMatrix::write(ostream& out) const
{
  writeHeader(out, "MemoryVMatrix");
  VMatrix::write(out);  // save higher-level stuff
  writeField(out, "data", data);
  writeFooter(out, "MemoryVMatrix");
}

void MemoryVMatrix::oldread(istream& in)
{
  readHeader(in, "MemoryVMatrix");
  VMatrix::oldread(in);  // read higher-level stuff
  readField(in, "data", data);
  readFooter(in, "MemoryVMatrix");
}

Mat MemoryVMatrix::toMat() const
{ return data; }

VMat MemoryVMatrix::subMat(int i, int j, int l, int w)
{ return new MemoryVMatrix(data.subMat(i,j,l,w)); }

real MemoryVMatrix::dot(int i1, int i2, int inputsize) const
{
#ifdef BOUNDCHECK
  if(inputsize>width())
    PLERROR("In MemoryVMatrix::dot inputsize>width()");
#endif
  real* v1 = data.rowdata(i1);
  real* v2 = data.rowdata(i2);
  real res = 0.;
  for(int k=0; k<inputsize; k++)
    res += v1[k]*v2[k];
  return res;
}
real MemoryVMatrix::dot(int i, const Vec& v) const
{
#ifdef BOUNDCHECK
  if(v.length()>width())
    PLERROR("In MemoryVMatrix::dot length of vector v is greater than VMat's width");
#endif
  real* v1 = data.rowdata(i);
  real* v2 = v.data();
  real res = 0.;
  for(int k=0; k<v.length(); k++)
    res += v1[k]*v2[k];
  return res;
}

} // end of namespcae PLearn
