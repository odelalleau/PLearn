
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
   * $Id: MountLucasIndex.cc,v 1.9 2003/09/09 21:03:15 ducharme Exp $ 
   ******************************************************* */

/*! \file MountLucasIndex.cc */
#include "MountLucasIndex.h"
#include "PDate.h"

namespace PLearn <%
using namespace std;


PLEARN_IMPLEMENT_OBJECT(MountLucasIndex, "ONE LINE DESCR", "NO HELP");

MountLucasIndex::MountLucasIndex()
  : last_day_of_month_column("is_last_day_of_month"), julian_day_column("Date"),
    risk_free_rate_column("risk_free_rate"), current_month(0), build_complete(false)
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
  if (commodity_price_columns.size() != commodity_start_year.size())
    PLERROR("The commodity_price_columns and commodity_start_year vectors must have the same length (%d != %d)", commodity_price_columns.size(), commodity_start_year.size());
  if (max_seq_len < 1)
    PLERROR("The field max_seq_len must be set before building the MountLucasIndex Object");

  nb_commodities = commodity_price_columns.length();

  is_long_position.resize(nb_commodities);
  tradable_commodity.resize(nb_commodities);
  twelve_month_moving_average.resize(nb_commodities);
  next_to_last_unit_asset_value.resize(max_seq_len,nb_commodities);
  unit_asset_value.resize(nb_commodities);
  index_value.resize(max_seq_len);

  if (train_set)
  {
    commodity_price_index.resize(nb_commodities);
    for (int i=0; i<nb_commodities; i++)
      commodity_price_index[i] = train_set->fieldIndex(commodity_price_columns[i]);
    last_day_of_month_index = train_set->fieldIndex(last_day_of_month_column);
    julian_day_index = train_set->fieldIndex(julian_day_column);
    risk_free_rate_index = train_set->fieldIndex(risk_free_rate_column);
    build_complete = true;
  }

  forget();
}

void MountLucasIndex::forget()
{
  inherited::forget();

  is_long_position.fill(true);
  tradable_commodity.fill(false);
  twelve_month_moving_average.fill(MISSING_VALUE);
  next_to_last_unit_asset_value.fill(MISSING_VALUE);
  unit_asset_value.fill(MISSING_VALUE);
  index_value.fill(MISSING_VALUE);

  current_month = 0;
  index_value[0] = 1000.0;
}

void MountLucasIndex::declareOptions(OptionList& ol)
{
  declareOption(ol, "commodity_price_columns", &MountLucasIndex::commodity_price_columns,
    OptionBase::buildoption, "The commodity price columns (in the input data) \n");

  declareOption(ol, "commodity_start_year", &MountLucasIndex::commodity_start_year,
    OptionBase::buildoption, "The year we begin to trade the corresponding commodity (as defined in commodity_price_columns) \n");

  declareOption(ol, "last_day_of_month_column", &MountLucasIndex::last_day_of_month_column,
    OptionBase::buildoption, "The last_day_of_month column (in the input data) \n");

  declareOption(ol, "julian_day_column", &MountLucasIndex::julian_day_column,
    OptionBase::buildoption, "The julian day number column (in the input data) \n");

  declareOption(ol, "risk_free_rate_column", &MountLucasIndex::risk_free_rate_column,
    OptionBase::buildoption, "The risk free rate column (in the input data) \n");

  inherited::declareOptions(ol);
}

