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
   * $Id: LaplacianKernel.cc,v 1.4 2004/04/07 23:15:17 morinf Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "LaplacianKernel.h"

namespace PLearn {
using namespace std;


PLEARN_IMPLEMENT_OBJECT(LaplacianKernel, "ONE LINE DESCR", "NO HELP");

real LaplacianKernel::evaluate(const Vec& x1, const Vec& x2) const
{ 
#ifdef BOUNDCHECK
  if(x1.length()!=x2.length())
    PLERROR("IN LaplacianKernel::evaluate x1 and x2 must have the same length");
#endif

  real summ = 0.0;
  real* v1=x1.data();
  real* v2=x2.data();
  int n=x1.length();
  for(int i=0; i<n; i++)
    summ += fabs(v1[i]-v2[i]);
  return exp(-phi*summ);
}

void LaplacianKernel::declareOptions(OptionList &ol)
{
    declareOption(ol, "phi", &LaplacianKernel::phi, OptionBase::buildoption,
                  "TODO: Some comments");
    inherited::declareOptions(ol);
}

} // end of namespace PLearn

