
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
   * $Id: AddTradableColumnVMatrix.cc,v 1.1 2003/08/07 20:21:37 ducharme Exp $ 
   ******************************************************* */

/*! \file AddTradableColumnVMatrix.cc */
#include "AddTradableColumnVMatrix.h"

namespace PLearn <%
using namespace std;


AddTradableColumnVMatrix::AddTradableColumnVMatrix()
  : parentclass()
{}

AddTradableColumnVMatrix::AddTradableColumnVMatrix(VMat vm, int nb_assets,
    int threshold)
  :parentclass(vm->length(), vm->width()+nb_assets),
   min_volume_threshold(threshold), underlying(vm), name_col(nb_assets),
   row_buffer(vm->width())
{
}

PLEARN_IMPLEMENT_OBJECT_METHODS(AddTradableColumnVMatrix, "AddTradableColumnVMatrix", RowBufferedVMatrix);

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
    real volume = underlying->get(i, it->second);
    if (!is_missing(volume) && (int)volume>min_volume_threshold)
      v[pos] = 1.0;
    else
      v[pos] = 0.0;
  }
}

void AddTradableColumnVMatrix::declareOptions(OptionList& ol)
{
  declareOption(ol, "min_volume_threshold", &AddTradableColumnVMatrix::min_volume_threshold, OptionBase::buildoption,
                "The threshold saying if the asset is tradable or not.");

  // Now call the parent class' declareOptions
  parentclass::declareOptions(ol);
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
      if (*it == "volume")
      {
        int pos = underlying->fieldIndex(fieldname);
        name_col[nb_assets++].first = is_tradable_name;
        name_col[nb_assets++].second = pos;
      }
    }
  }

  if (nb_assets != name_col.size())
    PLERROR("In AddTradableColumnVMatrix::build_: bad number of assets.");

  setVMFields();
}

// ### Nothing to add here, simply calls build_
void AddTradableColumnVMatrix::build()
{
  parentclass::build();
  build_();
}

void AddTradableColumnVMatrix::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  parentclass::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("AddTradableColumnVMatrix::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

%> // end of namespace PLearn

