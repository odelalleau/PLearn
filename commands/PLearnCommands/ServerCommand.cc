// -*- C++ -*-

// ServerCommand.cc
//
// Copyright (C) 2005 Pascal Vincent 
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
   * $Id: ServerCommand.cc,v 1.1 2005/01/06 02:09:37 plearner Exp $ 
   ******************************************************* */

// Authors: Pascal Vincent

/*! \file ServerCommand.cc */


#include "ServerCommand.h"
#include "plearn/misc/PLearnServer.h"

namespace PLearn {
using namespace std;

//! This allows to register the 'ServerCommand' command in the command registry
PLearnCommandRegistry ServerCommand::reg_(new ServerCommand);

ServerCommand::ServerCommand():
    PLearnCommand("server",

                  "Launches plearn in computation server mode",

                  "server\n"
                  "  Launches plearn in stdin/stdout server mode, \n"
                  "  listening for commands on standard in \n"
                  "  and outputing results on standard out. \n"
                  " \n"
                  "server [<listen_port>] \n"
                  "  Launches plearn in TCP server mode, (not yet implemented) \n"
                  "  Will initially output to stdout: hostname port \n"
                  "  and wait for a connection on the given TCP port. \n"
                  "  All i/o for commands are then done through that connection. \n"
                  " \n"
                  "Summary of commands:\n"
                  "  > \"dirpath\"             # change directory to dirpath. Returns: R 0 \n"
                  "  N object_specification    # creates new object. Returns: R 1 objid \n"
                  "  M objid methodname nargs arg1 ... # calls method on object objid. Returns: R <nreturn> ret1 ... \n"
                  "  D objid                   # deletes object objid. Returns: R 0 \n"
                  "  Z                         # delete all objects. Returns: R 0 \n"
                  "  Q                         # Quit. Returns nothing. \n"
                  )
  {}

//! The actual implementation of the 'ServerCommand' command 
void ServerCommand::run(const vector<string>& args)
{
  PStream io(&std::cin, &std::cout);
  PLearnServer server(io);
  server.run();
}

} // end of namespace PLearn

