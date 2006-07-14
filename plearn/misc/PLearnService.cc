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
 * $Id$ 
 ******************************************************* */

// Authors: Pascal Vincent

/*! \file PLearnService.cc */


#include "PLearnService.h"
#include "RemotePLearnServer.h"
#include <plearn/io/openFile.h>
#include <plearn/io/openSocket.h>
#include <plearn/io/pl_log.h>
#include <plearn/io/Poll.h>

namespace PLearn {
using namespace std;


/*
  void PLearnService::remoteLaunchServers(int nservers, int tcpport, const string& launch_command)
  {
  string full_command = launch_command+'--tcp '+tostring(tcpport);
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
    string pid;
    int tcpport = -1;

    TVec< pair<string,int> > hostname_and_port;

    while(in)
    {
        in.skipBlanksAndComments();
        if(!bool(in))
            break;
        in >> hostname >> tcpport >> pid;
        if(in)
            hostname_and_port.append(pair<string,int>(hostname,tcpport));
    }
    connectToServers(hostname_and_port);
}

void PLearnService::connectToServers(TVec< pair<string,int> > hostname_and_port)
{
    if(available_servers.size()>0)
        disconnectFromServers();
    for(int k=0; k<hostname_and_port.length(); k++)
    {
        pair<string, int> host_port = hostname_and_port[k];
        PStream servio = openSocket(host_port.first, host_port.second, PStream::plearn_binary);
        // PStream servio = openSocket(host_port.first, host_port.second, PStream::plearn_ascii);
        PP<RemotePLearnServer> serv = new RemotePLearnServer(servio);
        serv->callFunction("binary");
        serv->expectResults(0);
        available_servers.push(serv);
        //serversio.append(servio);
        //available_servers.push(k);
    }
}

void PLearnService::disconnectFromServers()
{
    available_servers = TVec< PP<RemotePLearnServer> >();
}


int PLearnService::availableServers() const
{    
    return available_servers.size();
}

/*
  RemotePLearnServer* PLearnService::OLD_reserveServer()
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
*/

PP<RemotePLearnServer> PLearnService::reserveServer()
{
    if(available_servers.size()==0)
        return 0;
    PP<RemotePLearnServer> serv = available_servers.pop();
    reserved_servers.insert(serv);
    return serv;
}

TVec< PP<RemotePLearnServer> > PLearnService::reserveServers(int nservers)
{
    TVec< PP<RemotePLearnServer> > servers;
    while(servers.length()<nservers)
    {
        PP<RemotePLearnServer> serv = reserveServer();          
        if(serv.isNull())
            break;
        servers.append(serv);
    }
    return servers;
}

void PLearnService::freeServer(PP<RemotePLearnServer> server)
{
    DBG_LOG << "PLearnService::freeServer(...)" << endl;
    server->clearMaps();
    server->deleteAllObjects();
    if(reserved_servers.erase(server)!=1)
        PLERROR("Problem in PLearnService::freeServer are you sure this server had been properly reserved?");
    available_servers.push(server);
}
    
void PLearnService::freeServers(TVec< PP<RemotePLearnServer> > servers)
{
    for(int k=0; k<servers.length(); k++)
        freeServer(servers[k]);
}

int PLearnService::watchServers(TVec< PP<RemotePLearnServer> > servers, int timeout)
{
    Poll p;
    int n = servers.size();
    vector<PStream> streams(n);
    for(int k=0; k<n; k++)
        streams[k] = servers[k]->io;
    p.setStreamsToWatch(streams);

    PRInt32 npending = p.waitForEvents(timeout);
    if(npending<=0)
        return -1;
    
    PStream io = p.getNextPendingEvent();    
    for(int k=0; k<n; k++)
        if(streams[k] == io)
            return k;
    PLERROR("stream returned by NextPendingEvent is none of the servers' io field. This should not happen!");
    return -1;  // To make the compiler happy (never reached).
}

/*
  void PLearnService::freeServer(RemotePLearnServer* remoteserv)
  {
  DBG_LOG << "PLearnService::freeServer(" << (unsigned long) remoteserv << ")" << endl;
  remoteserv->clearMaps();
  remoteserv->deleteAllObjects();
  std::map<RemotePLearnServer*,int>::iterator it = reserved_servers.find(remoteserv);
  if(it==reserved_servers.end())
  PLERROR("Strange bug in PLearnService::freeServer Nothing known about this remoteserv. This should never happen!");
  // put servernum back in available servers    
  available_servers.push(it->second);
  // erase servernum from reserved_servers
  reserved_servers.erase(it);
  }
*/

PLearnService::~PLearnService()
{
    disconnectFromServers();
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
