
// -*- C++ -*-

// RunCommand.cc
//
// Copyright (C) 2003  Pascal Vincent 
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
   * $Id: RunCommand.cc,v 1.11 2004/08/27 21:27:48 chrish42 Exp $ 
   ******************************************************* */

/*! \file RunCommand.cc */
#include "RunCommand.h"
#include <plearn/base/general.h>
#include <plearn/io/fileutils.h>
#include <plearn/base/plerror.h>
#include <plearn/base/stringutils.h>
#include <plearn/base/Object.h>
#include <plearn/sys/Popen.h>

namespace PLearn {
using namespace std;

//! This allows to register the 'RunCommand' command in the command registry
PLearnCommandRegistry RunCommand::reg_(new RunCommand);

//! The actual implementation of the 'RunCommand' command 
void RunCommand::run(const vector<string>& args)
{
  string scriptfile = args[0];
  if (!file_exists(scriptfile))
    PLERROR("Non existant script file: %s\n",scriptfile.c_str());

  map<string, string> vars;
  // populate vars with the arguments passed on the command line
  for (unsigned int i=1; i<args.size(); i++)
    {
      string option = args[i];
      // Skip --foo command-lines options.
      if (option.size() < 2 || option.substr(0, 2) != "--")
        {
          pair<string,string> name_val = split_on_first(option, "=");
          vars[name_val.first] = name_val.second;
        }
    }
  PStream pout(&cout);
  //  pout << vars << endl;

  const string extension = extract_extension(scriptfile);
  string script;
  if (extension == ".pyplearn")
    {
      bool do_help = false;
      bool do_dump = false;
      string command = "pyplearn_driver.py '" + scriptfile + '\'';

      // For pyplearn files, we support the following command-line
      // arguments: --help to print the doc string of the .pyplearn file,
      // and --dump to show the result of processing the .pyplearn file
      // instead of running it.
      if (args[1] == "--help")
        {
          do_help = true;
          command += " --help";
        }
      else if (args[1] == "--dump")
        {
          do_dump = true;
        }
      
      if (!do_help)
        {
          // Supply the PLearn command-line arguments to the pylearn driver
          // script.
          for (unsigned int i = 1; i < args.size(); i++)
            {
              string option = args[i];
              // Skip --foo command-lines options.
              if (option.size() < 2 || option.substr(0, 2) != "--")
                {
                  command += " '" + args[i] + '\'';
                }
            }

          // Add the standard PLearn variables (DATE, DATETIME, etc.) to the
          // list of variables used when runing the PLearn preprocessor on the
          // output of the .pyplearn file to handle $DATE, etc.
          // in PLearn strings.
          addFileAndDateVariables(scriptfile, vars);

          // Also add these variables (DATE, etc.) to the pyplearn driver
          // script so they show up as pyplearn arguments too.
          for (map<string, string>::const_iterator it = vars.begin();
               it != vars.end(); ++it)
            {
              command += " '" + it->first + '=' + it->second + '\'';
            }
          
        }

      Popen popen(command);      
      string script_before_preprocessing;
      while (!popen.in.eof())
	{
	  char c;
	  popen.in.get(c);
	  script_before_preprocessing += c;
	}

      if (do_help || do_dump)
        {
          cout << script_before_preprocessing;
          return;
        }
      else
        {
          istringstream is(script_before_preprocessing);
          script = readAndMacroProcess(is, vars);
        }
    }
  else
    {
      script = readFileAndMacroProcess(scriptfile, vars);
    }

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

} // end of namespace PLearn

