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
   * $Id: GeneralizedOneHotVMatrix.h,v 1.3 2004/03/23 23:08:08 morinf Exp $
   ******************************************************* */


/*! \file PLearnLibrary/PLearnCore/VMat.h */

#ifndef GeneralizedOneHotVMatrix_INC
#define GeneralizedOneHotVMatrix_INC

#include "RowBufferedVMatrix.h"
#include "VMat.h"

namespace PLearn {
using namespace std;
 

//!  This VMat is a generalization of OneHotVMatrix where all columns (given
//!  by the Vec index) are mapped, instead of just the last one.
class GeneralizedOneHotVMatrix: public RowBufferedVMatrix
{
  typedef RowBufferedVMatrix inherited;

 protected:
  VMat distr;
  Vec index;
  Vec nclasses;
  Vec cold_value;
  Vec hot_value;

 public:
  // ******************
  // *  Constructors  *
  // ******************
  GeneralizedOneHotVMatrix(); //!<  default constructor (for automatic deserialization)

  //!  Warning: VMFields are NOT YET handled by this constructor
  GeneralizedOneHotVMatrix(VMat the_distr, Vec the_index, Vec the_nclasses,
                           Vec the_cold_value, Vec the_host_value);

  virtual void getRow(int i, Vec samplevec) const;
  virtual void reset_dimensions() 
    { 
      distr->reset_dimensions(); 
      width_=distr->width(); 
      length_=distr->length(); 
    }
};

/*!   This VMat is built from another VMat and a 'mapping' matrix that indicates how the elements of the 
  last column should be remapped. It's a sort of generalisation of the OneHot VMatrix, which can also
  be used to do grouping of classes...
  There are two available modes with two different associated constructors:
*/

/*!   1) An exhaustive mapping can be specified with the help of a mapping matrix.
     The 'mapping' matrix should specify for each possible value that the last column of the original VMat can take, 
     what value or vector of values it should be replaced with.
     Ex: if you have a VMat containing samples from 6 classes with classnum (0..5) indicated in the last column, 
     and wish to remap classes 1 and 2 to vector (0,1), classes 3 and 4 to vector (1,0) 
     and classes 0 and 5 to vector (0.5, 0.5) you could use the following 'mapping' matrix:
     1  0 1
     2  0 1
     3  1 0
     4  1 0
     0  .5 .5
     5  .5 .5
     In case a value is found in the last column for which no mapping is defined, an error is issued
*/

/*!   2) Alternatively, you can specify a triplet of values (if_equals_value,
  then_value, else_value) in which case the last column value will be
  replaced by (value==if_equals_value ?then_value :else_value)
*/

} // end of namespcae PLearn
#endif
