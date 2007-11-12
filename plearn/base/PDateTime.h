// -*- C++ -*-

// PDateTime
// Copyright (c) 2002 by Nicolas Chapados

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


/*! \file PDateTime.h */

#ifndef PDateTime_INC
#define PDateTime_INC

#include <iostream>
#include <string>

namespace PLearn {
using namespace std;

#define SECONDS_PER_DAY 86400 // 24*60*60

/*!  PDateTime must be a concrete class with no virtual function.  Hence it
  MUST NOT UNDER ANY CONSIDERATION UNDER HEAVY PRISON PENALTY derive from
  Object.  The reason that PDateTime is different from the related PDate
  (which stores only a date) is that some clients (e.g. SimpleDB and
  friends) make fairly stringent assumptions on the memory layout of a
  PDate, and leave no room for time.  Also, a PDateTime should really be
  considered a "time-point", and for added precision converts itself to a
  double. */

class PDateTime
{
public:
    short year; //!<  ex: 1983, 2001, -350, ...
    unsigned char month; //!<  1..12
    unsigned char day; //!<  1..31
    unsigned char hour; //!< 00..23
    unsigned char min;  //!< 00..59
    unsigned char sec;  //!< 00..61, allowing for leap seconds (!)

    //!  Create a missing date by default
    PDateTime();

    //!  Create a date only, with time assumed to be midnight
    PDateTime(short the_year, unsigned char the_month, unsigned char the_day)
        : year(the_year), month(the_month), day(the_day),
          hour(0), min(0), sec(0) {}

    PDateTime(int the_year, int the_month, int the_day)
        : year(the_year), month(int(the_month)), day(int(the_day)),
          hour(0), min(0), sec(0) {}

    //!  Create a full date/time
    PDateTime(short the_year, unsigned char the_month, unsigned char the_day,
              unsigned char the_hour, unsigned char the_min,
              unsigned char the_sec)
        : year(the_year), month(the_month), day(the_day),
          hour(the_hour), min(the_min), sec(the_sec) {}
  
    PDateTime(int the_year, int the_month, int the_day,
              int the_hour, int the_min, int the_sec)
        : year(the_year), month(the_month), day(the_day),
          hour(the_hour), min(the_min), sec(the_sec) {}
  
    //!  convert a Julian day into a PDateTime. Day 0 of the Julian calendar
    //!  is about 6000 years ago...  The julian day is passed as a DOUBLE, to
    //!  allow specifying hours/minutes/seconds as a FRACTION of days.
    PDateTime(double julian_day);

    //!  Convert a string in "YYYY/MM/DD" or "YYYY/MM/DD hh:mm:ss" into
    //!  a PDateTime.  The format has to match exactly.
    PDateTime(string datetime);
  
    //!  Missing date/time handling
    bool isMissing() const;
    void setMissing();

    //!<  returns the date in the format yyyy/mm/dd hh:mm:ss
    string info() const; 

    //!  Equality comparison
    bool operator==(const PDateTime& rhs) const {
        return year == rhs.year && month == rhs.month && day == rhs.day
            && hour == rhs.hour && min == rhs.min && sec == rhs.sec;
    }
  
    bool operator!=(const PDateTime& rhs) const {
        return ! (*this == rhs);
    }
  
    bool operator<(const PDateTime& rhs) const {
        double datetime_to_double(const PDateTime& t); // declare the function
        return datetime_to_double(*this) < datetime_to_double(rhs);
    }
  
    bool operator<=(const PDateTime& rhs) const {
        return *this == rhs || *this < rhs;
    }
  
    bool operator>(const PDateTime& rhs) const {
        return ! (*this <= rhs);
    }
  
    bool operator>=(const PDateTime& rhs) const {
        return ! (*this < rhs);
    }

    bool sameDay(const PDateTime& rhs) const {
        return day   == rhs.day && month == rhs.month && year  == rhs.year;
    }
    
/*!     Convert a PDateTime into a Julian day.
  See http://quasar.as.utexas.edu/BillInfo/JulianDatesG.html
  (if still existing...) for details.
*/
    double toJulianDay() const;

    //! Increment the current date by a given number of seconds.
    void incSecond  (int sec_inc);
    //! Increment the current date by a given number of minutes.
    void incMinute  (int min_inc);
    //! Increment the current date by a given number of hours.
    void incHour    (int hour_inc);
    //! Increment the current date by a given number of days.
    void incDay     (int day_inc);

    //! Return a PDateTime object corresponding to the current local date
    //! and time.
    static PDateTime currentLocalTime();
};

//!  subtract two dates, the result being counted in days (+ fractions)
inline double operator-(const PDateTime& to_date, const PDateTime& from_date)
{
    return to_date.toJulianDay() - from_date.toJulianDay();
}

inline ostream& operator<<(ostream& os, const PDateTime& date)
{
    os << date.info();
    return os;
}
  
//! converts date/time to double:
//! for example: September 29 1972: 720929; December 25 2002: 1021225.
//! Hours/minutes/seconds are represented as FRACTIONS of days.
//! Also converts missing date to missing double value and vice-versa.
double datetime_to_double(const PDateTime& t);
PDateTime double_to_datetime(double f);

//! converts an hours/minutes/seconds to a day fraction
double hhmmss_to_double(int hh, int mm, int ss);

//! convert a day fraction (< 1) to hours/minutes/seconds
void double_to_hhmmss(double fraction, int& hh, int& mm, int& ss);

//! Return the number of seconds between the two dates. Current date *can* be
//! lower than past one: the delta will be negative.
int delta_seconds(const PDateTime& current, const PDateTime& past);

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
