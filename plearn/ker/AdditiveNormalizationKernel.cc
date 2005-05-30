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
   * $Id: AdditiveNormalizationKernel.cc,v 1.11 2005/05/30 20:14:54 tihocan Exp $ 
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
: data_will_change(false),
  double_centering(false),
  remove_bias(false),
  remove_bias_in_evaluate(false)
{}

AdditiveNormalizationKernel::AdditiveNormalizationKernel
  (Ker the_source, bool the_remove_bias, bool the_remove_bias_in_evaluate,
                   bool the_double_centering)
: data_will_change(false),
  double_centering(the_double_centering),
  remove_bias(the_remove_bias),
  remove_bias_in_evaluate(the_remove_bias_in_evaluate)
{
  source_kernel = the_source;
  build();
}

PLEARN_IMPLEMENT_OBJECT(AdditiveNormalizationKernel,
    "Normalizes additively an underlying kernel with respect to a training set.",
    "From a kernel K, defines a new kernel K' such that:\n"
    "  K'(x,y) = K(x,y) - E[K(x,x_i)] - E[K(x_i,y)] + E[K(x_i,x_j)]\n"
    "where the expectation is performed on the data set.\n"
    "If the 'remove_bias' option is set, then the expectation will not\n"
    "take into account terms of the form K(x_i,x_i).\n"
    "If the 'double_centering' option is set, this kernel K' will be\n"
    "multiplied by -1/2 (this turns a squared distance kernel into a\n"
    "centered dot product kernel).\n"
);

////////////////////
// declareOptions //
////////////////////
void AdditiveNormalizationKernel::declareOptions(OptionList& ol)
{
  // Build options.

  declareOption(ol, "double_centering", &AdditiveNormalizationKernel::double_centering, OptionBase::buildoption,
      "If set to 1, then the resulting kernel will be multiplied by -1/2,\n"
      "which corresponds to the double-centering formula.");
  
  declareOption(ol, "data_will_change", &AdditiveNormalizationKernel::data_will_change, OptionBase::buildoption,
      "If set to 1, then the Gram matrix will be always recomputed, even if\n"
      "it's not completely sure the data has changed.");

  declareOption(ol, "remove_bias", &AdditiveNormalizationKernel::remove_bias, OptionBase::buildoption,
      "If set to 1, then the bias induced by the K(x_i,x_i) will be removed.\n");

  declareOption(ol, "remove_bias_in_evaluate", &AdditiveNormalizationKernel::remove_bias_in_evaluate, OptionBase::buildoption,
      "If set to 1, then the bias induced by the K(x_i,x_i) will be removed, but only when\n"
      "evaluating K(x,y) on test points (you don't need to do this if 'remove_bias' == 1).");

  // Learnt options.

  declareOption(ol, "average_col", &AdditiveNormalizationKernel::average_col, OptionBase::learntoption,
      "The average of the underlying kernel over each column of the Gram matrix.");

  declareOption(ol, "average_row", &AdditiveNormalizationKernel::average_row, OptionBase::learntoption,
      "The average of the underlying kernel over each row of the Gram matrix.");

  declareOption(ol, "total_average_unbiased", &AdditiveNormalizationKernel::total_average_unbiased, OptionBase::learntoption,
      "The average of the underlying kernel over the whole Gram matrix, without\n"
      "the diagonal terms.");

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
  if (double_centering)
    factor = -0.5;
  else
    factor = 1;
}

////////////////////
// computeAverage //
////////////////////
real AdditiveNormalizationKernel::computeAverage(const Vec& x, bool on_row, real squared_norm_of_x) const {
  all_k_x.resize(n_examples);
  if (is_symmetric || !on_row) {
    source_kernel->evaluate_all_i_x(x, all_k_x, squared_norm_of_x);
  } else {
    source_kernel->evaluate_all_x_i(x, all_k_x, squared_norm_of_x);
  }
  return sum(all_k_x) / real(n_examples);
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
  real avg_1 = computeAverage(x1, true);
  real avg_2 = computeAverage(x2, false);
  if (remove_bias || !remove_bias_in_evaluate) {
    // We can use the 'total_average'.
    return factor * (source_kernel->evaluate(x1, x2) - avg_1 - avg_2 + total_average);
  } else {
    // We need to use the 'total_average_unbiased'.
    return factor * (source_kernel->evaluate(x1, x2) - avg_1 - avg_2 + total_average_unbiased);
  }
}

