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
   * $Id: RemapLastColumnVMatrix.h,v 1.6 2004/07/07 17:30:48 tihocan Exp $
   ******************************************************* */


/*! \file PLearnLibrary/PLearnCore/VMat.h */

#ifndef RemapLastColumnVMatrix_INC
#define RemapLastColumnVMatrix_INC

#include "RowBufferedVMatrix.h"
#include "VMat.h"

namespace PLearn {
using namespace std;

class RemapLastColumnVMatrix: public RowBufferedVMatrix
{
    typedef RowBufferedVMatrix inherited;

 protected:
    VMat underlying_distr;

    //!  If this is not empty, then it represents the mapping to apply
    Mat mapping;

/*!       These are used only if mapping is an empty Mat, in which case the
      value in the last column will be replaced by 'then_val' if it is
      equal to 'if_equals_val', otherwise it will be replaced by
      'else_val'
*/
    real if_equals_val;
    real then_val;
    real else_val;

  public:
    // ******************
    // *  Constructors  *
    // ******************
    RemapLastColumnVMatrix(); //!<  default constructor (for automatic deserialization)

    //!  full explicit mapping.
    //!  Warning: VMFields are NOT YET handled by this constructor
    RemapLastColumnVMatrix(VMat the_underlying_distr, Mat the_mapping);

    //!  if-then-else mapping.
    //!  Warning: VMFields are NOT YET handled by this constructor
    RemapLastColumnVMatrix(VMat the_underlying_distr, real if_equals_value, real then_value=+1, real else_value=-1);    

    PLEARN_DECLARE_OBJECT(RemapLastColumnVMatrix);
    static void declareOptions(OptionList &ol);

    virtual void build();

    virtual void getNewRow(int i, const Vec& samplevec) const;
    virtual void reset_dimensions() 
    { 
      underlying_distr->reset_dimensions(); 
      width_=underlying_distr->width(); 
      length_=underlying_distr->length(); 
    }
private:
    void build_();
};

inline VMat remapLastColumn(VMat d, Mat mapping)
{ return new RemapLastColumnVMatrix(d, mapping); }

inline VMat remapLastColumn(VMat d, real if_equals_value, real then_value=1.0, real else_value=-1.0)
{ return new RemapLastColumnVMatrix(d, if_equals_value, then_value, else_value); }

DECLARE_OBJECT_PTR(RemapLastColumnVMatrix);

} // end of namespcae PLearn
#endif
