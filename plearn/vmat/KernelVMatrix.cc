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
   * $Id: KernelVMatrix.cc,v 1.1 2004/04/02 18:31:56 tihocan Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "KernelVMatrix.h"

// From Old Kernel.cc: all includes are putted in every file.
// To be revised manually 
/*#include <cmath>
#include "stringutils.h"
#include "Kernel.h"
#include "TMat_maths.h"
#include "PLMPI.h"*/
//////////////////////////
namespace PLearn {
using namespace std;


// *****************
// * KernelVMatrix *
// *****************

KernelVMatrix::KernelVMatrix(VMat data1, VMat data2, Ker the_ker)
  : VMatrix(data1->length(), data2->length()), 
    d1(data1), d2(data2), ker(the_ker), 
    input1(data1->width()), input2(data2->width())
{}


/*
IMPLEMENT_NAME_AND_DEEPCOPY(KernelVMatrix);
void KernelVMatrix::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  Kernel::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(d1, copies);
  deepCopyField(d2, copies);
  deepCopyField(ker, copies);
  deepCopyField(input1, copies);
  deepCopyField(input2, copies);
}
*/

real KernelVMatrix::get(int i, int j) const
{
#ifdef BOUNDCHECK
  if(i<0 || i>=length() || j<0 || j>=width())
    PLERROR("In KernelVMatrix::get OUT OF BOUNDS");
#endif

  d1->getRow(i,input1);
  d2->getRow(j,input2);
  return ker(input1,input2);
}


void KernelVMatrix::getSubRow(int i, int j, Vec v) const
{
#ifdef BOUNDCHECK
  if(i<0 || i>=length() || j<0 || j+v.length()>width())
    PLERROR("In KernelVMatrix::getRow OUT OF BOUNDS");
#endif

  d1->getRow(i,input1);
  for(int jj=0; jj<v.length(); jj++)
    {
      d2->getRow(j+jj,input2);
      v[jj] = ker(input1,input2);
    }
}



} // end of namespace PLearn

