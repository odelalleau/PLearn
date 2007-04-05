// -*- C++ -*-

// PLearnCommandRegistry.cc
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
 * $Id$
 * This file is part of the PLearn library.
 ******************************************************* */

#include "PLearnCommandRegistry.h"
#include <iostream>
#include <vector>
#include <plearn/base/HelpSystem.h>
#include <plearn/io/PStream.h>

namespace PLearn {
using namespace std;

PLearnCommandRegistry::command_map& PLearnCommandRegistry::commands()
{
    static PLearnCommandRegistry::command_map commands_;
    return commands_;
}

void PLearnCommandRegistry::do_register(PLearnCommand* command)
{ commands()[command->name] = command; }

bool PLearnCommandRegistry::is_registered(const string& commandname)
{ return commands().find(commandname)!=commands().end(); }

/*  
void PLearnCommandRegistry::print_command_summary(ostream& out)
{
    command_map::iterator it = commands().begin();
    command_map::iterator itend = commands().end();
    while(it!=itend)
    {
        out << it->first << "\t:  " << it->second->description << endl;
        ++it;
    }
    out << endl;
}
*/

//! Issues a "bad command" message
void PLearnCommandRegistry::badcommand(const string& commandname)
{
    perr << "No '" << commandname << "' command available." << endl;
    perr << "Available commands are: " << endl;
    //print_command_summary(cerr);
    perr << HelpSystem::helpCommands() << flush;
    perr << "You can get more help for any of these commands by invoking the help command" << endl;
}

void PLearnCommandRegistry::run(const string& commandname, const vector<string>& args)
{ 
    command_map::iterator it = commands().find(commandname);
    if(it==commands().end())
        badcommand(commandname);
    else
        it->second->run(args);
}

PLearnCommand* PLearnCommandRegistry::getCommand(const string& commandname)
{
    command_map::iterator it = commands().find(commandname);
    if(it==commands().end()) badcommand(commandname);
    return it->second;
}


/*  
void PLearnCommandRegistry::help(const string& commandname, ostream& out)
{ 
    command_map::iterator it = commands().find(commandname);
    if(it==commands().end())
        badcommand(commandname);
    else
    {
        out << "*** Help for command '" << commandname << "' ***" << endl;
        out << it->second->description << endl;
        out << it->second->helpmsg << endl;        
    }
}
*/

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
