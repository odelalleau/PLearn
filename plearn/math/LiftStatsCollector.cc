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
 * $Id: LiftStatsCollector.cc,v 1.6 2003/11/19 19:02:36 tihocan Exp $
 * This file is part of the PLearn library.
 ******************************************************* */

/*! \file LiftStatsCollector.cc */

#include "LiftStatsCollector.h"
#include "TMat_maths.h"

namespace PLearn <%
using namespace std;

////////////////////////
// LiftStatsCollector //
////////////////////////
LiftStatsCollector::LiftStatsCollector() 
  : inherited(),
  is_finalized(false),
  nstored(0),
  nsamples(0),
  npos(0),
  lift_fraction(0.1),
  output_column(0),
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
  "be wrongly discarded and further statistics may be wrong\n"
  );

void LiftStatsCollector::declareOptions(OptionList& ol)
{

  declareOption(ol, "lift_fraction", &LiftStatsCollector::lift_fraction, OptionBase::buildoption,
      "    the % of samples to consider (default = 0.1)\n");

  declareOption(ol, "output_column", &LiftStatsCollector::output_column, OptionBase::buildoption,
      "    the column in which is the output value (default = 0)\n");

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
  if (sign_trick == 1) {
    cout << "Warning, the sign trick has been implemented but not tested. If it works, "
      "you can remove this warning, and if it doesn't, you can fix it ;)" << endl;
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
  if (verbosity >= 2) {
    cout << "There is a total of " << npos_in_n_first + npos <<
      " positive examples to discover." << endl;
    cout << "The learner found " << npos_in_n_first << 
      " of them in the fraction considered (" << lift_fraction << ")." << endl;
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
void LiftStatsCollector::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  cout << "In LiftStatsCollector::makeDeepCopyFromShallowCopy Warning, this function was not"
    " tested properly, maybe n_first_updates should be declared as an option... If it works,"
    " please remove this warning.";
  inherited::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(n_first_updates, copies);
}

////////////
// update //
////////////
void LiftStatsCollector::update(const Vec& x, real w)
{
  if (nstored == n_first_updates.length()) {
    n_first_updates.resize(MAX(1000,10*n_first_updates.length()), 2);
  }
  real output_val = x[output_column];
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
        target = 0;
      } else {
        target = 1;
      }
      break;
    default:
      PLERROR("Wrong value for sign_trick in LiftStatsCollector");
      break;
  }
  n_first_updates(nstored, 1) = target;
  if (target != 0 && target != 1) {
    PLERROR("In LiftStatsCollector::update Target must be 0 or 1 !");
  }
  nsamples++;
  nstored++;
  is_finalized = false;

  inherited::update(x,w);
}

%> // end of namespace PLearn
