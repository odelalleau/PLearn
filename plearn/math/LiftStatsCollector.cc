// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
// Copyright (C) 2003 Olivier Delalleau
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
 * $Id: LiftStatsCollector.cc,v 1.16 2004/11/22 17:54:24 lapalmej Exp $
 * This file is part of the PLearn library.
 ******************************************************* */

/*! \file LiftStatsCollector.cc */

#include "LiftStatsCollector.h"
#include "TMat_maths.h"
#include <plearn/base/stringutils.h>

namespace PLearn {
using namespace std;

////////////////////////
// LiftStatsCollector //
////////////////////////
LiftStatsCollector::LiftStatsCollector() 
  : inherited(),
  count_fin(0),
  is_finalized(false),
  nstored(0),
  nsamples(0),
  npos(0),
  output_column_index(0),
  lift_file(""),
  lift_fraction(0.1),
  opposite_lift(0),
  output_column(""),
  roc_file(""),
  sign_trick(0),
  target_column(1),
  verbosity(0)
{
}

//////////////////
// Object stuff //
//////////////////
PLEARN_IMPLEMENT_OBJECT(
  LiftStatsCollector,
  "Computes the performance of a binary classifier",
  "The following statistics can be requested out of getStat():\n"
  "- LIFT = % of positive examples in the first n samples, divided by the % of positive examples in the whole database\n"
  "- LIFT_MAX = best performance that could be achieved, if all positive examples were selected in the first n samples\n"
  "(where n = lift_fraction * nsamples).\n"
  "IMPORTANT: if you add more samples after you call finalize() (or get any of the statistics above), some samples may\n"
  "be wrongly discarded and further statistics may be wrong\n\n"
  "Here are the typical steps to follow to optimize the lift with a neural network:\n"
  "- add a lift_output cost to cost_funcs (e.g. cost_funcs = [ \"stable_cross_entropy\" \"lift_output\"];)\n"
  "- change the template_stats_collector of your PTester:\n"
  "    template_stats_collector =\n"
  "      LiftStatsCollector (\n"
  "        output_column = \"lift_output\" ;\n"
  "        opposite_lift = 1 ; # if you want to optimize the lift\n"
  "        sign_trick = 1 ;\n"
  "      )\n"
  "- add the lift to its statnames:\n"
  "    statnames = [ \"E[train.E[stable_cross_entropy]]\",\"E[test.E[stable_cross_entropy]]\",\n"
  "                  \"E[train.LIFT]\", \"E[test.LIFT]\" ]\n"
  "- maybe also change which_cost in your HyperOptimize strategy.\n"

  );

void LiftStatsCollector::declareOptions(OptionList& ol)
{

  declareOption(ol, "count_fin", &LiftStatsCollector::count_fin, OptionBase::learntoption,
      "    the number of times finalize() has been called since the last forget()");

  declareOption(ol, "lift_fraction", &LiftStatsCollector::lift_fraction, OptionBase::buildoption,
      "    the % of samples to consider (default = 0.1)\n");

  declareOption(ol, "opposite_lift", &LiftStatsCollector::opposite_lift, OptionBase::buildoption,
      "    if set to 1, the LIFT stat will return -LIFT, so that it can be considered as a cost (default = 0)\n");

  declareOption(ol, "output_column", &LiftStatsCollector::output_column, OptionBase::buildoption,
      "    the name of the column in which is the output value (the default value, \"\", assumes it is the first column))\n");

  declareOption(ol, "sign_trick", &LiftStatsCollector::sign_trick, OptionBase::buildoption,
      "    if set to 1, then you won't have to specify a target column: if the output is\n"
      "    negative, the target will be assumed to be 0, and 1 otherwise - and in both cases\n"
      "    we only consider the absolute value of the output\n"
      "    (default = 0)\n"
  );

  declareOption(ol, "target_column", &LiftStatsCollector::target_column, OptionBase::buildoption,
      "    the column in which is the target value (default = 1)\n");

  declareOption(ol, "verbosity", &LiftStatsCollector::verbosity, OptionBase::buildoption,
      "    to be set >= 2 in order to display more info (default = 0)\n");

  declareOption(ol, "roc_file", &LiftStatsCollector::roc_file, OptionBase::buildoption,
      "If provided, the points of the ROC curve computed for different fractions (see\n"
      "'roc_fractions') will be appended in ASCII format to the given file.");

  declareOption(ol, "lift_file", &LiftStatsCollector::lift_file, OptionBase::buildoption,
      "If provided, the lifts computed for different fractions (see 'roc_fractions')\n"
      "will be appended in ASCII format to the given file.");

  declareOption(ol, "roc_fractions", &LiftStatsCollector::roc_fractions, OptionBase::buildoption,
      "(Ordered) fractions used to compute and save points in the ROC curve, or additional lifts.");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

///////////
// build //
///////////
void LiftStatsCollector::build()
{
  inherited::build();
  build_();
}

////////////
// build_ //
////////////
void LiftStatsCollector::build_()
{
  if (output_column != "") {
    int i = this->getFieldNum(output_column);
    if (i >= 0) {
      output_column_index = i;
    } else {
      // Not found.
      output_column_index = 0;
    }
  } else {
    output_column_index = 0;
  }
}

/////////////////
// computeLift //
/////////////////
real LiftStatsCollector::computeLift() {
  if (!is_finalized)
    finalize();
  // Compute statistics.

  int npos_in_n_first = (int) sum(n_first_updates.column(1));
  real first_samples_perf = npos_in_n_first/ (real) n_samples_to_keep;
  real targets_perf = (npos_in_n_first + npos) / (real) nsamples;
  real lift = first_samples_perf/targets_perf*100.0;
  if (verbosity >= 10) {
    cout << "LiftStatsCollector : is_finalized=" << is_finalized << ", nstored="
         << nstored << ", nsamples=" << nsamples << ", npos=" << npos
         << ", n_samples_to_keep=" << n_samples_to_keep << ", lift_fraction="
         << lift_fraction << ", output_column=" << output_column << ", sign_trick="
         << sign_trick << ", target_column=" << target_column << ", verbosity= "
         << verbosity << endl;
  }
  if (verbosity >= 2) {
    cout << "There is a total of " << npos_in_n_first + npos <<
      " positive examples to discover." << endl;
    cout << "The learner found " << npos_in_n_first << 
      " of them in the fraction considered (" << lift_fraction << ")." << endl;
  }
  if (opposite_lift == 1) {
    return -lift;
  }
  return lift;
}

////////////////////
// computeLiftMax //
////////////////////
real LiftStatsCollector::computeLiftMax() {
  if (!is_finalized)
    finalize();
  int npos_in_n_first = (int) sum(n_first_updates.column(1));
  real nones = npos_in_n_first + npos;
  real max_first_samples_perf =
    MIN(nones,(real)n_samples_to_keep) / (real) n_samples_to_keep;
  real targets_perf = (npos_in_n_first + npos) / (real) nsamples;
  real max_lift = max_first_samples_perf/targets_perf*100.0;
  return max_lift;
}

//////////////
// finalize //
//////////////
void LiftStatsCollector::finalize()
{
  n_first_updates.resize(nstored,2); // get rid of the extra space allocated.

  n_samples_to_keep = int(lift_fraction*nsamples);

  if (nstored > n_samples_to_keep) {
    // If not, then no change has to be made to n_first_updates.

    // Compute additional lifts (hack) if required.
    if (roc_fractions.length() > 0 && (roc_file != "" || lift_file != "")) {
      // Copy data to make sure we do not change anything.
      Mat data(n_first_updates.length(), n_first_updates.width());
      data << n_first_updates;
      sortRows(data, 0, false);
      // Create result file if does not exist already.
      string command;
      if (roc_file != "") {
        command = "touch " + roc_file;
        // cout << "Command: " << command << endl;
        system(command.c_str());
      }
      if (lift_file != "") {
        command = "touch " + lift_file;
        // cout << "Command: " << command << endl;
        system(command.c_str());
      }
      // Compute lifts.
      int nones = npos + int(sum(data.column(1)) + 1e-3);
      real frac_pos = nones / real(nsamples * 100);
      int lift_index = 0;
      int count_pos = 0;
      int sample_index = 0;
      string result_roc = "";
      string result_lift = "";
      while (lift_index < roc_fractions.length()) {
        while (sample_index < real(nsamples) * roc_fractions[lift_index]) {
          if (data(sample_index, 1) == 1)
            count_pos++;
          sample_index++;
        }
        real lift_value = real(count_pos) / real(sample_index) / frac_pos;
        real roc_value = real(count_pos) / real(nones);
        lift_index++;
        result_roc += tostring(roc_value) + "\t";
        result_lift += tostring(lift_value) + "\t";
      }
      // Save the lifts in the given file.
      if (lift_file != "") {
        command = "echo " + result_lift + " >> " + lift_file;
        // cout << "Command: " << command << endl;
        system(command.c_str());
      }
      if (roc_file != "") {
        command = "echo " + result_roc + " >> " + roc_file;
        // cout << "Command: " << command << endl;
        system(command.c_str());
      }
    }

    // Make sure the highest ouputs are in the last n_samples_to_keep elements
    // of n_first_updates.
    if (n_samples_to_keep > 0) {
      selectAndOrder(n_first_updates, nstored - n_samples_to_keep);
    }

    // Count the number of positive examples in the lowest outputs.
    for (int i = 0; i < nstored - n_samples_to_keep; i++) {
      if (n_first_updates(i,1) == 1) {
        npos++;
      }
    }
  
    // Clear the lowest outputs, that are now useless.
    for (int i = 0; i < n_samples_to_keep; i++) {
      n_first_updates(i,0) = n_first_updates(i + nstored - n_samples_to_keep, 0);
      n_first_updates(i,1) = n_first_updates(i + nstored - n_samples_to_keep, 1);
    }
    n_first_updates.resize(n_samples_to_keep, 2);
    nstored = n_samples_to_keep;
  }

  inherited::finalize();
  is_finalized = true;
  count_fin++;
  if (verbosity >= 10) {
    cout << "Called finalized " << count_fin << " times" << endl;
  }
}

////////////
// forget //
////////////
void LiftStatsCollector::forget()
{
  is_finalized = false;
  nstored = 0;
  npos = 0;
  nsamples = 0;
  n_first_updates.resize(0,0);
  n_first_updates.resize(1000,2);
  inherited::forget();
  count_fin = 0;
}

/////////////
// getStat //
/////////////
double LiftStatsCollector::getStat(const string& statspec)
{
  PIStringStream str(statspec);
  string parsed;
  str.smartReadUntilNext("(",parsed);
  if (parsed == "LIFT") {
    return computeLift();
  }
  else if (parsed == "LIFT_MAX") {
    return computeLiftMax();
  }
  else
    return inherited::getStat(statspec);
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void LiftStatsCollector::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(n_first_updates, copies);
  deepCopyField(roc_fractions, copies);
}

////////////////////////
// remove_observation //
////////////////////////
void LiftStatsCollector::remove_observation(const Vec& x, real weight) {
  // Not supported.
  PLERROR("In LiftStatsCollector::remove_observation - This method is not implemented");
}

////////////
// update //
////////////
void LiftStatsCollector::update(const Vec& x, real w)
{
  if (count_fin > 0) {
    // Depending on whether we compute additional lifts, this may be fatal or not.
    string msg = "In LiftStatsCollector::update - Called update after finalize (see help of LiftStatsCollector)";
    if (roc_file != "")
      PLERROR(msg.c_str());
    else
      PLWARNING(msg.c_str());
  }
  if (nstored == n_first_updates.length()) {
    n_first_updates.resize(MAX(1000,10*n_first_updates.length()), 2);
  }
  real output_val = x[output_column_index];
  if (is_missing(output_val)) {
    // Missing value: we just discard it.
    is_finalized = false;
    inherited::update(x,w);
    return;
  }
  real target = -1;
  switch(sign_trick) {
    case 0:
      // Normal behavior.
      n_first_updates(nstored, 0) = output_val;
      target = x[target_column];
      break;
    case 1:
      // Sign trick.
      n_first_updates(nstored, 0) = FABS(output_val);
      if (output_val <= 0) {
        x[output_column_index] = -output_val;
        target = 0;
      } else {
        target = 1;
//        cout << "Positive example : " << x << " (output_val = " << output_val << ")" << endl;
      }
      break;
    default:
      PLERROR("Wrong value for sign_trick in LiftStatsCollector");
      break;
  }
  n_first_updates(nstored, 1) = target;
  if (target != 0 && target != 1) {
    PLERROR("In LiftStatsCollector::update - Target must be 0 or 1 !");
  }
  nsamples++;
  nstored++;
  is_finalized = false;

  inherited::update(x,w);
}

} // end of namespace PLearn
