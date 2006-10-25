// -*- C++ -*-

// openUrl.cc
//
// Copyright (C) 2004 Pascal Vincent 
// Copyright (C) 2006 Olivier Delalleau
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
 * $Id: openUrl.cc 6002 2006-07-11 19:47:36Z plearner $ 
 ******************************************************* */

// Authors: Olivier Delalleau, Pascal Vincent and Christian Hudon

/*! \file openUrl.cc */


#include "openUrl.h"
#include <plearn/io/openSocket.h>

#define PL_LOG_MODULE_NAME "openUrl"
#include <plearn/io/pl_log.h>

namespace PLearn {
using namespace std;

/////////////
// openUrl //
/////////////
PStream openUrl(const PPath& url, PStream::mode_t io_formatting)
{
    // Connect to host.
    string hostname = url.hostname();
    PStream sock = openSocket(hostname, 80, PStream::raw_ascii);
    // Ask for document.
    size_t path_start = url.find(hostname);
    string path_on_server = url.substr(path_start + hostname.size());
    PLASSERT( !path_on_server.empty() );
    string http_request = "GET " + path_on_server + " HTTP/1.0\n\n";
    MODULE_LOG << "HTTP request to " << hostname << ": " << http_request
               << endl;
    sock.write(http_request);
    sock.flush();
    // Get answer.
    string token;
    sock >> token;  // HTTP/x.y type of document.
    sock >> token;  // Return code.
    // The current code is very basic and not very forgiving: if the server
    // does not answer 200 (everything is ok), then an error is thrown.
    // Future improvements would include taking redirection codes into account.
    if (token != "200")
        PLERROR("In openUrl(%s) - Received return code %s instead of 200",
                url.c_str(), token.c_str());
    while (!token.empty())
        sock.getline(token); // Skip other headers.
    // Set the required IO formatting mode.
    sock.setMode(io_formatting);
    return sock;
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
