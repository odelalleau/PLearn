// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2001 Yoshua Bengio and University of Montreal
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
   * $Id: Profiler.h,v 1.3 2004/02/26 07:38:35 nova77 Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

/*! \file PLearnLibrary/PLearnUtil/Profiler.h */

#ifndef PROFILER_INC
#define PROFILER_INC

#define PROFILE

#ifdef WIN32
// For the moment I put a compiler error
// but I could throw a PERROR..
#error Profiler is not working on WIN32
#endif

#include <sys/times.h>
#include "general.h"
#include "plerror.h"
#include "stringutils.h"

namespace PLearn {
using namespace std;

/*!     Profiling tools, to count average time elapsed and number of times 
    traversed for pieces of code delimited by two calls to the static
    functions
    
       Profiler::start("name_of_piece_of_code");
       ...
       Profiler::end("name_of_piece_of_code");
    
    A static field of Profiler is used to maintain the statistics of 
    occurence and durations for each piece of code. Calls to start/end
    for the same name cannot be nested.
    
    Before the above calls, usually in the main program, the user
    must call
    
       Profiler::activate();
    
    and after all the above calls, a report for all such pieces of code can then be obtained by calling 
    
       Profiler::report(cout);
    
    on an output stream. 
    
    This code is based on statistical averaging (using the C "times" function)
    because the individual measurements of elapsed time with times are too
    coarse (e.g. 100th of a second).
*/

  class Profiler {
    protected:

    class Stats {
      public:
      int frequency_of_occurence;
      int total_duration;
      clock_t time_of_last_start;
      bool on_going;
      
      Stats(int f=0, int d=0) : 
        frequency_of_occurence(f), total_duration(d), 
        time_of_last_start(0), on_going(false) {}
    };

    public:

    static void activate() { active=true; }
    static void disactivate() { active=false; }

    //!  start recording time for named piece of code
#ifdef PROFILE
    static void start(const string& name_of_piece_of_code);
#else
    static inline void start(const string& name_of_piece_of_code) { }
#endif

    //!  end recording time for named piece of code, and increment
    //!  frequency of occurence and total duration of this piece of code.
#ifdef PROFILE
    static void end(const string& name_of_piece_of_code);
#else
    static inline void end(const string& name_of_piece_of_code) { } 
#endif
    
    //!  output a report on the output stream, giving
    //!  the statistics recorded for each of the named pieces of codes.
    static void report(ostream& out);

    protected:
    
    static map<string,Stats> codes_statistics;
    static struct tms t;
    static bool active;

  };

} // end of namespace PLearn

#endif

