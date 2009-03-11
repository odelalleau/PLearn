// -*- C++ -*-

// PTimer.cc
//
// Copyright (C) 2006 Olivier Delalleau 
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

// Authors: Olivier Delalleau

/*! \file PTimer.cc */


#include "PTimer.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    PTimer,
    "Allows measurement of time elapsed between two events.",
    "A single PTimer object can contain many timers, each being identified\n"
    "by a unique name.\n"
    "The time is measured in seconds. For short durations (less than 30\n"
    "minutes), it is measured precisely, while for longer durations it is\n"
    "approximated to an integer number.\n"
    "\n"
    "Note that for advanced profiling, one should probably use the\n"
    "Profiler class instead.\n"
    "For short duration and when the option use_times_fct is false\n"
    "we will use the clock() fct that take the sum of all chield threads\n"
    "Otherwise we use the times(0) fct that report the wall time.\n"
    );

////////////
// PTimer //
////////////
PTimer::PTimer()
    :use_times_fct(false)
{}

PTimer::PTimer(bool use_times_fct_)
    :use_times_fct(use_times_fct_)
{}

///////////
// build //
///////////
void PTimer::build()
{
    inherited::build();
    build_();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void PTimer::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(start_clock_t, copies);
    deepCopyField(start_long,    copies);
    deepCopyField(total_times,   copies);
}

////////////////////
// declareOptions //
////////////////////
void PTimer::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify  
    // ### one of OptionBase::buildoption, OptionBase::learntoption or 
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    declareOption(ol, "name_to_idx", &PTimer::name_to_idx,
                                     OptionBase::learntoption,
        "Map a timer's name to its internal integer index.");

    declareOption(ol, "total_times", &PTimer::total_times,
                                     OptionBase::learntoption,
        "Contains the current total times of all timers.");

    declareOption(ol, "use_times_fct", &PTimer::use_times_fct,
                  OptionBase::buildoption,
                  "If true, we will use the times(0) fct. So we will report the"
                  " wall time. Usefull in multithread case.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void PTimer::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation. 
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
    // ### You should assume that the parent class' build_() has already been called.
    int n_timers = total_times.length();
    start_long.resize(n_timers);
    start_clock_t.resize(n_timers);
}

//////////////
// getTimer //
//////////////
real PTimer::getTimer(const string& timer_name)
{
    PLASSERT( name_to_idx.find(timer_name) != name_to_idx.end() );
    int timer_id = name_to_idx[timer_name];
    return total_times[timer_id];
}

//////////////
// newTimer //
//////////////
void PTimer::newTimer(const string& timer_name, bool can_exist)
{
    if(!can_exist)
        PLASSERT( name_to_idx.find(timer_name) == name_to_idx.end() );
    int n_timers = total_times.length();
    name_to_idx[timer_name] = n_timers;
    total_times.append(0);
    build();
}

////////////////////
// resetAllTimers //
////////////////////
void PTimer::resetAllTimers()
{
    map<string, int>::const_iterator it;
    for (it = name_to_idx.begin(); it != name_to_idx.end(); it++)
        resetTimer(it->first);
}

////////////////
// resetTimer //
////////////////
void PTimer::resetTimer(const string& timer_name)
{
    PLASSERT( name_to_idx.find(timer_name) != name_to_idx.end() );
    int timer_id = name_to_idx[timer_name];
    total_times[timer_id] = 0;
}

////////////////
// startTimer //
////////////////
void PTimer::startTimer(const string& timer_name)
{
    map<string, int>::const_iterator it = name_to_idx.find(timer_name);
    if (it == name_to_idx.end()) {
        // The timer does not already exist.
        this->newTimer(timer_name);
        it = name_to_idx.find(timer_name);
    }
    int timer_id = it->second;
    start_long[timer_id] = long(time(0));
    start_clock_t[timer_id] = clock();
}

///////////////
// stopTimer //
///////////////
void PTimer::stopTimer(const string& timer_name)
{
    PLASSERT( name_to_idx.find(timer_name) != name_to_idx.end() );
    int timer_id = name_to_idx[timer_name];
    clock_t time_clock_t = clock() - start_clock_t[timer_id];
    long time_long = long(time(0)) - start_long[timer_id];
    if (time_long > 1800 || use_times_fct)
        // More than 30 mins: we use the approximate length.
        total_times[timer_id] += time_long;
    else
        // Less than 30 mins: we use the more precise length.
        total_times[timer_id] += time_clock_t / real(CLOCKS_PER_SEC);
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
