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
   * $Id: AffineTransformVariable.h,v 1.3 2003/09/17 15:27:30 yoshua Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#ifndef AffineTransformVariable_INC
#define AffineTransformVariable_INC

#include "BinaryVariable.h"

namespace PLearn <%
using namespace std;


//! Affine transformation of a vector variable.
//! Should work for both column and row vectors: result vector will be of same kind (row or col)
//! First row of transformation matrix contains bias b, following rows contain linear-transformation T
//! Will compute b + x.T (if you consider b and x to be row vectors)
//! which is equivalent to b + 
class AffineTransformVariable: public BinaryVariable
{
protected:
    typedef BinaryVariable inherited;
  //!  Default constructor for persistence
  AffineTransformVariable() {}

public:
  AffineTransformVariable(Variable* vec, Variable* transformation):
    BinaryVariable(vec, transformation, 
                   vec->isRowVec()?1:transformation->width(),
                   vec->isColumnVec()?1:transformation->width())
  {
    cout << "building AffineTransformVariable at input address " << vec->valuedata << endl;
    if(!vec->isVec())
      PLERROR("In AffineTransformVariable: expecting a vector Var (row or column) as first argument");
  }
  PLEARN_DECLARE_OBJECT(AffineTransformVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
};


%> // end of namespace PLearn

#endif 
