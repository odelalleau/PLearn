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
   * $Id: ConcatRowsVMatrix.h,v 1.3 2003/08/13 08:13:46 plearner Exp $
   ******************************************************* */


/*! \file PLearnLibrary/PLearnCore/VMat.h */

#ifndef ConcatRowsVMatrix_INC
#define ConcatRowsVMatrix_INC

#include "VMat.h"

namespace PLearn <%
using namespace std;
 
/*!   This class concatenates several distributions:
  it samples from each of them in sequence, i.e.,
  sampling all the samples from the first distribution,
  then all the samples from the 2nd one, etc...
  This works only for distributions with a finite 
  number of samples (length()!=-1).
*/
class ConcatRowsVMatrix: public VMatrix
{
  typedef VMatrix inherited;
 protected:
  Array<VMat> array;

  //!  returns the index of the correct VMat in the array and the the row
  //!  number in this VMat that correspond to row i in the ConcatRowsVMat
  void getpositions(int i, int& whichvm, int& rowofvm) const; 

 public:
  //! The fields names are copied from the FIRST VMat
  ConcatRowsVMatrix(Array<VMat> the_array = Array<VMat>()) : array(the_array) { if (array.size()) build_(); };
  ConcatRowsVMatrix(VMat d1, VMat d2) : array(d1, d2) { build_(); };

  virtual real get(int i, int j) const;
  virtual void getSubRow(int i, int j, Vec v) const;
  virtual real getStringVal(int col, const string & str) const;
  virtual string getValString(int col, real val) const;
  virtual string getString(int row,int col) const;
  // this function does not really makes sense since there could be as much mapping as the number of VMatrices composing this ConcatRowsVMatrix
  // It will return the first's mapping
  const map<string,real>& getStringToRealMapping(int col) const;

  virtual void reset_dimensions() 
    { 
      for (int i=0;i<array.size();i++) array[i]->reset_dimensions(); 
      width_=array[0]->width();
      length_=0;
      for (int i=0;i<array.size();i++) 
        {
          if (array[i]->width()!=width_) 
            PLERROR("ConcatRowsVMatrix: underlying-distr %d has %d width, while 0-th has %d",
                  i,array[i]->width(),width_);
          length_ += array[i]->length();
        }
    }
  virtual real dot(int i1, int i2, int inputsize) const;
  virtual real dot(int i, const Vec& v) const;

  PLEARN_DECLARE_OBJECT(ConcatRowsVMatrix);
  static void declareOptions(OptionList &ol);
  virtual void build();
  void build_();
};

DECLARE_OBJECT_PTR(ConcatRowsVMatrix);

inline VMat vconcat(VMat d1, VMat d2)
{ return new ConcatRowsVMatrix(d1,d2); }

inline VMat vconcat(Array<VMat> ds)
{ return new ConcatRowsVMatrix(ds); }


%> // end of namespcae PLearn
#endif
