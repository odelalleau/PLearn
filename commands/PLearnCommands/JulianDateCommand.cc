// -*- C++ -*-

// JulianDateCommand.cc
//
// Copyright (C) 2003 Rejean Ducharme 
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
   * $Id: JulianDateCommand.cc,v 1.1 2003/09/09 20:20:04 ducharme Exp $ 
   ******************************************************* */

// Authors: Rejean Ducharme

/*! \file JulianDateCommand.cc */


#include "JulianDateCommand.h"
#include "PDate.h"
#include "stringutils.h"
#include "plerror.h"

namespace PLearn <%
using namespace std;

//! This allows to register the 'JulianDateCommand' command in the command registry
PLearnCommandRegistry JulianDateCommand::reg_(new JulianDateCommand);

JulianDateCommand::JulianDateCommand():
    PLearnCommand("jdate",

                  "Convert a Julian Date into a JJ/MM/YYYY date",

                  "Usage: jdate <julian_day_number> \n"
                  "       To get the equivalent JJ/MM/YYYY date \n"
                  ) 
  {}

//! The actual implementation of the 'JulianDateCommand' command 
void JulianDateCommand::run(const vector<string>& args)
{
  if (args.size() != 1)
    PLERROR("jdate expect only 1 argument, the juliay day number");

  int jdn = toint(args[0]);
  PDate date(jdn);
  int dd = date.day;
  int mm = date.month;
  int yyyy = date.year;

  cout << args[0] << " correspond to " << dd << "/" << mm << "/" << yyyy << endl;
}

%> // end of namespace PLearn

