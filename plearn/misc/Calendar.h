// -*- C++ -*-

// Calendar.h
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

/*! \file Calendar.h */


#ifndef Calendar_H
#define Calendar_H

// C++ stdlib
#include <map>
#include <limits.h>

#include <plearn/base/Object.h>
#include "PRange.h"

namespace PLearn {
using namespace std;

class Calendar;
typedef PP<Calendar> PCalendar;

//! Julian time.
typedef double JTime;

//! Calendar time.
typedef int    CTime;

//! Vector of Julian times.
typedef TVec<JTime> JTimeVec;

//! Vector of calendar times.
typedef TVec<CTime> CTimeVec;

//! Maximum time.
const JTime MAX_TIME   = DBL_MAX;

//! Minimum time.
const JTime MIN_TIME   = -DBL_MAX;

//! Smallest valid increment (= 1/1000 second).
const JTime SMALL_TIME = 1./(24*60*60*1000);

//! Smallest representable increment.
const JTime EPS_TIME   = 1e-9; // Epsilon is found as follows:
                               // - reserve 20 bits for the day
                               // - 52-20=32 bits for intra-day:
                               //   ==> 1/2^32 ~ 2.33e-10 < 1e-9;

//! Maximum calendar time.
const CTime MAX_CTIME   = INT_MAX;

//! Minimum calendar time.
const CTime MIN_CTIME   = INT_MIN;

//! Range of calendar times.
typedef PRange<CTime> CTimeRange;


/**
 *  Encapsulates the concept of a calendar as an ordered finite list of timestamps.
 *
 *  This class encapsulates the concept of a calendar as an ordered
 *  finite list of timestamps. The idea is to provide a tool to
 *  convert a continuous representation of time (as julian dates)
 *  into a discrete one. Not only the calendar time units (CTime) are
 *  different, but the time axis may "leap-over" time ranges in
 *  the continous time (JTime) axis.
 *   
 *  For instance, one may want to represent a conception of time
 *  where time is sampled daily but only during business week days
 *  (i.e. monday to friday) s.t. when time t is a friday, time (t+1)
 *  will be the next monday (and not saturday). Such a calendar would
 *  contain, in its internal representation, a list of timestamps that
 *  correspond to, say, Midnight of every Mon/Tues/Wednes/Thurs/Fri-days
 *  from, say, 1900 to 2099.
 * 
 *  In addition, the class supports a set of global (static) calendars keyed
 *  by a string.  Functions are provided to set/get global calendars
 *  associated with string keys.  A Remote-Method interface is provided as
 *  well to set global calendars.  Special operators are available in
 *  VMatLanguage to access those global calendars.
 */
class Calendar : public Object
{
private:
    typedef Object inherited;

protected:
    // Internal use.

    //! Input value to last call to getCalendarTime
    mutable JTime last_julian_time_;

    //! Value returned by last call to getCalendarTime
    mutable CTime last_calendar_time_;

    //! Last argument used when calling getCalendarTime
    mutable bool last_use_lower_bound;

    //! Internal map containing memoized resamplings for the calendar.
    //! The interpretation is as follows: for each calendar c in the map,
    //! we have a vector of integers v_c.  The entry v_c[i] indicates what
    //! would be the index in calendar c of the index i in this calendar.
    map< Calendar*, TVec<int> > active_resamplings;

    //! The set of currently-installed global calendars
    static map<string, PP<Calendar> > global_calendars;
  
public:
    //! The list of julian timestamps that define this calendar.
    JTimeVec timestamps_;

    /**
     *  Alternative list of "human timestamps" that may be provided in any of
     *  the following formats: Julian, CYYMMDD, YYYYMMDD.  The format is
     *  recognized automatically.  The dates need not be sorted; they will be
     *  sorted automatically.  After construction of the calendar, the option
     *  'timestamps' is filled out with the converted Julian timesteps.
     */
    Vec dwim_timestamps_;

public:
    //! Default constructor.
    Calendar();
  
    //! Constructor with specified timestamps (julian dates)
    Calendar(const JTimeVec& timestamps);

    /**
     *  This returns a calendar from a vector of "dates".  The following are
     *  supported: YYYYMMDD, CYYMMDD, julian dates.  The format is recognized
     *  automatically.  The dates need not be sorted; they will be sorted
     *  automatically.
     *
     *  The storage for the dates vector is copied, thus 'dates' is not altered
     *  by this function, and can be safely modified afterwards.
     */
    static PCalendar makeCalendar(const Vec& dates);

private:
    //! This does the actual building. 
    void build_();

protected:
    //! Declares this class' options.
    static void declareOptions(OptionList& ol);
  
