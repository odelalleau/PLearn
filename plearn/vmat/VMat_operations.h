// -*- C++ -*-

// VMat_operations.h
//
// Copyright (C) 2004 Pascal Vincent
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
 * $Id$
 ******************************************************* */

// Authors: Pascal Vincent

/*! \file VMat_operations.h */


#ifndef VMat_operations_INC
#define VMat_operations_INC

// Put includes here
#include <map>
#include <plearn/math/TVec.h>

namespace PLearn {
using namespace std;

class VMat;

/*!   If exclude==false (the default)
  returns a VMat containing only the rows
  whose column col has a value that belongs
  to the given set of authorized values
  If exclude==true
  returns a VMat with all the other rows
  (corresponds to grep -v)
  [MISSING_VALUE is a possible value and is handled correctly]
*/
VMat grep(VMat d, int col, Vec values, bool exclude=false);

//! returns a map mapping all different values appearing in column col to their number of occurences
map< real, int> countOccurencesInColumn(VMat m, int col);

//! returns a map mapping all different values appearing in column col to a vector of the corresponding row indices in the VMat
//! (this proceeds in 2 passes, first calling countOccurencesInColumn to allocate the exact memory)
map< real, TVec<int> > indicesOfOccurencesInColumn(VMat m, int col);

/*!   Same as above, except that the indexes of the rows are stored on disk
  rather than in memory
  (a SelectRowsFileIndexVMatrix is returned rather than a SelectRowsVMatrix)
  BEWARE: If the indexfile already exists, it is *not* recomputed, but used as is.
*/
VMat grep(VMat d, int col, Vec values, const string& indexfile, bool exclude=false);

/*!   returns a VMat that contains only the lines that do not have any MISSING_VALUE
  The indexes of the rows of the original matrix are recorded in the indexfile
  BEWARE: If the indexfile already exists, it is *not* recomputed, but used as is.
*/
VMat filter(VMat d, const string& indexfile);

//!  returns a SelectRowsVMatrix that has d's rows shuffled
VMat shuffle(VMat d);

//! returns a SelectRowsVMatrix that has d's rows bootstrapped (sample with replacement
//! and optionally re-ordered). Optionally the repeated rows are eliminated (this is actually
//! done by shuffling and taking the first 2/3 of the rows, so the length() will be always the
//! same).  Note that the default values are fine for "on-line"
//! learning algorithms but does not correspond to the usual "bootstrap".
//!
VMat bootstrap(VMat d, bool reorder=true, bool norepeat=true);

/*!   Rebalance the input VMatrix so that each class has a probability 1/nclasses.
  Also, the return VMat class alternates between all classes cyclicly.
  The resulting VMat is a SelectRowsFileIndexVMatrix which has its IntVecFile
  load if filename already exist, or computed if not.
*/
VMat rebalanceNClasses(VMat inputs, int nclasses, const string& filename);

//!  Rebalance a 2-class VMat such as to keep all the examples of the
//!  dominant class.
void fullyRebalance2Classes(VMat inputs, const string& filename, bool save_indices=true);

/*!   This VMat is a SelectRowsVMatrix which, given a threshold date,
  keep only the rows earlier (or later) than this date.  The thresdold date
  is given as a YYYYMMDD date, and the date on the original VMatrix are kept
  on 1 column (YYYYMMDD) or 3 (YYYY, MM and DD).
*/
VMat temporalThreshold(VMat distr, int threshold_date, bool is_before,
                       int yyyymmdd_col);
VMat temporalThreshold(VMat distr, int threshold_date, bool is_before,
                       int yyyy_col, int mm_col, int dd_col);



} // end of namespace PLearn

#endif


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
