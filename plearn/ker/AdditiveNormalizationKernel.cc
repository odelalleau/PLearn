// -*- C++ -*-

// AdditiveNormalizationKernel.cc
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
   * $Id: AdditiveNormalizationKernel.cc,v 1.2 2004/04/21 17:38:56 tihocan Exp $ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file AdditiveNormalizationKernel.cc */


#include "AdditiveNormalizationKernel.h"

namespace PLearn {
using namespace std;

/////////////////////////////////
// AdditiveNormalizationKernel //
/////////////////////////////////
AdditiveNormalizationKernel::AdditiveNormalizationKernel() 
/* ### Initialize all fields to their default value here */
: data_will_change(false)
{}

PLEARN_IMPLEMENT_OBJECT(AdditiveNormalizationKernel,
    "Normalizes additively an underlying kernel.",
    "From a kernel K, defines a new kernel K' such that:\n"
    "  K'(x,y) = K(x,y) - E[K(x,x_i)] - E[K(x_i,y)] + E[K(x_i,x_j)]\n"
    "where the expectation is performed on the data set."
);

////////////////////
// declareOptions //
////////////////////
void AdditiveNormalizationKernel::declareOptions(OptionList& ol)
{
  // ### Declare all of this object's options here
  // ### For the "flags" of each option, you should typically specify  
  // ### one of OptionBase::buildoption, OptionBase::learntoption or 
  // ### OptionBase::tuningoption. Another possible flag to be combined with
  // ### is OptionBase::nosave

  // Build options.
  
  declareOption(ol, "data_will_change", &AdditiveNormalizationKernel::data_will_change, OptionBase::buildoption,
      "If set to 1, then the Gram matrix will be always recomputed, even if\n"
      "it's not completely sure the data has changed.");

  // Learnt options.

  declareOption(ol, "average_col", &AdditiveNormalizationKernel::average_col, OptionBase::learntoption,
      "The average of the underlying kernel over each column of the Gram matrix.");

  declareOption(ol, "average_row", &AdditiveNormalizationKernel::average_row, OptionBase::learntoption,
      "The average of the underlying kernel over each row of the Gram matrix.");

  declareOption(ol, "total_average", &AdditiveNormalizationKernel::total_average, OptionBase::learntoption,
      "The average of the underlying kernel over the whole Gram matrix.");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

///////////
// build //
///////////
void AdditiveNormalizationKernel::build()
{
  // ### Nothing to add here, simply calls build_
  inherited::build();
  build_();
}

////////////
// build_ //
////////////
void AdditiveNormalizationKernel::build_()
{
  // ### This method should do the real building of the object,
  // ### according to set 'options', in *any* situation. 
  // ### Typical situations include:
  // ###  - Initial building of an object from a few user-specified options
  // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
  // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
  // ### You should assume that the parent class' build_() has already been called.

}

////////////////////
// computeAverage //
////////////////////
real AdditiveNormalizationKernel::computeAverage(const Vec& x, bool on_row, real squared_norm_of_x) const {
  Vec k_x(data->length());
  if (is_symmetric || !on_row) {
    source_kernel->evaluate_all_i_x(x, k_x, squared_norm_of_x);
  } else {
    source_kernel->evaluate_all_x_i(x, k_x, squared_norm_of_x);
  }
  return sum(k_x) / real(data->length());
}

///////////////////////
// computeGramMatrix //
///////////////////////
void AdditiveNormalizationKernel::computeGramMatrix(Mat K) const {
  // Uses default Kernel implementation.
  Kernel::computeGramMatrix(K);
}

//////////////
// evaluate //
//////////////
real AdditiveNormalizationKernel::evaluate(const Vec& x1, const Vec& x2) const {
  // ### Evaluate the kernel on a pair of points.
  real avg_1 = computeAverage(x1, true);
  real avg_2 = computeAverage(x2, false);
  return source_kernel->evaluate(x1, x2) - avg_1 - avg_2 + total_average;
}

//////////////////
// evaluate_i_j //
//////////////////
real AdditiveNormalizationKernel::evaluate_i_j(int i, int j) const {
  return source_kernel->evaluate_i_j(i,j) - average_row[i] - average_col[j] + total_average;
}

//////////////////
// evaluate_i_x //
//////////////////
real AdditiveNormalizationKernel::evaluate_i_x(int i, const Vec& x, real squared_norm_of_x) const {
  return source_kernel->evaluate_i_x(i, x, squared_norm_of_x)
       - average_row[i] - computeAverage(x, false, squared_norm_of_x) + total_average;
}

////////////////////////
// evaluate_i_x_again //
////////////////////////
real AdditiveNormalizationKernel::evaluate_i_x_again(int i, const Vec& x, real squared_norm_of_x, bool first_time) {
  if (first_time) {
    avg_evaluate_i_x_again = computeAverage(x, false, squared_norm_of_x);
  }
  return source_kernel->evaluate_i_x_again(i, x, squared_norm_of_x, first_time)
       - average_row[i] - avg_evaluate_i_x_again + total_average;
}

//////////////////
// evaluate_x_i //
//////////////////
real AdditiveNormalizationKernel::evaluate_x_i(const Vec& x, int i, real squared_norm_of_x) const {
  return source_kernel->evaluate_x_i(x, i, squared_norm_of_x)
       - average_col[i] - computeAverage(x, true, squared_norm_of_x) + total_average;
}

////////////////////////
// evaluate_x_i_again //
////////////////////////
real AdditiveNormalizationKernel::evaluate_x_i_again(const Vec& x, int i, real squared_norm_of_x, bool first_time) {
  if (first_time) {
    avg_evaluate_x_i_again = computeAverage(x, true, squared_norm_of_x);
  }
  return source_kernel->evaluate_x_i_again(x, i, squared_norm_of_x, first_time)
       - average_col[i] - avg_evaluate_x_i_again + total_average;
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void AdditiveNormalizationKernel::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("AdditiveNormalizationKernel::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

////////////////////////////
// setDataForKernelMatrix //
////////////////////////////
void AdditiveNormalizationKernel::setDataForKernelMatrix(VMat the_data) {
  bool there_was_data_and_it_changed = data && !(data->looksTheSameAs(the_data));
  // Set the data for this kernel as well as for the underlying kernel.
  inherited::setDataForKernelMatrix(the_data);
  // Check whether we need to recompute the Gram matrix and its average.
  int n = the_data->length();
  if (   data_will_change
      || average_row.length() != n
      || there_was_data_and_it_changed) {
    // Compute the underlying Gram matrix.
    Mat gram(n, n);
    source_kernel->computeGramMatrix(gram);
    // Compute the row (and column) average.
    average_row.resize(n);
    average_row.fill(0);
    if (is_symmetric) {
      average_col = average_row;
    } else {
      average_col.resize(n);
      average_col.fill(0);
    }
    for (int i = 0; i < n; i++) {
      if (is_symmetric) {
        real v;
        average_row[i] += gram(i,i);
        for (int j = i + 1; j < n; j++) {
          v = gram(i,j);
          average_row[i] += v;
          average_row[j] += v;
        }
      } else {
        for (int j = 0; j < n; j++) {
          average_row[i] += gram(i,j);
          average_col[i] += gram(j,i);
        }
      }
    }
    total_average = sum(average_row) / real(n * n);
    average_row /= real(n);
    if (!is_symmetric) {
      average_col /= real(n);
    }
  }
}

} // end of namespace PLearn

