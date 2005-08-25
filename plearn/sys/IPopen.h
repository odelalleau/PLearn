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
 * $Id$
 * This file is part of the PLearn library.
 ******************************************************* */

#ifndef IPopen_INC
#define IPopen_INC

#include <fstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <plearn/base/PP.h>
#include <plearn/base/general.h> 

#ifndef _MINGW_
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#endif

namespace PLearn {
using namespace std;

#ifndef _MINGW_
class IPServer {
public:
    IPServer(int port_no, int max_connections) {
        DelayedConstructor(port_no, max_connections);
    };
    IPServer(int max_connections_) {
        DelayedConstructor(ip_port, max_connections_);
    };
    IPServer() {
        DelayedConstructor(ip_port, max_connections);
    };
    ~IPServer() {
        shutdown(socket_fd, 2);
        close(socket_fd);
    };

    int get_socket_fd() { return socket_fd; };
    struct sockaddr_in *get_address() { return &address; };
    string machine_name() { return hostname(); };
    int port_no() { return port; };
        
protected:
    int socket_fd;
    int port;
    struct sockaddr_in address;
private:
    void DelayedConstructor(int port_no, int max_connections_) {
        port = port_no;

        int addr_len = sizeof(struct sockaddr_in);
            
        socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_fd <= 0)
            PLERROR("Cannot create socket");
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port_no);

        if (bind(socket_fd, (struct sockaddr *)&address, addr_len))
            PLERROR("Cannot bind socket"); // ... and in the darknest bind them
        listen(socket_fd, max_connections);
        int nodelay = 1;
        setsockopt(socket_fd, IPPROTO_TCP, TCP_NODELAY, (char *)nodelay, sizeof(int));
    }
public:
    static int ip_port;
    static int max_connections;
    static void set_ip_port(int port_no, int max_connections_ = 100) { IPServer::ip_port = port_no; IPServer::max_connections = max_connections_; };
}; // class IPServer


class IPopen: public PPointable {        
public:
    // Attributs
    PStream pipe;  // Bi-directional pipe

    // Methods
    IPopen(IPServer &server)
    { establish_communication(server); };
    IPopen(IPServer &server, const string &command, bool the_verbose = false) 
    { verbose = the_verbose; launch(server, command); }        
    ~IPopen();

    int get_socket_fd() const { return socket_fd; };

protected:
    //!  full text variant: this one is interpreted like a console
    //!  /bin/sh command
    void launch(IPServer &server, const string &command); 
    void establish_communication(IPServer &server);

    bool verbose;
    int socket_fd;
}; // class IPopen

// This is called by the client side to establish communication.
// Note that the command-line arguments from the client's program
// are directly sent to this function. This is because argument
// argv[1] and argv[2] should be, respectively, the server's machine
// name and the port number to use.
int establish_connection(const int n_hosts, const char *hostnames[], int port_no);
inline int establish_connection(const char *hostname, int port_no)
{ return establish_connection(1, &hostname, port_no); }
inline int establish_connection(const int argc, const char *argv[])
{
    if (argc >= 3) return establish_connection(1, &argv[1], atoi(argv[2]));
    PLERROR("Wrong number of arguments");
    return -1; // Dummy return value
}

#endif // ~_MINGW_

} // end of namespace PLearn

#endif


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