//////////////////
// evaluate_i_j //
//////////////////
real AdditiveNormalizationKernel::evaluate_i_j(int i, int j) const {
  return factor * (source_kernel->evaluate_i_j(i,j) - average_row[i] - average_col[j] + total_average);
}

//////////////////
// evaluate_i_x //
//////////////////
real AdditiveNormalizationKernel::evaluate_i_x(int i, const Vec& x, real squared_norm_of_x) const {
  return factor * (source_kernel->evaluate_i_x(i, x, squared_norm_of_x)
       - average_row[i] - computeAverage(x, false, squared_norm_of_x) + total_average);
}

////////////////////////
// evaluate_i_x_again //
////////////////////////
real AdditiveNormalizationKernel::evaluate_i_x_again(int i, const Vec& x, real squared_norm_of_x, bool first_time) const {
  if (first_time) {
    avg_evaluate_i_x_again = computeAverage(x, false, squared_norm_of_x);
  }
  return factor * (source_kernel->evaluate_i_x_again(i, x, squared_norm_of_x, first_time)
       - average_row[i] - avg_evaluate_i_x_again + total_average);
}

//////////////////
// evaluate_x_i //
//////////////////
real AdditiveNormalizationKernel::evaluate_x_i(const Vec& x, int i, real squared_norm_of_x) const {
  return factor * (source_kernel->evaluate_x_i(x, i, squared_norm_of_x)
       - average_col[i] - computeAverage(x, true, squared_norm_of_x) + total_average);
}

////////////////////////
// evaluate_x_i_again //
////////////////////////
real AdditiveNormalizationKernel::evaluate_x_i_again(const Vec& x, int i, real squared_norm_of_x, bool first_time) const {
  if (first_time) {
    avg_evaluate_x_i_again = computeAverage(x, true, squared_norm_of_x);
  }
  return factor * (source_kernel->evaluate_x_i_again(x, i, squared_norm_of_x, first_time)
       - average_col[i] - avg_evaluate_x_i_again + total_average);
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void AdditiveNormalizationKernel::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(all_k_x, copies);
  deepCopyField(average_col, copies);
  deepCopyField(average_row, copies);
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
    real k_x_x;
    total_average_unbiased = 0;
    for (int i = 0; i < n; i++) {
      if (is_symmetric) {
        real v;
        k_x_x = gram(i,i);
        if (!remove_bias) {
          average_row[i] += k_x_x;
          total_average_unbiased -= k_x_x;
        }
        for (int j = i + 1; j < n; j++) {
          v = gram(i,j);
          average_row[i] += v;
          average_row[j] += v;
        }
      } else {
        for (int j = 0; j < n; j++) {
          if (!remove_bias || j != i) {
            average_row[i] += gram(i,j);
            average_col[i] += gram(j,i);
            if (j == i) {
              total_average_unbiased -= gram(i,j);
            }
          }
        }
      }
    }
    total_average = sum(average_row);
    if (remove_bias) {
      // The sum is already unbiased.
      total_average_unbiased = total_average;
    } else {
      // At this point, 'total_average_unbiased' = - \sum K(x_i,x_i).
      total_average_unbiased += total_average;
    }
    real n_terms_in_sum;    // The number of terms summed in average_row.
    if (remove_bias) {
      // The diagonal terms were not added.
      n_terms_in_sum = real(n - 1);
    } else {
      n_terms_in_sum = real(n);
    }
    total_average /= real(n * n_terms_in_sum);
    total_average_unbiased /= real(n * (n-1));
    average_row /= n_terms_in_sum;
    if (!is_symmetric) {
      average_col /= n_terms_in_sum;
    }
  }
}

} // end of namespace PLearn

