// -*- C++ -*-

// PyPLearnScript.cc
//
// Copyright (C) 2005 Christian Dorion 
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
   * $Id: PyPLearnScript.cc,v 1.1 2005/02/11 09:07:50 dorionc Exp $ 
   ******************************************************* */

// Authors: Christian Dorion

/*! \file PyPLearnScript.cc */


#include "PyPLearnScript.h"

#include <plearn/sys/Popen.h>
#include <plearn/io/openFile.h>
#include <plearn/io/fileutils.h>
#include <plearn/base/stringutils.h>


namespace PLearn {
using namespace std;

PP<PyPLearnScript>
PyPLearnScript::
process( const string& scriptfile,
         const vector<string>& args,
         const string& drivername   )
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

  string plearn_script;
  bool do_help = false;
  bool do_dump = false;

  vector<string> final_args;
  final_args.push_back(scriptfile);
  
  // This option is understood by the pyplearn driver so that is returns a
  // plearn representation of a PyPLearnScript object.
  final_args.push_back("--PyPLearnScript");    
  
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
      if ( option.size() < 2 || option.substr(0, 2) != "--" )
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
  PP<PyPLearnScript> pyplearn_script = new PyPLearnScript();

  popen.in >> *pyplearn_script;

  pyplearn_script->vars    = vars;
  pyplearn_script->do_help = do_help;
  pyplearn_script->do_dump = do_dump;

  pyplearn_script->build(); 
  return pyplearn_script;  
}
  
  
PyPLearnScript::PyPLearnScript() :
  plearn_script(""),
  vars(),
  do_help(false),
  do_dump(false),
  metainfos(""),
  expdir("")
{}
  
PLEARN_IMPLEMENT_OBJECT( PyPLearnScript,
                         "Manage the output of a pyplearn driver.",
                         "Pyplearn files are Python scripts used to generate the complex plearn\n"
                         "scripts needed for vast experiments. The driver generating the plearn\n"
                         "version of the scripts from their pyplearn counterpart is also writen\n"
                         "in Python. Hence, this class is needed to bridge between the output of\n"
                         "this driver and the PLearn commands." );

void PyPLearnScript::declareOptions(OptionList& ol)
{
  declareOption( ol, "plearn_script", &PyPLearnScript::plearn_script,
                 OptionBase::buildoption,
                 "The plearn_script" );

  declareOption( ol, "vars", &PyPLearnScript::vars,
                 OptionBase::buildoption,
                 "Variables set at command line" );
 
  declareOption( ol, "do_help", &PyPLearnScript::do_help,
                 OptionBase::buildoption,
                 "Script is in fact an help message" );

  declareOption( ol, "do_dump", &PyPLearnScript::do_dump,
                 OptionBase::buildoption,
                 "Dump the script and forget it" );

  declareOption( ol, "metainfos", &PyPLearnScript::metainfos,
                 OptionBase::buildoption,
                 "Informations relative to the experiment settings, to be wrote in an\n"
                 "expdir file." );

  declareOption( ol, "expdir", &PyPLearnScript::expdir,
                 OptionBase::buildoption,
                 "The expdir of the experiment described by the script" );

  
  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void PyPLearnScript::close()
{
  if ( expdir != "" )
  {
    if ( ! file_exists( expdir ) )
      PLERROR( "The expdir '%s' managed by this PyPLearnScript was not created.",
               expdir.c_str() );
    
    PStream out = openFile( expdir / "metainfos.txt", PStream::raw_ascii, "w" );
    out << metainfos << endl;

    out = openFile( expdir / "experiment.plearn", PStream::raw_ascii, "w" );
    out << plearn_script << endl;
  }
}

void PyPLearnScript::build_()
{
  if ( do_help || do_dump )
  {
    cout << plearn_script << endl;
    plearn_script = "";
    expdir        = ""; // Disabling the close() method.
  }
  else
  {
    PStream is     = openString( plearn_script, PStream::raw_ascii );
    plearn_script  = readAndMacroProcess( is, vars );
  }
}

// ### Nothing to add here, simply calls build_
void PyPLearnScript::build()
{
  inherited::build();
  build_();
}

void PyPLearnScript::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
}

} // end of namespace PLearn
