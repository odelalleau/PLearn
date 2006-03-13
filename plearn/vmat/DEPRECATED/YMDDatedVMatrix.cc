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

#include "YMDDatedVMatrix.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(YMDDatedVMatrix, "ONE LINE DESC", "NO HELP");

YMDDatedVMatrix::YMDDatedVMatrix()
{
}

YMDDatedVMatrix::YMDDatedVMatrix(VMat data_, Mat years_, Mat months_, Mat days_)
    : inherited(data_->length(),data_->width()),data(data_), years(years_), 
      months(months_), days(days_), day_of_ith_pos(days_.length())
{
    build();
}

YMDDatedVMatrix::YMDDatedVMatrix(Mat& YMD_and_data)
    : inherited(YMD_and_data.length(),YMD_and_data.width()-3),
      data(YMD_and_data.subMatColumns(3,YMD_and_data.width()-3)), 
      years(YMD_and_data.subMatColumns(0,1)),
      months(YMD_and_data.subMatColumns(1,1)),
      days(YMD_and_data.subMatColumns(2,1)), day_of_ith_pos(YMD_and_data.length())
{
    build();
}

void
YMDDatedVMatrix::build()
{
    inherited::build();
    build_();
}

void YMDDatedVMatrix::build_()
{
    if ( data.isNotNull() && years.isNotEmpty() && months.isNotEmpty() && days.isNotEmpty() && !day_of_ith_pos.isEmpty()) {
        // check that the dates are in increasing chronological order and
        // compute the pos_of_ith_{year,month,day} vectors
        if (years.length()!=data->length() ||
            months.length()!=data->length() ||
            days.length()!=data->length())
            PLERROR("YMDDatedVMatrix: arguments should all have the same length");

        // build pos_of_date
        int first_year = (int)years(0,0);
        int last_year = (int)years(data->length()-1,0);
        int ny=last_year-first_year+1;
        pos_of_date.resize(ny);
        for (int y=0;y<ny;y++)
        {
            pos_of_date[y].resize(12,31);
            for (int m=0;m<12;m++)
                for (int d=0;d<31;d++)
                    pos_of_date[y](m,d)= -1; // -1 will mean unseen date
        }

        int n_different_years=1;
        int n_different_months=1;
        int n_different_days=1;
        for (int i=1;i<years.length();i++)
        {
            if (years(i,0)>years(i-1,0)) 
            {
                n_different_years++;
                n_different_months++;
                n_different_days++;
                pos_of_date[int(years(i,0)-first_year)](int(months(i,0)-1), int(days(i,0)-1)) = i;
            }
            else
            {
                if (years(i,0)<years(i-1,0)) 
                    PLERROR("YMDDatedVMatrix: %d-th year = %d < %d-th year= %d",
                            i,(int)years(i,0),i-1,(int)years(i-1,0));
                if (months(i,0)>months(i-1,0)) 
                {
                    n_different_months++;
                    n_different_days++;
                    pos_of_date[int(years(i,0)-first_year)](int(months(i,0)-1),int(days(i,0)-1)) = i;
                }
                else
                {
                    if (months(i,0)<months(i-1,0)) 
                        PLERROR("YMDDatedVMatrix: %d-th month = %d < %d-th month= %d",
                                i,(int)months(i,0),i-1,(int)months(i-1,0));
                    if (days(i,0)>days(i-1,0)) 
                    {
                        n_different_days++;
                        pos_of_date[int(years(i,0)-first_year)](int(months(i,0)-1),int(days(i,0)-1)) = i;
                    }
                    else  if (days(i,0)<days(i-1,0)) 
                        PLERROR("YMDDatedVMatrix: %d-th day = %d < %d-th day= %d",
                                i,(int)days(i,0),i-1,(int)days(i-1,0));
                }
            }
        }
        pos_of_ith_year.resize(n_different_years+1);
        pos_of_ith_month.resize(n_different_months+1);
        pos_of_ith_day.resize(n_different_days+1);
        int y=1;
        int m=1;
        int d=1;
        day_of_ith_pos[0]=0;
        for (int i=1;i<years.length();i++)
        {
            if (years(i,0)>years(i-1,0)) 
            {
                pos_of_ith_year[y++]=i;
                pos_of_ith_month[m++]=i;
                pos_of_ith_day[d++]=i;
            }
            else
            {
                if (years(i,0)<years(i-1,0)) 
                    PLERROR("YMDDatedVMatrix: %d-th year = %d < %d-th year= %d",
                            i,(int)years(i,0),i-1,(int)years(i-1,0));
                if (months(i,0)>months(i-1,0)) 
                {
                    pos_of_ith_month[m++]=i;
                    pos_of_ith_day[d++]=i;
                }
                else
                {
                    if (months(i,0)<months(i-1,0)) 
                        PLERROR("YMDDatedVMatrix: %d-th month = %d < %d-th month= %d",
                                i,(int)months(i,0),i-1,(int)months(i-1,0));
                    if (days(i,0)>days(i-1,0)) 
                        pos_of_ith_day[d++]=i;
                    else  if (days(i,0)<days(i-1,0)) 
                        PLERROR("YMDDatedVMatrix: %d-th day = %d < %d-th day= %d",
                                i,(int)days(i,0),i-1,(int)days(i-1,0));
                }
            }
            day_of_ith_pos[i]=d-1;
        }
        pos_of_ith_year[y]=data->length();
        pos_of_ith_month[m]=data->length();
        pos_of_ith_day[d]=data->length();
    }
}

