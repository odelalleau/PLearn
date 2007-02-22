// -*- C++ -*-

// ServerLogStreamBuf.cc
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

/*! \file ServerLogStreamBuf.cc */


#include "ServerLogStreamBuf.h"

namespace PLearn {
using namespace std;

ServerLogStreamBuf::ServerLogStreamBuf(PStream log_, const string& module_name_, int verbosity_)
    : PStreamBuf(false, true, 4096, 4096), 
      log(log_), module_name(module_name_), verbosity(verbosity_)
{}

ServerLogStreamBuf::~ServerLogStreamBuf()
{
    flush();
}

ServerLogStreamBuf::streamsize ServerLogStreamBuf::read_(char* p, streamsize n)
{
    PLERROR("ServerLogStreamBuf::read_ should never be used!");
    return 0; // never reached 
}

//! writes exactly n characters from p (unbuffered, must flush)
void ServerLogStreamBuf::write_(const char* p, streamsize n)
{
    log.write("*L "); 
    string msg(p, n);
    log << module_name << verbosity << msg;
    log.flush();
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
