
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
   * $Id: MountLucasIndex.cc,v 1.12 2003/09/22 18:34:21 ducharme Exp $ 
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
  last_month_last_price.resize(nb_commodities);
  last_month_next_to_last_price.resize(nb_commodities);
  last_tradable_price.resize(nb_commodities);
  next_to_last_tradable_price.resize(nb_commodities);

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
  last_tradable_price.fill(MISSING_VALUE);
  next_to_last_tradable_price.fill(MISSING_VALUE);

  s=0,s2=0,sf=0,sf2=0;
  ns=0;
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

  declareOption(ol, "transaction_multiplicative_cost", &MountLucasIndex::transaction_multiplicative_cost,
    OptionBase::buildoption, "transaction_multiplicative_cost \n");

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

  tbill_return.resize(index_value.size());
  mlm_return.resize(index_value.size(),2);
  mlm_index.resize(index_value.size(),4);

  //int start_t = last_train_t+1;
  int start_t = MAX(last_train_t+1,last_test_t+1);
  ProgressBar* pb;
  if (report_progress)
    pb = new ProgressBar("Training MountLucasIndex learner",train_set.length()-start_t);

  Vec input(train_set->inputsize()), target(train_set->targetsize());
  real w=1.0;
  for (int t=start_t; t<train_set.length(); t++)
  {
    train_set->getExample(t, input, target, w);
    TrainTestCore(input);
    state(t) << predictions(current_month);
    last_train_t = t;
    if (pb) pb->update(t-start_t);
  }
  last_call_train_t = train_set.length()-1;
  if (pb) delete pb;

/*
  //index_value.resize(current_month,1);
  //mlm_index.resize(current_month,mlm_index.width());
  //saveAscii("MLMIndex.amat", mlm_index);
  //saveAscii("MLMIndex.avec", index_value);
  //cout << "Rendement du MLM Index = " << exp(log(index_value[current_month-1]/index_value[0])/(current_month/12.0)) << endl;
  cout << "Rendement du MLM Index = " << exp(log(index_value[current_month-1]/index_value[0])/(ns/12.0)) << endl;

  //mlm_return.resize(current_month,2);
  saveAscii("MLM_return.amat", mlm_return);

  //tbill_return.resize(current_month);
  //saveAscii("TBill_return.avec", tbill_return);
  //cout << "Rendement du TBill = " << exp(log(tbill_return[current_month-1]/tbill_return[17])/((current_month-17)/12.0)) << endl;

  real r = s/ns;
  real v = s2/ns - r*r;
  cout << "Average annual return (composed monthly) = " << exp(r*12) << endl;
  cout << "Sharpe Ratio of monthly log-returns = " << r/sqrt(v) << endl;
  real rf = sf/ns;
  cout << "Average annual return (composed monthly) with transaction fees = " << exp(rf*12) << endl;
  real vf = sf2/ns - rf*rf;
  cout << "Sharpe Ratio of monthly log-returns with transaction fees = " << rf/sqrt(vf) << endl;
*/
}

void MountLucasIndex::test(VMat testset, PP<VecStatsCollector> test_stats,
    VMat testoutputs, VMat testcosts) const
{
  int start_t = MAX(last_train_t+1,last_test_t+1);
  ProgressBar* pb;
  if (report_progress)
    pb = new ProgressBar("Testing MountLucasIndex learner",testset.length()-start_t);

  Vec input(testset->inputsize()), target(testset->targetsize());
  real w=1.0;
  for (int t=start_t; t<testset.length(); t++)
  {
    testset->getExample(t, input, target, w);
    TrainTestCore(input, testoutputs, testcosts);
    state(t) << predictions(current_month);
    if (pb) pb->update(t-start_t);
  }
  last_test_t = testset.length()-1;
  if (pb) delete pb;

/*
  index_value.resize(current_month,1);
  mlm_index.resize(current_month,mlm_index.width());
  saveAscii("MLMIndex.amat", mlm_index);
  //saveAscii("MLMIndex.avec", index_value);
  //cout << "Rendement du MLM Index = " << exp(log(index_value[current_month-1]/index_value[0])/(current_month/12.0)) << endl;
  cout << "Rendement du MLM Index = " << exp(log(index_value[current_month-1]/index_value[0])/(ns/12.0)) << endl;

  mlm_return.resize(current_month,2);
  saveAscii("MLM_return.amat", mlm_return);

  //tbill_return.resize(current_month);
  //saveAscii("TBill_return.avec", tbill_return);
  //cout << "Rendement du TBill = " << exp(log(tbill_return[current_month-1]/tbill_return[17])/((current_month-17)/12.0)) << endl;
*/

  real r = s/ns;
  real v = s2/ns - r*r;
  cout << "Average annual return (composed monthly) = " << exp(r*12) << endl;
  cout << "Sharpe Ratio of monthly log-returns = " << r/sqrt(v) << endl;
  real rf = sf/ns;
  cout << "Average annual return (composed monthly) with transaction fees = " << exp(rf*12) << endl;
  real vf = sf2/ns - rf*rf;
  cout << "Sharpe Ratio of monthly log-returns with transaction fees = " << rf/sqrt(vf) << endl;
}

