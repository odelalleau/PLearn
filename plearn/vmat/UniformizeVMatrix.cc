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
   * $Id: UniformizeVMatrix.cc,v 1.2 2004/02/20 21:14:44 chrish42 Exp $
   ******************************************************* */

#include "UniformizeVMatrix.h"

namespace PLearn {
using namespace std;


/** UniformizeVMatrix **/

UniformizeVMatrix::UniformizeVMatrix(VMat the_distr, Mat the_bins,
  Vec the_index, real the_a, real the_b)
  : RowBufferedVMatrix(the_distr->length(), the_distr->width()),
  distr(the_distr), bins(the_bins), index(the_index), a(the_a), b(the_b)
{
  fieldinfos = distr->getFieldInfos();
  
  if (a >= b)
    PLERROR("In UniformizeVMatrix: a (%f) must be strictly smaller than b (%f)", a, b);
  if (index.length() != bins.length())
    PLERROR("In UniformizeVMatrix: the number of elements in index (%d) must equal the number of rows in bins (%d)", index.length(), bins.length());
  if (min(index)<0 || max(index)>distr->length()-1)
    PLERROR("In UniformizeVMatrix: all values of index must be in range [0,%d]",
      distr->length()-1);
}

void UniformizeVMatrix::getRow(int i, Vec v) const
{
#ifdef BOUNDCHECK
  if(i<0 || i>=length())
    PLERROR("In UniformizeVMatrix::getRow OUT OF BOUNDS");
  if(v.length() != width())
    PLERROR("In UniformizeVMatrix::getRow v.length() must be equal to the VMat's width");
#endif

  distr->getRow(i, v);
  for(int j=0; j<v.length(); j++) {
    if (vec_find(index, (real)j) != -1) {
      Vec x_bin = bins(j);
      real xx = estimatedCumProb(v[j], x_bin);
      v[j] = xx*(b-a) - a;
    }
  }
}

} // end of namespcae PLearn
