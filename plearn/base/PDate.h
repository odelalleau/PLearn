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
   * $Id: PDate.h,v 1.1 2002/07/30 09:01:26 plearner Exp $
   * This file is part of the PLearn library.
   ******************************************************* */


/*! \file PLearnLibrary/PLearnCore/PDate.h */

#ifndef PDate_INC
#define PDate_INC

#include <iostream>
#include <string>
#include <ctime>

namespace PLearn <%
using namespace std;

/*!   PDate must be a concrete class with no virtual function.  Hence it
  MUST NOT UNDER ANY CONSIDERATION UNDER HEAVY PRISON PENALTY
  derive from Object.
*/

class PDate
{
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

  //!  Missing date handling
  bool isMissing() const;
  void setMissing();  
    
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

inline ostream& operator<<(ostream& os, const PDate& date)
{
  os << date.info();
  return os;
}
  
//! converts date to float: ex: September 29 1972: 720929; December 25 2002: 1021225
//! Also converts missing date to missing flat value and vice-versa.
float date_to_float(const PDate& t);
PDate float_to_date(float f);

%> // end of namespace PLearn

#endif
