// -*- C++ -*-

// LLEKernel.cc
//
// Copyright (C) 2004 Olivier Delalleau 
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
   * $Id: LLEKernel.cc,v 1.6 2004/07/20 21:53:44 tihocan Exp $ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file LLEKernel.cc */


#include "LLEKernel.h"

namespace PLearn {
using namespace std;

//////////////////
// LLEKernel //
//////////////////
LLEKernel::LLEKernel() 
: build_in_progress(false),
  reconstruct_ker(new ReconstructionWeightsKernel()),
  knn(5),
  reconstruct_coeff(-1),
  regularizer(1e-6)
{
}

PLEARN_IMPLEMENT_OBJECT(LLEKernel,
    "The kernel used in Locally Linear Embedding.",
    "This kernel is the (weighted) sum of two kernels K' and K'', such that:\n"
    " - K'(x_i, x_j) = \\delta_{ij}\n"
    " - K'(x_i, x) = K'(x, x_i) = w(x, x_i), where w(x, x_i) is the weight of\n"
    "   x_i in the reconstruction of x by its knn nearest neighbors in the\n"
    "   training set\n"
    " - K'(x, y) = 0\n"
    " - K''(x_i, x_j) = W_{ij} + W_{ji} - \\sum_k W_{ki} W{kj}, where W is the\n"
    "   matrix of weights w(x_i, x_j)\n"
    " - K''(x, x_i) = K''(x_i, x) = 0\n"
    "The weight of K' is given by the 'reconstruct_coeff' option: when this\n"
    "weight tends to infinity, the mapping obtained is the same as the\n"
    "out-of-sample extension proposed in (Saul and Roweis, 2002). To obtain\n"
    "such a behavior, one should set 'reconstruct_coeff' to -1. This is the\n"
    "default behavior, and it is suggested to keep it.\n"
);

////////////////////
// declareOptions //
////////////////////
void LLEKernel::declareOptions(OptionList& ol)
{
  declareOption(ol, "knn", &LLEKernel::knn, OptionBase::buildoption,
      "The number of nearest neighbors considered.");

  declareOption(ol, "reconstruct_coeff", &LLEKernel::reconstruct_coeff, OptionBase::buildoption,
      "The weight of K' in the weighted sum of K' and K''. If set to a negative\n"
      "value, it will behave as its absolute value when evaluated on the training\n"
      "points with the evaluate_i_j method, but will return only its absolute value\n"
      "times K' when evaluated with the evaluate_i_x method.");

  declareOption(ol, "regularizer", &LLEKernel::regularizer, OptionBase::buildoption,
      "The regularization factor used to make the linear systems stable.");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

///////////
// build //
///////////
void LLEKernel::build()
{
  build_in_progress = true;
  inherited::build();
  build_();
}

////////////
// build_ //
////////////
void LLEKernel::build_()
{
  // Let's make sure the value of 'reconstruct_coeff' is not set accidentally
  // to an unusual value.
  if (reconstruct_coeff == 0) {
    PLWARNING("In LLEKernel::build_ - 'reconstruct_coeff' is set to 0, you won't be able to apply this kernel out-of-sample");
  } else if (reconstruct_coeff > 0) {
    PLWARNING("In LLEKernel::build_ - 'reconstruct_coeff' is > 0, this may give weird results out-of-sample for small coefficients");
  } else if (fabs(reconstruct_coeff + 1) > 1e-6) {
    PLWARNING("In LLEKernel::build_ - 'reconstruct_coeff' is negative but not -1, this may give some very weird stuff out-of-sample");
  }
  reconstruct_ker->regularizer = this->regularizer;
  reconstruct_ker->knn = this->knn;
  reconstruct_ker->report_progress = this->report_progress;
  reconstruct_ker->build();
  build_in_progress = false;
  // This code, normally executed in Kernel::build_, can only be executed
  // now beause the kernel 'reconstruct_ker' has to be initialized.
  if (specify_dataset) {
    this->setDataForKernelMatrix(specify_dataset);
  }
}

///////////////////////
// computeGramMatrix //
///////////////////////
void LLEKernel::computeGramMatrix(Mat K) const {
  reconstruct_ker->computeLLEMatrix(K);
  if (reconstruct_coeff != 0) {
    for (int i = 0; i < n_examples; i++) {
      K(i, i) += fabs(reconstruct_coeff);
    }
  }
}

//////////////
// evaluate //
//////////////
real LLEKernel::evaluate(const Vec& x1, const Vec& x2) const {
  PLERROR("In LLEKernel::evaluate - Not implemented");
  return 0;
}

//////////////////
// evaluate_i_j //
//////////////////
real LLEKernel::evaluate_i_j(int i, int j) const {
  if (i == j) {
    return
      fabs(reconstruct_coeff) +
      2 * reconstruct_ker->evaluate_i_j(i,i) -
      reconstruct_ker->evaluate_sum_k_i_k_j(i,i);
  } else {
    return
      reconstruct_ker->evaluate_i_j(j,i) +
      reconstruct_ker->evaluate_i_j(i,j) -
      reconstruct_ker->evaluate_sum_k_i_k_j(i,j);
  }
}

//////////////////
// evaluate_i_x //
//////////////////
real LLEKernel::evaluate_i_x(int i, const Vec& x, real squared_norm_of_x) const {
  static int j;
  PLERROR("In LLEKernel::evaluate_i_x - Not implemented");
  if (isInData(x, &j)) {
    return evaluate_i_j(i, j);
  } else {
    return reconstruct_coeff * reconstruct_ker->evaluate_x_i(x, i, squared_norm_of_x);
  }
}

////////////////////////
// evaluate_i_x_again //
////////////////////////
real LLEKernel::evaluate_i_x_again(int i, const Vec& x, real squared_norm_of_x, bool first_time) const {
  static int j;
  static bool is_training_point;
  if (reconstruct_coeff == 0) {
    // This kernel should only be evaluated on training points.
    if (first_time) {
      is_training_point = isInData(x, &j);
    }
    if (!is_training_point)
      return 0;
    return evaluate_i_j(i, j);
  } else if (reconstruct_coeff > 0) {
    if (first_time) {
      is_training_point = isInData(x, &j);
    }
    if (is_training_point) {
      return evaluate_i_j(i, j);
    } else {
      return reconstruct_coeff * reconstruct_ker->evaluate_x_i_again(x, i, squared_norm_of_x, first_time);
    }
  } else {
    // reconstruct_coeff < 0: we assume x is not a training point.
    return fabs(reconstruct_coeff) * reconstruct_ker->evaluate_x_i_again(x, i, squared_norm_of_x, first_time);
  }
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void LLEKernel::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("LLEKernel::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

////////////////////////////
// setDataForKernelMatrix //
////////////////////////////
void LLEKernel::setDataForKernelMatrix(VMat the_data) {
  if (build_in_progress)
    return;
  inherited::setDataForKernelMatrix(the_data);
  reconstruct_ker->setDataForKernelMatrix(the_data);
}

} // end of namespace PLearn

