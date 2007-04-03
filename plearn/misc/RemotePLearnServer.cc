// -*- C++ -*-

// RemotePLearnServer.cc
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

/*! \file RemotePLearnServer.cc */


#ifndef BUGGED_SERVER
#include "RemotePLearnServer.h"
#include "PLearnService.h"
#include <plearn/io/pl_log.h>

namespace PLearn {
using namespace std;

RemotePLearnServer::RemotePLearnServer(const PStream& serverio)
    :io(serverio)
{
    io.remote_plearn_comm= true;
}
  
void RemotePLearnServer::clearMaps()
{
    io.clearInOutMaps();
    // copy local object map to stream's out map
    for(ObjMap::iterator it= objmap.begin(); it != objmap.end(); ++it)
        io.copies_map_out[it->second]= it->first;
}
  
void RemotePLearnServer::link(unsigned int objid, void* obj)
{ 
    io.copies_map_out[obj]= objid;
    objmap[objid]= obj; 
    rev_objmap[obj]= objid;

    DBG_LOG << "copies map: " << objmap << endl;
}

void RemotePLearnServer::unlink(unsigned int objid)
{ 
    ObjMap::iterator it= objmap.find(objid);
    if(it == objmap.end())
        PLERROR("in RemotePLearnServer::unlink : cannot unlink an object which is not linked");
    objmap.erase(it);
    rev_objmap.erase(it->second);
    io.copies_map_out.erase(it->second);
}

void RemotePLearnServer::unlink(void* obj)
{ 
    ReverseObjMap::iterator it= rev_objmap.find(obj);
    if(it == rev_objmap.end())
        PLERROR("in RemotePLearnServer::unlink : cannot unlink an object which is not linked");
    rev_objmap.erase(it);
    objmap.erase(it->second);
    io.copies_map_out.erase(obj);
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

int RemotePLearnServer::newObject(const Object& model)
{ 
    clearMaps();
    io.write("!O "); io << model << endl;
    int objid;
    getResults(objid);
    return objid;
}

int RemotePLearnServer::newObject(PP<Object> model)
{
    if(model.isNull())
        PLERROR("In RemotePLearnServer::newObject model is a Null pointer");
    return newObject(*model);
}

int RemotePLearnServer::newObject(const string& description)
{ 
    clearMaps();
    io.write("!O "); 
    io.write(description);
    io << endl;
    int objid;
    getResults(objid);
    return objid;
}


void RemotePLearnServer::newObjectAsync(int objid, const Object& model)
{ 
    clearMaps();
    io.write("!N "); io << objid << model << endl;
}

void RemotePLearnServer::newObjectAsync(int objid, PP<Object> model)
{
    if(model.isNull())
        PLERROR("In RemotePLearnServer::newObject model is a Null pointer");
    newObjectAsync(objid, *model);
}

void RemotePLearnServer::newObjectAsync(int objid, const string& description)
{ 
    clearMaps();
    io.write("!N "); io << objid; io.put(' ');
    io.write(description);
    io << endl;
}

void RemotePLearnServer::newObjectAsync(const Object& model)
{ 
    clearMaps();
    io.write("!O "); io << model << endl;
}

void RemotePLearnServer::newObjectAsync(const PP<Object>& model)
{
    if(model.isNull())
        PLERROR("In RemotePLearnServer::newObject model is a Null pointer");
    newObjectAsync(*model);
}

void RemotePLearnServer::newObjectAsync(const string& description)
{ 
    clearMaps();
    io.write("!O "); 
    io.write(description);
    io << endl;
}



void RemotePLearnServer::deleteObject(int objid)
{
    deleteObjectAsync(objid);
    expectResults(0);
}

void RemotePLearnServer::deleteObjectAsync(int objid)
{
    io.write("!D "); io << objid << endl;
}

void RemotePLearnServer::deleteAllObjects()
{
    deleteAllObjectsAsync();
    getResults();
/*
    if(io)
    {
        io.write("!Z "); 
        io << endl;
        expectResults(0);
    }
    else
        DBG_LOG << "in RemotePLearnServer::deleteAllObjects() : stream not good." << endl;
*/
}

void RemotePLearnServer::deleteAllObjectsAsync()
{
    if(io)
    {
        io.write("!Z "); 
        io << endl;
    }
    else
        DBG_LOG << "in RemotePLearnServer::deleteAllObjectsAsync() : stream not good." << endl;
}


void RemotePLearnServer::expectResults(int nargs_expected)
{
    PLearnService& service= PLearnService::instance();
    service.waitForResultFrom(this);

    //DBG_LOG << "RemotePLearnServer entering expectResults" << endl;
    io.skipBlanksAndComments();
    int headchar = io.get();
    if(headchar!='!')
        PLERROR(" Answers from plearn server are expected to start with a !, but I received a %c",headchar);
    int command = io.get();
    //DBG_LOG << "RemotePLearnServer expectResults received command: " << (char)command << endl;
    int nreturned;
    string msg;
    switch(command)
    {
    case 'R':
        io >> nreturned;
        //DBG_LOG << "RemotePLearnServer expectResults nreturned= " << nreturned << endl;
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
    // The PLearnService is responsible for most of RemotePLearnServer destruction 
    DBG_LOG << "ENTERING RemotePLearnServer destructor" << endl;
    io.remote_plearn_comm= false;
    DBG_LOG << "LEAVING RemotePLearnServer destructor" << endl;
}


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
