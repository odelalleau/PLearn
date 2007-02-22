// -*- C++ -*-

// PLearnService.h
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

/*! \file PLearnService.h */


#ifndef PLearnService_INC
#define PLearnService_INC

#include <plearn/base/PP.h>
#include <plearn/io/PPath.h>
#include <plearn/io/PStream.h>
#include <plearn/math/TVec.h>
#include <map>
#include <set>
#include <string>
#include <plearn/misc/RemotePLearnServer.h>

namespace PLearn {

//  class RemotePLearnServer;

class PLearnService: public PPointable
{
private:
    PLearnService();

    // TVec<PStream> serversio; 
    // TVec<int> available_servers;
    // std::map<RemotePLearnServer*,int> reserved_servers;

    TVec< PP<RemotePLearnServer> > available_servers;
    std::set< PP<RemotePLearnServer> > reserved_servers;

    typedef map<RemotePLearnServer*, map<unsigned int, PP<ProgressBar> > > progress_bars_t;
    static progress_bars_t progress_bars;

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

    void disconnectFromServer(PP<RemotePLearnServer> server);

    //! returns the number of available processing ressources
    int availableServers() const;

    //! Reserves a remote processing ressource from the pool of servers.
    //! If sucessful returns a pointer to a new RemotePLearnServer
    //! If no server is available, returns 0.
    PP<RemotePLearnServer> reserveServer(); 

    //! Reserves up to nservers servers
    TVec< PP<RemotePLearnServer> > reserveServers(int nservers);

    //! Frees a previously reserved servers.
    //! Putting it back into the list of available_servers
    void freeServer(PP<RemotePLearnServer> server);
    
    //! Frees all the servers in the list
    void freeServers(TVec< PP<RemotePLearnServer> > servers);

    int watchServers(TVec< PP<RemotePLearnServer> > servers, int timeout=0);

    typedef void (*log_callback_t)(PP<RemotePLearnServer> server, const string& module_name, int vlevel, const string& msg);
    typedef void (*progress_callback_t)(PP<RemotePLearnServer> server, unsigned int pbar, char action, 
                                        unsigned int pos, const string& title);

    static void log_callback(PP<RemotePLearnServer> server, const string& module_name, int vlevel, const string& msg);
    static void progress_callback(PP<RemotePLearnServer> server, unsigned int pbar, char action, 
                                  unsigned int pos= 0, const string& title= "");

    int watchServers(TVec< PP<RemotePLearnServer> > servers, 
                     log_callback_t log_callback,
                     progress_callback_t progress_callback);

    PP<RemotePLearnServer> waitForResult(TVec< PP<RemotePLearnServer> > servers= TVec< PP<RemotePLearnServer> >(), 
                                         log_callback_t log_callback= log_callback,
                                         progress_callback_t progress_callback= progress_callback);

    void waitForResultFrom(PP<RemotePLearnServer> from,
                           log_callback_t log_callback= log_callback,
                           progress_callback_t progress_callback= progress_callback);
    
    ~PLearnService();
};

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
