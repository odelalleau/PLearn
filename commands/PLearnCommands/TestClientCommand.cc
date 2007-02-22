// -*- C++ -*-

// TestClientCommand.cc
//
// Copyright (C) 2007 Xavier Saint-Mleux, Apstat Technologies, inc.
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

// Authors: Xavier Saint-Mleux

/*! \file TestClientCommand.cc */


#include "TestClientCommand.h"
#include <plearn/misc/PLearnService.h>
#include <plearn/io/pl_log.h>
#include <plearn/vmat/MemoryVMatrix.h>
#include <plearn/base/PDateTime.h>
#include <plearn/base/ProgressBar.h>

namespace PLearn {
using namespace std;

//! This allows to register the 'TestClientCommand' command in the command registry
PLearnCommandRegistry TestClientCommand::reg_(new TestClientCommand);

TestClientCommand::TestClientCommand():
    PLearnCommand("testclient",

                  "Launches plearn in test client mode",

                  "testclient\n"
                  "  Launches plearn in test client mode \n"
                  " \n"
        )
{}


//! The actual implementation of the 'TestClientCommand' command 
void TestClientCommand::run(const vector<string>& args)
{

    cout << "test client run" << endl;
    

    PLearnService& ps(PLearnService::instance()); 
  
    int ns= ps.availableServers();
    TVec<PP<RemotePLearnServer> > ss= ps.reserveServers(ns);


    cout << "test client has servers" << endl;

    TVec<int> objids(ns);

    for(int i= 0; i < ns; ++i)
    {
        //ss[i]->newObjectAsync(1, string("MemoryVMatrix(width= 5000, length= 7000)"));
        //ss[i]->newObjectAsync(string("MemoryVMatrix(width= 5000, length= 7000)"));
        objids[i]= ss[i]->newObject(string("MemoryVMatrix(width= 5000, length= 7000)"));
        cerr << "NEW OBJ " << i << '-' << objids[i] << endl;
    }
/*
    for(int i= 0; i < ns; ++i)
    {
        int j= ps.watchServers(ss, PLearnService::log_callback, PLearnService::progress_callback);
        cout << "result 1 from " << j << " at " << PDateTime::currentLocalTime() << endl;
        ss[j]->getResults(objids[j]);
    }
*/
    for(int i= 0; i < ns; ++i)
        ss[i]->callMethod(objids[i], "fill", 3.21*static_cast<real>(i+1));

    for(int i= 0; i < ns; ++i)
    {
/*
        int j= ps.watchServers(ss, PLearnService::log_callback, PLearnService::progress_callback);
        cout << "result 2 from " << j << " at " << PDateTime::currentLocalTime() << endl;
        ss[j]->getResults();
*/
        ps.waitForResult()->getResults();
    }

    cout << "before getRow" << endl;

    for(int i= 0; i < ns; ++i)
        ss[i]->callMethod(objids[i], "getRow", 3);

    cout << "after getRow" << endl;

    for(int i= 0; i < ns; ++i)
    {
        cout << "bws " << i << endl;

        int j= ps.watchServers(ss, PLearnService::log_callback, PLearnService::progress_callback);
        cout << "result 3 from " << j << " at " << PDateTime::currentLocalTime() << endl;
        Vec vv;

        cout << "getting it" << endl;

        ss[j]->getResults(vv);

        cout << "got it" << endl;

        pout << '\t' << vv[0] << endl;
    }

    cout << "before call dot" << endl;

    for(int i= 0; i < ns; ++i)
        ss[i]->callMethod(objids[i], "dot", 1, 3, 4500);

    cout << "after call dot" << endl;

    for(int i= 0; i < ns; ++i)
    {
        int j= ps.watchServers(ss, PLearnService::log_callback, PLearnService::progress_callback);
        cout << "result 4 from " << j << " at " << PDateTime::currentLocalTime() << endl;
        real x;
        ss[j]->getResults(x);
        cout << endl << '=' << x << endl;
    }

    cout << "before call save" << endl;

    for(int i= 0; i < ns; ++i)
        ss[i]->callMethod(objids[i], "savePMAT", string("/home/saintmlx/testclient/mat")+tostring(i)+string(".pmat"));

    cout << "after call save" << endl;

    for(int i= 0; i < ns; ++i)
    {
        int j= ps.watchServers(ss, PLearnService::log_callback, PLearnService::progress_callback);
        cout << "result 5 from " << j << " at " << PDateTime::currentLocalTime() << endl;
        ss[j]->getResults();
    }

    cout << "before delete" << endl;

    for(int i= 0; i < ns; ++i)
        ss[i]->deleteObjectAsync(objids[i]);

    cout << "after delete" << endl;

    for(int i= 0; i < ns; ++i)
    {
        int j= ps.watchServers(ss, PLearnService::log_callback, PLearnService::progress_callback);
        cout << "result 6 from " << j << " at " << PDateTime::currentLocalTime() << endl;
        ss[j]->getResults();
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
