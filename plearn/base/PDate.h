// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
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
 * This file is part of the PLearn library.
 ******************************************************* */


/*! \file PDate.h */

#ifndef PDate_INC
#define PDate_INC

//#include <iostream>
#include <string>
#include <ctime>
#include <plearn/io/PStream.h>
#include <plearn/base/TypeTraits.h>

namespace PLearn {
using namespace std;

/*!   PDate must be a concrete class with no virtual function.  Hence it
  MUST NOT UNDER ANY CONSIDERATION UNDER HEAVY PRISON PENALTY
  derive from Object.
*/

class PDate
{
public:
    static PDate lastDateOfMonth(int year, int month); // Builds a PDate
    static unsigned char lastDayOfMonth(int year, int month);

public:
    short year; //!<  ex: 1983, 2001, -350, ...
    unsigned char month; //!<  1..12
    unsigned char day; //!<  1..31

    //!  Create a missing date by default
    PDate();
    
    PDate(short the_year, unsigned char the_month, unsigned char the_day)
        :year(the_year), month(the_month), day(the_day) {}
  
    PDate(int the_year, int the_month, int the_day)
        :year(the_year), month(int(the_month)), day(int(the_day)) {}

    //!  convert a Julian day into a PDate. Day 0 of the Julian calendar
    //!  is about 6000 years ago... 
    PDate(int julian_day);

    //!  Initialize a date from a string. 
    //!  Currently recognized formats are: "2003/01/27", "27JAN2003"
    //!  if invalid_value_as_missing is true and the string is an invalid date, we set the date as missing
    PDate(string date,bool invalid_value_as_missing=false);

    //!  Missing date handling
    bool isMissing() const;
    void setMissing();
    
    //!  Returns true if the date is valid; manages leap years.
    bool isValid() const;
    PDate lastDateOfMonth() const { return PDate::lastDateOfMonth(year, month); }
    unsigned char lastDayOfMonth() const { return PDate::lastDayOfMonth(year, month); }
    
    string info() const; //!<  returns the date in the format //!<  yyyy/mm/dd

    //!  Equality comparison
    bool operator==(const PDate& rhs) const {
        return year == rhs.year && month == rhs.month && day == rhs.day;
    }
  
    bool operator!=(const PDate& rhs) const {
        return ! (*this == rhs);
    }
  
    bool operator<(const PDate& rhs) const {
        return year < rhs.year ||
            (year == rhs.year && month < rhs.month) ||
            (year == rhs.year && month == rhs.month && day < rhs.day);
    }
  
    bool operator<=(const PDate& rhs) const {
        return *this == rhs || *this < rhs;
    }
  
    bool operator>(const PDate& rhs) const {
        return ! (*this <= rhs);
    }
  
    bool operator>=(const PDate& rhs) const {
        return ! (*this < rhs);
    }

/*!     Convert a PDate into a Julian day.
  See http://quasar.as.utexas.edu/BillInfo/JulianDatesG.html
  (if still existing...) for details.
*/
    int toJulianDay() const;

    //0=monday ... 6=sunday
    int dayOfWeek() const
    { return toJulianDay()%7; }

    // 0 = january 1 of the year
    int dayOfYear() const;

    //! Returns week number in the year between 0 and 52 incl. (ISO 8601 minus 1) 
    int weekNumber() const;

    static PDate today()
    {
        time_t now(time(0));
        tm* snow = localtime(&now);
        return PDate(1900+snow->tm_year, 1+snow->tm_mon, snow->tm_mday); 
    }

};

//!  substract two dates, the result being counted in days.
inline int operator-(const PDate& to_date, const PDate& from_date)
{
    return to_date.toJulianDay() - from_date.toJulianDay();
}

//! add a number of days
inline PDate operator+(const PDate& pdate, int ndays)
{
    return PDate(pdate.toJulianDay()+ndays);
}

//! subtract a number of days
//! add a number of days
inline PDate operator-(const PDate& pdate, int ndays)
{
    return PDate(pdate.toJulianDay()-ndays);
}


inline ostream& operator<<(ostream& os, const PDate& date)
{
    os << date.info();
    return os;
}

//! Serialization to PStream
PStream& operator<<(PStream& out, const PDate& date);

//! De-serialization from PStream
PStream& operator>>(PStream& in, PDate& date);


//! Takes a date (in cyymmdd or yyyymmdd format) and adds the given number of months (may be negative)
//! Returns result in same format.
inline int add_months_to_date(int xyymmdd, int nmonths)  // x=c or x=yy
{
    int xyy  = xyymmdd/10000;
    int mmdd = xyymmdd%10000;
    int mm   = mmdd/100;
    int dd   = mmdd%100;
    
    int monthpos = xyy*12+(mm-1)+nmonths;
    xyy = monthpos/12;
    mm  = 1+monthpos%12;
    return xyy*10000+mm*100+dd;
}

//! Converts date to float: ex: September 29 1972: 720929; December 25 2002: 1021225
//! Also converts missing date to missing float value and vice-versa. The format for
//! a float date is CYYMMDD.
float date_to_float(const PDate& t);

PDate float_to_date(float f);

inline PDate float_to_date(double d) 
{ return float_to_date(float(d)); }


//! Converts date to double: ex: September 29 1972: 720929; December 25 2002: 20021225
//! Also converts missing date to missing double value and vice-versa. The format for
//! a double date is YYYYMMDD.
double date_to_double(const PDate& t);

PDate double_to_date(double d);

DECLARE_TYPE_TRAITS(PDate);

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
