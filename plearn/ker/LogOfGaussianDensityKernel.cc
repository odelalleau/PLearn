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
   * $Id: LogOfGaussianDensityKernel.cc,v 1.4 2004/04/07 23:17:52 morinf Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "LogOfGaussianDensityKernel.h"

namespace PLearn {
using namespace std;


PLEARN_IMPLEMENT_OBJECT(LogOfGaussianDensityKernel, "ONE LINE DESCR", "NO HELP");

real LogOfGaussianDensityKernel::evaluate(const Vec& x1, const Vec& x2) const
{ 
  // cerr << "LogOfGaussKernel mu: " << x1 << endl; 
  double sigmasq = sigma*sigma;
  // cerr << "sqnorm_xmu " << powdistance(x1, x2, real(2.0)) << endl;
  // cerr << "sigmasq " << sigmasq << endl;
  double q = powdistance(x1, x2, real(2.0))/sigmasq;
  // cerr << "log of gauss kernel q = " << q << endl; 
  //double logp = -0.5 * ( q + x1.length()*( log(2*M_PI) + log(sigmasq)) ); 
  double logp = -0.5 * ( q + x1.length()*( log(2*Pi) + log(sigmasq)) ); 
  // cerr << "logp = " << logp << endl;
  // exit(0);
  return real(logp);
}


void LogOfGaussianDensityKernel::declareOptions(OptionList& ol)
{
  declareOption(ol, "sigma", &LogOfGaussianDensityKernel::sigma, OptionBase::buildoption,
                "The width of the Gaussian");
  inherited::declareOptions(ol);
}



} // end of namespace PLearn

