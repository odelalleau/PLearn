// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
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
   * $Id: CompactVMatrixGaussianKernel.cc,v 1.4 2004/04/07 23:15:58 morinf Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "CompactVMatrixGaussianKernel.h"

namespace PLearn {
using namespace std;

// ** CompactVMatrixGaussianKernel **

PLEARN_IMPLEMENT_OBJECT(CompactVMatrixGaussianKernel, "ONE LINE DESCR", "NO HELP");

real CompactVMatrixGaussianKernel::evaluate(const Vec& x1, const Vec& x2) const
{ 
  // return exp(-powdistance(x1, x2, 2.0)/(sigma*sigma)); 
  real sqdiff=0;
  if (m)
  {
    sqdiff=m->squareDifference(int(x1[0]),int(x2[0]));
/*
#ifdef BOUNDCHECK
    m->setNormalMode();
    int actual_n_inputs = m->width()-1;
    Vec actual_x1(actual_n_inputs);
    Vec actual_x2(actual_n_inputs);
    m->getRow(int(x1[0]),actual_x1);
    m->getRow(int(x2[0]),actual_x2);
    m->setIndicesMode();
    real actual_sqdiff= powdistance(actual_x1, actual_x2, 2.0);
    if (fabs(sqdiff-actual_sqdiff)>0.1)
      PLWARNING("something wrong in CompactVMatrixGaussianKernel");
#endif
*/
  }
  else
    sqdiff= powdistance(x1, x2, real(2.0));
  return exp(-sqdiff/(sigma*sigma));
  
}

void CompactVMatrixGaussianKernel::setParameters(Vec paramvec)
{ sigma = paramvec[0]; }


void CompactVMatrixGaussianKernel::declareOptions(OptionList &ol)
{
    declareOption(ol, "sigma", &CompactVMatrixGaussianKernel::sigma, OptionBase::buildoption,
                  "TODO: Some comments");
    declareOption(ol, "m", &CompactVMatrixGaussianKernel::m, OptionBase::buildoption, "");
    inherited::declareOptions(ol);
}

} // end of namespace PLearn
