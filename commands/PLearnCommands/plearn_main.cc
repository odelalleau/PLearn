// -*- C++ -*-

// plearn_main.cc
// Copyright (C) 2002 Pascal Vincent
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
   * $Id: plearn_main.cc,v 1.1 2002/10/22 09:35:54 plearner Exp $
   ******************************************************* */

#include "plearn_main.h"
#include "PLearnCommandRegistry.h"
#include "stringutils.h"

namespace PLearn <%
using namespace std;

int plearn_main(int argc, char** argv)
{
  string programname = argv[0];
  if(argc<=1)
    {
      cerr << "Type '" << programname << " help' for help" << endl;
      return 0;
    }
  else 
    {
      string command = argv[1];
      if(command=="help")
        {
          if(argc==2)
            {
              cerr << "Available commands are: " << endl;
              PLearnCommandRegistry::print_command_summary(cerr);
              cerr << "Type '" << programname << " help xxx' to get detailed help on command xxx" << endl;
            }
          else
            PLearnCommandRegistry::help(argv[2], cout);
        }
      else
        {
          vector<string> args = stringvector(argc-1, argv+1);
          PLearnCommandRegistry::run(command, args);
        }
    }
  return 0;
}


%> // end of namespace PLearn
