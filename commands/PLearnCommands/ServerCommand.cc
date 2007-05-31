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
 * $Id$ 
 ******************************************************* */

// Authors: Pascal Vincent

/*! \file ServerCommand.cc */


#include "ServerCommand.h"
#include <plearn/misc/PLearnServer.h>
#include <plearn/io/PStream.h>
// #include <plearn/io/StdPStreamBuf.h>
// #include <plearn/io/FdPStreamBuf.h>
#include <plearn/io/pl_log.h>
#include <plearn/io/PrPStreamBuf.h>
#include <plearn/io/openFile.h>
#include <plearn/base/tostring.h>
#include <plearn/base/PrUtils.h>
#include <nspr/prio.h>
#include <nspr/prerror.h>
#include <nspr/prnetdb.h>

#ifndef WIN32
// POSIX includes for getpid() and gethostname()
#include <sys/types.h>
#include <unistd.h>
#endif 


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
                  "server <tcp_port> [<outfile>] [-singleuse]\n"
                  "  Launches plearn in TCP server mode \n"
                  "  Will then print a line to outfile or to stdout (if no outfile is specified) of the form: \n"
                  "    PLEARN_SERVER_TCP hostname port pid\n"
                  "  Will then wait for a connection on the given TCP port. \n"
                  "  All i/o for commands are then done through that connection. \n"
                  "  If you specify -singleuse, then the server will exit when the conneciton \n"
                  "  with its first client is closed or lost."
        )
{}

//! The actual implementation of the 'ServerCommand' command 
void ServerCommand::run(const vector<string>& args)
{
    if(args.size()>0) // Start TCP server on given port
    {
        PRStatus st;
        char buf[256];
        int port = toint(args[0]);

        bool singleuse=false;
        string outfile;
        if(args.size()>1)
        {
            if(args[1]=="-singleuse")
                singleuse = true;
            else
                outfile = args[1];
            if(args.size()>2 && args[2]=="-singleuse")
                singleuse = true;
        }

        PRFileDesc* sock = PR_NewTCPSocket();
        if (!sock)
            PLERROR("Servercommand: socket creation failed! (Maybe you ran out of file descriptors?)");
        PRNetAddr server_address;
        PR_InitializeNetAddr(PR_IpAddrAny, port, &server_address);
        if (PR_Bind(sock, &server_address) != PR_SUCCESS)
            PLERROR("ServerCommand: could not bind to port %d!", port);

        // Allow reuse of the socket immediately after the server shuts down
        PRSocketOptionData socket_option_data;
        socket_option_data.option = PR_SockOpt_Reuseaddr;
        socket_option_data.value.reuse_addr = PR_TRUE;
        PR_SetSocketOption(sock, &socket_option_data);

        string myhostname = "UNKNOWN_HOSTNAME";
        string mypid = "UNKNOWN_PID";      
#ifndef WIN32
        mypid = tostring(getpid());
        if(gethostname(buf, sizeof(buf))==0)
            myhostname = buf;
#endif

        PRNetAddr assigned_addr;
        PR_InitializeNetAddr(PR_IpAddrAny, PR_htons(0), &assigned_addr);
        st= PR_GetSockName(sock, &assigned_addr);
        if(port==0 && st==PR_SUCCESS)
            port= PR_ntohs(assigned_addr.inet.port);
        
        if(outfile.size()>0)
        {
            string tmpoutfile =outfile+".tmp";
            PStream out = openFile(tmpoutfile, PStream::raw_ascii, "w");
            out << "PLEARN_SERVER_TCP " << myhostname << " " << port << " " << mypid << endl;            
            out = 0; // close it
            PR_Rename(tmpoutfile.c_str(), outfile.c_str());
        }
        else
            pout << "PLEARN_SERVER_TCP " << myhostname << " " << port << " " << mypid << endl;

        NORMAL_LOG << "PLEARN_SERVER STARTING IN TCP MODE ON "  << myhostname << ", PORT " << port << ", PID " << mypid << endl;
        NORMAL_LOG << "singleuse = " << singleuse << endl;

        bool running = true;
        do  {
            NORMAL_LOG << "\nPLEARN_SERVER WAITING FOR CONNECTION"  << endl;
            
            ///***///***
            pout << " JBL " << endl;
            ///***///***

            st = PR_Listen(sock,0);
            if(st!=PR_SUCCESS)
                PLERROR("serverCommand: listen on socket failed");
            PRNetAddr addr;
            PRFileDesc* fd = PR_Accept(sock, &addr, PR_INTERVAL_NO_TIMEOUT);
            if(fd==0)
                PLERROR("ServerCommand: accept returned 0, error code is: %d : %s",
                        PR_GetError(), getPrErrorString().c_str());
            st = PR_NetAddrToString(&addr, buf, sizeof(buf));
            NORMAL_LOG << "PLEARN_SERVER CONNECTION_FROM "  << buf << endl;
            PStream io(new PrPStreamBuf(fd,fd,true,true));

            PLearnServer server(io);
            running = server.run();
            io.flush();
            if (PR_Close(fd) != PR_SUCCESS)
                PLERROR("ServerCommand: couldn't close client socket from %s!", buf);

        } while(running && !singleuse);

        NORMAL_LOG << "PLEARN_SERVER CLOSING SOCKET" << endl;
        if (PR_Shutdown(sock, PR_SHUTDOWN_BOTH) != PR_SUCCESS)
            PLERROR("ServerCommand: couldn't shutdown socket %d!", port);
        if (PR_Close(sock) != PR_SUCCESS)
            PLERROR("ServerCommand: couldn't close port %d!", port);
    }
    else // Start stdin/stdout server
    {
        perr << "Type !? to get some help." << endl;
        // PStream io(&std::cin, &std::cout);
        // PStream io(new StdPStreamBuf(&std::cin,&std::cout));
        // PStream io(new FdPStreamBuf(0,1));
        PStream io = get_pio();
        io.setMode(PStream::plearn_ascii);
        PLearnServer server(io);
        server.run();
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
