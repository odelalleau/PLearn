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
   * $Id: UniformizeVMatrix.h,v 1.5 2004/06/29 19:55:55 tihocan Exp $
   ******************************************************* */


/*! \file PLearnLibrary/PLearnCore/VMat.h */

#ifndef UniformizeVMatrix_INC
#define UniformizeVMatrix_INC

#include "RowBufferedVMatrix.h"
#include "VMat.h"

namespace PLearn {
using namespace std;
 

/*!   VMatrix that can be used to uniformize (between a and b)
  each feature in index of the underlying distribution such that:
  P(x') = .5   if  a < x'< b
        =  0   otherwise
  
  We suppose that the original distribution of x, P(x), could be anything,
  and we map "a" with bins[0] and "b" with bins[N-1].
*/
class UniformizeVMatrix: public RowBufferedVMatrix
{
  typedef RowBufferedVMatrix inherited;

protected:
  VMat distr;
  Mat bins;
  Vec index;
  real a;
  real b;

public:
  // ******************
  // *  Constructors  *
  // ******************
  UniformizeVMatrix(); //!<  default constructor (for automatic deserialization)

  //! The original VMFields are copied upon construction
  UniformizeVMatrix(VMat the_distr, Mat the_bins, Vec the_index,
                    real the_a=0.0, real the_b=1.0);

  PLEARN_DECLARE_OBJECT(UniformizeVMatrix);
  static void declareOptions(OptionList &ol);

  virtual void build();

  virtual void getNewRow(int i, Vec& v) const;
  virtual void reset_dimensions()
    { distr->reset_dimensions(); width_=distr->width(); length_=distr->length(); }
private:
    void build_();
};

DECLARE_OBJECT_PTR(UniformizeVMatrix);

} // end of namespcae PLearn
#endif
