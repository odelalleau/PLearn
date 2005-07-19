// -*- C++ -*-

// PDistribution.cc
//
// Copyright (C) 2003  Pascal Vincent 
// Copyright (C) 2004  Université de Montréal
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
   * $Id$ 
   ******************************************************* */

/*! \file PDistribution.cc */

#include "PDistribution.h"
#include <plearn/base/tostring.h>
#include <plearn/math/TMat_maths_impl.h>

namespace PLearn {
using namespace std;

///////////////////
// PDistribution //
///////////////////
PDistribution::PDistribution() 
: n_input(0),
  n_margin(0),
  n_target(0),
  already_sorted(false),
  delta_curve(0.1),
  full_joint_distribution(true),
  need_set_input(true),
  lower_bound(0.),
  upper_bound(0.),
  n_curve_points(-1),
  outputs_def("l")
{}

PLEARN_IMPLEMENT_OBJECT(PDistribution, 
    "PDistribution is the base class for distributions.\n",
    "PDistributions derive from PLearner (as some of them may be fitted to data with train()),\n"
    "but they have additional methods allowing for ex. to compute density or generate data points.\n"
    "The default implementations of the learner-type methods for computing outputs and costs work as follows:\n"
    "  - the outputs_def option allows to choose which outputs are produced\n"
    "  - cost is a vector of size 1 containing only the negative log-likelihood (NLL), i.e. -log_density.\n"
    "A PDistribution may be conditional P(Y|X), if the option 'conditional_flags' is set. If it is the case,\n"
    "the input should always be made of both the 'input' part (X) and the 'target' part (Y), even if the\n"
    "output may not need to use the Y part. The exception is when computeOutput() needs to be called\n"
    "successively with the same value of X: in this case, after a first call with both X and Y, one may\n"
    "only provide Y as input, and X will be assumed to be unchanged.\n"
);

////////////////////
// declareOptions //
////////////////////
void PDistribution::declareOptions(OptionList& ol)
{

  // Build options.

  declareOption(
    ol, "outputs_def", &PDistribution::outputs_def, OptionBase::buildoption,
    "Defines what will be given in output. This is a string where the characters\n"
    "have the following meaning:\n"
    "- 'l' : log_density\n"
    "- 'd' : density\n"
    "- 'c' : cdf\n"
    "- 's' : survival_fn\n"
    "- 'e' : expectation\n"
    "- 'v' : variance.\n"
    "\n"
    "If these options are specified in lower case they give the value associated with a given observation.\n"
    "In upper case, a curve is evaluated at regular intervals and produced in\n"
    "output (as a histogram). For 'L', 'D', 'C', 'S', it is the target part that\n"
    "varies, while for 'E' and 'V' it is the input part (for conditional distributions).\n"
    "The number of curve points is determined by the 'n_curve_points' option.\n"
    "Note that the upper case letters only work for SCALAR variables.\n"
    );

  declareOption(ol, "conditional_flags", &PDistribution::conditional_flags, OptionBase::buildoption,
      "This vector should be set for conditional distributions. It indicates what\n"
      "each input variable corresponds to:\n"
      " - 0 = it is marginalized (it does not appear in the distribution Y|X)\n"
      " - 1 = it is an input (the X in Y|X)\n"
      " - 2 = it is a target (the Y in Y|X)\n"
      "If this vector is empty, then all variables are considered targets (thus\n"
      "it is an unconditional distribution)."
      );

  declareOption(ol, "provide_input", &PDistribution::provide_input, OptionBase::buildoption,
      "If provided, then setInput() will be called at build time with this input\n"
      "(this defines the input part for conditional distributions).");

  declareOption(ol, "n_curve_points", &PDistribution::n_curve_points, OptionBase::buildoption,
      "The number of points for which the output is evaluated when outputs_defs\n"
      "is upper case (produces a histogram).\n"
      "The lower_bound and upper_bound options specify where the curve begins and ends.\n"
      "Note that these options (upper case letters) only work for SCALAR variables."
      );

  declareOption(ol, "lower_bound",  &PDistribution::lower_bound, OptionBase::buildoption,
      "The lower bound of scalar Y values to compute a histogram of the distribution\n"
      "when upper case outputs_def are specified.\n");

  declareOption(ol, "upper_bound",  &PDistribution::upper_bound, OptionBase::buildoption,
      "The upper bound of scalar Y values to compute a histogram of the distribution\n"
      "when upper case outputs_def are specified.\n");

  // Learnt options.

  declareOption(ol, "cond_sort",  &PDistribution::cond_sort, OptionBase::learntoption,
      "A vector containing the indices of the variables, so that they are ordered like\n"
      "this: input, target, margin.");

  declareOption(ol, "n_input",  &PDistribution::n_input, OptionBase::learntoption,
      "The size of the input x in p(y|x).");

  declareOption(ol, "n_target",  &PDistribution::n_target, OptionBase::learntoption,
      "The size of the target y in p(y|x).");
      
  declareOption(ol, "n_margin",  &PDistribution::n_margin, OptionBase::learntoption,
      "The size of the variables that are marginalized in p(y|x). E.g., if the whole\n"
      "input contains (x,y,z), and we want to compute p(y|x), then n_margin = z.length().");
      
  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

///////////
// build //
///////////
void PDistribution::build()
{
  inherited::build();
  build_();
}

////////////
// build_ //
////////////
void PDistribution::build_()
{
  resetGenerator(seed_);
  if (n_curve_points > 0)
    delta_curve = (upper_bound - lower_bound) / real(n_curve_points);
  // Precompute the stuff associated to the conditional flags.
  setConditionalFlagsWithoutUpdate(conditional_flags);
}

///////////////////
// computeOutput //
///////////////////
void PDistribution::computeOutput(const Vec& input, Vec& output) const
{
  need_set_input = splitCond(input);
  if (need_set_input) {
    // There is an input part, and it is not the same as in the previous call.
    setInput(input_part);
  }
  int l = (int) outputs_def.length();
  int k = 0;
  for(int i=0; i<l; i++)
  {
    switch(outputs_def[i])
    {
      case 'l':
        output[k++] = log_density(target_part);
        break;
      case 'd':
        output[k++] = density(target_part);
        break;
      case 'c':
        output[k++] = cdf(target_part);
        break;
      case 's':
        output[k++] = survival_fn(target_part);
        break;
      case 'e':
        store_expect = output.subVec(k, n_target);
        expectation(store_expect);
        k += n_target;
        break;
      case 'v':
        store_cov = output.subVec(k, square(n_target)).toMat(n_target, n_target);
        variance(store_cov);
        k += square(n_target);
        break;
      case 'E':
      case 'V':
        if (n_target > 1)
          PLERROR("In PDistribution::computeOutput - Can only plot histogram of expectation or variance for one-dimensional target");
        if (n_target == 0)
          PLERROR("In PDistribution::computeOutput - Cannot plot histogram of expectation or variance for unconditional distributions");
      case 'L':
      case 'D':
      case 'C':
      case 'S':
        real t;
        store_result.resize(1);
        store_result[0] = lower_bound;
        for (int j = 0; j < n_curve_points; j++) {
          switch(outputs_def[i]) {
            case 'L':
              t = log_density(store_result);
              break;
            case 'D':
              t = density(store_result);
              break;
            case 'C':
              t = cdf(store_result);
              break;
            case 'S':
              t = survival_fn(store_result);
              break;
            case 'E':
              setInput(store_result);
              expectation(store_expect);
              t = store_expect[0];
              break;
            case 'V':
              setInput(store_result);
              store_cov = store_expect.toMat(1,1);
              variance(store_cov);
              t = store_expect[0];
              break;
            default:
              PLERROR("In PDistribution::computeOutput - This should never happen");
              t = 0; // To make the compiler happy.
          }
          output[j + k] = t;
          store_result[0] += delta_curve;
        }
        k += n_curve_points;
        break;
      default:
        // Maybe a subclass knows about this output?
        unknownOutput(outputs_def[i], input, output, k);
        break;
    }
  }
}

/////////////////////////////
// computeCostsFromOutputs //
/////////////////////////////
void PDistribution::computeCostsFromOutputs(const Vec& input, const Vec& output, 
    const Vec& target, Vec& costs) const
{
  costs.resize(1);
  if(outputs_def[0] == 'l')
  {
    costs[0] = -output[0];
  }
  else if(outputs_def[0] == 'd')
  {
    costs[0] = -log(output[0]);
  }
  else
    PLERROR("In PDistribution::computeCostsFromOutputs currently can only 'compute' \n"
        "negative log likelihood from log likelihood or density returned as first output \n");

}                                

/////////////////////////////////
// ensureFullJointDistribution //
/////////////////////////////////
bool PDistribution::ensureFullJointDistribution(TVec<int>& old_flags) {
  bool restore_flags = false;
  if (!full_joint_distribution) {
    // Backup flags.
    restore_flags = true;
    old_flags.resize(conditional_flags.length());
    old_flags << conditional_flags;
    // Set flags to compute the full joint distribution.
    TVec<int> tmp;
    setConditionalFlags(tmp);
  } else {
    old_flags.resize(0);
  }
  return restore_flags;
}

////////////////////////////
// finishConditionalBuild //
////////////////////////////
void PDistribution::finishConditionalBuild() {
  updateFromConditionalSorting();
  // Set the input part for a conditional distribution, if provided.
  if (provide_input.isNotEmpty() && provide_input.length() == n_input) {
    input_part << provide_input;
    setInput(input_part);
  }
}

//////////////////////
// getTestCostNames //
//////////////////////
TVec<string> PDistribution::getTestCostNames() const
{
  static TVec<string> nll_cost;
  if (nll_cost.isEmpty())
    nll_cost.append("NLL");
  return nll_cost;
}

///////////////////////
// getTrainCostNames //
///////////////////////
TVec<string> PDistribution::getTrainCostNames() const
{
  // Default = no train cost computed. This may be overridden in subclasses.
  static TVec<string> c;
  return c;
}

///////////////
// generateN //
///////////////
void PDistribution::generateN(const Mat& Y) const
{
  Vec v;
  if (Y.width()!=inputsize())
    PLERROR("In PDistribution::generateN  matrix width (%d) differs from inputsize() (%d)", Y.width(), inputsize());
  int N = Y.length();  
  for(int i=0; i<N; i++)
  {
    v = Y(i);
    generate(v);
  }
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void PDistribution::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(store_expect, copies);
  deepCopyField(store_result, copies);
  deepCopyField(store_cov, copies);
  deepCopyField(cond_sort, copies);
  deepCopyField(cond_swap, copies);
  deepCopyField(input_part, copies);
  deepCopyField(target_part, copies);
  deepCopyField(conditional_flags, copies);
  deepCopyField(provide_input, copies);
}

////////////////
// outputsize //
////////////////
int PDistribution::outputsize() const
{
  int l = 0;
  for (size_t i=0; i<outputs_def.length(); i++) {
    if (outputs_def[i]=='L' || outputs_def[i]=='D' || outputs_def[i]=='C' || outputs_def[i]=='S'
        || outputs_def[i]=='E' || outputs_def[i]=='V')
      l+=n_curve_points;
    else if (outputs_def[i]=='e')
      l += n_target;
    else if (outputs_def[i]=='v') // by default assume variance is full nxn matrix 
      l += n_target * n_target;
    else l++;
  }
  return l;
}

////////////////////
// resetGenerator //
////////////////////
void PDistribution::resetGenerator(long g_seed) const
{
// seed_ = seed; // TODO See how to do this
  random.manual_seed(g_seed);
}

/////////////////
// resizeParts //
/////////////////
void PDistribution::resizeParts() {
  input_part.resize(n_input);
  target_part.resize(n_target);
}

/////////////////////////
// setConditionalFlags //
/////////////////////////
void PDistribution::setConditionalFlags(TVec<int>& flags) {
  // Update the conditional flags.
  setConditionalFlagsWithoutUpdate(flags);
  // And call the method that updates the internal Vec and Mat given the new
  // sorting (this method should be written in subclasses).
  updateFromConditionalSorting();
}

//////////////////////////////////////
// setConditionalFlagsWithoutUpdate //
//////////////////////////////////////
void PDistribution::setConditionalFlagsWithoutUpdate(TVec<int>& flags) {
  static TVec<int> input;
  static TVec<int> target;
  static TVec<int> margin;
  if (inputsize_ <= 0) {
    // No dataset has been specified yet.
    return;
  }
  int is = inputsize();
  input.resize(0);
  target.resize(0);
  margin.resize(0);
  if (flags.isEmpty()) {
    // No flags: everything is target.
    for (int i = 0; i < is; i++) {
      target.append(i);
    }
  } else {
    for (int i = 0; i < flags.length(); i++) {
      switch (flags[i]) {
        case 0:
          margin.append(i);
          break;
        case 1:
          input.append(i);
          break;
        case 2:
          target.append(i);
          break;
        default:
          PLERROR("In PDistribution::setConditionalFlagsWithoutUpdate - Unknown flag value");
      }
    }
  }
  // Update the sizes.
  n_input = input.length();
  n_target = target.length();
  n_margin = margin.length();
  resizeParts();
  if (n_input == 0 && n_margin == 0) {
    // Only full joint distribution.
    full_joint_distribution = true;
  } else {
    full_joint_distribution = false;
  }
  // Fill the new vector of sorted indices.
  TVec<int> new_cond_sort(is);
  new_cond_sort.subVec(0, n_input) << input;
  new_cond_sort.subVec(n_input, n_target) << target;
  new_cond_sort.subVec(n_input + n_target, n_margin) << margin;
  // Check whether we are in the 'easy' case where input, target and margin
  // are correctly sorted.
  if ((n_input == 0 || max(input) == n_input - 1) &&
      (n_target == 0 || max(target) == n_target + n_input - 1)) {
    already_sorted = true;
  } else {
    already_sorted = false;
  }
  // Deduce the indices to be swapped compared to the previous sorting.
  bool found;
  int j;
  int index;
  cond_swap.resize(0);
  if (cond_sort.length() != is) {
    // The previous cond_sort is not valid anymore, we probably
    // have a new training set.
    cond_sort = TVec<int>(0, is - 1, 1);
  }
  for (int i = 0; i < is; i++) {
    found = false;
    j = 0;
    index = new_cond_sort[i];
    while (!found) {
      if (cond_sort[j] == index) {
        found = true;
        if (i != j) {
          // There is really a need to swap the indices.
          cond_swap.append(i);
          cond_swap.append(j);
        }
      } else {
        j++;
      }
    }
  }
  // Copy the new vector of sorted indices.
  cond_sort << new_cond_sort;
  // Copy the new flags.
  conditional_flags.resize(flags.length());
  conditional_flags << flags;
}

//////////////
// setInput //
//////////////
void PDistribution::setInput(const Vec& input) const {
  // Default behavior: only fill input_part with input.
  input_part << input;
}

////////////////////
// setTrainingSet //
////////////////////
void PDistribution::setTrainingSet(VMat training_set, bool call_forget) {
  inherited::setTrainingSet(training_set, call_forget);
  // Update internal data according to conditional_flags.
  setConditionalFlags(conditional_flags);
}

///////////////////
// sortFromFlags //
///////////////////
void PDistribution::sortFromFlags(Vec& v) {
  static Vec tmp_copy;
  tmp_copy.resize(v.length());
  tmp_copy << v;
  for (int i = 0; i < cond_swap.length();) {
    v[cond_swap[i++]] = tmp_copy[cond_swap[i++]];
  }
}

void PDistribution::sortFromFlags(Mat& m, bool sort_columns, bool sort_rows) {
  static int j,k;
  static Mat tmp_copy;
  static Vec row;
  if (sort_columns) {
    for (int r = 0; r < m.length(); r++) {
      row = m(r);
      sortFromFlags(row);
    }
  }
  if (sort_rows && m.length() > 0 && m.width() > 0) {
    tmp_copy.resize(m.length(), m.width());
    tmp_copy << m;
    for (int i = 0; i < cond_swap.length();) {
      j = cond_swap[i++];
      k = cond_swap[i++];
      // The new j-th row is the old k-th row.
      m(j) << tmp_copy(k);
    }
  }
}

///////////////
// splitCond //
///////////////
bool PDistribution::splitCond(const Vec& input) const {
  if (n_input == 0 || (n_input > 0 && input.length() == n_target + n_margin)) {
    // No input part provided: this means this is the same as before.
    if (already_sorted) {
      target_part << input.subVec(0, n_target);
    } else {
      // A bit messy here. It probably won't happen, so it's not implemented
      // for now (but wouldn't be that hard to do it).
      PLERROR("In PDistribution::splitCond - You'll need to implement this case!");
    }
    return false;
  }
  if (already_sorted) {
    input_part << input.subVec(0, n_input);
    target_part << input.subVec(n_input, n_target);
  } else {
    for (int i = 0; i < n_input; i++) {
      input_part[i] = input[cond_sort[i]];
    }
    for (int i = 0; i < n_target; i++) {
      target_part[i] = input[cond_sort[i + n_input]];
    }
  }
  return true;
}

////////////////////
// subclass stuff //
////////////////////

void PDistribution::forget() {
  PLERROR("forget not implemented for this PDistribution");
}

real PDistribution::log_density(const Vec& y) const
{ PLERROR("density not implemented for this PDistribution"); return 0; }

real PDistribution::density(const Vec& y) const
{ return exp(log_density(y)); }

real PDistribution::survival_fn(const Vec& y) const
{ PLERROR("survival_fn not implemented for this PDistribution"); return 0; }

real PDistribution::cdf(const Vec& y) const
{ PLERROR("cdf not implemented for this PDistribution"); return 0; }

void PDistribution::expectation(Vec& mu) const
{ PLERROR("expectation not implemented for this PDistribution"); }

void PDistribution::variance(Mat& covar) const
{ PLERROR("variance not implemented for this PDistribution"); }

void PDistribution::generate(Vec& y) const
{ PLERROR("generate not implemented for this PDistribution"); }

void PDistribution::train()
{ PLERROR("train not implemented for this PDistribution"); }

///////////////////
// unknownOutput //
///////////////////
void PDistribution::unknownOutput(char def, const Vec& input, Vec& output, int& k) const {
  // Default is to throw an error.
  PLERROR("In PDistribution::unknownOutput - Unrecognized outputs_def character: %c", def);
}

//////////////////////////////////
// updateFromConditionalSorting //
//////////////////////////////////
void PDistribution::updateFromConditionalSorting() {
  // Default does nothing.
  return;
}

} // end of namespace PLearn
