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

//#include <limits.h>                 // from stdc
#include "PDate.h"
#include <algorithm>                //!< For std::transform.
#include "stringutils.h"            //!< For toint.
#include <plearn/base/tostring.h>  
//#include "general.h"                // for MISSING_VALUE
#include <plearn/math/pl_math.h>                //!< For MISSING_VALUE.
//#include <ctype.h>

namespace PLearn {
using namespace std;



//#####  Static Methods  ######################################################

PDate PDate::lastDateOfMonth(int year, int month)
{
    PDate date = PDate(year, month, int(lastDayOfMonth(year, month)));
    PLASSERT( date.isValid() );
    return date;
}

unsigned char PDate::lastDayOfMonth(int year, int month)
{
    unsigned char day = 28;
    switch ( month ) {
    case 1: case 3: case 5: case 7: case 8: case 10: case 12:
        day = 31;
        break;
        
    case 4: case 6: case 9: case 11:
        day = 30;
        break;

    case 2:
        day = 28;
        if ( year %  4  == 0 ) day = 29;
        if ( year % 100 == 0 ) day = 28;
        if ( year % 400 == 0 ) day = 29;
        break;

    default:
        PLERROR("%s: Invalid month argument %d", __FUNCTION__, month);
    }

    return day;
}


//#####  Instance Methods  ####################################################

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
    PLASSERT( isValid() );
}

PDate::PDate(string date,bool invalid_value_as_missing)
{
    date = removeblanks(date);
    search_replace(date, "-", "");      // remove dashes

    // Format "2003/01/27"
    if (date.size() == 10 && date[4] == '/' && date[7] == '/') 
    {
        year = toint(date.substr(0,4));
        month = toint(date.substr(5,2));
        day = toint(date.substr(8,2));
    }
    // Format "27JAN2003" or "27-jan-2003" or "27-jan-03"
    else if((date.size()==9 || date.size()==7)
            && isalpha(date[2]) && isalpha(date[3]) && isalpha(date[4]))
    {
        // Convert to YYYY; assume cutoff point of 39 for years in 2000
        if (date.size() == 7) {
            int yy = toint(date.substr(5));
            int yyyy = -1;
            if (yy <= 40)
                yyyy = 2000 + yy;
            else
                yyyy = 1900 + yy;
            date = date.substr(0,5) + tostring(yyyy);
        }
      
        year = toint(date.substr(5,4));
        day = toint(date.substr(0,2));
        string mo = date.substr(2,3);
        // Make uppercase
        std::transform(mo.begin(), mo.end(), mo.begin(), (int(*)(int)) toupper);

        if(mo=="JAN")
            month = 1;
        else if(mo=="FEB")
            month = 2;
        else if(mo=="MAR")
            month = 3;
        else if(mo=="APR")
            month = 4;
        else if(mo=="MAY")
            month = 5;
        else if(mo=="JUN")
            month = 6;
        else if(mo=="JUL")
            month = 7;
        else if(mo=="AUG")
            month = 8;
        else if(mo=="SEP")
            month = 9;
        else if(mo=="OCT")
            month = 10;
        else if(mo=="NOV")
            month = 11;
        else if(mo=="DEC")
            month = 12;
        else if(invalid_value_as_missing)
            setMissing();
        else
            PLERROR("Invalid month string: '%s'",mo.c_str());
    }

    // Format "20020917"
    else if (date.size() == 8 && pl_isnumber(date))
    {
        year = toint(date.substr(0,4));
        month = toint(date.substr(4,2));
        day = toint(date.substr(6,2));
    }

    // Format 1020917  (102 stands for 2002)
    else if (date.size()==7 && pl_isnumber(date))
    {
        year = 1900+toint(date.substr(0,3));
        month = toint(date.substr(3,2));
        day = toint(date.substr(5,2));
    }
    // Format 980917  (99 stands for 1998)
    else if (date.size()==6 && pl_isnumber(date))
    {
        year = 1900+toint(date.substr(0,2));
        month = toint(date.substr(2,2));
        day = toint(date.substr(4,2));
    }
    else if(invalid_value_as_missing)
        setMissing();
    else
        PLERROR("PDate::PDate: the passed date string is not in a known format: %s", date.c_str());

    if( !isValid() )
        if(invalid_value_as_missing){
            setMissing();
            PLWARNING("Invalid date string: %s",date.c_str());
        }else
            PLERROR("Invalid date string: %s",date.c_str());
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

bool PDate::isValid() const
{
    // Managing leap years
    if (month==2 && day==29)
    {
        bool valid = false;
        if ( year %  4  == 0 ) valid = true;
        if ( year % 100 == 0 ) valid = false;
        if ( year % 400 == 0 ) valid = true;
        return valid;
    }

    return year  >= 1582  &&  year  <= 3000 &&
           month >=    1  &&  month <=   12 &&
           day   >=    1  &&  day   <= lastDayOfMonth();
}

string PDate::info() const
{
    return tostring(year)+slash+
        right(tostring(int(month)), 2, '0') +slash+
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

int PDate::dayOfYear() const
{
    return (*this)-PDate(year,(unsigned char)1,(unsigned char)1);
}


int PDate::weekNumber() const
{
    // Hack for now, not yet the true iso week number
    // See: http://personal.ecu.edu/mccartyr/ISOwdALG.txt

    return dayOfYear()/7;
}


PStream& operator<<(PStream& out, const PDate& date)
{
    char tmpbuf[50];

    switch(out.outmode)
    {
    case PStream::plearn_binary:
    case PStream::raw_binary:
        out.put((char)0xFE);
        out << date_to_double(date);
        break;
    case PStream::plearn_ascii:        
        sprintf(tmpbuf,"%04d/%02d/%02d ",date.year,date.month,date.day);
        out.write(tmpbuf);
        break;

    case PStream::raw_ascii:
    case PStream::pretty_ascii:
        // same as plearn_ascii but with no ending space
        sprintf(tmpbuf,"%04d/%02d/%02d",date.year,date.month,date.day);
        out.write(tmpbuf);
        break;
    }

    return out;
}

PStream& operator>>(PStream& in, PDate& date)
{
    in.skipBlanksAndComments();
    int c = in.peek();
    if(c==0xFE) // binary PDate
    {
        double yyyymmdd;
        in >> yyyymmdd;
        date = double_to_date(yyyymmdd);
    }
    else if(isdigit(c))
    {
        in.readAsciiNum(date.year);
        in.readExpected('/');
        in.readAsciiNum(date.month);
        in.readExpected('/');
        in.readAsciiNum(date.day);
    }
    else
        PLERROR("Not a valid serialized PDate");
    return in;
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
        date.year = short(1900 + d/10000);
        d %= 10000;
        date.month = (unsigned char) (d/100);
        date.day = (unsigned char) (d%100);
    }
    return date;
}

double date_to_double(const PDate& t)
{
    if (t.isMissing())
        return MISSING_VALUE;
    else
        return double((t.year)*10000 + t.month*100 + t.day);  
}

PDate double_to_date(double d)
{
    PDate date;                     // missing by default
    if (! is_missing(d)) {
        long l = long(d);
        date.year = short(l/10000);
        l %= 10000;
        date.month = (unsigned char) (l/100);
        date.day = (unsigned char) (l%100);
    }
    return date;
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
