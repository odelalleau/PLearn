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

//#include <limits.h>                 // from stdc
#include "PDateTime.h"
#include <plearn/base/stringutils.h> 
#include <plearn/base/lexical_cast.h>  // for toint  
#include <plearn/base/tostring.h>  
#include "general.h"                // for MISSING_VALUE

namespace PLearn {
using namespace std;

PDateTime::PDateTime()
{
    setMissing();
}

PDateTime::PDateTime(double julian_day)
{
    int jw = (int)((julian_day - 1867216.25)/36524.25);
    int jx = (int)(jw/4);
    int ja = (int)(julian_day + 1 + jw - jx);
    int jb = ja + 1524;
    int jc = (int)((jb - 122.1)/365.25);
    int jd = (int)(365.25*jc);
    int je = (int)((jb - jd)/30.6001);
    int jf = (int)(30.6001*je);

    day = jb - jd - jf;
    month = (je>13) ? je-13 : je-1;
    year = (month>2) ? jc-4716 : jc-4715;

    double fraction = julian_day - floor(julian_day);
    int hh,mm,ss;
    double_to_hhmmss(fraction,hh,mm,ss);
    hour = hh;
    min = mm;
    sec = ss;
}

PDateTime::PDateTime(string date)
{
    // Parse either "YYYY/MM/DD" or "YYYY/MM/DD hh:mm:ss" format
    if ((date.size() == 10 || date.size() == 19) &&
        date[4] == '/' && date[7] == '/' &&
        (date.size() == 19? date[13] == ':' && date[16] == ':' : true)) {
        year = toint(date.substr(0,4));
        month = toint(date.substr(5,2));
        day = toint(date.substr(8,2));
        if (date.size() == 10)
            hour = min = sec = 0;
        else {
            hour = toint(date.substr(11,2));
            min  = toint(date.substr(14,2));
            sec  = toint(date.substr(17,2));
        }
    }
    else
        PLERROR("PDateTime::PDateTime: the passed date-time string is not in "
                "\"YYYY/MM/DD\" or \"YYYY/MM/DD hh:mm:ss\" format");
}

bool PDateTime::isMissing() const
{
    return year == SHRT_MIN && month == 0 && day == 0;
}

void PDateTime::setMissing()
{
    year = SHRT_MIN;
    month = 0;
    day = 0;
}  
  
//////////
// info //
//////////
string PDateTime::info() const
{
    return tostring(year)+ slash+
        right(tostring(int(month)), 2, '0') + slash+
        right(tostring(int(day)),   2, '0') + " "+
        right(tostring(int(hour)),  2, '0') + ":"+
        right(tostring(int(min)),   2, '0') + ":"+
        right(tostring(int(sec)),   2, '0');
}

//////////////////////
// currentLocalTime //
//////////////////////
PDateTime PDateTime::currentLocalTime() {
    time_t current;
    tm* ptr;
    time(&current);
    ptr = ::localtime(&current);
    return PDateTime(ptr->tm_year + 1900, ptr->tm_mon + 1, ptr->tm_mday,
                     ptr->tm_hour, ptr->tm_min, ptr->tm_sec);
}

/////////////////
// toJulianDay //
/////////////////
double PDateTime::toJulianDay() const
{
    if (year < 1582)
        PLERROR("toJulianDay works safely only for year > 1581 (%d)", year);

    double fraction = hhmmss_to_double(hour,min,sec);
  
    int jy = (month>2) ? year : year-1;
    int jm = (month>2) ? month : month+12;
    int ja = (int)(jy/100);
    int jb = (int)(ja/4);
    int jc = 2 - ja + jb;
    int je = (int)(365.25*(jy + 4716));
    int jf = (int)(30.6001*(jm + 1));

    return jc + day + je + jf - 1524 + fraction;
}

double datetime_to_double(const PDateTime& t)
{
    if (t.isMissing())
        return MISSING_VALUE;
    else
        return double((t.year-1900)*10000 + t.month*100 + t.day) +
            hhmmss_to_double(t.hour,t.min,t.sec);
}

PDateTime double_to_datetime(double f)
{
    PDateTime date;                     // missing by default
    if (! is_missing(f)) {
        long d = long(f);
        double fraction = f-d;
        date.year = short(1900 + d/10000);
        d %= 10000;
        date.month = (unsigned char) (d/100);
        date.day = (unsigned char) (d%100);

        int hh,mm,ss;
        double_to_hhmmss(fraction,hh,mm,ss);
        date.hour = hh;
        date.min  = mm;
        date.sec  = ss;
    }
    return date;
}

double hhmmss_to_double(int hh, int mm, int ss)
{
    // There are 1440 minutes in a day.
    // There are 86400 seconds in a day.
    return double(hh)/24. + double(mm)/1440. + double(ss)/86400;
}

void double_to_hhmmss(double fraction, int& hh, int& mm, int& ss)
{
    hh = int(fraction *= 24);
    fraction -= hh;
    mm = int(fraction *= 60);
    fraction -= mm;
    ss = int(fraction * 60);
}

int delta_seconds(const PDateTime& current, const PDateTime& past)
{
    if ( current == past )
        return 0;

    double jcurrent   = current.toJulianDay();
    double jpast      = past.toJulianDay();
    double delta_days = jcurrent - jpast;

    return int( round(delta_days*SECONDS_PER_DAY) ); 
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
