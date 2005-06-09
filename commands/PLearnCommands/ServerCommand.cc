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
   * $Id: ServerCommand.cc,v 1.6 2005/06/09 21:16:01 plearner Exp $ 
   ******************************************************* */

// Authors: Pascal Vincent

/*! \file ServerCommand.cc */


#include "ServerCommand.h"
#include <plearn/misc/PLearnServer.h>

#include <plearn/io/StdPStreamBuf.h>
#include <plearn/io/PrPStreamBuf.h>
// #include <plearn/io/FdPStreamBuf.h>
#include <mozilla/nspr/prio.h>
#include <mozilla/nspr/prerror.h>
#include <mozilla/nspr/prnetdb.h>

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
                  "server [<tcp_port>] \n"
                  "  Launches plearn in TCP server mode \n"
                  "  Will wait for a connection on the given TCP port. \n"
                  "  All i/o for commands are then done through that connection. \n"
                  " \n"
                  )
  {}

//! The actual implementation of the 'ServerCommand' command 
void ServerCommand::run(const vector<string>& args)
{
  if(args.size()>0) // Start TCP server on given port
    {
      PRStatus st;
      int port = toint(args[0]);
      PRFileDesc* sock = PR_NewTCPSocket();
      if (!sock)
        PLERROR("Servercommand: socket creation failed! (Maybe you ran out of file descriptors?)");
      PRNetAddr server_address;
      PR_InitializeNetAddr(PR_IpAddrAny, port, &server_address);
      if (PR_Bind(sock, &server_address) != PR_SUCCESS)
        PLERROR("ServerCommand: could not bind to port %d!", port);      
      for(;;)
        {
          cerr << "\n-----------------------------------\n";
          cerr << "PLEARN SERVER LISTENING ON PORT " << port << "\n"; 
          st = PR_Listen(sock,1);
          if(st!=PR_SUCCESS)
            PLERROR("serverCommand: listen on socket failed");
          PRNetAddr addr;
          PRFileDesc* fd = PR_Accept(sock, &addr, PR_INTERVAL_NO_TIMEOUT);
          if(fd==0)
            PLERROR("ServerCommand: accept returned 0, error code is: %d",PR_GetError());
          char addrstr[100];
          st = PR_NetAddrToString(&addr, addrstr, sizeof(addrstr));
          cerr << "CONNECTION FROM " << addrstr << endl;
          
          PStream io(new PrPStreamBuf(fd,fd,true,true));
          PLearnServer server(io);
          server.run();
        }
    }
  else // Start stdin/stdout seerver
    {
      cerr << "Type !? to get some help." << endl;
      // PStream io(&std::cin, &std::cout);
      PStream io(new StdPStreamBuf(&std::cin,&std::cout));
      // PStream io(new FdPStreamBuf(0,1));
      PLearnServer server(io);
      server.run();
    }
}

} // end of namespace PLearn

