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
   * $Id: plearn_main.cc,v 1.16 2004/10/29 14:48:09 dorionc Exp $
   ******************************************************* */

//#include "general.h"
#include "plearn_main.h"
#include "PLearnCommandRegistry.h"
//#include "stringutils.h"
#include <plearn/math/random.h>
#include <plearn/sys/PLMPI.h>
//#include "Object.h"
//#include "RunCommand.h"

namespace PLearn {
using namespace std;

int plearn_main(int argc, char** argv)
{
  try {

  PLMPI::init(&argc, &argv);

  seed();

  // set program name
  prgname(argv[0]);

  if(argc<=1)
    {
      cerr << "Type '" << prgname() << " help' for help" << endl;
      return 0;
    }
  else 
    {
      string command = argv[1];
						
      if(PLearnCommandRegistry::is_registered(command))
        {
          vector<string> args = stringvector(argc-2, argv+2);
          PLearnCommandRegistry::run(command, args);

        }
      else if(file_exists(command))
        {
          vector<string> args = stringvector(argc-1, argv+1);
          PLearnCommandRegistry::run("run", args);
        }
      else
        PLERROR("%s appears to neither be a known PLearn command, nor an existing .plearn script",command.c_str());
    }


  PLMPI::finalize();

  } // end of try
  catch(const PLearnError& e)
  {
    cerr << "FATAL ERROR: " << e.message() << endl;
  }
  catch (...) 
  {
    cerr << "FATAL ERROR: uncaught unknown exception" << endl;
  }

  return 0;
}

void version( int& argc, char** argv, 
              int major_version, int minor_version, int fixlevel )
{
  if (argc >= 2 && !strcmp(argv[1], "--no-version")) 
  {
    // eat the "--no-version" argument
    for (int i=2; i<argc; ++i)
      argv[i-1] = argv[i];
    argc--;
  }  
  else 
  {
    cerr << argv[0] << " " << major_version
         << "." << minor_version
         << "." << fixlevel
         << "  (" << __DATE__ << " " << __TIME__ << ")" << endl;
  }
}

} // end of namespace PLearn
