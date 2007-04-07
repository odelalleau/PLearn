// -*- C++ -*-

// Calendar.cc
//
// Copyright (C) 2004-2005 ApSTAT Technologies Inc.
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

// Authors: Jean-Sébastien Senécal

/*! \file Calendar.cc */


#include "Calendar.h"
#include <plearn/base/PDate.h>
#include <plearn/math/TMat_sort.h>
#include <set>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    Calendar,
    "Encapsulates the concept of a calendar as an ordered finite list of timestamps.",
    "This class encapsulates the concept of a calendar as an ordered "
    "finite list of timestamps. The idea is to provide a tool to "
    "convert a continuous representation of time (as julian dates) "
    "into a discrete one. Not only the calendar time units (CTime) are "
    "different, but the time axis may \"leap-over\" time ranges in "
    "the continous time (JTime) axis.\n"
    "\n"
    "For instance, one may want to represent a conception of time "
    "where time is sampled daily but only during business week days "
    "(i.e. monday to friday) s.t. when time t is a friday, time (t+1) "
    "will be the next monday (and not saturday). Such a calendar would "
    "contain, in its internal representation, a list of timestamps that "
    "correspond to, say, Midnight of every Mon/Tues/Wednes/Thurs/Fri-days "
    "from, say, 1900 to 2099.\n"
    "\n"
    "In addition, the class supports a set of global (static) calendars keyed\n"
    "by a string.  Functions are provided to set/get global calendars\n"
    "associated with string keys.  A Remote-Method interface is provided as\n"
    "well to set global calendars.  Special operators are available in\n"
    "VMatLanguage to access those global calendars.\n"
    );


// Set of global calendars
map<string,PP<Calendar> > Calendar::global_calendars;

  
Calendar::Calendar()
    : inherited(),
      last_julian_time_(MISSING_VALUE),
      last_calendar_time_(-1),
      last_use_lower_bound(false),
      timestamps_()
{ }

Calendar::Calendar(const JTimeVec& timestamps)
    : inherited(),
      last_julian_time_(MISSING_VALUE),
      last_calendar_time_(-1),
      last_use_lower_bound(false),
      timestamps_(timestamps)
{
    build_();
}


PCalendar Calendar::makeCalendar(const Vec& dates)
{
    static int Jan1_1900_JDN  = PDate(1900,01,01).toJulianDay();
    static int Dec31_2199_JDN = PDate(2199,12,31).toJulianDay();
    TVec<JTime> jdates(dates.length());
  
    // Start by converting the dates to Julian
    for (int i=0, n=dates.size() ; i<n ; ++i) {
        real date_i = dates[i];

        // YYYYMMDD
        if (date_i >= 19000101 && date_i <= 21991231) {
            int year  = int(date_i / 10000);
            int month = (int(date_i) % 10000) / 100;
            int day   = int(date_i) % 100;
            jdates[i] = PDate(year,month,day).toJulianDay();
        }

        // CYYMMDD (up to 2099/12/31, since 1900/01/01 == JDN-2415021)
        else if (date_i >= 000101 && date_i <= 1991231) {
            jdates[i] = float_to_date(date_i).toJulianDay();
        }

        // Otherwise check it's in the proper range for a JDN
        else if (date_i < Jan1_1900_JDN ||
                 date_i > Dec31_2199_JDN)
            PLERROR("The date value '%g' at index %d could not be interpreted\n"
                    "as a date in YYYYMMDD, CYYMMDD or JDN in the 1900/01/01 to\n"
                    "2199/12/31 range.", date_i, i);
        else
            jdates[i] = JTime(date_i);
    }

    // Next sort by increasing value...
    sortElements(jdates);

    return new Calendar(jdates);
}

void Calendar::build_()
{
    // If we have dwim_timestamps_ and no timestamps_, fill latter from former
    if (dwim_timestamps_.size() > 0 && timestamps_.size() == 0) {
        PCalendar cal = makeCalendar(dwim_timestamps_);
        timestamps_ = cal->timestamps_;
    }

    // perform some preventive checks
    bool doublons = false;
    bool disordered = false;
    for (int i=1; i<timestamps_.length(); i++)
    {
        if (fast_exact_is_equal(timestamps_[i], timestamps_[i-1]))
        {
            doublons = true;
            if (disordered) break;
        }
        else if (timestamps_[i] < timestamps_[i-1])
        {
            disordered = true;
            if (doublons) break;
        }
    }
    if (doublons)
        PLWARNING("In Calendar::build_(), provided timestamps contained repetitions.\n"
                  "The calendar's behavior is therefore undefined. Please check it.");
    if (disordered)
        PLWARNING("In Calendar::build_(), provided timestamps were disordered.\n"
                  "The calendar's behavior is therefore undefined. Please check it.");
}

