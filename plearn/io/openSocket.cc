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
   * $Id: openSocket.cc,v 1.2 2005/01/05 19:15:55 chrish42 Exp $ 
   ******************************************************* */

// Authors: Christian Hudon

/*! \file openSocket.cc */

#include <string>
#include <plearn/io/PStream.h>
#include <plearn/io/PrPStreamBuf.h>
#include "openSocket.h"
#include <mozilla/nspr/prio.h>
#include <mozilla/nspr/prerror.h>
#include <mozilla/nspr/prnetdb.h>


namespace {

/** Utility function that returns a std::string describing the last
 *  NSPR error.
 *  @todo Move to a separate file if the function finds other users.
 */
std::string getPrErrorString()
{
  const int text_length = PR_GetErrorTextLength();
  if (text_length == 0)
    {
      return "[No error]";
    }
  
  char* s = new char[text_length+1];
  PR_GetErrorText(s);
  
  std::string error_string = s;
  delete[] s;
  return error_string;
} 

}

namespace PLearn {
using namespace std;

PStream openSocket(const string& hostname, int port,
                   PStream::mode_t io_formatting,
                   const int timeout)
{
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
      if (PR_Connect(socket, &address, timeout) == PR_SUCCESS)
        {
          PStream st = new PrPStreamBuf(socket, socket, true, true);
          st.setMode(io_formatting);
          return st;
        }
    }

  PLERROR("openSocket(%s, %d) failed while trying to connect: %s",
          hostname.c_str(), port, getPrErrorString().c_str());
}

} // end of namespace PLearn
