
// -*- C++ -*-

// MountLucasIndex.cc
//
// Copyright (C) 2003  Rejean Ducharme 
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
   * $Id: MountLucasIndex.cc,v 1.1 2003/07/10 18:22:53 ducharme Exp $ 
   ******************************************************* */

/*! \file MountLucasIndex.cc */
#include "MountLucasIndex.h"

namespace PLearn <%
using namespace std;


PLEARN_IMPLEMENT_OBJECT_METHODS(MountLucasIndex, "MountLucasIndex", SequentialLearner);

MountLucasIndex::MountLucasIndex()
  : current_month(0), y_index(-1), m_index(-1), d_index(-1), first_price_index(-1),
    time_to_last_day_index(-1), nb_commodities(-1)
{
}

void MountLucasIndex::build()
{
  parentclass::build();
  build_();
}

void MountLucasIndex::build_()
{
  if (nb_commodities < 1)
    PLERROR("The field nb_commodities must be set before building the MountLucasIndex Object");
  if (max_seq_len < 1)
    PLERROR("The field max_seq_len must be set before building the MountLucasIndex Object");

  next_to_last_closing_price.resize(nb_commodities);
  last_closing_price.resize(nb_commodities);
  is_long_position.resize(nb_commodities);
  twelve_month_moving_average.resize(nb_commodities);
  monthly_unit_asset_value.resize(max_seq_len,nb_commodities);
  monthly_rate_return.resize(max_seq_len,nb_commodities);
  index_value.resize(max_seq_len);

  forget();
}

void MountLucasIndex::forget()
{
  next_to_last_closing_price.fill(MISSING_VALUE);
  last_closing_price.fill(MISSING_VALUE);
  is_long_position.fill(true);
  twelve_month_moving_average.fill(MISSING_VALUE);
  monthly_unit_asset_value.fill(MISSING_VALUE);
  monthly_rate_return.fill(MISSING_VALUE);
  index_value.fill(MISSING_VALUE);

  current_month = 0;
  index_value[0] = 1000.0;
}

void MountLucasIndex::declareOptions(OptionList& ol)
{
  declareOption(ol, "nb_commodities", &MountLucasIndex::nb_commodities,
    OptionBase::buildoption, "The number of commodities included in the MLM Index \n");

  declareOption(ol, "year_index", &MountLucasIndex::y_index,
    OptionBase::buildoption, "The year index (in the input data) \n");

  declareOption(ol, "month_index", &MountLucasIndex::m_index,
    OptionBase::buildoption, "The month index (in the input data) \n");

  declareOption(ol, "day_index", &MountLucasIndex::d_index,
    OptionBase::buildoption, "The day index (in the input data) \n");

  declareOption(ol, "first_price_index", &MountLucasIndex::first_price_index,
    OptionBase::buildoption, "The price index (in the input data) \n");

  declareOption(ol, "time_to_last_day_index", &MountLucasIndex::time_to_last_day_index,
    OptionBase::buildoption, "The time_to_last_day_index index (in the input data) \n");

  parentclass::declareOptions(ol);
}

void MountLucasIndex::train()
{
  ProgressBar* pb;
  if (report_progress)
    pb = new ProgressBar("Training SequentialModelSelector learner",train_set.length());

  Vec input(train_set->inputsize()), target(train_set->targetsize());
  real w=1.0;
  for (int t=0; t<train_set.length(); t++)
  {
    train_set->getExample(t, input, target, w);
    Vec price_value = input.subVec(first_price_index, nb_commodities);
    int time_to_last_day = int(input[time_to_last_day_index]);

    if (time_to_last_day == 0) // last trading day of the month
    {
      if (current_month == 0)
      {
        monthly_rate_return.firstRow().fill(1.0);
        index_value[0] = 1000.0; // arbitrary initial value
      }
      else
      {
        Vec rate_return = monthly_rate_return(current_month);
        for (int i=0; i<nb_commodities; i++)
        {
          rate_return[i] = (price_value[i]/last_closing_price[i] - 1);
          if (!is_long_position[i]) rate_return[i] = -rate_return[i];
          last_closing_price[i] = price_value[i];
          is_long_position[i] = next_position(i, monthly_unit_asset_value);
        }
        index_value[current_month] = index_value[current_month-1]*mean(rate_return);
        ++current_month;
      }
    }
    else if (time_to_last_day == 1) // next-to-last trading day of the month
    {
      if (current_month == 0)
      {
        monthly_unit_asset_value.firstRow().fill(1.0);
        next_to_last_closing_price << price_value;
      }
      else
      {
        Vec last_value = monthly_unit_asset_value(current_month-1);
        Vec this_value = monthly_unit_asset_value(current_month);
        for (int i=0; i<nb_commodities; i++)
        {
          this_value[i] = last_value[i]*(price_value[i]/next_to_last_closing_price[i]);
          next_to_last_closing_price[i] = price_value[i];
        }
      }
    }
  }
}

// nothing to do in test
void MountLucasIndex::test(VMat testset, PP<VecStatsCollector> test_stats,
    VMat testoutputs, VMat testcosts) const
{}

bool MountLucasIndex::next_position(int pos, const Mat& unit_asset_value) const
{
  real the_month_unit_asset_value = unit_asset_value(current_month, pos);
  real moving_average = 0;
  int start = MAX(0,current_month+1-12);
  for (int t=start; t<=current_month; t++)
    moving_average += unit_asset_value(t,pos);
  moving_average /= (current_month+1-start);

  return (the_month_unit_asset_value >= moving_average) ? true : false;
}

void MountLucasIndex::computeOutputAndCosts(const Vec& input,
    const Vec& target, Vec& output, Vec& costs) const
{ PLERROR("The method computeOutputAndCosts is not defined for the MountLucasIndex class"); }

void MountLucasIndex::computeCostsOnly(const Vec& input, const Vec& target,
    Vec& costs) const
{ PLERROR("The method computeCostsOnly is not defined for the MountLucasIndex class"); }

void MountLucasIndex::computeOutput(const Vec& input, Vec& output) const
{ PLERROR("The method computeOutput is not defined for the MountLucasIndex class"); }

void MountLucasIndex::computeCostsFromOutputs(const Vec& input,
    const Vec& output, const Vec& target, Vec& costs) const
{ PLERROR("The method computeCostsFromOutputs is not defined for the MountLucasIndex class"); }

TVec<string> MountLucasIndex::getTrainCostNames() const
{
  TVec<string> dummy_string;
  return dummy_string;
}

TVec<string> MountLucasIndex::getTestCostNames() const
{ return getTrainCostNames(); }

/*
void MountLucasIndex::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  parentclass::makeDeepCopyFromShallowCopy(copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("MountLucasIndex::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}
*/

%> // end of namespace PLearn

