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
 * $Id: LiftStatsCollector.cc,v 1.2 2003/11/04 21:24:03 tihocan Exp $
 * This file is part of the PLearn library.
 ******************************************************* */

/*! \file LiftStatsCollector.cc */

#include "LiftStatsCollector.h"
#include "TMat_maths.h"
//#include "VMat.h"
//#include "MemoryVMatrix.h" // TODO see what we keep
//#include "stringutils.h"

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
  output_column(0),
  target_column(1),
  lift_fraction(0.1)
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
  );

void LiftStatsCollector::declareOptions(OptionList& ol)
{
  declareOption(ol, "n_first_samples", &LiftStatsCollector::n_first_samples,
      OptionBase::learntoption,
      "Matrix storing the output and target of the first n samples with highest output, "
      "as well as all the other data retrieved since the last call to finalize");

  declareOption(ol, "output_column", &LiftStatsCollector::output_column, OptionBase::buildoption,
      "    the column in which is the output value\n");

  declareOption(ol, "target_column", &LiftStatsCollector::target_column, OptionBase::buildoption,
      "    the column in which is the target value\n");

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
/*  // We do not use resize on purpose, so
  // that the previous result Vec does not get overwritten
  result = Vec(2);
  const int initial_length = 1000;
  output_and_pos.resize(initial_length, 2);  // 1 output + 1 pos
  targets.resize(initial_length);
  nsamples = 0; */ // TODO adapt to new framework
}

/////////////////
// computeLift //
/////////////////
real LiftStatsCollector::computeLift() {
  if (!is_finalized)
    finalize();
  // Compute statistics.
  int npos_in_n_first = (int) sum(n_first_samples.column(1));
  real first_samples_perf = npos_in_n_first/n_samples_to_keep;
  real targets_perf = (npos_in_n_first + npos) / nsamples;
  real lift = first_samples_perf/targets_perf*100.0;
  return lift;
}

////////////////////
// computeLiftMax //
////////////////////
real LiftStatsCollector::computeLiftMax() {
  if (!is_finalized)
    finalize();
  int npos_in_n_first = (int) sum(n_first_samples.column(1));
  real nones = npos_in_n_first + npos;
  real max_first_samples_perf =
    MIN(nones,(real)n_samples_to_keep) / (real) n_samples_to_keep;
  real targets_perf = (npos_in_n_first + npos) / nsamples;
  real max_lift = max_first_samples_perf/targets_perf*100.0;
  return max_lift;
}

//////////////
// finalize //
//////////////
void LiftStatsCollector::finalize()
{
  n_first_samples.resize(nstored,2); // get rid of the extra space allocated.

  n_samples_to_keep = int(lift_fraction*nsamples);
//  int n_last_samples = nsamples - n_first_samples; // TODO remove that
  // Make sure the highest ouputs are in the last n_samples_to_keep elements
  // of n_first_samples
  selectAndOrder(n_first_samples, nstored - n_samples_to_keep); // TODO clear the crap

  // Count the number of positive examples in the lowest outputs.
  for (int i = 0; i < nstored - n_samples_to_keep; i++) {
    if (n_first_samples(i,1) == 1) {
      npos++;  // TODO initialize npos
    }
  }
  
  // Clear the lowest outputs, that are now useless.
  for (int i = 0; i < n_samples_to_keep; i++) {
    n_first_samples(i,0) = n_first_samples(i + nstored - n_samples_to_keep, 0);
    n_first_samples(i,1) = n_first_samples(i + nstored - n_samples_to_keep, 1);
  }
  n_first_samples.resize(n_samples_to_keep, 2);

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
  n_first_samples.resize(0,0);
  n_first_samples.resize(1000,2);
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
  inherited::makeDeepCopyFromShallowCopy(copies);

//  deepCopyField(all_updates, copies);   // TODO see if we use those fields
//  deepCopyField(sorted_updates, copies);
}

////////////
// update //
////////////
void LiftStatsCollector::update(const Vec& x, real w)
{
  if (nstored == n_first_samples.length()) {
    n_first_samples.resize(10*n_first_samples.length(), 2);
  }
  n_first_samples(nstored, 0) = x[output_column];
  n_first_samples(nstored, 1) = x[target_column];
  if (x[target_column] != 0 && x[target_column] != 1) {
    PLERROR("In LiftStatsCollector::update Target must be 0 or 1 !");
  }
  nsamples++;
  nstored++;
  is_finalized = false;

  inherited::update(x,w);
}

 // TODO make sure we deleted all the useless stuff
%> // end of namespace PLearn
