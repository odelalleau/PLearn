// -*- C++ -*-

// RemotePLearnServer.h
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

/*! \file RemotePLearnServer.h */


#ifndef RemotePLearnServer_INC
#define RemotePLearnServer_INC

#include <plearn/base/Object.h>
#include <plearn/base/PP.h>
#include <plearn/io/PStream.h>
#include <plearn/sys/Popen.h>
#include <plearn/io/pl_log.h>

namespace PLearn {


class RemotePLearnServer: public PPointable
{    
private:
    friend class PLearnService;
    PStream io; // io communication channel with remote PLearnServer
    RemotePLearnServer(const PStream& serverio);

    typedef map<unsigned int, void*> ObjMap;
    ObjMap objmap;
    typedef map<void*, unsigned int> ReverseObjMap;
    ReverseObjMap rev_objmap;

public:

    void killServer() { io << "!K " << endl; }
    
    //! Builds an object based on the given model on the remote server,
    //! assigning it the given id.
    void newObject(int objid, const Object& model);

    void newObject(int objid, PP<Object> model);

    //! Builds an object on the remote server, from its description
    //! in serialised form.
    void newObject(int objid, const string& description);  

    //! Builds an object based on the given model on the remote server,
    //! returns an assigned id.
    int newObject(const Object& model);
    int newObject(PP<Object> model);
    //! Builds an object on the remote server, from its description
    //! in serialised form.
    int newObject(const string& description);  
    
    //! Builds an object based on the given model on the remote server,
    //! assigning it the given id.
    void newObjectAsync(int objid, const Object& model);
    void newObjectAsync(int objid, PP<Object> model);
    //! Builds an object on the remote server, from its description
    //! in serialised form.
    void newObjectAsync(int objid, const string& description);  

    //! Builds an object based on the given model on the remote server,
    //! id is assigned by the server and returned.
    void newObjectAsync(const Object& model);
    void newObjectAsync(const PP<Object>& model);
    //! Builds an object on the remote server, from its description
    //! in serialised form.
    void newObjectAsync(const string& description);  

    //! Deletes an object of the remote server.
    void deleteObject(int objid);

    //! Deletes an object of the remote server.
    void deleteObjectAsync(int objid);

    //! Deletes all objects of the remote server.
    void deleteAllObjects();

    //! Deletes all objects of the remote server.
    void deleteAllObjectsAsync();

    // object map related methods
    void clearMaps();
    // Should link/unlink be called automatically in newObject/deleteObject ? -xsm
    void link(unsigned int objid, void* obj);
    void unlink(unsigned int objid);
    void unlink(void* obj);

    //! Users generally won't have to call this, but rather one of the callFunction methods.
    inline void sendFunctionCallHeader(const string& function_name, int nargs)
    { 
        clearMaps();
        io.write("!F "); io << function_name << nargs; 
    }

    //! Users generally won't have to call this, but rather one of the callMethod methods.
    inline void sendMethodCallHeader(int objid, const string& method_name, int nargs)
    { 
        clearMaps();
        io.write("!M "); io << objid << method_name << nargs; 
    }
    
    //! Users generally won't have to call this, but rather one of the getResults methods.
    /*! Reads a 'R' (return) command (expecting to read 'R nargs_expected')
      If it gets a 'E' (error or exception command) it will thow a PLearnException.
      If the returned nargs differs from nargs_expected, it will also throw a PLearnException.
    */
    void expectResults(int nargs_expected);

    //! call a method with 0 input arguments
    inline void callFunction(const string& name)
    {
        sendFunctionCallHeader(name, 0);
        io << endl;
    }
    //! call a method with 1 input argument
    template<class Arg1>
    inline void callFunction(const string& name, const Arg1& arg1)
    {
        sendFunctionCallHeader(name, 1);
        io << arg1 << endl;
    }
    //! call a method with 2 input arguments
    template<class Arg1, class Arg2>
    inline void callFunction(const string& name, const Arg1& arg1, const Arg2& arg2)
    {
        sendFunctionCallHeader(name, 2);
        io << arg1 << arg2 << endl;
    }
    //! call a method with 3 input arguments
    template<class Arg1, class Arg2, class Arg3>
    inline void callFunction(const string& name, const Arg1& arg1, const Arg2& arg2, const Arg3& arg3)
    {
        sendFunctionCallHeader(name, 3);
        io << arg1 << arg2 << arg3 << endl;
    }
    //! call a method with 4 input arguments
    template<class Arg1, class Arg2, class Arg3, class Arg4>
    inline void callFunction(const string& name, const Arg1& arg1, const Arg2& arg2, const Arg3& arg3, const Arg4& arg4)
    {
        sendFunctionCallHeader(name, 4);
        io << arg1 << arg2 << arg3 << arg4 << endl;
    }
    //! call a method with 5 input arguments
    template<class Arg1, class Arg2, class Arg3, class Arg4, class Arg5>
    inline void callFunction(const string& name, const Arg1& arg1, const Arg2& arg2, const Arg3& arg3, const Arg4& arg4, const Arg5& arg5)
    {
        sendFunctionCallHeader(name, 5);
        io << arg1 << arg2 << arg3 << arg4 << arg5 << endl;
    }
    //! call a method with 6 input arguments
    template<class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6>
    inline void callFunction(const string& name, const Arg1& arg1, const Arg2& arg2, const Arg3& arg3, const Arg4& arg4, const Arg5& arg5, const Arg6& arg6)
    {
        sendFunctionCallHeader(name, 6);
        io << arg1 << arg2 << arg3 << arg4 << arg5 << arg6 << endl;
    }

