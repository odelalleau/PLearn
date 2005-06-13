// -*- C++ -*-

// ThresholdedKernel.cc
//
// Copyright (C) 2005 Olivier Delalleau 
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
   * $Id: ThresholdedKernel.cc,v 1.1 2005/06/13 18:36:16 tihocan Exp $ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file ThresholdedKernel.cc */


#include "ThresholdedKernel.h"

namespace PLearn {
using namespace std;

///////////////////////
// ThresholdedKernel //
///////////////////////
ThresholdedKernel::ThresholdedKernel() 
: knn(2),
  method("knn"),
  threshold(0)
{
}

PLEARN_IMPLEMENT_OBJECT(ThresholdedKernel,
    "Thresholds an underlying kernel.",
    ""
);

////////////////////
// declareOptions //
////////////////////
void ThresholdedKernel::declareOptions(OptionList& ol)
{
  // ### Declare all of this object's options here
  // ### For the "flags" of each option, you should typically specify  
  // ### one of OptionBase::buildoption, OptionBase::learntoption or 
  // ### OptionBase::tuningoption. Another possible flag to be combined with
  // ### is OptionBase::nosave

  declareOption(ol, "method", &ThresholdedKernel::method, OptionBase::buildoption,
      "Which method is used to threshold the underlying kernel:\n"
      " - 'knn' : if y is such that K(x,y) is strictly less than K(x,n_k(x)) where n_k(x)\n"
      "           is the k-th neighbor of x as given by K, then K(x,y) is thresholded (in \n"
      "           addition, the condition K(x,y) < K(n_k(y), y) is also tested)\n");

  declareOption(ol, "threshold", &ThresholdedKernel::threshold, OptionBase::buildoption,
      "The value returned when K(x,y) is thresholded.");

  declareOption(ol, "knn", &ThresholdedKernel::knn, OptionBase::buildoption,
      "When 'method' is 'knn', this is 'k' in n_k(x) (x will be counted if in data matrix).");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

///////////
// build //
///////////
void ThresholdedKernel::build()
{
  inherited::build();
  build_();
}

////////////
// build_ //
////////////
void ThresholdedKernel::build_()
{
}

///////////////////////
// computeGramMatrix //
///////////////////////
void ThresholdedKernel::computeGramMatrix(Mat K) const {
  // Default = uses the Kernel implementation.
  Kernel::computeGramMatrix(K);
}

//////////////
// evaluate //
//////////////
real ThresholdedKernel::evaluate(const Vec& x1, const Vec& x2) const {
  real k_x1_x2 = source_kernel->evaluate(x1, x2);
  if (method == "knn") {
    source_kernel->evaluate_all_x_i(x1, k_x_xi);
    negateElements(k_x_xi);
    partialSortRows(k_x_xi_mat, knn);
    if (k_x1_x2 < - k_x_xi[knn-1])
      return threshold;
    source_kernel->evaluate_all_i_x(k_x_xi, x2);
    partialSortRows(k_x_xi_mat, knn);
    negateElements(k_x_xi);
    if (k_x1_x2 < -k_x_xi[knn-1])
      return threshold;
    return k_x1_x2;
  }
  return MISSING_VALUE;
}

//////////////////
// evaluate_i_j //
//////////////////
real ThresholdedKernel::evaluate_i_j(int i, int j) const {
  real k_i_j = source_kernel->evaluate_i_j(i, j);
  if (method == "knn") {
    if (k_i_j < knn_kernel_values[i] || k_i_j < knn_kernel_values[j])
      return threshold;
    else
      return k_i_j;
  }
  return MISSING_VALUE;
}

//////////////////
// evaluate_i_x //
//////////////////
real ThresholdedKernel::evaluate_i_x(int i, const Vec& x, real squared_norm_of_x) const {
  // Default = uses the Kernel implementation.
  // Alternative = return source_kernel->evaluate_i_x(i,x,squared_norm_of_x);
  return Kernel::evaluate_i_x(i, x, squared_norm_of_x);
}

////////////////////////
// evaluate_i_x_again //
////////////////////////
real ThresholdedKernel::evaluate_i_x_again(int i, const Vec& x, real squared_norm_of_x, bool first_time) const {
  if (method == "knn") {
    if (first_time) {
      source_kernel->evaluate_all_i_x(x, k_x_xi);
      negateElements(k_x_xi);
      partialSortRows(k_x_xi_mat, knn);
      k_x_threshold = - k_x_xi[knn - 1];
    }
    real k_i_x = source_kernel->evaluate_i_x_again(i, x, squared_norm_of_x, first_time);
    if (k_i_x < k_x_threshold || k_i_x < knn_kernel_values[i])
      return threshold;
    else
      return k_i_x;
  }
  return MISSING_VALUE;
}

//////////////////
// evaluate_x_i //
//////////////////
real ThresholdedKernel::evaluate_x_i(const Vec& x, int i, real squared_norm_of_x) const {
  return Kernel::evaluate_x_i(x, i, squared_norm_of_x);
}

////////////////////////
// evaluate_x_i_again //
////////////////////////
real ThresholdedKernel::evaluate_x_i_again(const Vec& x, int i, real squared_norm_of_x, bool first_time) const {
  if (method == "knn") {
    if (first_time) {
      source_kernel->evaluate_all_x_i(x, k_x_xi);
      negateElements(k_x_xi);
      partialSortRows(k_x_xi_mat, knn);
      k_x_threshold = - k_x_xi[knn - 1];
    }
    real k_x_i = source_kernel->evaluate_x_i_again(x, i, squared_norm_of_x, first_time);
    if (k_x_i < k_x_threshold || k_x_i < knn_kernel_values[i])
      return threshold;
    else
      return k_x_i;
  }
  return MISSING_VALUE;
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void ThresholdedKernel::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("ThresholdedKernel::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

////////////////////////////
// setDataForKernelMatrix //
////////////////////////////
void ThresholdedKernel::setDataForKernelMatrix(VMat the_data) {
  inherited::setDataForKernelMatrix(the_data);
  int n = the_data->length();
  k_x_xi.resize(n);
  k_x_xi_mat = k_x_xi.toMat(n, 1);
  if (method == "knn") {
    knn_kernel_values.resize(n);
    Mat K(n,n);
    source_kernel->computeGramMatrix(K);
    negateElements(K); // For later sorting.
    ProgressBar* pb = 0;
    if (report_progress)
      pb = new ProgressBar("Finding nearest neighbors", n);
    for (int i = 0; i < n; i++) {
      Mat k_i = K.column(i);
      partialSortRows(k_i, knn);
      knn_kernel_values[i] = - k_i(knn - 1, 0);
      if (report_progress)
        pb->update(i+1);
    }
    if (pb)
      delete pb;
  }
}

} // end of namespace PLearn

