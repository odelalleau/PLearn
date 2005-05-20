// -*- C++ -*-

// CorrelationKernel.cc
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
   * $Id: CorrelationKernel.cc,v 1.1 2005/05/20 13:55:09 tihocan Exp $ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file CorrelationKernel.cc */


#include "CorrelationKernel.h"
#include <plearn/math/TMat_sort.h>          //!< For sortElements().
#include <plearn/vmat/TransposeVMatrix.h>
#include <plearn/vmat/VMat_basic_stats.h>

namespace PLearn {
using namespace std;

///////////////////////
// CorrelationKernel //
///////////////////////
CorrelationKernel::CorrelationKernel() 
: correlation("linear"),
  transform(""),
  var_threshold(0)
{
}

PLEARN_IMPLEMENT_OBJECT(CorrelationKernel,
    "Compute a similarity measure between two variables by their correlation.",
    "Here, the two examples x and y in K(x,y) are understood as being samples\n"
    "of a two random variables.\n"
);

////////////////////
// declareOptions //
////////////////////
void CorrelationKernel::declareOptions(OptionList& ol)
{
  // ### Declare all of this object's options here
  // ### For the "flags" of each option, you should typically specify  
  // ### one of OptionBase::buildoption, OptionBase::learntoption or 
  // ### OptionBase::tuningoption. Another possible flag to be combined with
  // ### is OptionBase::nosave

  declareOption(ol, "correlation", &CorrelationKernel::correlation, OptionBase::buildoption,
      "The correlation method used to compute the similarity, among:\n"
      "- 'linear' : linear correlation\n");

  declareOption(ol, "transform", &CorrelationKernel::transform, OptionBase::buildoption,
      "An additional transformation applied on the similarity, among:\n"
      "- ''               : no transformation\n"
      "- 'minus_log'      : - log(similarity)\n"
      "- 'minus_log_abs'  : - log(|similarity|)\n");

  declareOption(ol, "var_threshold", &CorrelationKernel::var_threshold, OptionBase::buildoption,
      "If set to a value > 0, denote by x_i the training point whose variance is the\n"
      "'var_threshold' quantile of all training variances. If v_i is its variance, then\n"
      "all pairs of points whose product of variances is less than v_i^2 will be given a\n"
      "similarity (correlation) of 1e-3.\n");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

///////////
// build //
///////////
void CorrelationKernel::build()
{
  inherited::build();
  build_();
}

////////////
// build_ //
////////////
void CorrelationKernel::build_()
{
  // ### This method should do the real building of the object,
  // ### according to set 'options', in *any* situation. 
  // ### Typical situations include:
  // ###  - Initial building of an object from a few user-specified options
  // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
  // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
  // ### You should assume that the parent class' build_() has already been called.
}

//////////////
// evaluate //
//////////////
real CorrelationKernel::evaluate(const Vec& x1, const Vec& x2) const {
  real result = 0;
  if (correlation == "linear") {
    int n = x1.length();
#ifdef BOUNDCHECK
    if (x2.length() != n)
      PLERROR("In CorrelationKernel::evaluate - x1 and x2 must have same size");
#endif
    VMat x1_(x1.toMat(n, 1));
    VMat x2_(x2.toMat(n, 1));
    x1_->defineSizes(n, 0, 0);
    x2_->defineSizes(n, 0, 0);
    correlations(x1_, x2_, correl, pvalues, true);
    result = correl(0,0);
  } else
    PLERROR("In CorrelationKernel::evaluate - Unknown value for 'correlation': "
            "%s", correlation.c_str());
  if (var_threshold > 0) {
    real v_1 = variance(x1, mean(x1));
    real v_2 = variance(x2, mean(x2));
    if (v_1 * v_2 < min_product_var)
      result = 1e-3;
  }
  if (transform.empty())
    return result;
  else if (transform == "minus_log")
    return -log(result);
  else if (transform == "minus_log_abs")
    return -log(fabs(result));
  else {
    PLERROR("In CorrelationKernel::evaluate - Unknown value for 'transform': "
            "%s", transform.c_str());
    return 0; // To make the compiler happy.
  }
}

/* ### This method will very often be overridden.
//////////////////
// evaluate_i_j //
//////////////////
real CorrelationKernel::evaluate_i_j(int i, int j) const {
  // ### Evaluate the kernel on a pair of training points.
}
*/

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void CorrelationKernel::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("CorrelationKernel::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

////////////////////////////
// setDataForKernelMatrix //
////////////////////////////
void CorrelationKernel::setDataForKernelMatrix(VMat the_data) {
  inherited::setDataForKernelMatrix(the_data);
  if (var_threshold > 0) {
    // Compute variances.
    VMat transp = new TransposeVMatrix(the_data);
    computeMeanAndVariance(transp, mean_vec, var_vec);
    // Compute quantile.
    sortElements(var_vec);
    int q = (int) floor(var_threshold * (var_vec.length() - 1));
    min_product_var = var_vec[q] * var_vec[q];
  }
}

} // end of namespace PLearn

