// -*- C++ -*-

// PyPlearnDriver.cc
//
// Copyright (C) 2004 ApSTAT Technologies Inc. 
// All rights reserved.
//
// This program may not be modified, translated, 
// copied or distributed in whole or in part or 
// as part of a derivative work, in any form, 
// whether as source code or binary code or any 
// other form of translation, except if authorized 
// by a prior agreement signed with ApSTAT Technologies Inc.

/* *******************************************************      
   * $Id: PyPlearnDriver.cc,v 1.5 2005/02/10 01:12:06 tihocan Exp $ 
   ******************************************************* */

// Authors: Nicolas Chapados

/*! \file PyPlearnDriver.cc */

#include "PyPlearnDriver.h"
#include <plearn/sys/Popen.h>
#include <plearn/base/general.h>
#include <plearn/base/plerror.h>
#include <plearn/base/stringutils.h>

#include <plearn/io/PPath.h>
#include <plearn/io/fileutils.h>

namespace PLearn {
using namespace std;

string process_pyplearn_script(const string& scriptfile,
                               const vector<string>& args,
                               const string& drivername)
{
  // Handle command-line bindings of the form x=y
  map<string, string> vars;
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

  string script;
  bool do_help = false;
  bool do_dump = false;

  vector<string> final_args;
  final_args.push_back(scriptfile);

  // For pyplearn files, we support the following command-line
  // arguments: --help to print the doc string of the .pyplearn file,
  // and --dump to show the result of processing the .pyplearn file
  // instead of running it.
  if (args.size() >= 2)
  {
    if (args[1] == "--help")
    {
      do_help = true;
    }
    else if (args[1] == "--dump")
    {
      do_dump = true;
    }
  }
      
  if (do_help)
  {
    final_args.push_back("--help");
  }
  else
  {
    // Supply the PLearn command-line arguments to the pylearn driver
    // script.
    for (unsigned int i = 1; i < args.size(); i++)
    {
      string option = args[i];
      // Skip --foo command-lines options.
      if (option.size() < 2 || option.substr(0, 2) != "--")
      {
        final_args.push_back(args[i]);
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
      final_args.push_back(it->first + '=' + it->second);
    }          
  }
      
  Popen popen(drivername, final_args);
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
    return "";
  }
  else
  {
    PStream is = openString(script_before_preprocessing, PStream::raw_ascii);
    script = readAndMacroProcess(is, vars);
  }

  return script;
}

} // end of namespace PLearn
