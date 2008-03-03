// -*- C++ -*-

// AddMissingVMatrix.cc
//
// Copyright (C) 2005 Olivier Delalleau
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
   * $Id: AddMissingVMatrix.cc,v 1.2 2005/08/26 16:50:57 delallea Exp $
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file AddMissingVMatrix.cc */


#include "AddMissingVMatrix.h"

namespace PLearn {
using namespace std;

//////////////////
// AddMissingVMatrix //
//////////////////
AddMissingVMatrix::AddMissingVMatrix():
    random_gen(new PRandom()),
    add_missing_target(true),
    missing_prop(0),
    only_on_first(-1),
    seed(-1)
{}

PLEARN_IMPLEMENT_OBJECT(AddMissingVMatrix,
                        "Adds missing values to an underlying VMatrix",
                        "The columns which will be filled with missing values can be defined\n" 
                        "by the user using the field missing_values_columns or determined\n"
                        "randomly, for each sample (if missing_values_columns is empty).\n"
                        "When randomly adding missing values to an underlying VMatrix,\n"
                        "note that you should precompute such a VMatrix, since the missing\n"
                        "values will change within the same row if it is accessed more than\n"
                        "once.\n"
);

////////////////////
// declareOptions //
////////////////////
void AddMissingVMatrix::declareOptions(OptionList& ol)
{
  declareOption(ol, "missing_prop", &AddMissingVMatrix::missing_prop, OptionBase::buildoption,
      "Percentage of missing values.");

  declareOption(ol, "only_on_first", &AddMissingVMatrix::only_on_first, OptionBase::buildoption,
      "Only add missing values in the first 'only_on_first' samples (ignored if < 0).");

  declareOption(ol, "seed", &AddMissingVMatrix::seed, OptionBase::buildoption,
      "Random numbers seed.");

  declareOption(ol, "missing_values_columns", &AddMissingVMatrix::missing_values_columns, OptionBase::buildoption,
      "Columns which will be filled with missing values.");

    declareOption(ol, "add_missing_target",
                  &AddMissingVMatrix::add_missing_target,
                  OptionBase::buildoption,
        "Whether or not to add missing values in the target part.");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

///////////
// build //
///////////
void AddMissingVMatrix::build()
{
  // ### Nothing to add here, simply calls build_
  inherited::build();
  build_();
}

////////////
// build_ //
////////////
void AddMissingVMatrix::build_()
{
    updateMtime(source);
    // Ensure we are not using both a missing values proportion and
    // user-specified missing columns.
    if (!fast_exact_is_equal(missing_prop, 0) &&
            !missing_values_columns.isEmpty())
        PLERROR("In AddMissingVMatrix::build_ - You may only use one of these "
                "two options: 'missing_prop' or 'missing_values_columns'");

    random_gen->manual_seed(seed);
    setMetaInfoFromSource();
}

///////////////
// getNewRow //
///////////////
void AddMissingVMatrix::getNewRow(int i, const Vec& v) const
{
  source->getRow(i, v);
  if (only_on_first >= 0 && i >= only_on_first)
    return;
  int n = v.length();

  if (missing_values_columns.length()>0)
  {
      for (int j = 0; j < missing_values_columns.length(); j++)
          v[missing_values_columns[j]] = MISSING_VALUE;
  }
  else   
  {
      for (int j = 0; j < n; j++) {
          bool is_target = (j >= source->inputsize() &&
                            j < source->inputsize() + source->targetsize());
          if ((add_missing_target || !is_target) &&
          random_gen->uniform_sample() < missing_prop)
          {
              v[j] = MISSING_VALUE;
          }
      }
  }
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void AddMissingVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(random_gen, copies);
  deepCopyField(missing_values_columns, copies);
}

real AddMissingVMatrix::getStringVal(int col, const string & str) const
{
#ifdef BOUNDCHECK
    if(col>=width_)
        PLERROR("access out of bound. Width=%i accessed col=%i",width_,col);
#endif
    return source->getStringVal(col,str);
}

string AddMissingVMatrix::getValString(int col, real val) const
{
#ifdef BOUNDCHECK
    if(col>=width_)
        PLERROR("access out of bound. Width=%i accessed col=%i",width_,col);
#endif
    return source->getValString(col,val);
}

PP<Dictionary> AddMissingVMatrix::getDictionary(int col) const
{
#ifdef BOUNDCHECK
    if(col>=width_)
        PLERROR("access out of bound. Width=%i accessed col=%i",width_,col);
#endif
    return source->getDictionary(col);
}


void AddMissingVMatrix::getValues(int row, int col, Vec& values) const
{
#ifdef BOUNDCHECK
    if(col>=width_)
        PLERROR("access out of bound. Width=%i accessed col=%i",width_,col);
#endif
    source->getValues(row,col,values);
}

void AddMissingVMatrix::getValues(const Vec& input, int col, Vec& values) const
{
#ifdef BOUNDCHECK
    if(col>=width_)
        PLERROR("access out of bound. Width=%i accessed col=%i",width_,col);
#endif
    source->getValues(input, col,values);
}

} // end of namespace PLearn

/*
  Local Variables:
  mode:c++
  c-basic-offset:4
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:79
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=79 :