void Calendar::declareOptions(OptionList& ol)
{
    declareOption(
        ol, "timestamps", &Calendar::timestamps_,
        OptionBase::buildoption,
        "List of the time stamps (in julian time) that are the\n"
        "skeleton of this calendar.");

    declareOption(
        ol, "dwim_timestamps", &Calendar::dwim_timestamps_,
        OptionBase::buildoption,
        "Alternative list of \"human timestamps\" that may be provided in any of\n"
        "the following formats: Julian, CYYMMDD, YYYYMMDD.  The format is\n"
        "recognized automatically.  The dates need not be sorted; they will be\n"
        "sorted automatically.  After construction of the calendar, the option\n"
        "'timestamps' is filled out with the converted Julian timesteps.\n");
        
    inherited::declareOptions(ol);
}

void Calendar::declareMethods(RemoteMethodMap& rmm)
{
    // Insert a backpointer to remote methods; note that this
    // different than for declareOptions()
    rmm.inherited(inherited::_getRemoteMethodMap_());

    declareMethod(
        rmm, "setGlobalCalendar", &Calendar::remote_setGlobalCalendar,
        (BodyDoc("Set a global calendar keyed to the given string.\n"
                 "The calendar is created if it does not already exist."),
         ArgDoc ("calendar_name",  "Name of the global calendar to be set"),
         ArgDoc ("calendar_dates", "List of dates that belong to the calendar,\n"
                                   "either in CYYMMDD or YYYYMMDD format")));

    declareMethod(
        rmm, "getGlobalCalendar", &Calendar::remote_getGlobalCalendar,
        (BodyDoc("Return the dates that belong to the global calendar,\n"
                 "given its name"),
         ArgDoc ("calendar_name", "Name of the global calendar to be accessed"),
         RetDoc ("Set of dates that belong to the calendar, in PLearn\n"
                 "Julian Date format")));
}

void Calendar::build()
{
    inherited::build();
    build_();
}

void Calendar::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(timestamps_, copies);
}

CTime Calendar::getCalendarTime(JTime julian_time, bool use_lower_bound) const
{
    if (timestamps_.isEmpty())
        PLERROR("In Calendar::getCalendarTime(), this calendar is empty.");
    if (last_calendar_time_ == -1 ||
        !fast_exact_is_equal(last_julian_time_, julian_time) ||
        last_use_lower_bound != use_lower_bound) // *** that one could be made more efficient
    {
        JTimeVec::iterator result;
        if (use_lower_bound)
            result = lower_bound(timestamps_.begin(), timestamps_.end(), julian_time);
        else
            result = upper_bound(timestamps_.begin(), timestamps_.end(), julian_time);
        if (result == timestamps_.end()) // don't go after
            --result;
        last_calendar_time_ = (CTime) (result - timestamps_.begin());
        last_julian_time_ = julian_time;
        last_use_lower_bound = use_lower_bound;
    }
  
    return last_calendar_time_;
}

CTime Calendar::getLastDayOfMonth(const PDate& day_of_the_month) const
{
    PDate last_day_of_month = day_of_the_month.lastDateOfMonth();
    PLASSERT(day_of_the_month.month==last_day_of_month.month);

    JTime jtime_ldom = last_day_of_month.toJulianDay();
    CTime ctime_ldom = getCalendarTime(jtime_ldom);

    // Was first day of the following month returned?
    if ( ctime_ldom > 0 && timestamps_[ctime_ldom] > jtime_ldom )
    {
        ctime_ldom--;
        PLASSERT( timestamps_[ctime_ldom] <= jtime_ldom );
    }

    return ctime_ldom;
}


CTime Calendar::convertCalendarTime(const Calendar& source_calendar,
                                    const Calendar& dest_calendar,
                                    CTime source_time,
                                    bool use_lower_bound)
{
    if (source_calendar.isEmpty() || dest_calendar.isEmpty())
        PLERROR("In convertCalendarTime(), one of the calendars is empty.");
    return dest_calendar.getCalendarTime(source_calendar.getTime(source_time), use_lower_bound);
}


bool Calendar::containsTime(JTime julian_time, CTime *calendar_time) const
{
    CTime found = getCalendarTime(julian_time);
    if (fast_exact_is_equal(timestamps_[found], julian_time))
    {
        if (calendar_time)
            *calendar_time = found;
        return true;
    }
    else
        return false;
}


JTime Calendar::calendarTimeOnOrAfter(JTime julian_time) const
{
    CTime start_point = getCalendarTime(julian_time);
    if (getTime(start_point) < julian_time)    // if at very end of calendar
        return MAX_TIME;
    else
        return getTime(start_point);
}


