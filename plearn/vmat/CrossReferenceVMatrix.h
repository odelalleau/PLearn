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
   * $Id: CrossReferenceVMatrix.h,v 1.1 2002/10/03 07:35:28 plearner Exp $
   ******************************************************* */


/*! \file PLearnLibrary/PLearnCore/VMat.h */

#ifndef CrossReferenceVMatrix_INC
#define CrossReferenceVMatrix_INC

#include "VMat.h"

namespace PLearn <%
using namespace std;
 

/*!   This VMat is a concatenation of 2 VMat.  The row of vm2, corresponding
  to the index set by col1 of vm1, is merged with vm1.
  
  for i=1,vm1.length()
    vm.row(i) <- vm1.row(i) + vm2.row(vm1(i,col1))
  
*/
class CrossReferenceVMatrix: public VMatrix
{
 protected:
  VMat vm1;
  int col1;
  VMat vm2;

 public:
  //! The column headings are simply the concatenation of the headings in
  //! the two vmatrices.
  CrossReferenceVMatrix(VMat v1, int c1, VMat v2);
  virtual void getRow(int i, Vec samplevec) const;
  virtual real get(int i, int j) const;
  virtual void reset_dimensions() { PLERROR("CrossReferenceVMatrix::reset_dimensions() not implemented"); }
};

%> // end of namespcae PLearn
#endif