void MountLucasIndex::TrainTestCore(const Vec& input, VMat testoutputs, VMat testcosts) const
{
  bool is_last_day_of_month = bool(input[last_day_of_month_index]);
  Vec price = input(commodity_price_index);
  Vec rate_return(nb_commodities);
  int julian_day = (int)input[julian_day_index];
  real risk_free_rate = input[risk_free_rate_index];
  int n_traded=0;

  for (int i=0; i<nb_commodities; i++)
  {
    if (!is_missing(price[i]))
    {
      next_to_last_tradable_price[i] = last_tradable_price[i];
      last_tradable_price[i] = price[i];
    }
  }

  if (is_last_day_of_month)
  {
    PDate julian_date(julian_day);
    int this_year = julian_date.year;
    int this_month = julian_date.month;
    int this_day = julian_date.day;
    real risk_free_rate_return=0;
    mlm_return(current_month,0) = this_year;
    mlm_index(current_month,0) = this_year;
    mlm_index(current_month,1) = this_month;
    mlm_index(current_month,2) = this_day;
    if (current_month == 0)
    {
      // next-to-last trading day of the month
      next_to_last_unit_asset_value.firstRow().fill(1.0);
      last_month_next_to_last_price << next_to_last_tradable_price;

      // last trading day of the month
      unit_asset_value.fill(1000.0); // arbitrary initial value
      index_value[0] = 1000.0; // MLM Index initial value
      tbill_return[0] = 1000.0; // MLM Index initial value
      mlm_return(0,1) = 0;
      mlm_index(current_month,3) = 1000.0;
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
          this_value[i] = last_value[i]*(next_to_last_tradable_price[i]/last_month_next_to_last_price[i]);
          if (is_missing(this_value[i])) this_value[i] = last_value[i];
          rate_return[i] = (last_tradable_price[i]/last_month_last_price[i] - 1.0);
          if (!is_long_position[i]) rate_return[i] = -rate_return[i];
          unit_asset_value[i] *= last_tradable_price[i]/last_month_last_price[i];
          n_traded++;
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

      if (n_traded>0)
      {
        risk_free_rate_return = exp(log(last_month_risk_free_rate + 1.0)/12.0) - 1.0;
        real delta = mean_with_missing(rate_return) + risk_free_rate_return;
        real log_return = log(1+delta);
        s += log_return;
        s2 += log_return*log_return;
        ns++;
        if (is_missing(delta)) PLWARNING("delta=nan"); //delta = 0.0; // first year
        index_value[current_month] = index_value[current_month-1]*(1.0 + delta);
        mlm_return(current_month,1) = delta*100.0;
      }
      else 
      {
        index_value[current_month] = index_value[current_month-1];
        mlm_return(current_month,1) = 0;
      }
      mlm_index(current_month,3) = index_value[current_month];
      last_month_risk_free_rate = risk_free_rate;
      tbill_return[current_month] = is_missing(risk_free_rate_return) ? tbill_return[current_month-1] : tbill_return[current_month-1]*(1.0+risk_free_rate_return);
    }
    errors(current_month,0) = index_value[current_month];
    real total_return_with_transaction_fees = 0;
    for (int i=0; i<nb_commodities; i++)
    {
      real old_relative_position = (is_long_position[i]?1.0:-1.0);
      bool next_pos = next_position(i, next_to_last_unit_asset_value);
      real new_relative_position = (next_pos?1.0:-1.0);
      if (current_month>0 && tradable_commodity[i])
      {
        real old_position_in_dollars = old_relative_position * index_value[current_month-1];
        real new_position_in_dollars = new_relative_position * index_value[current_month];
        real transaction_amount = old_position_in_dollars*(1+rate_return[i])+fabs(old_position_in_dollars)*(1+risk_free_rate_return) - new_position_in_dollars;
        real return_with_transaction_fees = rate_return[i] + risk_free_rate_return - transaction_multiplicative_cost*fabs(transaction_amount/old_position_in_dollars);
        if (!is_missing(return_with_transaction_fees))
          total_return_with_transaction_fees += return_with_transaction_fees;
        predictions(current_month,i) = next_pos*index_value[current_month]/(n_traded*last_tradable_price[i]);
      }
      else
        predictions(current_month,i) = 0.0;
      is_long_position[i] = next_pos;
    }
    if (current_month>0 && n_traded>0)
    {
      total_return_with_transaction_fees = log(1+total_return_with_transaction_fees/n_traded);
      sf += total_return_with_transaction_fees; 
      sf2 += total_return_with_transaction_fees*total_return_with_transaction_fees; 
    }
    if (testoutputs) testoutputs->appendRow(predictions(current_month));
    if (testcosts) testcosts->appendRow(errors(current_month));
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
}

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

