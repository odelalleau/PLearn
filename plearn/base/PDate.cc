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
   * $Id: PDate.cc,v 1.1 2002/07/30 09:01:26 plearner Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include <limits.h>                 // from stdc
#include "stringutils.h"            // from PLearn
#include "PDate.h"
#include "general.h"                // for MISSING_VALUE

namespace PLearn <%
using namespace std;

PDate::PDate()
{
  setMissing();
}

PDate::PDate(int julian_day)
{
  int jw = (int)((julian_day - 1867216.25)/36524.25);
  int jx = (int)(jw/4);
  int ja = julian_day + 1 + jw - jx;
  int jb = ja + 1524;
  int jc = (int)((jb - 122.1)/365.25);
  int jd = (int)(365.25*jc);
  int je = (int)((jb - jd)/30.6001);
  int jf = (int)(30.6001*je);
 
  day = jb - jd - jf;
  month = (je>13) ? je-13 : je-1;
  year = (month>2) ? jc-4716 : jc-4715;
}

bool PDate::isMissing() const
{
  return year == SHRT_MIN && month == 0 && day == 0;
}

void PDate::setMissing()
{
  year = SHRT_MIN;
  month = 0;
  day = 0;
}  
  
string PDate::info() const
{
  return tostring(year)+"/"+
    right(tostring(int(month)), 2, '0') +"/"+
    right(tostring(int(day)),   2, '0');
}

int PDate::toJulianDay() const
{
  if (year < 1582)
    PLERROR("toJulianDay works safely only for year > 1581 (%d)", year);
  int jy = (month>2) ? year : year-1;
  int jm = (month>2) ? month : month+12;
  int ja = (int)(jy/100);
  int jb = (int)(ja/4);
  int jc = 2 - ja + jb;
  int je = (int)(365.25*(jy + 4716));
  int jf = (int)(30.6001*(jm + 1));

  return jc + day + je + jf - 1524;
}

float date_to_float(const PDate& t)
{
  if (t.isMissing())
    return MISSING_VALUE;
  else
    return float((t.year-1900)*10000 + t.month*100 + t.day);
}

PDate float_to_date(float f)
{
  PDate date;                     // missing by default
  if (! is_missing(f)) {
    long d = long(f);
    date.year = 1900 + d/10000;
    d %= 10000;
    date.month = d/100;
    date.day = d%100;
  }
  return date;
}

%> // end of namespace PLearn