void
YMDDatedVMatrix::declareOptions(OptionList &ol)
{
    declareOption(ol, "data", &YMDDatedVMatrix::data, OptionBase::buildoption, "");
    declareOption(ol, "years", &YMDDatedVMatrix::years, OptionBase::buildoption, "");
    declareOption(ol, "months", &YMDDatedVMatrix::months, OptionBase::buildoption, "");
    declareOption(ol, "days", &YMDDatedVMatrix::days, OptionBase::buildoption, "");
    declareOption(ol, "day_of_ith_pos", &YMDDatedVMatrix::day_of_ith_pos, OptionBase::buildoption, "");
    inherited::declareOptions(ol);
}

VMat YMDDatedVMatrix::subDistrRelativeYears(int first_relative_year, int n_years)
{
    if (first_relative_year<0 || n_years<0 ||
        first_relative_year+n_years >=pos_of_ith_year.length())
        PLERROR("YMDDatedVMatrix::subDistrRelativeYears(%d,%d) : incorrect arguments",
                first_relative_year, n_years);
    return data.subMatRows(int(pos_of_ith_year[first_relative_year]),
                           int(pos_of_ith_year[first_relative_year+n_years]-
                               pos_of_ith_year[first_relative_year]));
}

VMat YMDDatedVMatrix::subDistrRelativeMonths(int first_relative_month, int n_months)
{
    if (first_relative_month<0 || n_months<0 ||
        first_relative_month+n_months >=pos_of_ith_month.length())
        PLERROR("YMDDatedVMatrix::subDistrRelativeMonths(%d,%d) : incorrect arguments",
                first_relative_month, n_months);
    return data.subMatRows(int(pos_of_ith_month[first_relative_month]),
                           int(pos_of_ith_month[first_relative_month+n_months]-
                               pos_of_ith_month[first_relative_month]));
}

VMat YMDDatedVMatrix::subDistrRelativeDays(int first_relative_day, int n_days)
{
    if (first_relative_day<0 || n_days<0 ||
        first_relative_day+n_days >=pos_of_ith_day.length())
        PLERROR("YMDDatedVMatrix::subDistrRelativeDays(%d,%d) : incorrect arguments",
                first_relative_day, n_days);
    return data.subMatRows(int(pos_of_ith_day[first_relative_day]),
                           int(pos_of_ith_day[first_relative_day+n_days]-
                               pos_of_ith_day[first_relative_day]));
} 

int YMDDatedVMatrix::positionOfDate(int year, int month, int day)
{
    if (year<years(0,0))
        PLERROR("YMDDatedVMatrix::positionOfDate(%d,%d,%d): year %d < first year %d",
                year,month,day,year,(int)years(0,0));
    if (year>years(years.length()-1,0))
        return years.length();
    if (month<1 || month>12)
        PLERROR("YMDDatedVMatrix::positionOfDate(%d,%d,%d): month %d should be in [1,12]",
                year,month,day,month);
    if (day<1 || day>31)
        PLERROR("YMDDatedVMatrix::positionOfDate(%d,%d,%d): day %d should be in [1,31]",
                year,month,day,day);
    return int(pos_of_date[year-int(years(0,0))](month-1,day-1));
}

