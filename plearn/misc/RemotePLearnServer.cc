// -*- C++ -*-

// RemotePLearnServer.cc
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
   * $Id: RemotePLearnServer.cc,v 1.1 2005/06/10 15:45:11 plearner Exp $ 
   ******************************************************* */

// Authors: Pascal Vincent

/*! \file RemotePLearnServer.cc */


#include "RemotePLearnServer.h"
#include "PLearnService.h"
#include <plearn/io/pl_log.h>

namespace PLearn {
using namespace std;

  RemotePLearnServer::RemotePLearnServer(const PStream& serverio)
    :io(serverio)
  {}
  
  void RemotePLearnServer::clearMaps()
  {
    io.copies_map_in.clear();
    io.copies_map_out.clear();
  }
  
  void RemotePLearnServer::newObject(int objid, const Object& model)
  { 
    clearMaps();
    io.write("!N "); io << objid << model << endl;
    expectResults(0);
  }

  void RemotePLearnServer::newObject(int objid, PP<Object> model)
  {
    if(model.isNull())
      PLERROR("In RemotePLearnServer::newObject model is a Null pointer");
    newObject(objid, *model);
  }

  void RemotePLearnServer::newObject(int objid, const string& description)
  { 
    clearMaps();
    io.write("!N "); io << objid; io.put(' ');
    io.write(description);
    io << endl;
    expectResults(0);
  }

  void RemotePLearnServer::deleteObject(int objid)
  {
    io.write("!D "); io << objid << endl;
    expectResults(0);
  }

  void RemotePLearnServer::deleteAllObjects()
  {
    io.write("!Z "); 
    io << endl;
    expectResults(0);
  }


  void RemotePLearnServer::expectResults(int nargs_expected)
  {
    DBG_LOG << "Entering expectResults" << endl;
    io.skipBlanksAndComments();
    int headchar = io.get();
    if(headchar!='!')
      PLERROR("Answers from plearn server are expected to start with a !, but I received a %c",headchar);
    int command = io.get();
    DBG_LOG << "expectResults received command: " << (char)command << endl;
    int nreturned;
    string msg;
    switch(command)
      {
      case 'R':
        io >> nreturned;
        if(nreturned!=nargs_expected)
          PLERROR("RemotePLearnServer: expected %d return arguments, but read R %d",nargs_expected,nreturned);
        break;
      case 'E':
        io >> msg;
        PLERROR(msg.c_str());
        break;
      default:
        PLERROR("RemotePLearnServer: expected R (return command), but read %c ????",command);
      }
  }

  RemotePLearnServer::~RemotePLearnServer()
  {
    DBG_LOG << "ENTERING RemotePLearnServer destructor" << endl;
    // io.write("!Q");
    // io = 0;
    // DBG_LOG << "BEFORE wait" << endl;
    // prg->wait();
    // DBG_LOG << "AFTER wait" << endl;
    PLearnService::instance().freeServer(this);
    DBG_LOG << "LEAVING RemotePLearnServer destructor" << endl;
  }


} // end of namespace PLearn
