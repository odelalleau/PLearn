// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2002 Frederic Morin
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
   * $Id: IPopen.cc,v 1.2 2002/08/08 22:54:05 morinf Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

#include <sys/types.h>
#if !defined(_MSC_VER) && !defined(_MINGW_)
#include <sys/wait.h>
#endif
#include "stringutils.h"
#include "IPopen.h"

namespace PLearn <%
using namespace std;

#ifndef _MINGW_
    // Default values for static (state) variables
    int IPServer::ip_port = 15000;
    int IPServer::max_connections = 100;

    void IPopen::launch(IPServer &server, const string& command) {
        if (verbose)
            cout << "IPopen launches:" << endl << command << endl;

        if (fork() == 0) {
            // Child process
            system(command.c_str());
            exit(1);
        }
        establish_communication(server);
    }

    IPopen::~IPopen() {
        pipe.close(); // Not strictly necessary
        shutdown(socket_fd, 2);
        close(socket_fd);
    }

    // Wait for client to connect with server
    void
    IPopen::establish_communication(IPServer &server)
    {
        int addr_len = sizeof(struct sockaddr_in);
        
        // Wait for some client to connect
        socket_fd = accept(server.get_socket_fd(),
			   (struct sockaddr *)server.get_address(),
#ifndef SGI
			   (socklen_t *)&addr_len);
#else //def SGI
			   &addr_len);
#endif //ndef SGI
        if (socket_fd <= 0)
            PLERROR("Failure to connect with client");

        pipe.attach(socket_fd);
        pipe.setbuf(0, 0); // Somehow this solves some problems
    }

    // Called from the client program to establish communication
    // with the server.
    int
    establish_connection(int n_hosts, const char *hostnames[], int port_no)
    {
        struct sockaddr_in address;
        struct hostent *hostinfo;

        // Setup socket
        int server_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (server_socket <= 0)
            PLERROR("Cannot create socket");

        address.sin_family = AF_INET;
        address.sin_port = htons(port_no);

        for (int i = 0; i < n_hosts; ++i) {
            hostinfo = gethostbyname(hostnames[i]);
            if (!hostinfo)
                inet_pton(AF_INET, hostnames[i], &address.sin_addr);
                //address.sin_addr.s_addr = inet_addr(hostnames[i]);
            else
                address.sin_addr = *(struct in_addr *)*hostinfo->h_addr_list;

            // Connect to server
            if (connect(server_socket, (struct sockaddr *)&address, sizeof(address)))
                // Try next one
                continue;
            
            // Not sure this makes a difference here...
            int nodelay = 1;
            setsockopt(server_socket, IPPROTO_TCP, TCP_NODELAY, (char *)nodelay, sizeof(int));

            // Return socket descriptor
            return server_socket;
        }
        PLERROR("Connection to server failed");
        return -1; // Never reached
    }
#endif // ~_MINGW_

%> // end of namespace PLearn