VMat YMDDatedVMatrix::subDistrAbsoluteYears(int year, int month, int day, int n_years)
{
    int first_pos = positionOfDate(year,month,day);
    int last_pos = positionOfDate(year+n_years,month,day);
    return data.subMatRows(first_pos,last_pos-first_pos);
}

VMat YMDDatedVMatrix::subDistrAbsoluteMonths(int year, int month, int day, int n_months)
{
    int first_pos = positionOfDate(year,month,day);
    int extra_years = (month + n_months) / 12;
    int new_month = (month + n_months) - extra_years*12;
    int last_pos = positionOfDate(year+extra_years,new_month,day);
    return data.subMatRows(first_pos,last_pos-first_pos);
}

VMat YMDDatedVMatrix::subDistrAbsoluteDays(int year, int month, int day, int n_days)
{
    int first_pos = positionOfDate(year,month,day);
    int nthday = (int)day_of_ith_pos[first_pos];
    int last_pos = (int)pos_of_ith_day[nthday+n_days];
    return data.subMatRows(first_pos,last_pos-first_pos);
}

VMat YMDDatedVMatrix::subDistrAbsoluteUnits(int year, int month, int day, int n_units, const string& units)
{
    if (units[0]=='y' || units[0]=='Y')
        return subDistrAbsoluteYears(year,month,day,n_units);
    if (units[0]=='m' || units[0]=='M')
        return subDistrAbsoluteMonths(year,month,day,n_units);
    if (units[0]=='d' || units[0]=='D')
        return subDistrAbsoluteDays(year,month,day,n_units);
    else
        PLERROR("YMDDatedVMatrix::subDistrAbsoluteUnits, unknown units, expected years,months or days!",
                units.c_str());
    Mat m;
    return VMat(m);
}

VMat YMDDatedVMatrix::subDistrRelativeDates(int first, int n, const string& units)
{
    if (units[0]=='y' || units[0]=='Y')
        return subDistrRelativeYears(first,n);
    if (units[0]=='m' || units[0]=='M')
        return subDistrRelativeMonths(first,n);
    if (units[0]=='d' || units[0]=='D')
        return subDistrRelativeDays(first,n);
    else
        PLERROR("YMDDatedVMatrix::subDistrRelativeDates(%d,%d,%s), unknown units, expected years,months or days!",
                first,n,units.c_str());
    return VMat(Mat());
}

int YMDDatedVMatrix::lengthInDates(const string& units)
{
    if (units[0]=='y' || units[0]=='Y')
        return pos_of_ith_year.length()-1;
    if (units[0]=='m' || units[0]=='M')
        return pos_of_ith_month.length()-1;
    if (units[0]=='d' || units[0]=='D')
        return pos_of_ith_day.length()-1;
    else
        PLERROR("YMDDatedVMatrix::lengthInDates(%s), unknown units, expected years,months or days!",
                units.c_str());
    return 0;
}

int YMDDatedVMatrix::positionOfRelativeDate(int first, const string& units)
{
    if (units[0]=='y' || units[0]=='Y')
        return (int)pos_of_ith_year[first];
    if (units[0]=='m' || units[0]=='M')
        return (int)pos_of_ith_month[first];
    if (units[0]=='d' || units[0]=='D')
        return (int)pos_of_ith_day[first];
    else
        PLERROR("YMDDatedVMatrix::positionOfRelativeDate(%s), unknown units, expected years,months or days!",
                units.c_str());
    return 0;
}

void YMDDatedVMatrix::copyDatesOfRows(int from_row, int n_rows, Mat& dates)
{
    dates.resize(n_rows,3);
    for (int i=from_row;i<from_row+n_rows;i++)
    {
        dates(i-from_row,0)=years(i,0);
        dates(i-from_row,1)=months(i,0);
        dates(i-from_row,2)=days(i,0);
    }
}

Vec YMDDatedVMatrix::copyRowDataAndDate(int row, int &year, int &month, int &day)
{
    year = (int)years(row,0);
    month = (int)months(row,0);
    day = (int)days(row,0);
    return data(row);
}
void YMDDatedVMatrix::copyDateOfRow(int row, int &year, int &month, int &day)
{
    year = (int)years(row,0);
    month = (int)months(row,0);
    day = (int)days(row,0);
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
