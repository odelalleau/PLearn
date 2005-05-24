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
   * $Id: Calendar.cc,v 1.1 2005/05/24 18:37:46 chapados Exp $ 
   ******************************************************* */

// Authors: Jean-Sébastien Senécal

/*! \file Calendar.cc */


#include "Calendar.h"
#include <plearn/math/TMat_sort.h>
#include <set>

namespace PLearn {
using namespace std;

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
  "from, say, 1900 to 2099."
  );

void Calendar::build_()
{
  // perform some preventive checks
  bool doublons = false;
  bool disordered = false;
  for (int i=1; i<timestamps_.length(); i++)
  {
    if (timestamps_[i] == timestamps_[i-1])
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
    PLWARNING("In Calendar::build_(), provided timestamps contained repetitions. The calendar's behavior is therefore undefined. Please check it.");
  if (disordered)
    PLWARNING("In Calendar::build_(), provided timestamps were disordered.  The calendar's behavior is therefore undefined. Please check it.");
}

void Calendar::declareOptions(OptionList& ol)
{
  declareOption(ol, "timestamps", &Calendar::timestamps_, OptionBase::buildoption,
                "List of the time stamps (in julian time) that are the skeleton of this calendar.");
  inherited::declareOptions(ol);
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
      last_julian_time_ != julian_time ||
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
  if (timestamps_[found] == julian_time)
  {
    if (calendar_time)
      *calendar_time = found;
    return true;
  }
  else
    return false;
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
    united->timestamps_.resize(timestamps.size());
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
        intersection.subVec(0,curend-intersection.begin()).copy();
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

  JTimeVec new_timestamps(cal_times.size());
  copy(cal_times.begin(), cal_times.end(), new_timestamps.begin());
  return new Calendar(new_timestamps);
}

PCalendar Calendar::clamp(JTime lower, JTime upper)
{
  PCalendar clamped = new Calendar;

  CTime t = 0;
  for (; t < size() && getTime(t) < lower ; ++t);
  for (; t < size() && getTime(t) <= upper ; ++t)
    clamped->timestamps_.append(getTime(t));

  return clamped;
}

}
