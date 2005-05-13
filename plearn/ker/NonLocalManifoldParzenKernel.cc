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
   * $Id: NonLocalManifoldParzenKernel.cc,v 1.1 2005/05/13 20:41:36 larocheh Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "NonLocalManifoldParzenKernel.h"

namespace PLearn {
using namespace std;


  PLEARN_IMPLEMENT_OBJECT(NonLocalManifoldParzenKernel, 
                          "Kernel that uses the diagonal gaussians learned by Manifold Parzen.", 
                          "It uses the evaluate method from the NonLocalManifoldParzen2 class.");

real NonLocalManifoldParzenKernel::evaluate(const Vec& x1, const Vec& x2) const
{ 
  real ret;
  if(is_symmetric)
    ret = mp->evaluate(x1,x2,scale) * mp->evaluate(x2,x1,scale);
  else
    ret = mp->evaluate(x1,x2,scale);
  
  return ret;
}

void NonLocalManifoldParzenKernel::declareOptions(OptionList& ol)
{
  declareOption(ol, "scale", &NonLocalManifoldParzenKernel::scale, OptionBase::buildoption,
                "The scale factor of the eigen values");
  declareOption(ol, "mp", &NonLocalManifoldParzenKernel::mp, OptionBase::buildoption,
                "Manifold Parzen distribution");
  declareOption(ol, "train_mp", &NonLocalManifoldParzenKernel::train_mp, OptionBase::buildoption,
                "Indication that the NonLocalManifoldParzen distribution should be trained");
  inherited::declareOptions(ol);
}

void NonLocalManifoldParzenKernel::setDataForKernelMatrix(VMat the_data)
{
  inherited::setDataForKernelMatrix(the_data);
  if(train_mp && data)
  {
    mp->setTrainingSet(data);
    PP<VecStatsCollector> stats = new VecStatsCollector();
    mp->setTrainStatsCollector(stats);
    mp->train();
    stats->finalize();
  }
  
  if(!train_mp && data) PLWARNING("NonLocalManifoldParzenKernel::setDataForKernelMatrix: data of kernel is possibly different from data of NonLocalManifoldParzen distribution.");
  
}

} // end of namespace PLearn

