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
   * $Id: ClassificationLossVariable.h,v 1.4 2004/02/20 21:11:50 chrish42 Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#ifndef ClassificationLossVariable_INC
#define ClassificationLossVariable_INC

#include "BinaryVariable.h"
// For inline function that can return either a ClassificationLossVariable or a MiniBatchClassificationLossVariable
#include "MiniBatchClassificationLossVariable.h"

namespace PLearn {
using namespace std;


//! Indicator(classnum==argmax(netout))
class ClassificationLossVariable: public BinaryVariable
{
protected:
  typedef BinaryVariable inherited;
  //!  Default constructor for persistence
  ClassificationLossVariable() {}

public:
  ClassificationLossVariable(Variable* netout, Variable* classnum);
  PLEARN_DECLARE_OBJECT(ClassificationLossVariable);
  virtual void recomputeSize(int& l, int& w) const;
  virtual void fprop();
  //! can't bprop through a hard classification error...
  virtual void bprop() {}
};

inline Var classification_loss(Var network_output, Var classnum)
{ 
  if(classnum->isScalar())
    return new ClassificationLossVariable(network_output, classnum); 
  else return new MiniBatchClassificationLossVariable(network_output, classnum); 
}


} // end of namespace PLearn

#endif 
