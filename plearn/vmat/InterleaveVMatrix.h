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
   * $Id: InterleaveVMatrix.h,v 1.1 2002/10/03 07:35:28 plearner Exp $
   ******************************************************* */


/*! \file PLearnLibrary/PLearnCore/VMat.h */

#ifndef InterleaveVMatrix_INC
#define InterleaveVMatrix_INC

#include "VMat.h"

namespace PLearn <%
using namespace std;
 

/*!   This class interleaves several VMats (with consecutive rows
  always coming from a different underlying VMat) thus possibly
  including more than once the rows of the small VMats.
  For example, if VM1.length()==10 and VM2.length()==30 then
  the resulting VM will have 60 rows, and 3 repetitions
  of each row of VM1, with rows taken as follows: 
   VM1.row(0), VM2.row(0), VM1.row(1), VM2.row(1), ..., 
   VM1.row(9), VM2.row(9), VM1.row(0), VM2.row(10), ...
  Note that if VM2.length() is not a multiple of VM1.length()
  some records from VM1 will be repeated once more than others.
*/
class InterleaveVMatrix: public VMatrix
{
  protected:
    Array<VMat> vm;

  public:
  //! The field names are copied from the first VMat in the array
  InterleaveVMatrix(Array<VMat> distributions);

  //! The field names are copied from the first VMat d1
  InterleaveVMatrix(VMat d1, VMat d2);
  
  virtual real get(int i, int j) const;
  virtual void getSubRow(int i, int j, Vec v) const;
  virtual void reset_dimensions() 
    { 
      for (int i=0;i<vm.size();i++) vm[i]->reset_dimensions(); 
      width_=vm[0]->width();
      int maxl = 0;
      int n=vm.size();
      for (int i=0;i<n;i++) 
        {
          if (vm[i]->width()!=width_) 
            PLERROR("InterleaveVMatrix: underlying-distr %d has %d width, while 0-th has %d",
                  i,vm[i]->width(),width_);
          int l= vm[i]->length();
          if (l>maxl) maxl=l;
        }
      length_=n*maxl;
    }
};


%> // end of namespcae PLearn
#endif