    //! Declare the methods that are remote-callable
    static void declareMethods(RemoteMethodMap& rmm);

public:
    PLEARN_DECLARE_OBJECT(Calendar);

    //! Simply calls inherited::build() then build_().
    virtual void build();

    //! Transforms a shallow copy into a deep copy.
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);


    //#####  Calendar-Specific Functions  #####################################
  
    //! Returns true iff this calendar contains no timestamps.
    bool isEmpty() const { return timestamps_.isEmpty(); }

    //! Returns a reference to the timestamps.
    const JTimeVec& getTimeStamps() const { return timestamps_; }
  
    //! Returns true iff the calendar have the exact same timestamps.
    bool operator==(const Calendar& cal)
    { return (timestamps_ == cal.getTimeStamps()); }

    //! Returns true iff the calendar has different timestamps.
    bool operator!=(const Calendar& cal)
    { return (timestamps_ != cal.getTimeStamps()); }
  
    //! Returns the number of timestamps.
    int size() const { return timestamps_.length(); }

    //! Returns timestamp corresponding to "calendar_time".
    inline JTime getTime(CTime calendar_time) const;
    inline JTime operator[](CTime calendar_time) const;

    /**
     *  Returns the calendar time corresponding to julian time "julian_time".
     *  If not found, returns the index of the first timestamp that is greater
     *  or equal to "julian_time", or the last index if "julian_time" is bigger
     *  than all of the timestamps. Returns an error if the calendar contains
     *  no timestamps.
     */
    CTime getCalendarTime(JTime julian_time, bool use_lower_bound = true) const;

    /**
     *  Returns true iff julian_time is a valid timestamp. If specified,
     *  argument calendar_time will be filled with the right value if true and
     *  will be left unchanged if not.
     */
    bool containsTime(JTime julian_time, CTime *calendar_time = 0) const;

    //! Return the JTime of the day in the calendar that comes ON OR AFTER
    //! the specified day, or MAX_TIME if no such day exist.
    JTime calendarTimeOnOrAfter(JTime julian_time) const;

    //! Return the JTime of the day in the calendar that comes ON OR BEFORE
    //! the specified day, or MIN_TIME if no such day exist.
    JTime calendarTimeOnOrBefore(JTime julian_time) const;  
  
    //! Return the subset of dates of this calendar that that are between
    //! the given lower and upper times (both endpoints are included in the
    //! the subset).
    PCalendar clamp(JTime lower, JTime upper);


    //#####  Resamplings  ######################################################

    //! Associate or modify a resampling to another calendar
    void setResampling(Calendar* other_cal, const TVec<int>& resampling)
    { active_resamplings[other_cal] = resampling; }

    //! Return the resampling to another calendar or an empty vector if
    //! it does not exist
    TVec<int> getResampling(Calendar* other_cal)
    { return active_resamplings[other_cal]; }

  
    //#####  Static Calendar Conversion  ######################################

    //! Converts a calendar
    static CTime convertCalendarTime(const Calendar& source_calendar,
                                     const Calendar& dest_calendar,
                                     CTime source_time,
                                     bool use_lower_bound = true);

    //! Return the union of a set of calendars
    static PCalendar unite(const TVec<PCalendar>& calendars);

    //! Return the intersection of a set of calendars
    static PCalendar intersect(const TVec<PCalendar>& calendars);

    //! Return a new calendar containing the dates of a starting calendar
    //! MINUS the dates given in the list
    static PCalendar calendarDiff(const Calendar* cal, const JTimeVec& to_remove);


    //#####  Global Static Calendars  ##########################################

    //! Set a global calendar keyed to the given string
    static void setGlobalCalendar(const string& calendar_name,
                                  PCalendar calendar);

    //! Return a pointer to the global calendar given the string.
    //! Return a NULL pointer if the calendar does not exist
    static const Calendar* getGlobalCalendar(const string& calendar_name);


    //#####  Remote-Callables  ################################################
    
    void remote_setGlobalCalendar(string calendar_name, Vec calendar_dates);
    Vec remote_getGlobalCalendar(string calendar_name);
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(Calendar);

JTime Calendar::getTime(CTime calendar_time) const
{
    if (calendar_time < 0 || calendar_time >= timestamps_.length())
        PLERROR("In Calendar::getTime(), argument calendar_time = %d out of bounds.", calendar_time);
    return timestamps_[calendar_time];
}

JTime Calendar::operator[](CTime calendar_time) const
{
    return getTime(calendar_time);
}

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
