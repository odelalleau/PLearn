// -*- C++ -*-

// PLearnService.cc
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
   * $Id: PLearnService.cc,v 1.4 2005/06/09 21:12:41 plearner Exp $ 
   ******************************************************* */

// Authors: Pascal Vincent

/*! \file PLearnService.cc */


#include "PLearnService.h"
#include "RemotePLearnServer.h"
#include <plearn/io/openFile.h>
#include <plearn/io/openSocket.h>
#include <plearn/io/pl_log.h>

namespace PLearn {
using namespace std;


  /*
  void PLearnService::remoteLaunchServers(int nservers, int tcpport, const string& launch_command)
  {
    string full_command = launch_command+'--tcp '+tostring2(tcpport);
    for(int k=0; k<nservers; k++)
      {
        PP<Popen> p = new Popen(full_command);
      }
    reserved_servers.add()
  }
  */

  PLearnService& PLearnService::instance()
  {
    static PP<PLearnService> inst;
    if(inst.isNull())
      inst = new PLearnService();
    return *inst;
  }
  
  PLearnService::PLearnService()
  {}


  void PLearnService::connectToServers(PPath serversfile)
  {
    PStream in = openFile(serversfile, PStream::raw_ascii, "r");

    string hostname;
    int pid = 0;
    int tcpport = -1;

    TVec< pair<string,int> > hostname_and_port;

    while(in)
      {
        in >> hostname >> pid >> tcpport;
        if(in)
          hostname_and_port.append(pair<string,int>(hostname,pid));
      }
    connectToServers(hostname_and_port);
  }

  void PLearnService::connectToServers(TVec< pair<string,int> > hostname_and_port)
  {
    if(serversio.length()>0)
      disconnectFromServers();
    for(int k=0; k<hostname_and_port.length(); k++)
      {
        pair<string, int> host_port = hostname_and_port[k];
        PStream sock = openSocket(host_port.first, host_port.second, PStream::plearn_binary);
        serversio.append(sock);
        available_servers.push(k);
      }
  }

  void PLearnService::disconnectFromServers()
  {
    serversio = TVec<PStream>();
    available_servers.resize(0);
  }


  int PLearnService::availableServers() const
  {    
    return available_servers.length();
  }

  RemotePLearnServer* PLearnService::reserveServer()
  {
    RemotePLearnServer* serv = 0;
    if(available_servers.size()>0)
      {
        int servnum = available_servers.pop();
        serv = new RemotePLearnServer(serversio[servnum]);
        reserved_servers[serv] = servnum;
      }
    return serv;
  }

  void PLearnService::freeServer(RemotePLearnServer* remoteserv)
  {
    DBG_LOG << "PLearnService::freeServer(" << (unsigned long) remoteserv << endl;
    std::map<RemotePLearnServer*,int>::iterator it = reserved_servers.find(remoteserv);
    if(it==reserved_servers.end())
      PLERROR("Strange bug in PLearnService::freeServer Nothing known about this remoteserv. This should never happen!");
    // put servernum back in available servers
    available_servers.push(it->second);
    // erase servernum from reserved_servers
    reserved_servers.erase(it);
  }

  PLearnService::~PLearnService()
  {
    disconnectFromServers();
  }

} // end of namespace PLearn
