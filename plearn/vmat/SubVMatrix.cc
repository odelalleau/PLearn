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
   * $Id: SubVMatrix.cc,v 1.10 2004/02/20 21:14:44 chrish42 Exp $
   ******************************************************* */

#include "SubVMatrix.h"

namespace PLearn {
using namespace std;


/** SubVMatrix **/

PLEARN_IMPLEMENT_OBJECT(SubVMatrix, "ONE LINE DESCR", "NO HELP");

////////////////
// SubVMatrix //
////////////////
SubVMatrix::SubVMatrix()
  :istart(0), 
   jstart(0),
   fistart(-1),
   flength(-1)
{}

SubVMatrix::SubVMatrix(VMat the_parent, int the_istart, int the_jstart, int the_length, int the_width)
  :VMatrix(the_length, the_width), parent(the_parent), istart(the_istart), jstart(the_jstart),
   fistart(-1), flength(-1)
{
  build_();
}

SubVMatrix::SubVMatrix(VMat the_parent, real the_fistart, int the_jstart, real the_flength, int the_width)
  : VMatrix(int(the_flength * the_parent->length()), the_width),
    parent(the_parent), istart(-1), jstart(the_jstart), fistart(the_fistart), flength(the_flength)
    // Note that istart will be set to the right value in build_().
{
  build_();
}

////////////////////
// declareOptions //
////////////////////
void SubVMatrix::declareOptions(OptionList &ol)
{
  declareOption(ol, "parent", &SubVMatrix::parent, OptionBase::buildoption, "Source VMatrix");
  declareOption(ol, "istart", &SubVMatrix::istart, OptionBase::buildoption, "Start i coordinate");
  declareOption(ol, "jstart", &SubVMatrix::jstart, OptionBase::buildoption, "Start j coordinate");
  declareOption(ol, "fistart", &SubVMatrix::fistart, OptionBase::buildoption,
      "If provided, will override istart to fistart * parent.length()");
  declareOption(ol, "flength", &SubVMatrix::flength, OptionBase::buildoption,
      "If provided, will override length to flength * parent.length()");
  inherited::declareOptions(ol);
}

void SubVMatrix::build()
{
  inherited::build();
  build_();
}

void SubVMatrix::build_()
{
  int pl = parent.length();
  if (fistart >= 0) {
    istart = int(fistart * pl);
  }
  if (flength >= 0) {
    length_ = int(flength * pl);
  }
  if(length_<0)
    length_ = parent->length() - istart;

  if(width_<0 && parent->width()>=0)
    width_ = parent->width() - jstart;

  if(istart+length()>parent->length() || jstart+width()>parent->width())
    PLERROR("In SubVMatrix constructor OUT OF BOUNDS of parent VMatrix");

  // Copy the parent field names
  fieldinfos.resize(width_);
  if (parent->getFieldInfos().size() > 0)
    for(int j=0; j<width_; j++)
      fieldinfos[j] = parent->getFieldInfos()[jstart+j];

  if (width_ == parent->width())
  {
    if(inputsize_<0) inputsize_ = parent->inputsize();
    if(targetsize_<0) targetsize_ = parent->targetsize();
    if(weightsize_<0) weightsize_ = parent->weightsize();
    target_is_last = parent->target_is_last;
  } else {
    // The width has changed: if no sizes are specified,
    // we assume it's all input and no target.
    if(targetsize_<0) targetsize_ = 0;
    if(weightsize_<0) weightsize_ = 0;
    if(inputsize_<0) inputsize_ = width_ - targetsize_ - weightsize_;
  }

  //  cerr << "inputsize: "<<inputsize_ << "  targetsize:"<<targetsize_<<"weightsize:"<<weightsize_<<endl;
}

void SubVMatrix::reset_dimensions() 
{ 
  int delta_length = parent->length()-length_;
  int delta_width = 0; // parent->width()-width_; HACK
  parent->reset_dimensions(); 
  length_=parent->length()-delta_length; 
  width_=parent->width()-delta_width; 
}

real SubVMatrix::get(int i, int j) const
{
#ifdef BOUNDCHECK
  if(i<0 || i>=length() || j<0 || j>=width())
    PLERROR("In SubVMatrix::get(i,j) OUT OF BOUND access");
#endif
  return parent->get(i+istart,j+jstart);
}
void SubVMatrix::getSubRow(int i, int j, Vec v) const
{
#ifdef BOUNDCHECK
  if(i<0 || i>=length() || j<0 || j+v.length()>width())
    PLERROR("In SubVMatrix::getSubRow(i,j,v) OUT OF BOUND access");
#endif
  parent->getSubRow(i+istart,j+jstart,v);
}

void SubVMatrix::getMat(int i, int j, Mat m) const
{
#ifdef BOUNDCHECK
  if(i<0 || i+m.length()>length() || j<0 || j+m.width()>width())
    PLERROR("In SubVMatrix::getMat OUT OF BOUND access");
#endif
  parent->getMat(i+istart, j+jstart, m);
}

void SubVMatrix::put(int i, int j, real value)
{
#ifdef BOUNDCHECK
  if(i<0 || i>=length() || j<0 || j>=width())
    PLERROR("In SubVMatrix::put(i,j,value) OUT OF BOUND access");
#endif
  return parent->put(i+istart,j+jstart,value);
}
void SubVMatrix::putSubRow(int i, int j, Vec v)
{
#ifdef BOUNDCHECK
  if(i<0 || i>=length() || j<0 || j>=width())
    PLERROR("In SubVMatrix::putSubRow(i,j,v) OUT OF BOUND access");
#endif
  parent->putSubRow(i+istart,j+jstart,v);
}

void SubVMatrix::putMat(int i, int j, Mat m)
{
#ifdef BOUNDCHECK
  if(i<0 || i+m.length()>length() || j<0 || j+m.width()>width())
    PLERROR("In SubVMatrix::putMat(i,j,m) OUT OF BOUND access");
#endif
  parent->putMat(i+istart, j+jstart, m);
}

VMat SubVMatrix::subMat(int i, int j, int l, int w)
{
  return parent->subMat(istart+i,jstart+j,l,w);
}

real SubVMatrix::dot(int i1, int i2, int inputsize) const
{
  if(jstart==0)
    return parent->dot(istart+i1, istart+i2, inputsize);
  else 
    return VMatrix::dot(i1,i2,inputsize);
}

real SubVMatrix::dot(int i, const Vec& v) const
{
  if(jstart==0)
    return parent->dot(istart+i,v);
  else
    return VMatrix::dot(i,v);
}

} // end of namespcae PLearn
