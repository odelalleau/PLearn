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
   * $Id: ThresholdedKernel.cc,v 1.3 2005/06/16 18:34:50 tihocan Exp $ 
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
  max_size_for_full_gram(5000),
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
      "           is the k-th neighbor of x as given by K, and K(x,y) < K(n_k(y), y), then\n"
      "           K(x,y) is thresholded\n");

  declareOption(ol, "threshold", &ThresholdedKernel::threshold, OptionBase::buildoption,
      "The value returned when K(x,y) is thresholded.");

  declareOption(ol, "knn", &ThresholdedKernel::knn, OptionBase::buildoption,
      "When 'method' is 'knn', this is 'k' in n_k(x) (x will be counted if in data matrix).");

  declareOption(ol, "max_size_for_full_gram", &ThresholdedKernel::max_size_for_full_gram, OptionBase::buildoption,
      "When the dataset has more than 'max_size_for_full_gram' samples, the full Gram\n"
      "matrix will not be computed in memory (less efficient, but scales better).");

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
  if (source_kernel && !source_kernel->is_symmetric)
    PLERROR("In ThresholdedKernel::build_ - The source kernel must currently "
            "be symmetric");
}

///////////////////////
// computeGramMatrix //
///////////////////////
void ThresholdedKernel::computeGramMatrix(Mat K) const {
  if (cache_gram_matrix && gram_matrix_is_cached) {
    K << gram_matrix;
    return;
  }
  source_kernel->computeGramMatrix(K);
  thresholdGramMatrix(K);
  if (cache_gram_matrix) {
    int l = K.length();
    gram_matrix.resize(l,l);
    gram_matrix << K;
    gram_matrix_is_cached = true;
  }
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
    if (k_x1_x2 >= - k_x_xi[knn-1])
      return k_x1_x2;
    source_kernel->evaluate_all_i_x(k_x_xi, x2);
    partialSortRows(k_x_xi_mat, knn);
    negateElements(k_x_xi);
    if (k_x1_x2 >= -k_x_xi[knn-1])
      return k_x1_x2;
    return threshold;
  }
  return MISSING_VALUE;
}

//////////////////
// evaluate_i_j //
//////////////////
real ThresholdedKernel::evaluate_i_j(int i, int j) const {
  real k_i_j = source_kernel->evaluate_i_j(i, j);
  if (method == "knn") {
    if (k_i_j >= knn_kernel_values[i] || k_i_j >= knn_kernel_values[j])
      return k_i_j;
    else
      return threshold;
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
    if (k_i_x >= k_x_threshold || k_i_x >= knn_kernel_values[i])
      return k_i_x;
    else
      return threshold;
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
    if (k_x_i >= k_x_threshold || k_x_i >= knn_kernel_values[i])
      return k_x_i;
    else
      return threshold;
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
  ProgressBar* pb = 0;
  if (method == "knn") {
    knn_kernel_values.resize(n);
    if (n <= max_size_for_full_gram) {
      // Can afford to store the Gram matrix in memory.
      gram_matrix.resize(n,n);
      source_kernel->computeGramMatrix(gram_matrix);
      if (report_progress)
        pb = new ProgressBar("Finding nearest neighbors", n);
      Mat sorted_k_i(n, 1);
      for (int i = 0; i < n; i++) {
        sorted_k_i << gram_matrix(i);
        negateElements(sorted_k_i);       // For sorting.
        partialSortRows(sorted_k_i, knn);
        knn_kernel_values[i] = - sorted_k_i(knn - 1, 0);
        if (report_progress)
          pb->update(i+1);
      }
      if (pb)
        delete pb;
      if (cache_gram_matrix) {
        // Since we have the Gram matrix at hand, we may cache it now.
        thresholdGramMatrix(gram_matrix);
        gram_matrix_is_cached = true;
      } else
        // Free memory.
        gram_matrix = Mat();
    } else {
      // Computing the whole Gram matrix will probably not fit in memory.
      Vec k_i(n);
      if (cache_gram_matrix) {
        // We will cache the sparse Gram matrix.
        sparse_gram_matrix.resize(n);
        for (int i = 0; i < n; i++)
          sparse_gram_matrix[i].resize(0,2);
      }
      if (report_progress)
        pb = new ProgressBar("Computing Gram matrix of source kernel and "
                             "finding nearest neighbors", n);
      Mat k_i_mat(n, 1);
      Vec row(2);
      TVec<int> neighb_i, neighb_j;
      for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++)
          k_i[j] = source_kernel->evaluate_i_j(i,j);
        k_i_mat << k_i;
        negateElements(k_i_mat);  // For sorting.
        partialSortRows(k_i_mat, knn);
        knn_kernel_values[i] = - k_i_mat(knn - 1, 0);
        if (report_progress)
          pb->update(i+1);
        if (cache_gram_matrix) {
          // Let us cache the sparse Gram matrix.
          if (threshold != 0)
            PLWARNING("In ThresholdedKernel::setDataForKernelMatrix - The sparse "
                      "Gram matrix will be cached based on a non-zero threshold");
          real k_min = knn_kernel_values[i];
          Mat& g_i = sparse_gram_matrix[i];
          int ki = g_i.length();
          neighb_i.resize(ki);
          for (int j = 0; j < ki; j++)
            neighb_i[j] = int(g_i(j,0));
          for (int j = 0; j < n; j++)
            if (k_i[j] >= k_min && neighb_i.find(j) == -1) {
              row[0] = j;
              row[1] = k_i[j];
              g_i.appendRow(row);
              Mat& g_j = sparse_gram_matrix[j];
              int kj = g_j.length();
              bool already_there = false;
              for (int l = 0; l < kj; l++)
                if (i == int(g_j(0,1))) {
                  already_there = true;
                  break;
                }
              if (!already_there) {
                row[0] = i;
                g_j.appendRow(row);
              }
            }
          sparse_gram_matrix_is_cached = true;
        }
      }
      if (pb)
        delete pb;
    }
  }
}

/////////////////////////
// thresholdGramMatrix //
/////////////////////////
void ThresholdedKernel::thresholdGramMatrix(const Mat& K) const {
  ProgressBar* pb = 0;
  int n = K.length();
  if (K.width() != n)
    PLERROR("In ThresholdedKernel::thresholdGramMatrix - A square matrix is expected");
  if (report_progress)
    pb = new ProgressBar("Thresholding Gram matrix", n);
  if (method == "knn") {
    for (int i = 0; i < n; i++) {
      real* K_i = K[i];
      real knn_kernel_values_i = knn_kernel_values[i];
      for (int j = 0; j < n; j++, K_i++)
        if (*K_i < knn_kernel_values_i && *K_i < knn_kernel_values[j])
          *K_i = threshold;
      if (report_progress)
        pb->update(i+1);
    }
  }
  if (pb)
    delete pb;
}


} // end of namespace PLearn