void MountLucasIndex::train()
{
  if (!build_complete)
  {
    commodity_price_index.resize(nb_commodities);
    for (int i=0; i<nb_commodities; i++)
      commodity_price_index[i] = train_set->fieldIndex(commodity_price_columns[i]);
    last_day_of_month_index = train_set->fieldIndex(last_day_of_month_column);
    julian_day_index = train_set->fieldIndex(julian_day_column);
    risk_free_rate_index = train_set->fieldIndex(risk_free_rate_column);
    build_complete = true;
  }


  Vec tbill_return(index_value.size());
  Mat mlm_return(index_value.size(),2);

  int start_t = last_train_t+1;
  ProgressBar* pb;
  if (report_progress)
    pb = new ProgressBar("Training MountLucasIndex learner",train_set.length()-start_t);

  Vec next_to_last_price(nb_commodities);
  Vec last_month_last_price(nb_commodities);
  Vec last_month_next_to_last_price(nb_commodities);
  Vec input(train_set->inputsize()), target(train_set->targetsize());
  real last_month_risk_free_rate;
  real w=1.0;
  for (int t=start_t; t<train_set.length(); t++)
  {
    train_set->getExample(t, input, target, w);
    bool is_last_day_of_month = bool(input[last_day_of_month_index]);
    Vec price = input(commodity_price_index);
    int julian_day = (int)input[julian_day_index];
    real risk_free_rate = input[risk_free_rate_index];

    if (is_last_day_of_month)
    {
      PDate julian_date(julian_day);
      int this_year = julian_date.year;
      int this_month = julian_date.month;
      mlm_return(current_month,0) = this_year;
      if (current_month == 0)
      {
        // next-to-last trading day of the month
        next_to_last_unit_asset_value.firstRow().fill(1.0);
        last_month_next_to_last_price << next_to_last_price;

        // last trading day of the month
        unit_asset_value.fill(1000.0); // arbitrary initial value
        index_value[0] = 1000.0; // MLM Index initial value
        tbill_return[0] = 1000.0; // MLM Index initial value
        mlm_return(0,1) = 0;
        last_month_last_price << price;
        last_month_risk_free_rate = risk_free_rate;
      }
      else
      {
        // next-to-last trading day of the month
        Vec last_value = next_to_last_unit_asset_value(current_month-1);
        Vec this_value = next_to_last_unit_asset_value(current_month);
        // last trading day of the month
        Vec rate_return(nb_commodities);
        for (int i=0; i<nb_commodities; i++)
        {
          if (tradable_commodity[i])
          {
            this_value[i] = last_value[i]*(next_to_last_price[i]/last_month_next_to_last_price[i]);
            if (is_missing(this_value[i])) this_value[i] = last_value[i];
            rate_return[i] = (price[i]/last_month_last_price[i] - 1.0);
            if (!is_long_position[i]) rate_return[i] = -rate_return[i];
            unit_asset_value[i] *= price[i]/last_month_last_price[i];
          }
          else
          {
            this_value[i] = 1.0;
            rate_return[i] = MISSING_VALUE;
            unit_asset_value[i] = 1000.0;
          }

          last_month_next_to_last_price[i] = next_to_last_price[i];
          last_month_last_price[i] = price[i];
        }
        real risk_free_rate_return = exp(log(last_month_risk_free_rate + 1.0)/12.0) - 1.0;
        real delta = mean_with_missing(rate_return) + risk_free_rate_return;
        if (is_missing(delta)) delta = 0.0; // first year
        index_value[current_month] = index_value[current_month-1]*(1.0 + delta);
        last_month_risk_free_rate = risk_free_rate;
        tbill_return[current_month] = is_missing(risk_free_rate_return) ? tbill_return[current_month-1] : tbill_return[current_month-1]*(1.0+risk_free_rate_return);
        mlm_return(current_month,1) = delta*100.0;
      }
      errors(current_month,0) = index_value[current_month];
      real mean_unit_asset_value = mean(unit_asset_value);
      for (int i=0; i<nb_commodities; i++)
      {
        predictions(current_month,i) = mean_unit_asset_value/price[i]*(is_long_position[i]?1.0:-1.0);
        is_long_position[i] = next_position(i, next_to_last_unit_asset_value);
      }
      ++current_month;
      
      // at the end of the year, choose which commodity will be included in the index the next year
      if (this_month == 12)
      {
        for (int i=0; i<nb_commodities; i++)
        {
          if (this_year+1 >= commodity_start_year[i])
            tradable_commodity[i] = true;
        }
      }
    }
    next_to_last_price << price;

    if (pb) pb->update(t-start_t);
  }
  last_train_t = train_set.length()-1;
  if (pb) delete pb;

  index_value.resize(current_month,1);
  Mat mlm_index(index_value.length(),1,index_value.data());
  saveAscii("MLMIndex.amat", mlm_index);
  //saveAscii("MLMIndex.avec", index_value);
  cout << "Rendement du MLM Index = " << exp(log(index_value[474]/index_value[18])/38.0) << endl;

  mlm_return.resize(current_month,2);
  saveAscii("MLM_return.amat", mlm_return);

  tbill_return.resize(current_month);
  saveAscii("TBill_return.avec", tbill_return);
  cout << "Rendement du TBill = " << exp(log(tbill_return[474]/tbill_return[18])/38.0) << endl;
}

// nothing to do in test
void MountLucasIndex::test(VMat testset, PP<VecStatsCollector> test_stats,
    VMat testoutputs, VMat testcosts) const
{}

bool MountLucasIndex::next_position(int pos, const Mat& unit_asset_value_) const
{
  real the_month_unit_asset_value = unit_asset_value_(current_month, pos);
  real moving_average = 0;
  int start = MAX(0,current_month-11);
  for (int t=start; t<=current_month; t++)
    moving_average += unit_asset_value_(t,pos);
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
  TVec<string> dummy_string(1, "MLM_Index_Value");
  return dummy_string;
}

TVec<string> MountLucasIndex::getTestCostNames() const
{ return getTrainCostNames(); }

void MountLucasIndex::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  deepCopyField(commodity_price_columns, copies);
  deepCopyField(commodity_start_year, copies);
  deepCopyField(is_long_position, copies);
  deepCopyField(tradable_commodity, copies);
  deepCopyField(twelve_month_moving_average, copies);
  deepCopyField(next_to_last_unit_asset_value, copies);
  deepCopyField(unit_asset_value, copies);
  deepCopyField(index_value, copies);
  deepCopyField(commodity_price_index, copies);
}

%> // end of namespace PLearn

