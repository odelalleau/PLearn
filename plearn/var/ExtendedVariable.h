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
   * $Id: ExtendedVariable.h,v 1.2 2003/08/13 08:13:17 plearner Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#ifndef ExtendedVariable_INC
#define ExtendedVariable_INC

#include "UnaryVariable.h"

namespace PLearn <%
using namespace std;


/*! * ExtendedVariable * */

/*!   Variable that extends the input variable by appending rows at 
  its top and bottom and columns at its left and right.
  The appended rows/columns are filled with the given fill_value
  This can be used for instance to easily implement the usual trick 
  to include the bias in the weights vectors, by appending a 1 to the inputs.
  It is also used in the symbolic bprop of SubMatVariable.
*/
class ExtendedVariable: public UnaryVariable
{
protected:
    typedef UnaryVariable inherited;
  //!  Default constructor for persistence
  ExtendedVariable() : top_extent(), bottom_extent(), left_extent(),
                       right_extent(), fill_value() {}

public:
  int top_extent;
  int bottom_extent;
  int left_extent;
  int right_extent;
  real fill_value; 

  ExtendedVariable(Variable* input, 
                   int the_top_extent, int the_bottom_extent, 
                   int the_left_extent, int the_right_extent, 
                   real the_fill_value);
  PLEARN_DECLARE_OBJECT(ExtendedVariable);
  virtual void recomputeSize(int& l, int& w) const;
  
  
  virtual void fprop();
  virtual void bprop();
  virtual void symbolicBprop();
  virtual void rfprop();
};


%> // end of namespace PLearn

#endif 
