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
   * $Id: GaussianKernel.cc,v 1.4 2004/02/20 21:11:45 chrish42 Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "GaussianKernel.h"

// From Old Kernel.cc: all includes are putted in every file.
// To be revised manually 
#include <cmath>
#include "stringutils.h"
#include "Kernel.h"
#include "TMat_maths.h"
#include "PLMPI.h"
//////////////////////////
namespace PLearn {
using namespace std;


// ** GaussianKernel **

PLEARN_IMPLEMENT_OBJECT(GaussianKernel, "ONE LINE DESCR", "NO HELP");

void GaussianKernel::declareOptions(OptionList& ol)
{
  declareOption(ol, "sigma", &GaussianKernel::sigma, OptionBase::buildoption,
                "The width of the Gaussian");
  inherited::declareOptions(ol);
}


void GaussianKernel::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  Kernel::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(squarednorms,copies);
}


void GaussianKernel::setDataForKernelMatrix(VMat the_data)
{ 
  // Will be needed if is_sequential is true
  int previous_data_length = 0;
  if(data.isNotNull())
    previous_data_length = data.length();

  Kernel::setDataForKernelMatrix(the_data);

  squarednorms.resize(data.length());

  int index = 0;
  if(is_sequential)
    index = previous_data_length;

  for(; index<data.length(); index++)
    squarednorms[index] = data->dot(index,index, data_inputsize);
}


inline real GaussianKernel::evaluateFromSquaredNormOfDifference(real sqnorm_of_diff) const
{ return exp(sqnorm_of_diff*minus_one_over_sigmasquare); }


real GaussianKernel::evaluate(const Vec& x1, const Vec& x2) const
{
#ifdef BOUNDCHECK
  if(x1.length()!=x2.length())
    PLERROR("IN GaussianKernel::evaluate x1 and x2 must have the same length");
#endif
  int l = x1.length();
  real* px1 = x1.data();
  real* px2 = x2.data();
  real sqnorm_of_diff = 0.;
  for(int i=0; i<l; i++)
    {
      real val = px1[i]-px2[i];
      sqnorm_of_diff += val*val;
    }
  return evaluateFromSquaredNormOfDifference(sqnorm_of_diff);
}


real GaussianKernel::evaluate_i_j(int i, int j) const
{ return evaluateFromDotAndSquaredNorm(squarednorms[i],data->dot(i,j,data_inputsize),squarednorms[j]); }


real GaussianKernel::evaluate_i_x(int i, const Vec& x, real squared_norm_of_x) const 
{ 
  if(squared_norm_of_x<0.)
    squared_norm_of_x = pownorm(x);
//   real dot_x1_x2 = data->dot(i,x);
//   cout << "data.row(" << i << "): " << data.row(i) << endl 
//        << "squarednorms[" << i << "]: " << squarednorms[i] << endl
//        << "data->dot(i,x): " << dot_x1_x2 << endl
//        << "x: " << x << endl
//        << "squared_norm_of_x: " << squared_norm_of_x << endl;
//   real sqnorm_of_diff = (squarednorms[i]+squared_norm_of_x)-(dot_x1_x2+dot_x1_x2);
//   cout << "a-> sqnorm_of_diff: " << sqnorm_of_diff << endl
//        << "b-> minus_one_over_sigmasquare: " << minus_one_over_sigmasquare << endl
//        << "a*b: " << sqnorm_of_diff*minus_one_over_sigmasquare << endl
//        << "res: " << exp(sqnorm_of_diff*minus_one_over_sigmasquare) << endl; 
  return evaluateFromDotAndSquaredNorm(squarednorms[i],data->dot(i,x),squared_norm_of_x); 
}


real GaussianKernel::evaluate_x_i(const Vec& x, int i, real squared_norm_of_x) const
{ 
  if(squared_norm_of_x<0.)
    squared_norm_of_x = pownorm(x);
  return evaluateFromDotAndSquaredNorm(squared_norm_of_x,data->dot(i,x),squarednorms[i]); 
}


void GaussianKernel::setParameters(Vec paramvec)
{ 
  PLWARNING("In GaussianKernel: setParameters is deprecated, use setOption instead");
  sigma = paramvec[0]; 
  minus_one_over_sigmasquare = -1.0/(sigma*sigma);
}


void GaussianKernel::oldread(istream& in)
{
  readHeader(in,"GaussianKernel");
  inherited::oldread(in);
  readField(in,"sigma",sigma);
  readFooter(in,"GaussianKernel");
  minus_one_over_sigmasquare = -1.0/(sigma*sigma);
}


void GaussianKernel::build_()
{
  minus_one_over_sigmasquare = -1.0/square(sigma);
}


void GaussianKernel::build()
{
  inherited::build();
  build_();
}



} // end of namespace PLearn

