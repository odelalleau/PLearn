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
   * $Id: OneHotVMatrix.h,v 1.2 2004/02/20 21:14:44 chrish42 Exp $
   ******************************************************* */


/*! \file PLearnLibrary/PLearnCore/VMat.h */

#ifndef OneHotVMatrix_INC
#define OneHotVMatrix_INC

#include "RowBufferedVMatrix.h"
#include "VMat.h"

namespace PLearn {
using namespace std;
 

/*!   This VMat is built from another VMat
  Sampling from this VMat will return the corresponding
  sample from the underlying VMat with last element ('target_classnum') replaced
  by a vector of target_values of size nclasses in which only target_values[target_classnum] 
  is set to  hot_value , and all the others are set to cold_value
  In the special case where the VMat is built with nclasses==1, then it is assumed 
  that we have a 2 class classification problem but we are using a single valued target. 
  For this special case only the_cold_value is used as target for classnum 0 
  and the_hot_value is used for classnum 1
*/
class OneHotVMatrix: public RowBufferedVMatrix
{
 protected:
  VMat underlying_distr;
  int nclasses;
  real cold_value;
  real hot_value;

 public:
  //!  (see special case when nclasses==1 desribed above)
  //!  Warning: VMFields are NOT YET handled by this constructor
  OneHotVMatrix(VMat the_underlying_distr, int the_nclasses, real the_cold_value=0.0, real the_host_value=1.0);
  virtual void getRow(int i, Vec samplevec) const;
  virtual void reset_dimensions() 
    { 
      underlying_distr->reset_dimensions(); 
      width_=underlying_distr->width(); 
      length_=underlying_distr->length(); 
    }
  virtual real dot(int i1, int i2, int inputsize) const;
  virtual real dot(int i, const Vec& v) const;
};

inline VMat onehot(VMat d, int nclasses, real cold_value=0.0, real hot_value=1.0)
{ return new OneHotVMatrix(d, nclasses, cold_value, hot_value); }

} // end of namespcae PLearn
#endif
