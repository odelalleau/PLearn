// -*- C++ -*-

// pl_log.cc
//
// Copyright (C) 2004 Nicolas Chapados 
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
   * $Id: pl_log.cc,v 1.6 2005/05/24 19:03:27 chrish42 Exp $ 
   ******************************************************* */

// Authors: Nicolas Chapados, Christian Dorion

/*! \file pl_log.cc */

#include <map>
#include <string>
#include <plearn/base/stringutils.h>
#include "pl_log.h"

namespace PLearn {
using namespace std;

PL_Log::PL_Log( )
  : runtime_verbosity(VLEVEL_NORMAL),
    output_stream(&cerr, false),
    null_stream(get_pnull()),
    logger_count(0)
{}

PStream& PL_Log::logger(int requested_verbosity)
{
  logger_count++;
  if (requested_verbosity <= runtime_verbosity)
    return output_stream;
  else
    return null_stream;
}

PL_Log& PL_Log::instance()
{
  static PL_Log global_logger;
  return global_logger;
}

/**
 * Parses a string to see whether or not it names a VerbosityLevel. If it
 * doesn't, tries the cast to an int.
 */
VerbosityLevel PL_Log::vlevel_from_string(const string& v)
{
  static map<string, VerbosityLevel> _vlevels;
  if ( _vlevels.size() == 0 )
  {
    _vlevels["VLEVEL_MAND"]    = VLEVEL_MAND;
    _vlevels["VLEVEL_IMP"]     = VLEVEL_IMP;
    _vlevels["VLEVEL_NORMAL"]  = VLEVEL_NORMAL ;
    _vlevels["VLEVEL_DBG"]     = VLEVEL_DBG    ;
    _vlevels["VLEVEL_EXTREME"] = VLEVEL_EXTREME;
  }
  
  map<string, VerbosityLevel>::iterator it = _vlevels.find(v);
  if ( it != _vlevels.end() )
    return it->second;
  return (VerbosityLevel)toint(v);
}


PStream& plsep(PStream& p)
{
  return p << plhead("");
}

PStream& operator<<(PStream& p, PL_Log::Heading heading)
{
  // The loggerCount is likely to change in test even if nothing more is
  // printed... PyTest dislikes. 
  //   string msg = "#####  " + tostring(PL_Log::instance().loggerCount())
  //     + (heading.h.size() > 0? (": "+heading.h) : string("")) + "  ";
  string msg = "#####  " + (heading.h.size() > 0? (heading.h + "  ") : string(""));
  string rest(max(75-int(msg.size()),5),'#');
  return p << endl << (msg + rest) << endl;
}

} // end of namespace PLearn
