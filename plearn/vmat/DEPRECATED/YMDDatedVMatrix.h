// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2001 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2002 Pascal Vincent, Julien Keable, Xavier Saint-Mleux
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


/*! \file YMDDatedVMatrix.h */

#ifndef YMDDatedVMatrix_INC
#define YMDDatedVMatrix_INC

#include "DatedVMatrix.h"
#include "VMat.h"

namespace PLearn {
using namespace std;
 
/*!   A DatedVMatrix that knows about years, months, and days,
  and for which the relative differences between two dates (e.g. days)
  depends on the number of different values of that unit (e.g. the
  number of days which actually occur).
*/
class YMDDatedVMatrix: public DatedVMatrix
{
    typedef DatedVMatrix inherited;

public:
    VMat data;
protected:
    Mat years; //!<  single column of years (e.g. 1987), one year per data row
    Mat months; //!<  single column of months (between 1 and 12), one month per data row
    Mat days; //!<  single column of days (between 1 and 31), one day per data row
    Vec pos_of_ith_year; //!<  row index of first row of i-th year in the data
    Vec pos_of_ith_month; //!<  row index of first row of i-th month in the data
    Vec pos_of_ith_day; //!<  row index of first row of i-th month in the data
    Vec day_of_ith_pos; //!<  inverse map of pos_of_ith_day
    Array< TMat<int> > pos_of_date; //!<  dates[year-years(0,0)][month-1][day-1] is the 
    //!  position of the absolute (year,month,day) date

public:
    // ******************
    // *  Constructors  *
    // ******************
    YMDDatedVMatrix(); //!<  default constructor (for automatic deserialization)

    //!  THE DATES MUST BE IN INCREASING CHRONOLOGICAL ORDER.
    //!  Warning: VMFields are NOT YET handled by this constructor
    YMDDatedVMatrix(VMat data_, Mat years_, Mat months_, Mat days_);

    //!  alternatively, the given matrix has (year, month, day) in the
    //!  first 3 columns and the rest of the data in the remaining columns.
    //!  Warning: VMFields are NOT YET handled by this constructor
    YMDDatedVMatrix(Mat& YMD_and_data);

    PLEARN_DECLARE_OBJECT(YMDDatedVMatrix);
    static void declareOptions(OptionList &ol);

    virtual void build();

    //!  return the number of real fields required to specify a date: here 3 for YMD
    int nDateFields() { return 3; }

/*!     this one calls one of subDistrRelative{Years,Months,Days} according
  to wether units=="years", "months", or "days" (or if the first letter
  matches, irrespective of upper/lower case distinctions)
*/
    VMat subDistrRelativeDates(int first, int n, const string& units);
    VMat subDistrRelativeYears(int first_relative_year, int n_years);
    VMat subDistrRelativeMonths(int first_relative_month, int n_months);
    VMat subDistrRelativeDays(int first_relative_day, int n_days);

    //!  sub-distribution starting at the given date, for the given
    //!  number of occured years, months or days
    VMat subDistrAbsoluteYears(int year, int month, int day, int n_years);
    VMat subDistrAbsoluteMonths(int year, int month, int day, int n_months);
    VMat subDistrAbsoluteDays(int year, int month, int day, int n_days);
    VMat subDistrAbsoluteUnits(int year, int month, int day, int n_units, const string& units);

    //!  return "size" in the given units (e.g. interval in years, months, etc...)
    //!  here e.g. the number of different years (if units=="years").
    int lengthInDates(const string& units);

    //!  return row position of example whose relative date is the first with
    //!  the given (relative) value, in the given time units
    int positionOfRelativeDate(int first, const string& units);

    //!  return row position associated with first sample whose date
    //!  is given
    int positionOfDate(int year, int month, int day);

    //!  copy the date fields for the relative positions starting at the
    //!  given row position for the given number of rows, into the given matrix
    void copyDatesOfRows(int from_row, int n_rows, Mat& dates);
  
    // added by Julien Keable :
    // returns vector of row at indice 'row'
    // and associated date trough year, month and day
    Vec copyRowDataAndDate(int row, int &year, int &month, int &day);
    void copyDateOfRow(int row, int &year, int &month, int &day);

    virtual void reset_dimensions() { PLERROR("YMDDatedVMatrix::reset_dimensions() not implemented"); }
private:
    void build_();
};

DECLARE_OBJECT_PTR(YMDDatedVMatrix);

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
