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


#ifndef ThresholdVMatrix_INC
#define ThresholdVMatrix_INC

#include "RowBufferedVMatrix.h"
#include "VMat.h"

namespace PLearn <%
using namespace std;
 
/* A bit like OneHotVMatrix with a threshold level
 * instead.  The last column will become cold_value
 * if it was <= threshold, or hot_value if it was
 * > threshold.
 * N.B. the gt_threshold boolean value can be set
 * to true so that we get hot_value when 
 * val > threshold, or false for val >= threshold.
 */
class ThresholdVMatrix: public RowBufferedVMatrix
{
 protected:
  VMat underlying_distr;
  real threshold;
  real cold_value;
  real hot_value;
  bool gt_threshold;

 public:
  ThresholdVMatrix(VMat the_underlying_distr, real threshold_, real the_cold_value=0.0, real the_hot_value=1.0,
		  bool gt_threshold_= true);
  virtual void getRow(int i, Vec v) const;
  virtual void reset_dimensions() 
    { 
      underlying_distr->reset_dimensions(); 
      width_=underlying_distr->width(); 
      length_=underlying_distr->length(); 
    }
};

inline VMat thresholdVMat(VMat d, real threshold, real cold_value=0.0, real hot_value=1.0,
			 bool gt_threshold= true)
{ return new ThresholdVMatrix(d, threshold, cold_value, hot_value, gt_threshold); }

%> // end of namespcae PLearn
#endif
