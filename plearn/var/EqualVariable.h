// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2001-2002 Nicolas Chapados, Ichiro Takeuchi, Jean-Sebastien Senecal
// Copyright (C) 2002 Xiangdong Wang, Christian Dorion

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
   * $Id: EqualVariable.h,v 1.5 2004/04/27 15:58:16 morinf Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#ifndef EqualVariable_INC
#define EqualVariable_INC

#include "BinaryVariable.h"

namespace PLearn {
using namespace std;



//!  A scalar var;  equal 1 if input1==input2, 0 otherwise
class EqualVariable: public BinaryVariable
{
  typedef BinaryVariable inherited;

public:
  //!  Default constructor for persistence
  EqualVariable() {}
  EqualVariable(Variable* input1, Variable* input2);

  PLEARN_DECLARE_OBJECT(EqualVariable);

  virtual void build();

  virtual void recomputeSize(int& l, int& w) const;  
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();

protected:
    void build_();
};

DECLARE_OBJECT_PTR(EqualVariable);

/*!   First case: v1 and v2 are two vectors of length() l
     resulting Var is 1 if for all i=0 to l-1,
     v1->value[i] == v2->value[i], 0 otherwise
  Second case: one of v1 or v2 is a scalar variable (length() 1)
  and the other is a vector of length() l
     resulting Var is a vector of length() l, doing an element-wise comparison
*/

Var isequal(Var v1, Var v2);


} // end of namespace PLearn

#endif 