JTime Calendar::calendarTimeOnOrBefore(JTime julian_time) const
{
    CTime start_point = getCalendarTime(julian_time);
    while (start_point >= 0 && getTime(start_point) > julian_time)
        start_point--;
    if (start_point < 0)
        return MIN_TIME;
    else
        return getTime(start_point);
}


PCalendar Calendar::unite(const TVec<PCalendar>& raw_calendars)
{
    PCalendar united = new Calendar;

    // In the case of shared calendars, the same calendar may be repeated
    // multiple times in raw_calendars.  Pick out unique copies
    set<PCalendar> calendars(raw_calendars.begin(), raw_calendars.end());
  
    if (!calendars.empty())
    {
        set<JTime> timestamps;
        for (set<PCalendar>::iterator it = calendars.begin(); it != calendars.end(); ++it)
        {
            set_union(timestamps.begin(), timestamps.end(),
                      (*it)->timestamps_.begin(),  (*it)->timestamps_.end(),
                      inserter(timestamps, timestamps.begin()));
        }
        united->timestamps_.resize(int(timestamps.size()));
        copy(timestamps.begin(), timestamps.end(), united->timestamps_.begin());
    }

    united->build();
  
    return united;
}


PCalendar Calendar::intersect(const TVec<PCalendar>& raw_calendars)
{
    PCalendar intersected = new Calendar;
  
    // In the case of shared calendars, the same calendar may be repeated
    // multiple times in raw_calendars.  Pick out unique copies
    set<PCalendar> calendars(raw_calendars.begin(), raw_calendars.end());
  
    if ( !calendars.empty() )
    {
        set<PCalendar>::iterator it  = calendars.begin();
        set<PCalendar>::iterator end = calendars.end(); 
    
        if ( it != end ) 
        {
            JTimeVec intersection = (*it)->timestamps_.copy();
            JTimeVec::iterator curend = intersection.end();
            ++it;
      
            for (; it != end; ++it)
            {        
                curend = set_intersection(intersection.begin(),
                                          curend,
                                          (*it)->timestamps_.begin(), (*it)->timestamps_.end(),
                                          intersection.begin());
            }
            intersected->timestamps_ =
                intersection.subVec(0, int(curend-intersection.begin())).copy();
        }
    }

    intersected->build();  
    return intersected;
}


PCalendar Calendar::calendarDiff(const Calendar* cal, const JTimeVec& to_remove)
{
    set<JTime> cal_times(cal->timestamps_.begin(), cal->timestamps_.end());
    for (int i=0, n=to_remove.size() ; i<n ; ++i)
        cal_times.erase(to_remove[i]);

    JTimeVec new_timestamps(int(cal_times.size()));
    copy(cal_times.begin(), cal_times.end(), new_timestamps.begin());
    return new Calendar(new_timestamps);
}


PCalendar Calendar::clamp(JTime lower, JTime upper)
{
    // Handle degenerate cases
    if (size() == 0 || upper < (*this)[0] || lower > (*this)[size()-1])
        return new Calendar;
    
    CTime lower_ctime = getCalendarTime(lower, /* use_lower_bound= */ true);
    CTime upper_ctime = getCalendarTime(upper, /* use_lower_bound= */ false);
    if ((*this)[upper_ctime] > upper)
        --upper_ctime;
    PLASSERT( lower_ctime <= upper_ctime );
    PLASSERT( (*this)[lower_ctime] >= lower );
    PLASSERT( (*this)[upper_ctime] <= upper );

    JTimeVec new_timestamps = timestamps_.subVec(lower_ctime,
                                                 upper_ctime - lower_ctime + 1);
    return new Calendar(new_timestamps);
}


void Calendar::setGlobalCalendar(const string& calendar_name,
                                 PCalendar calendar)
{
    if (calendar_name != "")
        global_calendars[calendar_name] = calendar;
    else
        PLERROR("Calendar::setGlobalCalendar: the calendar name must be specified");
}


const Calendar* Calendar::getGlobalCalendar(const string& calendar_name)
{
    map<string,PCalendar>::const_iterator it =
        global_calendars.find(calendar_name);
    if (it != global_calendars.end())
        return (const Calendar*)(it->second);
    else
        return 0;
}


void Calendar::remote_setGlobalCalendar(string calendar_name, Vec calendar_dates)
{
    setGlobalCalendar(calendar_name, makeCalendar(calendar_dates));
}


JTimeVec Calendar::remote_getGlobalCalendar(string calendar_name)
{
    const Calendar* cal = getGlobalCalendar(calendar_name);
    if (! cal) {
        PLERROR("Global calendar '%s' not found", calendar_name.c_str());
        return JTimeVec();                        //!< Shut up compiler
    }
    else
        return cal->timestamps_;
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
