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
   * $Id: PLearnService.cc,v 1.1 2005/01/07 18:18:14 plearner Exp $ 
   ******************************************************* */

// Authors: Pascal Vincent

/*! \file PLearnService.cc */


#include "PLearnService.h"
#include "RemotePLearnServer.h"
#include "plearn/io/PStream.h"

namespace PLearn {
using namespace std;

  PLearnService& PLearnService::instance()
  {
    static PP<PLearnService> inst;
    if(inst.isNull())
      inst = new PLearnService();
    return *inst;
  }


  PLearnService::PLearnService() 
  {
  }


  int PLearnService::availableServers() const
  {
    return 0;
  }

  RemotePLearnServer* PLearnService::newServer()
  {
    // search for a free processing ressource

    // open an io channel to a plearn server running on found ressource 
    PStream io;

    // return RemotePLearnServer object controlling the remote processing ressource
    RemotePLearnServer* serv = new RemotePLearnServer(io);
    
    reserved_servers.insert(serv);
    return serv;
  }

  void PLearnService::freeServer(RemotePLearnServer* remoteserv)
  {
    if(reserved_servers.erase(remoteserv)!=1)
      PLERROR("In PLearnService::freeServer, no such registered server. This should not happen!");
  }


} // end of namespace PLearn
