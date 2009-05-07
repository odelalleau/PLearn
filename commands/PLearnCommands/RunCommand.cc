
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
 * $Id$ 
 ******************************************************* */

/*! \file RunCommand.cc */
#include "RunCommand.h"
#include <plearn/base/general.h>
#include <plearn/io/fileutils.h>
#include <plearn/base/plerror.h>
#include <plearn/base/stringutils.h>
#include <plearn/base/Object.h>
#include <plearn/sys/Popen.h>

#include <plearn/io/PyPLearnScript.h>
#include <plearn/io/openString.h>
#include <plearn/io/openFile.h>
#include <plearn/io/pl_log.h>

#include <algorithm>

namespace PLearn {
using namespace std;

//! This allows to register the 'RunCommand' command in the command registry
PLearnCommandRegistry RunCommand::reg_(new RunCommand);

//! The actual implementation of the 'RunCommand' command 
void RunCommand::run(const vector<string>& args)
{
    const vector<string>* the_args = &args;
    vector<string> args_augmented;
    PPath scriptfile = args[0];
    if (!isfile(scriptfile)) {
        // There is no file with this exact name. Maybe there are parameters
        // appended to the name?
        string base;
        map<string, string> params;
        parseBaseAndParameters(scriptfile, base, params);
        if (!isfile(base))
            PLERROR("Non-existent script file: %s\n",scriptfile.c_str());
        // Add new arguments.
        args_augmented = args;
        map<string, string>::const_iterator it = params.begin();
        for (; it != params.end(); it++)
            args_augmented.push_back(it->first + "=" + it->second);
        the_args = &args_augmented;
        scriptfile = base;
    }

    string extension = scriptfile.extension();
    string script;

    PP<PyPLearnScript> pyplearn_script;
    PStream in;

    if (extension == "pyplearn")
    {
        // Make a copy of args with the first argument (the name of the script)
        // removed, leaving the first argument to the script at index 0.
        vector<string> pyplearn_args(the_args->size()-1);
        copy(the_args->begin() + 1, the_args->end(), pyplearn_args.begin());
    
        pyplearn_script = PyPLearnScript::process(scriptfile, pyplearn_args);
        script          = pyplearn_script->getScript();
    
        // When we call the pyplearn script with either
        // --help or --dump, everything will already have been done by
        // the time the PyPLearnScript is built. 
        if ( script == "" )
            return;    

        PL_Log::instance().enableNamedLogging(pyplearn_script->module_names);
        PL_Log::instance().verbosity(pyplearn_script->verbosity);

        in = openString( script, PStream::plearn_ascii );
    }
    else if(extension=="plearn")  // perform plearn macro expansion
    {
        map<string, string> vars;
        // populate vars with the arguments passed on the command line
        for (unsigned int i=1; i<the_args->size(); i++)
        {
            string option = (*the_args)[i];
            // Skip --foo command-lines options.
            if (option.size() < 2 || option.substr(0, 2) != "--")
            {
                pair<string, string> name_val = split_on_first(option, "=");
                vars[name_val.first] = name_val.second;
            }
        }

        script = readFileAndMacroProcess(scriptfile, vars);
        in = openString( script, PStream::plearn_ascii );
    }
    else if(extension=="psave") // do not perform plearn macro expansion
    {
        in = openFile(scriptfile, PStream::plearn_ascii);
    }
    else
        PLERROR("Invalid extension for script file. Must be one of .pyplearn .plearn .psave");

    while ( in.good() )
    {
        PP<Object> o = readObject(in);
        o->run();
        in.skipBlanksAndCommentsAndSeparators();
    }

    if ( pyplearn_script.isNotNull() )
        pyplearn_script->close();
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
