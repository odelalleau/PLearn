// -*- C++ -*-

// HelpCommand.cc
// 
// Copyright (C) 2003 Pascal Vincent 
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

/*! \file HelpCommand.cc */
#include "HelpCommand.h"

#include <plearn/io/PPath.h>
#include <plearn/sys/Popen.h>
#include <plearn/db/getDataSet.h>
#include <plearn/base/general.h>        //!< For prgname().
#include <plearn/base/stringutils.h>
#include <plearn/io/fileutils.h>        //!< For isfile().
#include <plearn/base/HelpSystem.h>

namespace PLearn {
using namespace std;

//! This allows to register the 'HelpCommand' command in the command registry
PLearnCommandRegistry HelpCommand::reg_(new HelpCommand);

void HelpCommand::helpOverview()
{
    pout << 
        "To run a .plearn script type:                       " + prgname() + " scriptfile.plearn \n"
        "To run a command type:                              " + prgname() + " command [ command arguments ] \n\n"
        "To get help on the script file format:              " + prgname() + " help scripts \n"
        "To get a short description of available commands:   " + prgname() + " help commands \n"
        "To get detailed help on a specific command:         " + prgname() + " help <command_name> \n"
        "To get help on a specific PLearn object:            " + prgname() + " help <object_type_name> \n"
        "To get help on datasets:                            " + prgname() + " help datasets \n\n" 
        ""
        "Global parameter:\n"
        "                 --no-progress: don't print progress bar\n"
        "                 --no-version: do not print the version \n"
        "                 --windows_endl: use windows end of line\n"
        "                 --profile: print some profiling information\n"
        "                     Must have been compiled with '-logging=dbg-profile'\n"
        "                 --verbosity LEVEL: The level of log to print.\n"
        "                     Must have been compiled with the same level of more\n"
        "                     Available level:\n"
        "                             VLEVEL_MAND    // Mandatory\n"
        "                             VLEVEL_IMP     // Important\n"
        "                             VLEVEL_NORMAL  // Normal (Default)\n"
        "                             VLEVEL_DBG     // Debug Info\n"
        "                             VLEVEL_EXTREME // Extreme Verbosity\n"
        "                 --enable-logging module1,module2,module3,...:\n"
        "                     Option to enable logging for the specified modules,\n"
        "                     specified a comma-separated list of modules (without spaces).\n"
        "                     Special keywords __ALL__ and __NONE__ can be specified to log\n"
        "                     for all modules or no modules respectively.\n"
        "                 --servers\n"
        "                 --global-calendars"
         << endl;
}

void HelpCommand::helpScripts()
{
    pout << 
        "You can run plearn with the name of a plearn script file as argument\n"
        "A plearn script file should have a name ending in .plearn\n\n"
        "A plearn script must contain at least one runnable PLearn object\n"
        "A typical runnable PLearn object is 'PTester' \n"
        "\n"
        "You can type '" + prgname() + " help xxx' to get a description and the list of build options\n"
        "for any PLearn object xxx linked with the program\n"
        "\n"
        "A plearn script can use macro variable definitions and expansion. Macro commands start by a $\n"
        "ex: $DEFINE{toto}{[1,2,3,4]}  ${toto}  $INCLUDE{otherfile.pscript} \n"
        "Macro variable definitions can also be provided on the command line in the form \n"
        "varname=varvalue with each such pair separated by a blank, thus\n"
        "allowing for scripts with arguments\n"
        "In addition, the following variables are automatically defined from the script's filepath: \n"
        "FILEPATH DIRPATH FILENAME FILEBASE FILEEXT \n" 
        "Ex: if the absolute path to the script file is /home/me/foo.plearn \n"
        " Then we'll get: \n"
        "FILEPATH = /home/me/foo.plearn \n"
        "DIRPATH  = /home/me \n"
        "FILENAME = foo.plearn \n"
        "FILEBASE = foo \n"
        "FILEEXT  = .plearn \n"
        "\n"
        "The additional variables are also available:\n"
        "DATE     = Date in YYYYMMDD format\n"
        "TIME     = Time in HHMMSS format\n"
        "DATETIME = Date and time in YYYYMMDD:HHMMSS format\n"
         << endl;
}

void HelpCommand::helpCommands()
{
    pout << "To run a command, type:"
         << "  % " + prgname()  + " command_name command_arguments \n" << endl;

    pout << "Available commands are: " << endl;
    pout << HelpSystem::helpCommands() << endl;

    pout << "For more details on a specific command, type: \n" 
         << "  % " << prgname() << " help <command_name> \n"
         << endl;
}

void HelpCommand::helpDatasets()
{
    pout << getDataSetHelp() << endl;
}

void HelpCommand::helpAboutPLearnScript(const string& fname)
{
    if(!isfile(fname))
        PLERROR("Could not open script file %s", fname.c_str());
    pout << 
        "Help about a .plearn script file not yet implemented \n"
         << endl;
}

void HelpCommand::helpAboutPyPLearnScript(const string& fname)
{
    if(!isfile(fname))
        PLERROR("Could not open script file %s", fname.c_str());
    
    pout << "#######################################################" << endl;
    pout << "### Help on pyplearn script " << fname << endl;
    pout << "#######################################################" << endl;
    string command;
#ifdef WIN32
    command = "python.exe "+PPath("PLEARNDIR:scripts/pyplearn_driver.py").absolute();
#else
    command = "pyplearn_driver.py";
#endif
    vector<string> helptext = execute(command+" "+fname+" --help");
    vector<string>::const_iterator it = helptext.begin();
    while(it!=helptext.end())
        pout << *it++ << endl;
}


//! The actual implementation of the 'HelpCommand' command 
void HelpCommand::run(const vector<string>& args)
{
    if(args.size()==0)
        helpOverview();//TODO: move to HelpSystem
    else
    {
        string about = args[0];
        string aboutext = extract_extension(about);
        
        if(args.size() > 1)//is option level present?
            OptionBase::setCurrentOptionLevel(
                OptionBase::optionLevelFromString(args[1]));

        // Note: some of these functions could be moved to HelpSystem

        if(aboutext==".plearn") // help is asked about a plearn script
            helpAboutPLearnScript(about);
        else if(aboutext==".pyplearn") // help is asked about a pyplearn script
            helpAboutPyPLearnScript(about);
        else if(about=="scripts")
            helpScripts();
        else if(about=="commands")
            helpCommands();
        else if(about=="datasets")
            helpDatasets();//TODO: move to HelpSystem
        else if(PLearnCommandRegistry::is_registered(about))
            pout << HelpSystem::helpOnCommand(about) << flush;
        else 
            pout << HelpSystem::helpOnClass(about) << flush;
    }
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
