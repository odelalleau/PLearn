// -*- C++ -*-

// PLearnServer.h
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

/*! \file PLearnServer.h */

#include <plearn/io/PStream.h>
#include <plearn/base/Object.h>
#include <map>

#ifndef PLearnServer_INC
#define PLearnServer_INC

// Put includes here

namespace PLearn {

// Put global function declarations here

class PLearnServer
{
private:

    static PLearnServer* instance;

    void callFunction(const string& name, int nargs);
    void printHelp();

protected:
    typedef map<int, PP<Object> > ObjMap;

    PStream io;
    bool clear_maps;
    ObjMap objmap;

    virtual int findFreeObjID() const;

public:
    PLearnServer(const PStream& input_output);
    virtual ~PLearnServer();

    //! Enters the server loop which listens for commands and executes them.
    //! Returns false only if server kill command '!K' was issued
    //! Returns true in all other cases (in particular for quit command '!Q')
    //! In principle this call does not throw any exception (exceptions are caught 
    //! inside the run loop and if possible transmitted to the client with a '!E' reply.)
    bool run();

    static PLearnServer* getInstance();

    // The following static methods are declared as remote functions (see .cc)

    //! change directory (calls chdir)
    static void cd(const string path);

    //! change the mode of the io of the PLearnServer instance to plearn_binary 
    static void binary();

    //! change the mode of the io of the PLearnServer instance to plearn_ascii
    static void ascii();

    //! change the implicit_storage mode of the io of the PLearnServer instance.
    static void implicit_storage(bool impl_stor);

    static void setVerbosity(int verbosity);

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
