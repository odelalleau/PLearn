// -*- C++ -*-

// ReadAndWriteCommand.cc
// 
// Copyright (C) 2002 Pascal Vincent
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
   * $Id: ReadAndWriteCommand.cc,v 1.1 2002/10/25 23:16:08 plearner Exp $ 
   ******************************************************* */

/*! \file ReadAndWriteCommand.cc */
#include "ReadAndWriteCommand.h"
#include "PStream.h"
#include "Object.h"

namespace PLearn <%
using namespace std;

//! This allows to register the 'ReadAndWriteCommand' command in the command registry
PLearnCommandRegistry ReadAndWriteCommand::reg_(new ReadAndWriteCommand);

//! The actual implementation of the 'ReadAndWriteCommand' command 
void ReadAndWriteCommand::run(const vector<string>& args)
{
  if(args.size()!=2)
    PLERROR("read_and_write takes 2 arguments");
  string source = args[0];
  string dest = args[1];
  PIFStream in(source);
  if(!in)
    PLERROR("Could not open file %s for reading",source.c_str());
  PP<Object> o;
  in >> o;
  POFStream out(dest);
  if(!out)
    PLERROR("Could not open file %s for writing",dest.c_str());
  out << *o;
}

%> // end of namespace PLearn

