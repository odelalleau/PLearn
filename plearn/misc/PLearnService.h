// -*- C++ -*-

// PLearnService.h
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
   * $Id: PLearnService.h,v 1.3 2005/06/09 21:12:41 plearner Exp $ 
   ******************************************************* */

// Authors: Pascal Vincent

/*! \file PLearnService.h */


#ifndef PLearnService_INC
#define PLearnService_INC

#include <plearn/base/PP.h>
#include <plearn/io/PPath.h>
#include <plearn/io/PStream.h>
#include <plearn/math/TVec.h>
#include <map>
#include <string>

namespace PLearn {

  class RemotePLearnServer;

class PLearnService: public PPointable
{
private:
  PLearnService();
  TVec<PStream> serversio; 
  TVec<int> available_servers;
  std::map<RemotePLearnServer*,int> reserved_servers;

  //! Frees a previously reserved servers.
  //! Putting it back into the list of available_servers
  //! This is called automatically by the RemotePLearnServer's destructor
  void freeServer(RemotePLearnServer* remoteserv);

public:
  friend class RemotePLearnServer;

  //  static void remoteLaunchServers(PPath serverfile, int nservers, int tcpport, const string& launch_command);

  // Returns the unique static instance of class PLearnService
  static PLearnService& instance();

  //! Will establish a TCP connection to all hostname,port in given list
  void connectToServers(TVec< pair<string,int> > server_names_and_port);
  
  //! Will get the list of hostname, pid, port from the serversfile
  //! and establish a TCP connection to those
  void connectToServers(PPath serversfile);

  void disconnectFromServers();

  //! returns the number of available processing ressources
  int availableServers() const;

  //! Reserves a remote processing ressource from the pool of servers.
  //! If sucessful returns a pointer to a new RemotePLearnServer
  //! If no server is available, returns 0.
  RemotePLearnServer* reserveServer(); 

  ~PLearnService();
};

} // end of namespace PLearn

#endif
