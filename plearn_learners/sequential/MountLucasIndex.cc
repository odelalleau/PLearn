
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
   * $Id: MountLucasIndex.cc,v 1.22 2003/10/22 21:31:54 ducharme Exp $ 
   ******************************************************* */

/*! \file MountLucasIndex.cc */
#include "MountLucasIndex.h"
#include "PDate.h"

namespace PLearn <%
using namespace std;


PLEARN_IMPLEMENT_OBJECT(MountLucasIndex, "ONE LINE DESCR", "NO HELP");

MountLucasIndex::MountLucasIndex()
  : last_day_of_month_column("is_last_day_of_month"), julian_day_column("Date"),
    risk_free_rate_column("risk_free_rate"), sp500_column("S&P500:close:level"),
    moving_average_window(12), positive_rebalance_threshold(INFINITY),
    negative_rebalance_threshold(-INFINITY), current_month(0),
    build_complete(false), trading_begin(false)
{
}

void MountLucasIndex::build()
{
  build_();
  inherited::build();
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

  position.resize(nb_commodities);
  tradable_commodity.resize(nb_commodities);
  next_to_last_unit_asset_value.resize(max_seq_len,nb_commodities);
  unit_asset_value.resize(nb_commodities);
  index_value.resize(max_seq_len);
  last_month_last_price.resize(nb_commodities);
  last_month_next_to_last_price.resize(nb_commodities);
  last_tradable_price.resize(nb_commodities);
  next_to_last_tradable_price.resize(nb_commodities);
  last_month_portfolio.resize(nb_commodities);
  last_month_predictions.resize(nb_commodities);

  if (train_set)
  {
    commodity_price_index.resize(nb_commodities);
    for (int i=0; i<nb_commodities; i++)
      commodity_price_index[i] = train_set->fieldIndex(commodity_price_columns[i]);
    last_day_of_month_index = train_set->fieldIndex(last_day_of_month_column);
    julian_day_index = train_set->fieldIndex(julian_day_column);
    risk_free_rate_index = train_set->fieldIndex(risk_free_rate_column);
    sp500_index = train_set->fieldIndex(sp500_column);
    build_complete = true;
  }

  forget();
}

void MountLucasIndex::forget()
{
  inherited::forget();

  position.fill(0); // default value is no position
  tradable_commodity.fill(false);
  next_to_last_unit_asset_value.fill(MISSING_VALUE);
  unit_asset_value.fill(MISSING_VALUE);
  index_value.fill(MISSING_VALUE);
  last_tradable_price.fill(MISSING_VALUE);
  next_to_last_tradable_price.fill(MISSING_VALUE);
  last_month_portfolio.fill(0.0);
  last_month_predictions.fill(0);

  last_sp500 = MISSING_VALUE, last_month_sp500 = MISSING_VALUE;
  trading_begin = false;

  current_month = 0;
  index_value[0] = 1000.0; // default initial value
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

  declareOption(ol, "sp500_column", &MountLucasIndex::sp500_column,
    OptionBase::buildoption, "The S&P500 column (in the input data) \n");

  declareOption(ol, "positive_rebalance_threshold", &MountLucasIndex::positive_rebalance_threshold,
    OptionBase::buildoption, "For positive returns (>1), the threshold over which we don't rebalance the asset. \n");

  declareOption(ol, "negative_rebalance_threshold", &MountLucasIndex::negative_rebalance_threshold,
    OptionBase::buildoption, "For negative returns (<1), the threshold under which we don't rebalance the asset. \n");

  declareOption(ol, "moving_average_window", &MountLucasIndex::moving_average_window,
    OptionBase::buildoption, "Thw window length (in month) on which we compute the moving average\n");

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
    sp500_index = train_set->fieldIndex(sp500_column);
    build_complete = true;
  }

  int start_t = MAX(last_train_t+1,last_test_t+1);
  ProgressBar* pb = NULL;
  if (report_progress)
    pb = new ProgressBar("Training MountLucasIndex learner",train_set.length()-start_t);

  Vec input(train_set->inputsize()), target(train_set->targetsize());
  real w=1.0;
  for (int t=start_t; t<train_set.length(); t++)
  {
    train_set->getExample(t, input, target, w);
    TrainTestCore(input, t);
    last_train_t = t;
    if (pb) pb->update(t-start_t);
  }
  last_call_train_t = train_set.length()-1;
  if (pb) delete pb;
}

void MountLucasIndex::test(VMat testset, PP<VecStatsCollector> test_stats,
    VMat testoutputs, VMat testcosts) const
{
  int start_t = MAX(last_train_t+1,last_test_t+1);
  ProgressBar* pb = NULL;
  if (report_progress)
    pb = new ProgressBar("Testing MountLucasIndex learner",testset.length()-start_t);

  Vec input(testset->inputsize()), target(testset->targetsize());
  real w=1.0;
  for (int t=start_t; t<testset.length(); t++)
  {
    testset->getExample(t, input, target, w);
    TrainTestCore(input, t, testoutputs, testcosts);
    test_stats->update(errors(t));
    if (pb) pb->update(t-start_t);
  }
  last_test_t = testset.length()-1;
  if (pb) delete pb;
}

