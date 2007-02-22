// -*- C++ -*-

// PLearnService.cc
//
// Copyright (C) 2005 Pascal Vincent 
// Copyright (C) 2007 Xavier Saint-Mleux, ApSTAT Technologies inc.
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

    DBG_LOG << "PLearnService::connectToServers(" << serversfile << ')' << endl;

    string hostname;
    string pid;
    int tcpport = -1;

    TVec< pair<string,int> > hostname_and_port;

    while(in)
    {
        in.skipBlanksAndComments();
        if(!in)
            break;
        in >> hostname >> tcpport >> pid;
        if(in)
            hostname_and_port.append(pair<string,int>(hostname,tcpport));
    }
    connectToServers(hostname_and_port);
}

void PLearnService::connectToServers(TVec< pair<string,int> > hostname_and_port)
{

    DBG_LOG << "PLearnService::connectToServers(" << hostname_and_port << ')' << endl;

    if(available_servers.size()>0)
        disconnectFromServers();
    for(int k=0; k<hostname_and_port.length(); k++)
    {
        pair<string, int> host_port = hostname_and_port[k];
        PStream servio = openSocket(host_port.first, host_port.second, PStream::plearn_ascii);
        PP<RemotePLearnServer> serv = new RemotePLearnServer(servio);
        serv->callFunction("binary");
        
        TVec<PP<RemotePLearnServer> > ss;
        ss.push_back(serv);

        watchServers(ss, log_callback, progress_callback);

        servio << PStream::plearn_binary;

        reserved_servers.insert(serv);
        serv->getResults();
        reserved_servers.erase(serv);
        available_servers.push(serv);
    }
}

void PLearnService::disconnectFromServers()
{
    //available_servers = TVec< PP<RemotePLearnServer> >();
    while(available_servers.length() > 0)
        disconnectFromServer(available_servers[0]);
}


