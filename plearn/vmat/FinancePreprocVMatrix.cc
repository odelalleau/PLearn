
// -*- C++ -*-

// FinancePreprocVMatrix.cc
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
   * $Id: FinancePreprocVMatrix.cc,v 1.1 2003/09/03 21:01:00 ducharme Exp $ 
   ******************************************************* */

/*! \file FinancePreprocVMatrix.cc */
#include "FinancePreprocVMatrix.h"
#include "PDate.h"

namespace PLearn <%
using namespace std;


FinancePreprocVMatrix::FinancePreprocVMatrix()
  :inherited(), add_tradable(false), add_last_day_of_month(false),
  add_moving_average(false)
{}

FinancePreprocVMatrix::FinancePreprocVMatrix(VMat vm, TVec<string> the_asset_names,
    bool add_tradable_info, bool add_last_day, bool add_moving_average_stats,
    int threshold, TVec<string> the_price_tags,
    TVec<int> moving_average_window_length,
    string the_volume_tag, string the_date_tag)
  :inherited(vm->length(), vm->width()+(add_tradable_info?the_asset_names.size():0) + (add_last_day?1:0) + (add_moving_average_stats?the_asset_names.size()*the_price_tags.size()*moving_average_window_length.size():0)),
   underlying(vm), asset_name(the_asset_names),
   add_tradable(add_tradable_info), add_last_day_of_month(add_last_day),
   add_moving_average(add_moving_average_stats),
   min_volume_threshold(threshold), prices_tag(the_price_tags),
   moving_average_window(moving_average_window_length),
   volume_tag(the_volume_tag), date_tag(the_date_tag),
   row_buffer(vm->width())
{
  build();
}

PLEARN_IMPLEMENT_OBJECT(FinancePreprocVMatrix, "ONE LINE DESCR", "NO HELP");

void FinancePreprocVMatrix::getRow(int i, Vec v) const
{
  v.resize(width());
  Vec row_buffer = v.subVec(0, underlying.width());
  underlying->getRow(i, row_buffer);

  int pos = underlying.width();
  if (add_tradable)
  {
    for (int k=0; k<asset_name.size(); ++k, ++pos)
    {
      real volume = row_buffer[volume_index[k]];
      if (!is_missing(volume) && (int)volume>=min_volume_threshold)
        v[pos] = 1.0;
      else
        v[pos] = 0.0;
    }
  }

  if (add_last_day_of_month)
    v[pos++] = (last_day_of_month_index.contains(i)) ? 1.0 : 0.0;

  if (add_moving_average)
  {
    int price_pos = 0;
    for (int j=0; j<asset_name.length(); j++)
    {
      for (int k=0; k<prices_tag.size(); k++)
      {
        int index = price_index[price_pos++];
        int prices_length = MAX(max_moving_average_window, i+1);
        int prices_start = i+1 - prices_length;
        Vec prices(prices_length);
        for (int l=prices_start; l<prices_length; l++)
          prices[l] = underlying->get(l,index);

        for (int l=0; l<moving_average_window.size(); l++)
        {
          int start = MAX(prices.length()-moving_average_window[l], 0);
          int len = prices.length() - start;
          v[pos++] = mean(prices.subVec(start,len));
        }
      }
    }
  }
}

/*
real FinancePreprocVMatrix::get(int i, int j) const
{
  if (j < underlying.width())
    return underlying->get(i,j);
  else if (j < underlying.width()+name_col.size())
  {
    pair<string,int> this_name_col = name_col[j - underlying.width()];
    real volume = underlying->get(i, this_name_col.second);
    if (!is_missing(volume) && (int)volume>=min_volume_threshold)
      return 1.0;
    else
      return 0.0;
  }
  else
  {
    if (!add_last_day_of_month)
      PLERROR("Out of bound!");
    return last_day_of_month_index.contains(i) ? 1.0 : 0.0;
  }
}
*/

