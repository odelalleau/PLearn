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
   * $Id: Profiler.cc,v 1.1 2002/07/30 09:01:28 plearner Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "Profiler.h"
#include <time.h>
namespace PLearn <%
using namespace std;

  // initialize static variables
  map<string,Profiler::Stats> Profiler::codes_statistics;
  struct tms Profiler::t;
  bool Profiler::active  = false;

#ifdef PROFILE
  // start recording time for named piece of code
  void Profiler::start(const string& name_of_piece_of_code)
    {
      if (active)
        {
          map<string,Profiler::Stats>::iterator it = 
            codes_statistics.find(name_of_piece_of_code);
          if (it == codes_statistics.end())
            {
              Stats stats;
              stats.on_going = true;
              stats.time_of_last_start = times(&t);
              codes_statistics[name_of_piece_of_code] = stats;
            }
          else
            {
              Profiler::Stats& stats = it->second;
              if (stats.on_going)
                PLERROR("Profiler::start(%s) called while previous start had not ended",
                      name_of_piece_of_code.c_str());
              stats.on_going = true;
              stats.time_of_last_start = times(&t);
            }
        }
    }

  

  // end recording time for named piece of code, and increment
  // frequency of occurence and total duration of this piece of code.
  void Profiler::end(const string& name_of_piece_of_code)
    {
      if (active)
        {
          clock_t end_time = times(&t);
          map<string,Profiler::Stats>::iterator it = 
            codes_statistics.find(name_of_piece_of_code);
          if (it == codes_statistics.end())
            PLERROR("Profiler::end(%s) called before any call to start(%s)",
                  name_of_piece_of_code.c_str(),name_of_piece_of_code.c_str());
          Profiler::Stats& stats = it->second;
          if (!stats.on_going)
            PLERROR("Profiler::end(%s) called before previous start was called",
                  name_of_piece_of_code.c_str());
          stats.on_going = false;
          stats.frequency_of_occurence++;
          int duration = end_time - stats.time_of_last_start;
          if (end_time < stats.time_of_last_start)
            { duration=1; PLWARNING("Profiler: negative duration measured with times!"); }
          stats.total_duration += duration;
        }
    }
#endif
  
    // output a report on the output stream, giving
    // the statistics recorded for each of the named pieces of codes.
  void Profiler::report(ostream& out)
    {
      map<string,Profiler::Stats>::iterator it =  
        codes_statistics.begin(), end =  codes_statistics.end();

      for (;it!=end; ++it)
      {
        out << " Ticks per second : " << sysconf(_SC_CLK_TCK)<<endl;
        out << "For " << it->first << " :" << endl;
        Profiler::Stats& stats = it->second;
        out << "Frequency of occurence = " << stats.frequency_of_occurence << endl;
        out << "Total duration = " << stats.total_duration << endl;
        double avg_duration = (double)stats.total_duration/stats.frequency_of_occurence;
        out << "Average duration = " << avg_duration << endl;
        out << endl;
      }
    }



%> // end of namespace PLearn
