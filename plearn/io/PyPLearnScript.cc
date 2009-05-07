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
 * $Id$ 
 ******************************************************* */

// Authors: Christian Dorion

/*! \file PyPLearnScript.cc */


#include "PyPLearnScript.h"

#include <plearn/sys/Popen.h>
#include <plearn/io/openFile.h>
#include <plearn/io/fileutils.h>
#include <plearn/base/stringutils.h>
#include <plearn/base/pl_repository_revision.h>
#include <plearn/vmat/VMatrix.h>
#include <plearn/io/pl_log.h>


namespace PLearn {
using namespace std;

PP<PyPLearnScript>
PyPLearnScript::
process( const string& scriptfile,
         const vector<string>& args,
         const string& drivername   )
{
    bool do_help = false;
    bool do_dump = false;
  
    // List of arguments passed to the pyplearn_driver.py script.
    vector<string> final_args;
    // The vars map is for PLearn arguments (i.e. arguments accessed through
    // the ${varname} syntax.
    map<string, string> vars;

#ifdef WIN32
    // Under Windows, we must specify that the driver needs to be run with
    // Python, thus the driver becomes an argument.
    final_args.push_back(drivername);
#endif
    final_args.push_back(scriptfile);
  
    // This option is understood by the pyplearn driver so that is returns a
    // plearn representation of a PyPLearnScript object.
    final_args.push_back("--PyPLearnScript");    
  
    // For pyplearn files, we support the following command-line
    // arguments: --help to print the doc string of the .pyplearn file,
    // and --dump to show the result of processing the .pyplearn file
    // instead of running it.
    if (args.size() >= 1)
    {
        if (args[0] == "--help")
        {
            do_help = true;
        }
        else if (args[0] == "--dump")
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
        // Supply the command-line arguments to the pylearn driver
        // script.
        for (unsigned int i = 0; i < args.size(); i++)
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
        addFileAndDateVariables(scriptfile, vars, 0);

        // Also add these variables (DATE, etc.) to the pyplearn driver
        // script so they show up as pyplearn arguments too.
        for (map<string, string>::const_iterator it = vars.begin();
             it != vars.end(); ++it)
        {
            final_args.push_back(it->first + '=' + it->second);
        }

        // Add the command-line arguments to the list of PLearn arguments.
        for (unsigned int i = 0; i < args.size(); i++)
        {
            string option = args[i];
            // Skip --foo command-lines options.
            if (option.size() < 2 || option.substr(0, 2) != "--")
            {
                pair<string,string> name_val = split_on_first(option, "=");
                vars[name_val.first] = name_val.second;
            }
        }
    }

    final_args.push_back(string("--enable-logging=")
                         + join(PL_Log::instance().namedLogging(), ","));
    final_args.push_back(string("--verbosity=")
                         + tostring(PL_Log::instance().verbosity()))    ;
#ifdef WIN32
    Popen popen("python.exe", final_args);
#else
    Popen popen(drivername, final_args);
#endif
    PP<PyPLearnScript> pyplearn_script = new PyPLearnScript();

    popen.in >> *pyplearn_script;

    pyplearn_script->vars    = vars;
    pyplearn_script->do_help = do_help;
    pyplearn_script->do_dump = do_dump;

    pyplearn_script->build(); 
    return pyplearn_script;  
}

PStream
PyPLearnScript::
openScriptFile(int argc, char** argv, const std::string& drivername)
{
    prgname(argv[0]);   // set program name
    vector<string> command_line = stringvector(argc-1, argv+1);   // neglecting progname

    string scriptfile = command_line[0];
    if (!isfile(scriptfile))
        PLERROR("Non-existent script file: %s\n",scriptfile.c_str());
    const string extension = extract_extension(scriptfile);

    string script;
    if (extension == ".pyplearn")
    {
        // Make a copy of args with the first argument (the name of the script)
        // removed, leaving the first argument to the script at index 0.
        vector<string> pyplearn_args(command_line.size()-1);
        copy(command_line.begin() + 1, command_line.end(), pyplearn_args.begin());
    
        PP<PyPLearnScript> pyplearn_script =
            PyPLearnScript::process(scriptfile, pyplearn_args, drivername);
        script = pyplearn_script->getScript();
        
        // When we call the pyplearn script with either
        // --help or --dump, everything will already have been done by
        // the time the PyPLearnScript is built. 
        if ( script == "" )
            exit(0);        
    }
    else
        PLERROR("PyPLearnScript::openScript -- Expecting a .pyplearn file.");
    
    return openString(script, PStream::plearn_ascii);
}

PyPLearnScript::PyPLearnScript() :
    plearn_script(""),
    vars(),
    do_help(false),
    do_dump(false),
    metainfos(""),
    expdir(""),
    verbosity(PL_Log::instance().verbosity()),
    module_names(PL_Log::instance().namedLogging())
{}
  
PLEARN_IMPLEMENT_OBJECT( PyPLearnScript,
                         "Manage the output of a PyPLearn driver.",
                         "PyPLearn files are Python scripts used to generate the complex PLearn\n"
                         "scripts needed for vast experiments. The driver generating the PLearn\n"
                         "version of the scripts from their PyPLearn counterpart is also written\n"
                         "in Python. Hence, this class is needed to bridge between the output of\n"
                         "this driver and the PLearn commands." );

void PyPLearnScript::run()
{
    PL_Log::instance().enableNamedLogging(module_names);
    PL_Log::instance().verbosity(verbosity);

    PStream in = openString( plearn_script, PStream::plearn_ascii );

    while ( in.good() )
    {
        PP<Object> o = readObject(in);
        o->run();
        in.skipBlanksAndCommentsAndSeparators();
    }

    close();
}

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

