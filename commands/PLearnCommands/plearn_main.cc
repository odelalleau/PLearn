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
   * $Id: plearn_main.cc,v 1.10 2003/05/07 05:39:16 plearner Exp $
   ******************************************************* */

#include "general.h"
#include "plearn_main.h"
#include "PLearnCommandRegistry.h"
#include "stringutils.h"
#include "random.h"
#include "PLMPI.h"
#include "Object.h"

namespace PLearn <%
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
      else // we suppose it's a filename of a .psave or .pexp file containing Objects to be run
        {
          if(!file_exists(command))
            {
              cerr << "** ERROR: " << command << " appears to be neither a valid plearn command type, nor an existing filename" << endl;
              cerr << "Type plearn with no argument to see the help." << endl;
              exit(0);
            }

          map<string, string> vars;
          // populate vars with the arguments passed on the command line
          for(int i=2; i<argc; i++)
            {
              string option = argv[i];
              pair<string,string> name_val = split_on_first(option, "=");
              vars[name_val.first] = name_val.second;
            }
          PStream pout(&cout);
          pout << vars << endl;
      
          string script = readFileAndMacroProcess(command, vars);
          PIStringStream in(script);

          while(in)
            {
              PP<Object> o = readObject(in);
              o->run();
              in.skipBlanksAndCommentsAndSeparators();
              // cerr << bool(in) << endl;
              // cerr << in.peek() << endl;
            }
        }

    }


  PLMPI::finalize();

  } // end of try
  catch(const PLearnError& e)
    {
      cerr << "FATAL ERROR: " << e.message() << endl;
    }
  return 0;
}


%> // end of namespace PLearn
