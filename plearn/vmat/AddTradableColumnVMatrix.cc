
// -*- C++ -*-

// AddTradableColumnVMatrix.cc
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
   * $Id: AddTradableColumnVMatrix.cc,v 1.3 2003/08/13 08:13:45 plearner Exp $ 
   ******************************************************* */

/*! \file AddTradableColumnVMatrix.cc */
#include "AddTradableColumnVMatrix.h"
#include "PDate.h"

namespace PLearn <%
using namespace std;


AddTradableColumnVMatrix::AddTradableColumnVMatrix()
  : inherited()
{}

AddTradableColumnVMatrix::AddTradableColumnVMatrix(VMat vm, int nb_assets,
    bool add_last_day, int threshold, string the_volume_tag, string the_date_tag)
  :inherited(vm->length(), vm->width()+nb_assets+(add_last_day?1:0)),
   underlying(vm), min_volume_threshold(threshold),
   add_last_day_of_month(add_last_day), volume_tag(the_volume_tag),
   date_tag(the_date_tag), name_col(nb_assets), row_buffer(vm->width())
{
  build();
}

PLEARN_IMPLEMENT_OBJECT(AddTradableColumnVMatrix, "ONE LINE DESCR", "NO HELP");

void AddTradableColumnVMatrix::getRow(int i, Vec v) const
{
  v.resize(width());
  Vec row_buffer = v.subVec(0, underlying.width());
  underlying->getRow(i, row_buffer);

  vector< pair<string,int> >::const_iterator
    it = name_col.begin(), last = name_col.end();
  int pos = underlying.width();
  for ( ; it != last; ++it, ++pos)
  {
    real volume = row_buffer[it->second];
    if (!is_missing(volume) && (int)volume>=min_volume_threshold)
      v[pos] = 1.0;
    else
      v[pos] = 0.0;
  }

  if (add_last_day_of_month)
    v[pos] = (last_day_of_month_index.contains(i)) ? 1.0 : 0.0;
}

real AddTradableColumnVMatrix::get(int i, int j) const
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

void AddTradableColumnVMatrix::declareOptions(OptionList& ol)
{
  declareOption(ol, "min_volume_threshold", &AddTradableColumnVMatrix::min_volume_threshold, OptionBase::buildoption,
                "The threshold saying if the asset is tradable or not.");

  declareOption(ol, "add_last_day_of_month", &AddTradableColumnVMatrix::add_last_day_of_month, OptionBase::buildoption,
                "Do we include the information about the last tradable day of the month or not.");

  declareOption(ol, "volume_tag", &AddTradableColumnVMatrix::volume_tag, OptionBase::buildoption,
                "The tag searched for the volume column.");

  declareOption(ol, "date_tag", &AddTradableColumnVMatrix::date_tag, OptionBase::buildoption,
                "The fieldInfo name of the date column.");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

string AddTradableColumnVMatrix::help()
{
  // ### Provide some useful description of what the class is ...
  return 
    "AddTradableColumnVMatrix implements a VMatrix with an extra column for each asset saying if this asset is tradable that day."
    + optionHelp();
}

void AddTradableColumnVMatrix::setVMFields()
{
  Array<VMField>& orig_fields = underlying->getFieldInfos();

  for (int i=0; i<orig_fields.size(); i++)
    declareField(i, orig_fields[i].name, orig_fields[i].fieldtype);

  vector< pair<string,int> >::const_iterator
    it = name_col.begin(), last = name_col.end();
  int pos = underlying.width();
  for ( ; it != last; ++it, ++pos)
    declareField(pos, it->first, VMField::DiscrGeneral);

  if (add_last_day_of_month)
    declareField(pos, "is_last_day_of_month", VMField::DiscrGeneral);
}

void AddTradableColumnVMatrix::build_()
{
  // we build all the name_col pairs
  unsigned int nb_assets = 0;
  Array<VMField>& avmf = underlying->getFieldInfos();
  for (int i=0; i<avmf.size(); i++)
  {
    string fieldname = avmf[i].name;
    vector<string> words = split(fieldname,":");
    string asset_name = words[0];
    string is_tradable_name = asset_name + ":is_tradable";
    for (vector<string>::iterator it=words.begin(), last=words.end(); it!=last; ++it)
    {
      if (*it == volume_tag)
      {
        int pos = underlying->fieldIndex(fieldname);
        name_col[nb_assets].first = is_tradable_name;
        name_col[nb_assets].second = pos;
        nb_assets++;
      }
    }
  }
  if (nb_assets != name_col.size())
    PLERROR("In AddTradableColumnVMatrix::build_: bad number of assets.");

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

  setVMFields();
  saveFieldInfos();
}

// ### Nothing to add here, simply calls build_
void AddTradableColumnVMatrix::build()
{
  inherited::build();
  build_();
}

void AddTradableColumnVMatrix::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("AddTradableColumnVMatrix::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

%> // end of namespace PLearn

