
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
   * $Id: MountLucasIndex.cc,v 1.5 2003/08/13 08:13:47 plearner Exp $ 
   ******************************************************* */

/*! \file MountLucasIndex.cc */
#include "MountLucasIndex.h"
#include "PDate.h"

namespace PLearn <%
using namespace std;


PLEARN_IMPLEMENT_OBJECT(MountLucasIndex, "ONE LINE DESCR", "NO HELP");

MountLucasIndex::MountLucasIndex()
  : current_month(0), julian_day_column("julian_day"), build_complete(false)
{
}

void MountLucasIndex::build()
{
  inherited::build();
  build_();
}

void MountLucasIndex::build_()
{
  if (commodity_price_columns.size() == 0)
    PLERROR("The different commodity price columns must be set before building the MountLucasIndex Object");
  if (max_seq_len < 1)
    PLERROR("The field max_seq_len must be set before building the MountLucasIndex Object");

  nb_commodities = commodity_price_columns.length();

  is_long_position.resize(nb_commodities);
  twelve_month_moving_average.resize(nb_commodities);
  monthly_unit_asset_value.resize(max_seq_len,nb_commodities);
  monthly_rate_return.resize(max_seq_len,nb_commodities);
  index_value.resize(max_seq_len);

  if (train_set)
  {
    julian_day_index = train_set->fieldIndex(julian_day_column);
    commodity_price_index.resize(nb_commodities);
    for (int i=0; i<nb_commodities; i++)
      commodity_price_index[i] = train_set->fieldIndex(commodity_price_columns[i]);
    build_complete = true;
  }

  forget();
}

void MountLucasIndex::forget()
{
  inherited::forget();

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
  declareOption(ol, "julian_day_column", &MountLucasIndex::julian_day_column,
    OptionBase::buildoption, "The julian day number column (in the input data) \n");

  declareOption(ol, "commodity_price_columns", &MountLucasIndex::commodity_price_columns,
    OptionBase::buildoption, "The commodity price columns (in the input data) \n");

  inherited::declareOptions(ol);
}

void MountLucasIndex::train()
{
  if (!build_complete)
  {
    julian_day_index = train_set->fieldIndex(julian_day_column);
    commodity_price_index.resize(nb_commodities);
    for (int i=0; i<nb_commodities; i++)
      commodity_price_index[i] = train_set->fieldIndex(commodity_price_columns[i]);
    build_complete = true;
  }

  int start_t = last_train_t+1;
  ProgressBar* pb;
  if (report_progress)
    pb = new ProgressBar("Training MountLucasIndex learner",train_set.length()-start_t);

  Vec last_price(nb_commodities);
  Vec next_to_last_price(nb_commodities);
  Vec last_month_last_price(nb_commodities);
  Vec last_month_next_to_last_price(nb_commodities);
  Vec input(train_set->inputsize()), target(train_set->targetsize());
  real w=1.0;
  int julian_day, julian_last_day_of_month;
  for (int t=start_t; t<train_set.length(); t++)
  {
    train_set->getExample(t, input, target, w);
    julian_day = int(input[julian_day_index]);
    Vec price_value = input(commodity_price_index);
    next_to_last_price << last_price;
    last_price << price_value;

    if (t == start_t)
      julian_last_day_of_month = lastJulianDayOfMonth(julian_day);

    if (julian_day > julian_last_day_of_month)
    {
      julian_last_day_of_month = lastJulianDayOfMonth(julian_day);
      if (current_month == 0)
      {
        // next-to-last trading day of the month
        monthly_unit_asset_value.firstRow().fill(1.0);
        last_month_next_to_last_price << next_to_last_price;

        // last trading day of the month
        monthly_rate_return.firstRow().fill(1.0);
        index_value[0] = 1000.0; // arbitrary initial value
        last_month_last_price << last_price;
      }
      else
      {
        // next-to-last trading day of the month
        Vec last_value = monthly_unit_asset_value(current_month-1);
        Vec this_value = monthly_unit_asset_value(current_month);
        for (int i=0; i<nb_commodities; i++)
        {
          this_value[i] = last_value[i]*(next_to_last_price[i]/last_month_next_to_last_price[i]);
          last_month_next_to_last_price[i] = next_to_last_price[i];
        }

        // last trading day of the month
        Vec rate_return = monthly_rate_return(current_month);
        for (int i=0; i<nb_commodities; i++)
        {
          rate_return[i] = (last_price[i]/last_month_last_price[i] - 1);
          if (!is_long_position[i]) rate_return[i] = -rate_return[i];
          last_month_last_price[i] = last_price[i];
          is_long_position[i] = next_position(i, monthly_unit_asset_value);
        }
        index_value[current_month] = index_value[current_month-1]*(1.0 + mean(rate_return));
        predictions(current_month,0) = index_value[current_month];
      }
      ++current_month;
    }

    if (pb) pb->update(t-start_t);
  }
  last_train_t = train_set.length()-1;
  if (pb) delete pb;

  savePVec("MLMIndex.pvec", index_value);
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
  TVec<string> dummy_string(1, "NoCost");
  return dummy_string;
}

TVec<string> MountLucasIndex::getTestCostNames() const
{ return getTrainCostNames(); }

int lastJulianDayOfMonth(int julian_day)
{
  PDate today(julian_day);
  int year_next_month = (today.month==12) ? today.year+1 : today.year;
  int month_next_month = (today.month==12) ? 1 : today.month+1;
  int day_next_month = 1;

  return PDate(year_next_month, month_next_month, day_next_month).toJulianDay() - 1;
}

/*
void MountLucasIndex::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("MountLucasIndex::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}
*/

%> // end of namespace PLearn

