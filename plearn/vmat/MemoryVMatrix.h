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
   * $Id: MemoryVMatrix.h,v 1.6 2004/04/05 22:58:23 morinf Exp $
   ******************************************************* */


/*! \file PLearnLibrary/PLearnCore/VMat.h */

#ifndef MemoryVMatrix_INC
#define MemoryVMatrix_INC

#include "VMat.h"

namespace PLearn {
using namespace std;
 
class MemoryVMatrix: public VMatrix
{
  typedef VMatrix inherited;

public:

  Mat data;

private:
  //! This does the actual building.
  // (Please implement in .cc)
  void build_();

 //!  necessary for the deepcopy mechanism 
protected:
  static void declareOptions(OptionList& ol);

public:
  MemoryVMatrix();
  MemoryVMatrix(const Mat& the_data);
  virtual real get(int i, int j) const;
  virtual void getSubRow(int i, int j, Vec v) const;
  virtual void getRow(int i, Vec v) const;
  virtual void getColumn(int i, Vec v) const;
  virtual void getMat(int i, int j, Mat m) const;
  virtual void put(int i, int j, real value);
  virtual void putSubRow(int i, int j, Vec v);
  virtual void putRow(int i, Vec v);
  virtual void appendRow(Vec v);
  virtual void fill(real value);
  virtual void putMat(int i, int j, Mat m);
  virtual Mat toMat() const;
  virtual VMat subMat(int i, int j, int l, int w);
  virtual real dot(int i1, int i2, int inputsize) const;
  virtual real dot(int i, const Vec& v) const;

    //virtual void write(ostream& out) const;
    //virtual void oldread(istream& in);

  //! simply calls inherited::build() then build_()
  virtual void build();

  PLEARN_DECLARE_OBJECT(MemoryVMatrix);
  void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);
};

DECLARE_OBJECT_PTR(MemoryVMatrix);

} // end of namespace PLearn
#endif