    declareOption( ol, "verbosity", &PyPLearnScript::verbosity,
                   OptionBase::buildoption,
                   "The verbosity level to set for this experiment" );

    declareOption( ol, "module_names", &PyPLearnScript::module_names,
                   OptionBase::buildoption,
                   "Modules for which logging should be activated" );

  
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void PyPLearnScript::close()
{
    if ( !expdir.isEmpty() )
    {
        if ( ! isdir( expdir ) ) {
            // TODO Why should the expdir be systematically created?
        //     PLERROR( "The expdir '%s' managed by this PyPLearnScript was not created.",
        //           expdir.c_str() );
        } else {

            PStream out = openFile( expdir / "metainfos.txt", PStream::raw_ascii, "w" );
            out << "__REVISION__ = " << pl_repository_revision() << endl;
            out << metainfos << endl;

            out = openFile( expdir / "experiment.plearn", PStream::raw_ascii, "w" );
            out << plearn_script << endl;
        }
    }
}

void PyPLearnScript::build_()
{
    if ( do_help || do_dump )
    {
        pout << plearn_script << endl;
        plearn_script = "";
        expdir        = ""; // Disabling the close() method.
    }
    else
    {
        PStream is     = openString( plearn_script, PStream::raw_ascii );
        time_t latest = 0 ;
        plearn_script  = readAndMacroProcess( is, vars, latest );
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

Object* smartLoadObject(PPath filepath, const vector<string>& args_, time_t& return_date)
{
    vector<string> args = args_;
    vector<string> args_augmented;

    if (!isfile(filepath)) {
        // There is no file with this exact name. Maybe there are parameters
        // appended to the name?
        string base;
        map<string, string> params;
        parseBaseAndParameters(filepath, base, params);
        if (!isfile(base))
            PLERROR("Non-existent script file: %s\n", filepath.c_str());
        // Add new arguments.
        args_augmented = args;
        map<string, string>::const_iterator it = params.begin();
        for (; it != params.end(); it++)
            args_augmented.push_back(it->first + "=" + it->second);
        args = args_augmented;
        filepath = base;
    }

    const string extension = extract_extension(filepath);
    string script;
    time_t date = 0;

    PP<PyPLearnScript> pyplearn_script;
    PStream in;

    if (extension == ".pyplearn" || extension==".pymat")
    {
        // Make a copy of args with the first argument (the name of the script)
        // removed, leaving the first argument to the script at index 0.
        vector<string> pyplearn_args(args.size()-1);
        copy(args.begin() + 1, args.end(), pyplearn_args.begin());
    
        pyplearn_script = PyPLearnScript::process(filepath, pyplearn_args);
        script          = pyplearn_script->getScript();
    
        // When we call the pyplearn script with either
        // --help or --dump, everything will already have been done by
        // the time the PyPLearnScript is built. 
        if ( script == "" )
            PLERROR("Empty script");

        in = openString( script, PStream::plearn_ascii );
    }
    else if(extension==".plearn" || extension==".vmat")  // perform plearn macro expansion
    {
        map<string, string> vars;
        // populate vars with the arguments passed on the command line
        for (unsigned int i=1; i<args.size(); i++)
        {
            string option = args[i];
            // Skip --foo command-lines options.
            if (option.size() < 2 || option.substr(0, 2) != "--")
            {
                pair<string, string> name_val = split_on_first(option, "=");
                vars[name_val.first] = name_val.second;
            }
        }
        script = readFileAndMacroProcess(filepath, vars, date);
        in = openString( script, PStream::plearn_ascii );
    }
    else if(extension==".psave") // do not perform plearn macro expansion
    {
        in = openFile(filepath, PStream::plearn_ascii);
        date=mtime(filepath);
    }
    else
        PLERROR("In smartLoadObject: unsupported file extension. Must be one of .pyplearn .pymat .plearn .vmat .psave");

    Object* o = readObject(in);
    if(extension==".vmat")
        ((VMatrix*)o)->updateMtime(date);
    return_date=date;
    if ( pyplearn_script.isNotNull() )
        pyplearn_script->close();

    return o;
}


} // end of namespace PLearn


/*
  Local Variables:
  mode:c++
  c-basic-offset:4
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:79
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=79 :