    //! call a method with 0 input arguments
    inline void callMethod(int objid, const string& name)
    {
        sendMethodCallHeader(objid, name, 0);
        io << endl;
    }
    //! call a method with 1 input argument
    template<class Arg1>
    inline void callMethod(int objid, const string& name, const Arg1& arg1)
    {
        sendMethodCallHeader(objid, name, 1);
        io << arg1 << endl;
    }
    //! call a method with 2 input arguments
    template<class Arg1, class Arg2>
    inline void callMethod(int objid, const string& name, const Arg1& arg1, const Arg2& arg2)
    {
        sendMethodCallHeader(objid, name, 2);
        io << arg1 << arg2 << endl;
    }
    //! call a method with 3 input arguments
    template<class Arg1, class Arg2, class Arg3>
    inline void callMethod(int objid, const string& name, const Arg1& arg1, const Arg2& arg2, const Arg3& arg3)
    {
        sendMethodCallHeader(objid, name, 3);
        io << arg1 << arg2 << arg3 << endl;
    }
    //! call a method with 4 input arguments
    template<class Arg1, class Arg2, class Arg3, class Arg4>
    inline void callMethod(int objid, const string& name, const Arg1& arg1, const Arg2& arg2, const Arg3& arg3, const Arg4& arg4)
    {
        sendMethodCallHeader(objid, name, 4);
        io << arg1 << arg2 << arg3 << arg4 << endl;
    }
    //! call a method with 5 input arguments
    template<class Arg1, class Arg2, class Arg3, class Arg4, class Arg5>
    inline void callMethod(int objid, const string& name, const Arg1& arg1, const Arg2& arg2, const Arg3& arg3, const Arg4& arg4, const Arg5& arg5)
    {
        sendMethodCallHeader(objid, name, 5);
        io << arg1 << arg2 << arg3 << arg4 << arg5 << endl;
    }
    //! call a method with 6 input arguments
    template<class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6>
    inline void callMethod(int objid, const string& name, const Arg1& arg1, const Arg2& arg2, const Arg3& arg3, const Arg4& arg4, const Arg5& arg5, const Arg6& arg6)
    {
        sendMethodCallHeader(objid, name, 6);
        io << arg1 << arg2 << arg3 << arg4 << arg5 << arg6 << endl;
    }

    //! Waits for completion of method with no output results
    //! and checks its status. (This must be called for 0 output methods!).
    inline void getResults()
    { expectResults(0); };

    //! get results for a method with 1 output result
    //! These are the results for the just previously called method
    template<class Arg1>
    inline void getResults(Arg1& arg1)
    {
        expectResults(1);
        //DBG_LOG << "RemotePLearnServer getResults(Arg1& arg1)" << endl;
        io >> arg1;
        //DBG_LOG << "RemotePLearnServer getResults got arg1" << endl;
    }
    //! get results for a method with 2 output results
    //! These are the results for the just previously called method
    template<class Arg1, class Arg2>
    inline void getResults(Arg1& arg1, Arg2& arg2)
    {
        expectResults(2);
        io >> arg1 >> arg2;
    }
    //! get results for a method with 3 output results
    //! These are the results for the just previously called method
    template<class Arg1, class Arg2, class Arg3>
    inline void getResults(Arg1& arg1, Arg2& arg2, Arg3& arg3)
    {
        expectResults(3);
        io >> arg1 >> arg2 >> arg3;
    }
    //! get results for a method with 4 output results
    //! These are the results for the just previously called method
    template<class Arg1, class Arg2, class Arg3, class Arg4>
    inline void getResults(Arg1& arg1, Arg2& arg2, Arg3& arg3, Arg4& arg4)
    {
        expectResults(4);
        io >> arg1 >> arg2 >> arg3 >> arg4;
    }
    //! get results for a method with 5 output results
    //! These are the results for the just previously called method
    template<class Arg1, class Arg2, class Arg3, class Arg4, class Arg5>
    inline void getResults(Arg1& arg1, Arg2& arg2, Arg3& arg3, Arg4& arg4, Arg5& arg5)
    {
        expectResults(5);
        io >> arg1 >> arg2 >> arg3 >> arg4 >> arg5;
    }
    //! get results for a method with 6 output results
    //! These are the results for the just previously called method
    template<class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6>
    inline void getResults(Arg1& arg1, Arg2& arg2, Arg3& arg3, Arg4& arg4, Arg5& arg5, Arg6& arg6)
    {
        expectResults(6);
        io >> arg1 >> arg2 >> arg3 >> arg4 >> arg5 >> arg6;
    }  

    ~RemotePLearnServer();
    
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