void FinancePreprocVMatrix::declareOptions(OptionList& ol)
{
  declareOption(ol, "vmat", &FinancePreprocVMatrix::underlying, OptionBase::buildoption,
      "The underlying VMat.");

  declareOption(ol, "add_tradable", &FinancePreprocVMatrix::add_tradable, OptionBase::buildoption,
                "Do we include the information telling if this day is tradable or not.");

  declareOption(ol, "add_last_day_of_month", &FinancePreprocVMatrix::add_last_day_of_month, OptionBase::buildoption,
                "Do we include the information about the last tradable day of the month or not.");

  declareOption(ol, "add_moving_average", &FinancePreprocVMatrix::add_moving_average, OptionBase::buildoption,
                "Do we include the moving average statistics on the price_tag indexes.");

  declareOption(ol, "min_volume_threshold", &FinancePreprocVMatrix::min_volume_threshold, OptionBase::buildoption,
                "The threshold saying if the asset is tradable or not.");

  declareOption(ol, "moving_average_window", &FinancePreprocVMatrix::moving_average_window, OptionBase::buildoption,
                "The window size of the moving average.");

  declareOption(ol, "prices_tag", &FinancePreprocVMatrix::prices_tag, OptionBase::buildoption,
                "The fieldInfo name for the prices columns.");

  declareOption(ol, "volume_tag", &FinancePreprocVMatrix::volume_tag, OptionBase::buildoption,
                "The fieldInfo name for the volume column.");

  declareOption(ol, "date_tag", &FinancePreprocVMatrix::date_tag, OptionBase::buildoption,
                "The fieldInfo name of the date column.");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

string FinancePreprocVMatrix::help()
{
  // ### Provide some useful description of what the class is ...
  return 
    "FinancePreprocVMatrix implements a VMatrix with extra preprocessing columns."
    + optionHelp();
}

void FinancePreprocVMatrix::setVMFields()
{
  Array<VMField>& orig_fields = underlying->getFieldInfos();

  for (int i=0; i<orig_fields.size(); i++)
    declareField(i, orig_fields[i].name, orig_fields[i].fieldtype);

  int pos = underlying.width();
  if (add_tradable)
  {
    for (int i=0; i<asset_name.size(); ++i)
    {
      string name = asset_name[i]+":is_tradable";
      declareField(pos++, name, VMField::DiscrGeneral);
    }
  }

  if (add_last_day_of_month)
    declareField(pos++, "is_last_day_of_month", VMField::DiscrGeneral);

  if (add_moving_average)
  {
    for (int i=0; i<asset_name.size(); i++)
    {
      for (int j=0; j<prices_tag.size(); j++)
      {
        for (int k=0; k<moving_average_window.size(); k++)
        {
          string moving_average_name_col = asset_name[i]+":"+prices_tag[j]+":"+tostring(moving_average_window[k]);
          declareField(pos++, moving_average_name_col, VMField::DiscrGeneral);
        }
      }
    }
  }
}

void FinancePreprocVMatrix::build_()
{
  // stuff about the tradable information
  int nb_assets = asset_name.size();
  if (add_tradable)
  {
    volume_index.resize(nb_assets);
    for (int i=0; i<nb_assets; i++)
    {
      string volume_name_col = asset_name[i]+":"+volume_tag;
      volume_index[i] = underlying->fieldIndex(volume_name_col);
    }
  }

  if (add_last_day_of_month)
  {
    int date_col = underlying->fieldIndex(date_tag);
    int julian_day = int(underlying->get(0,date_col));
    PDate first_day(julian_day);
    int previous_month = first_day.month;
    for (int i=1; i<underlying.length(); i++)
    {
      julian_day = int(underlying->get(i,date_col));
      PDate today(julian_day);
      int this_month = today.month;
      if (this_month != previous_month) last_day_of_month_index.append(i-1);
      previous_month = this_month;
    }
    // we set the last day as a last tradable day of month (by default)
    if (last_day_of_month_index.lastElement() != underlying.length()-1)
      last_day_of_month_index.append(underlying.length()-1);
  }

  if (add_moving_average)
  {
    max_moving_average_window = max(moving_average_window);

    int price_index_size = nb_assets*prices_tag.size();
    price_index.resize(price_index_size);
    int k = 0;
    for (int i=0; i<nb_assets; i++)
    {
      for (int j=0; j<prices_tag.size(); j++)
      {
        string moving_average_name_col = asset_name[i]+":"+prices_tag[j];
        price_index[k++] = underlying->fieldIndex(moving_average_name_col);
      }
    }
  }

  setVMFields();
  saveFieldInfos();
}

// ### Nothing to add here, simply calls build_
void FinancePreprocVMatrix::build()
{
  inherited::build();
  build_();
}

void FinancePreprocVMatrix::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  deepCopyField(prices_tag, copies);
  deepCopyField(moving_average_window, copies);
  deepCopyField(asset_name, copies);
  deepCopyField(volume_index, copies);
  deepCopyField(price_index, copies);
  deepCopyField(last_day_of_month_index, copies);
}

%> // end of namespace PLearn

