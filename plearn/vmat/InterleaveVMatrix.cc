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
   * $Id: InterleaveVMatrix.cc,v 1.2 2004/02/20 21:14:44 chrish42 Exp $
   ******************************************************* */

#include "InterleaveVMatrix.h"

namespace PLearn {
using namespace std;


/** InterleaveVMatrix **/

InterleaveVMatrix::InterleaveVMatrix(Array<VMat> the_vm)
  :vm(the_vm)
{
  int n=vm.size();
  if (n<1) 
    PLERROR("InterleaveVMatrix expects >= 1 underlying-distribution, got %d",n);

  // Copy the parent fields
  fieldinfos = vm[0]->getFieldInfos();
  
  width_ = vm[0]->width();
  int maxl = 0;
  for (int i=0;i<n;i++)
    {
      if (vm[i]->width() != width_)
        PLERROR("InterleaveVMatrix: underlying-distr %d has %d width, while 0-th has %d",i,vm[i]->width(),width_);
      int l=vm[i]->length();
      if (l>maxl) maxl=l;
    }
  length_ = n*maxl;
}

InterleaveVMatrix::InterleaveVMatrix(VMat d1, VMat d2)
  :vm(d1,d2)
{
  int n=vm.size();
  if (n<1) 
    PLERROR("InterleaveVMatrix expects >= 1 underlying-distribution, got %d",n);

  // Copy the parent fields
  fieldinfos = vm[0]->getFieldInfos();
  
  width_ = vm[0]->width();
  int maxl = 0;
  for (int i=0;i<n;i++)
    {
      if (vm[i]->width() != width_)
        PLERROR("InterleaveVMatrix: underlying-distr %d has %d width, while 0-th has %d",i,vm[i]->width(),width_);
      int l=vm[i]->length();
      if (l>maxl) maxl=l;
    }
  length_ = n*maxl;
}

real InterleaveVMatrix::get(int i, int j) const
{
#ifdef BOUNDCHECK
  if(i<0 || i>=length() || j<0 || j>=width())
    PLERROR("In InterleaveVMatrix::get OUT OF BOUNDS");
#endif
  int n=vm.size();
  int m = i%n; // which VM 
  int pos = int(i/n) % vm[m].length(); // position within vm[m]
  return vm[m]->get(pos,j);
}

void InterleaveVMatrix::getSubRow(int i, int j, Vec v) const
{
#ifdef BOUNDCHECK
  if(i<0 || i>=length() || j<0 || j+v.length()>width())
    PLERROR("In InterleaveVMatrix::getRow OUT OF BOUNDS");
#endif
  int n=vm.size();
  int m = i%n; // which VM 
  int pos = int(i/n) % vm[m].length(); // position within vm[m]
  vm[m]->getSubRow(pos, j, v);
}

} // end of namespcae PLearn