void PLearnService::disconnectFromServer(PP<RemotePLearnServer> server)
{
    for(int i= 0; i < available_servers.length(); ++i)
        if(available_servers[i] == server)
        {
            available_servers.remove(i);
            server->io.write("!Q ");
            server->io << endl;
            if(progress_bars.find(server) != progress_bars.end())
                progress_bars.erase(server);
            return;
        }
    PLERROR("PLearnService::disconnectFromServer : trying to disconnect from a server which is not available"
            " (not connected to or reserved)");
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
    server->deleteAllObjects();
    server->clearMaps();
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


int PLearnService::watchServers(TVec< PP<RemotePLearnServer> > servers, 
                                log_callback_t log_callback, 
                                progress_callback_t progress_callback)
{
    Poll p;
    int n = servers.size();
    vector<PStream> streams(n);
    for(int k=0; k<n; k++)
        streams[k] = servers[k]->io;
    p.setStreamsToWatch(streams);

    for(;;)
    {
        PRInt32 npending = p.waitForEvents(PR_INTERVAL_NO_TIMEOUT, true);
        if(npending<=0)
            return -1;

        int the_k= -1;
        PStream io = p.getNextPendingEvent();    

        for(int k=0; k<n; k++)
            if(streams[k] == io)
                the_k= k;

        if(the_k >= 0)
        {
            int c= io.peek();
            // skip blanks one at a time and poll again
            if(static_cast<char>(c) == ' ' || static_cast<char>(c) == '\t' 
               || static_cast<char>(c) == '\n' || static_cast<char>(c) == '\r') 
            {
                io.get();
                continue;
            }
            else if(static_cast<char>(c) == '*') //async. message (log or progress)
            {
                io.get(); // get '*'
                c= io.get();// get msg type ('L'og or 'P'rogress)

                int vlevel, c0;
                unsigned int pos, ptr;
                string mesg;
                string module("");
                switch(static_cast<char>(c))
                {
                case 'L' : // log message
                    io >> module >> vlevel >> mesg;
                    log_callback(servers[the_k], module, vlevel, mesg);
                    break;
                case 'P' : // progress message
                    c0= io.get(); // action: 'A'dd, 'U'pdate or 'K'ill
                    io >> ptr;// pbar id.
                    if(static_cast<char>(c0) != 'K')
                        io >> pos;// Add: maxpos; Update: curpos
                    if(static_cast<char>(c0) == 'A')
                        io >> mesg;// pbar title
                    progress_callback(servers[the_k], ptr, static_cast<char>(c0), pos, mesg);
                    break;
                default:
                    PLERROR("PLearnService::watchServers : Expected *L or *P, received *%c", c);
                    break;
                }
            }
            else //synchronous message, return server's id
            {
                return the_k;
            }
        }
        else
            PLERROR("stream returned by NextPendingEvent is none of the servers' io field. This should not happen!");
    }
    return -1;  // To make the compiler happy (never reached).
}



PP<RemotePLearnServer> PLearnService::waitForResult(TVec< PP<RemotePLearnServer> > servers, 
                                log_callback_t log_callback, 
                                progress_callback_t progress_callback)
{
    int min_server= 0;
    if(servers.isEmpty())
    {
        servers= available_servers;
        min_server= available_servers.length();
        for(std::set<PP<RemotePLearnServer> >::iterator it= reserved_servers.begin();
            it != reserved_servers.end(); ++it)
            servers.push_back(*it);
    }

    if(servers.isEmpty())
        PLERROR("in PLearnService::waitForResult : cannot wait for a result"
                " when you are not connected to any server.");
    int server= servers.length();

    //send results from reserved servers only even if polling all servers
    while(server >= 0 && server < min_server || server == servers.length())
        server= watchServers(servers, log_callback, progress_callback);

    if(server < 0)
        PLERROR("in PLearnService::waitForResult : no server returned anything.");
    return servers[server];
}


void PLearnService::waitForResultFrom(PP<RemotePLearnServer> from,
                                      log_callback_t log_callback,
                                      progress_callback_t progress_callback)
{
    PP<RemotePLearnServer> server= waitForResult();
    while(server != from)
        server= waitForResult();
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
    if(reserved_servers.size() != 0)
        PLERROR("PLearnService::~PLearnService : some servers are still reserved; free them first.");

    TVec<PP<RemotePLearnServer> > servers(available_servers.length());
    servers << available_servers;

    disconnectFromServers();

    //now, get what's remaining on the servers streams
    for(int i= 0; i < servers.length(); ++i)
    {
        try
        {
            for(;;) watchServers(servers, log_callback, progress_callback);
        }
        catch(const PLearnError& e)
        {
            // do nothing...
        }
    }
}


void PLearnService::log_callback(PP<RemotePLearnServer> server, const string& module_name, int vlevel, const string& msg)
{ 
    unsigned int server_id= reinterpret_cast<unsigned int>(static_cast<RemotePLearnServer*>(server));
    PL_LOG(vlevel) << "<From server " << server_id << "> [" << module_name << "] " << msg << flush; 
}

PLearnService::progress_bars_t PLearnService::progress_bars; // init

void PLearnService::progress_callback(PP<RemotePLearnServer> server, unsigned int pbar, char action, 
                                      unsigned int pos, const string& title)
{
    unsigned int server_id= reinterpret_cast<unsigned int>(static_cast<RemotePLearnServer*>(server));
    static bool need_to_set_pb_plugin= true;
    if(need_to_set_pb_plugin)
    {
        ProgressBar::setPlugin(new LineOutputProgressBarPlugin(cerr));
        need_to_set_pb_plugin= false;
    }

    switch(action)
    {
    case 'A': // add new progress bar
        if(progress_bars.find(server) == progress_bars.end())
            progress_bars[server]= map<unsigned int, PP<ProgressBar> >();
        {//local environment for 'fulltitle'... silly c++ switch/case...
            string fulltitle= string("<server#") + tostring(server_id) 
                + ":pb#" + tostring(pbar) + "> " + title;//adjust title w/server info
            progress_bars[server][pbar]= new ProgressBar(fulltitle, pos);
        }
        break;
    case 'U': // update progress bar
        progress_bars[server][pbar]->update(pos);
        break;
    case 'K': // kill progress bar
        progress_bars[server].erase(pbar);
        break;
    default:
        PLERROR("in PLearnService::progress_callback: unknown action %c", action);
        break;
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
