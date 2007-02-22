// -*- C++ -*-

// openSocket.cc
//
// Copyright (C) 2004 Christian Hudon
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

// Authors: Christian Hudon

/*! \file openSocket.cc */

#include <string>
#include <plearn/base/PrUtils.h>
#include <plearn/io/PStream.h>
#include <plearn/io/PrPStreamBuf.h>
#include "openSocket.h"
#include <nspr/prio.h>
#include <nspr/prerror.h>
#include <nspr/prnetdb.h>

#define PL_LOG_MODULE_NAME "openSocket"
#include <plearn/io/pl_log.h>

namespace PLearn {
using namespace std;

/** Opens a socket and returns an attached PStream.
 *
 * @param hostname The name or IP address of the host to connect to.
 *
 * @param port The port to connect to.
 *
 * @param io_formatting The PStream formatting that will be used when
 * reading/writing to the socket. Common modes include PStream::raw_ascii
 * (for a normal ascii text file) and PStream::plearn_ascii (for files in
 * the PLearn serialization format).
 *
 * @param timeout Amount of time to wait for a connection (in seconds)
 * before giving up.
 */
PStream openSocket(const string& hostname, int port,
                   PStream::mode_t io_formatting,
                   const int timeout)
{
    PStream st;
    st.setMode(io_formatting);
  
    PRFileDesc* socket = PR_NewTCPSocket();

    if (!socket)
        PLERROR("openSocket: socket creation failed! (Maybe you ran out of file descriptors?)");

    // Look up the host name.
    PRHostEnt host;
    char buf[PR_NETDB_BUF_SIZE];
    if (PR_GetHostByName(hostname.c_str(), buf, sizeof(buf), &host) != PR_SUCCESS)
        PLERROR("openSocket(%s, %d) failed during host name lookup: %s",
                hostname.c_str(), port, getPrErrorString().c_str());

    // Iterate on every address for the host, until we can connect to one.
    int host_entry_index = 0;
    PRNetAddr address;
    while ((host_entry_index = PR_EnumerateHostEnt(host_entry_index, &host,
                                                   port, &address)) != 0)
    {
        if (PR_Connect(socket, &address, PR_SecondsToInterval(timeout))
                                                                == PR_SUCCESS)
        {
            return new PrPStreamBuf(socket, socket, true, true);
        } 
        else 
        {
#ifdef BOUNDCHECK
            string ip_adr = "Unknown IP address";
            if (PR_NetAddrToString(&address, buf, sizeof(buf)) == PR_SUCCESS)
                ip_adr = buf;
            MODULE_LOG << "Error trying to connect to host entry index "
                       << host_entry_index << " (" << ip_adr << "): "
                       << getPrErrorString() << endl;
#endif
        }
    }

    PLERROR("openSocket(%s, %d) failed while trying to connect: %s",
            hostname.c_str(), port, getPrErrorString().c_str());
    return st;
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
