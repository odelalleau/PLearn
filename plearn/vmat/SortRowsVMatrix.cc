// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2001 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2002 Pascal Vincent, Julien Keable, Xavier Saint-Mleux
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
   * $Id: SortRowsVMatrix.cc,v 1.9 2004/10/29 18:38:50 tihocan Exp $
   ******************************************************* */

#include "SortRowsVMatrix.h"
#include "SubVMatrix.h"
#include <plearn/math/TMat_sort.h>    //!< For sortRows.

namespace PLearn {
using namespace std;

/** SortRowsVMatrix **/

PLEARN_IMPLEMENT_OBJECT(SortRowsVMatrix, 
    "Sort the samples of a VMatrix according to one (or more) given columns.",
    "The implementation is not efficient at all, feel free to improve it !"
);

SortRowsVMatrix::SortRowsVMatrix() 
  : increasing_order(1)
{
  // Default = no sorting.
  sort_columns.resize(0);
}

void SortRowsVMatrix::declareOptions(OptionList &ol)
{
    declareOption(ol, "sort_columns", &SortRowsVMatrix::sort_columns, OptionBase::buildoption,
        "    the column(s) that must be sorted (the first one is the first criterion)");
    
    declareOption(ol, "increasing_order", &SortRowsVMatrix::increasing_order, OptionBase::buildoption,
        "    if set to 1, the data will be sorted in increasing order");
    
    inherited::declareOptions(ol);

    redeclareOption(ol, "indices", &SortRowsVMatrix::indices, OptionBase::nosave,
        "The indices are computed at build time.");

}

void SortRowsVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  deepCopyField(sort_columns, copies);
  inherited::makeDeepCopyFromShallowCopy(copies);
}

///////////
// build //
///////////
void SortRowsVMatrix::build()
{
  inherited::build();
  build_();
}

////////////
// build_ //
////////////
void SortRowsVMatrix::build_()
{
  // Check we don't try to sort twice by the same column (this can be confusing).
  if (sort_columns.isNotEmpty()) {
    for (int i = 0; i < sort_columns.length(); i++) {
      for (int j = i + 1; j < sort_columns.length(); j++) {
        if (sort_columns[j] == sort_columns[i]) {
          PLERROR("In SortRowsVMatrix::build_ - You have a duplicated index in the 'sort_columns' vector");
        }
      }
    }
  }
  // Construct the indices vector.
  if (source) {
    indices = TVec<int>(0, source.length()-1, 1);
    if (sort_columns.length() > 1) {
      // We need to sort many columns: we use the unefficient method.
      sortRows(source, indices, sort_columns, 0, source->length()-1, 0, increasing_order);
    } else if (sort_columns.length() > 0) {
      // Only sorting one column: we can do this more efficiently.
      Mat to_sort(source->length(), 2);
      // Fill first column with the column to sort.
      to_sort.column(0) << source.subMatColumns(sort_columns[0], 1);
      // Fill 2nd column with indices.
      to_sort.column(1) << Vec(0, to_sort.length() - 1, 1);
      // Perform the sort.
      PLearn::sortRows(to_sort, 0, increasing_order);
      // Get the indices.
      indices << to_sort.column(1);
    }
    inherited::build(); // Since we have just changed the indices.
  }
}

//////////////
// sortRows //
//////////////
void SortRowsVMatrix::sortRows(VMat& m, TVec<int>& indices, TVec<int>& sort_columns, int istart, int iend, int colstart, bool increasing_order) {
  real best;
  real jval;
  int tmp;
  bool better;
  if (sort_columns.size() > colstart) {
    int col = sort_columns[colstart]; // The current column used to perform sorting.
    for (int i = istart; i <= iend-1; i++) {
      // Let's look for the i-th element of our result.
      best = m(indices[i],col);
      for (int j = i+1; j <= iend; j++) {
        better = false;
        jval = m(indices[j],col);
        if (increasing_order && jval < best) {
          better = true;
        } else if (!increasing_order && jval > best) {
          better = true;
        }
        if (better) {
          // Swap i and j.
          tmp = indices[j];
          indices[j] = indices[i];
          indices[i] = tmp;
          best = jval;
        }
      }
    }
    // At this point, we have only sorted according to one column.
    if (sort_columns.length() > colstart + 1) {
      // There are other sorting criterions.
      // Let's find where we need to apply them.
      int i = istart;
      real val;
      while (i <= iend - 1) {
        val = m(indices[i],col);
        int j = i+1;
        while (j <= iend && m(indices[j],col) == val)
          j++;
        j--;
        if (j > i) {
          // There are consecutive elements with the same value for the sorting
          // criterion, thus we must use the other criteria to sort them correctly.
          sortRows(m, indices, sort_columns, i, j, colstart + 1, increasing_order);
        }
        i = j+1;
      }
    }
  }
}

} // end of namespcae PLearn
