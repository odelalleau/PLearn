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
   * $Id: DistanceKernel.cc,v 1.5 2004/06/16 18:24:45 tihocan Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "DistanceKernel.h"
#include "SelectedOutputCostFunction.h"

namespace PLearn {
using namespace std;


PLEARN_IMPLEMENT_OBJECT(DistanceKernel, "ONE LINE DESCR", "NO HELP");

////////////////////
// DistanceKernel //
////////////////////
DistanceKernel::DistanceKernel(real the_Ln)
: n(the_Ln),
  pow_distance(false)
{}

////////////////////
// declareOptions //
////////////////////
void DistanceKernel::declareOptions(OptionList& ol)
{

  declareOption(ol, "n", &DistanceKernel::n, OptionBase::buildoption, 
      "This class implements a Ln distance (L2, the default is the usual euclidean distance).");

  declareOption(ol, "pow_distance", &DistanceKernel::pow_distance, OptionBase::buildoption, 
      "If set to 1, the distance computed will be elevated to power n.");

  inherited::declareOptions(ol);
}

//////////////
// evaluate //
//////////////
real DistanceKernel::evaluate(const Vec& x1, const Vec& x2) const {
  if (pow_distance) {
    return powdistance(x1, x2, n);
  } else {
    return dist(x1, x2, n);
  }
}

//////////////////
// evaluate_i_j //
//////////////////
real DistanceKernel::evaluate_i_j(int i, int j) const {
  static real d;
  if (n == 2.0) {
    d = squarednorms[i] + squarednorms[j] - 2 * data->dot(i, j, data_inputsize);
    if (pow_distance)
      return d;
    else
      return sqrt(d);
  } else {
    return inherited::evaluate_i_j(i,j);
  }
}

////////////////////////////
// setDataForKernelMatrix //
////////////////////////////
void DistanceKernel::setDataForKernelMatrix(VMat the_data)
{
  inherited::setDataForKernelMatrix(the_data);
  if (n == 2.0) {
    squarednorms.resize(data.length());
    for(int index=0; index<data.length(); index++)
      squarednorms[index] = data->dot(index, index, data_inputsize);
  }
}

////////////////////////
// absolute_deviation //
////////////////////////
CostFunc absolute_deviation(int singleoutputindex)
{ 
  if(singleoutputindex>=0)
    return new SelectedOutputCostFunction(new DistanceKernel(1.0),singleoutputindex); 
  else
    return new DistanceKernel(1.0); 
}

} // end of namespace PLearn