void MountLucasIndex::TrainTestCore(const Vec& input, int t, VMat testoutputs, VMat testcosts) const
{
  const real initial_investment = 25000000.0; // 25 millions

  bool is_last_day_of_month = bool(input[last_day_of_month_index]);
  Vec price = input(commodity_price_index);
  Vec rate_return(nb_commodities);
  int julian_day = (int)input[julian_day_index];
  real risk_free_rate = input[risk_free_rate_index];
  real sp500 = input[sp500_index];
  int n_traded=0;
  int cost_name_pos = 0;

  for (int i=0; i<nb_commodities; i++)
  {
    if (!is_missing(price[i]))
    {
      next_to_last_tradable_price[i] = last_tradable_price[i];
      last_tradable_price[i] = price[i];
    }
  }
  if (!is_missing(sp500)) last_sp500 = sp500;

  if (is_last_day_of_month)
  {
    PDate julian_date(julian_day);
    int this_year = julian_date.year;
    int this_month = julian_date.month;
    //int this_day = julian_date.day;
    real risk_free_rate_return=MISSING_VALUE;
    real monthly_return = MISSING_VALUE;
    real sp500_log_return = MISSING_VALUE;
    real mean_return = MISSING_VALUE; // sum_i w_i*r_i / sum_i w_i
    if (current_month == 0)
    {
      // next-to-last trading day of the month
      next_to_last_unit_asset_value.firstRow().fill(1.0);
      last_month_next_to_last_price << next_to_last_tradable_price;

      // last trading day of the month
      unit_asset_value.fill(1000.0); // arbitrary initial value
      index_value[0] = 1000.0; // MLM Index initial value
      last_month_last_price << last_tradable_price;
      last_month_risk_free_rate = risk_free_rate;
    }
    else
    {
      // next-to-last trading day of the month
      Vec last_value = next_to_last_unit_asset_value(current_month-1);
      Vec this_value = next_to_last_unit_asset_value(current_month);
      // last trading day of the month
      for (int i=0; i<nb_commodities; i++)
      {
        if (tradable_commodity[i])
        {
          if (!trading_begin)
            last_month_portfolio.fill(1.0); // hack: only the first time!
          trading_begin = true;
          this_value[i] = last_value[i]*(next_to_last_tradable_price[i]/last_month_next_to_last_price[i]);
          if (is_missing(this_value[i])) this_value[i] = last_value[i];
          rate_return[i] = (last_tradable_price[i]/last_month_last_price[i] - 1.0);
          rate_return[i] *= real(position[i]);
          unit_asset_value[i] *= last_tradable_price[i]/last_month_last_price[i];
          //if (position[i] != 0) n_traded++;
        }
        else
        {
          this_value[i] = 1.0;
          rate_return[i] = MISSING_VALUE;
          unit_asset_value[i] = 1000.0;
        }

        last_month_next_to_last_price[i] = next_to_last_tradable_price[i];
        last_month_last_price[i] = last_tradable_price[i];
      }

      if (trading_begin)
      {
        risk_free_rate_return = exp(log(last_month_risk_free_rate + 1.0)/12.0) - 1.0;
        //mean_return = mean(rate_return,true);
        mean_return = weighted_mean(rate_return,last_month_portfolio,true);
        monthly_return = mean_return + risk_free_rate_return;
        if (is_missing(monthly_return)) PLWARNING("monthly_return=nan");
        sp500_log_return = log(last_sp500/last_month_sp500);
        index_value[current_month] = index_value[current_month-1]*(1.0 + monthly_return);
      }
      else 
      {
        index_value[current_month] = index_value[current_month-1];
      }
      last_month_risk_free_rate = risk_free_rate;
    }

    // rebalancing
    Vec w(nb_commodities, 0.0);
    if (trading_begin)
    {
      TVec<bool> rebalancing(nb_commodities, false);
      TVec<int> next_pos(nb_commodities, 0);
      real total_amount = initial_investment*index_value[current_month]/1000.0;
      for (int i=0; i<nb_commodities; i++)
      {
        if (current_month>0 && tradable_commodity[i])
        {
          next_pos[i] = next_position(i, next_to_last_unit_asset_value);
          if (next_pos[i] == position[i])
          {
            real relative_return = (1.0+rate_return[i])/(1.0+mean_return);
            // no rebalancing
            if (relative_return>positive_rebalance_threshold || relative_return<negative_rebalance_threshold)
            {
              w[i] = last_month_predictions[i];
              total_amount -= abs(w[i])*last_tradable_price[i];
              //n_traded--;
            }
            else
            {
              position[i] = next_pos[i];
              if (position[i] != 0) n_traded++;
              rebalancing[i] = true;
            }
          }
          else
          {
            position[i] = next_pos[i];
            if (position[i] != 0) n_traded++;
            rebalancing[i] = true;
          }
        }
      }
      for (int i=0; i<nb_commodities; i++)
      {
        if (current_month>0 && tradable_commodity[i] && rebalancing[i])
        {
          if (n_traded == 0)
            PLERROR("n_traded == 0");
          w[i] = real(position[i])*total_amount/(n_traded*last_tradable_price[i]);
        }
      }
    }
/*
    // rebalancing
    for (int i=0; i<nb_commodities; i++)
    {
      position[i] = next_position(i, next_to_last_unit_asset_value);
      if (current_month>0 && tradable_commodity[i])
      {
        w[i] = real(position[i])*initial_investment*index_value[current_month]/(n_traded*last_tradable_price[i]*1000.0);
      }
      else
      {
        w[i] = 0.0;
      }
    }
*/
    last_month_sp500 = last_sp500;

    real log_return = log(1.0+monthly_return);
    real tbill_log_return = log(1.0+risk_free_rate_return);
    errors(t,cost_name_pos++) = index_value[current_month];
    errors(t,cost_name_pos++) = monthly_return;
    errors(t,cost_name_pos++) = log_return;
    errors(t,cost_name_pos++) = tbill_log_return;
    errors(t,cost_name_pos++) = sp500_log_return;
    errors(t,cost_name_pos++) = log_return - tbill_log_return;
    errors(t,cost_name_pos++) = log_return - sp500_log_return;
    for (int i=0; i<nb_commodities; i++)
    {
      errors(t,cost_name_pos++) = log(1.0+rate_return[i]);
      predictions(t,i) = w[i];
      last_month_predictions[i] = w[i];
      real portfolio = abs(w[i])*last_tradable_price[i];
      last_month_portfolio[i] = !is_missing(portfolio) ? portfolio : 0.0;
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
  else if (t > 0)
  {
    predictions(t) << predictions(t-1);
    errors(t,cost_name_pos++) = errors(t-1,0);
    errors(t,cost_name_pos++) = MISSING_VALUE;
    errors(t,cost_name_pos++) = MISSING_VALUE;
    errors(t,cost_name_pos++) = MISSING_VALUE;
    errors(t,cost_name_pos++) = MISSING_VALUE;
    errors(t,cost_name_pos++) = MISSING_VALUE;
    errors(t,cost_name_pos++) = MISSING_VALUE;
    for (int i=0; i<nb_commodities; i++)
      errors(t,cost_name_pos++) = MISSING_VALUE;
  }
  else // t==0
  {
    predictions(0) = 0.0;
    errors(0,cost_name_pos++) = 1000.0;
    errors(0,cost_name_pos++) = MISSING_VALUE;
    errors(0,cost_name_pos++) = MISSING_VALUE;
    errors(0,cost_name_pos++) = MISSING_VALUE;
    errors(0,cost_name_pos++) = MISSING_VALUE;
    errors(0,cost_name_pos++) = MISSING_VALUE;
    errors(0,cost_name_pos++) = MISSING_VALUE;
    for (int i=0; i<nb_commodities; i++)
      errors(0,cost_name_pos++) = MISSING_VALUE;
  }

  if (testoutputs) testoutputs->appendRow(predictions(t));
  if (testcosts) testcosts->appendRow(errors(t));
}

int MountLucasIndex::next_position(int pos, const Mat& unit_asset_value_) const
{
  real the_month_unit_asset_value = unit_asset_value_(current_month, pos);
  real moving_average = 0;
  //int start = MAX(0,current_month-11);
  //int start = MAX(0,current_month-(moving_average_window-1));
  //for (int t=start; t<=current_month; t++)
  int start = MAX(0,current_month-moving_average_window);
  for (int t=start; t<current_month; t++)
    moving_average += unit_asset_value_(t,pos);
  moving_average /= (current_month-start);
  //moving_average /= (current_month+1-start);

  return (the_month_unit_asset_value >= moving_average) ? 1 : -1;
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
  TVec<string> cost_names(1, "MLM_Index_Value");
  cost_names.append("MLM_monthly_return");     // r
  cost_names.append("MLM_monthly_log_return"); // log(1+r)
  cost_names.append("TBill_monthly_log_return");
  cost_names.append("SP500_monthly_log_return");
  cost_names.append("TBill_relative_monthly_log_return");
  cost_names.append("SP500_relative_monthly_log_return");
  //! The individual returns
  for (int i=0; i<nb_commodities; i++)
  {
    string name = commodity_price_columns[i]+"_monthly_log_return";
    cost_names.append(name);
  }
  return cost_names;
}

TVec<string> MountLucasIndex::getTestCostNames() const
{ return getTrainCostNames(); }

void MountLucasIndex::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  deepCopyField(commodity_price_columns, copies);
  deepCopyField(commodity_start_year, copies);
  deepCopyField(position, copies);
  deepCopyField(tradable_commodity, copies);
  deepCopyField(next_to_last_unit_asset_value, copies);
  deepCopyField(unit_asset_value, copies);
  deepCopyField(index_value, copies);
  deepCopyField(commodity_price_index, copies);
  deepCopyField(last_month_last_price, copies);
  deepCopyField(last_month_next_to_last_price, copies);
  deepCopyField(last_tradable_price, copies);
  deepCopyField(next_to_last_tradable_price, copies);
  deepCopyField(last_month_portfolio, copies);
  deepCopyField(last_month_predictions, copies);
}

%> // end of namespace PLearn

