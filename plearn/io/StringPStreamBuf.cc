// -*- C++ -*-

// StringPStreamBuf.cc
//
// Copyright (C) 2004 Pascal Vincent 
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
   * $Id: StringPStreamBuf.cc,v 1.4 2005/01/14 19:40:49 plearner Exp $ 
   ******************************************************* */

// Authors: Pascal Vincent

/*! \file StringPStreamBuf.cc */


#include "StringPStreamBuf.h"
#include <unistd.h>

namespace PLearn {
using namespace std;

  StringPStreamBuf::StringPStreamBuf(string* ptrs, const string& openmode, bool own_string_,
                   streamsize inbuf_capacity, streamsize outbuf_capacity, streamsize unget_capacity)
    :PStreamBuf(openmode=="r", openmode=="w" || openmode=="a", inbuf_capacity, outbuf_capacity, unget_capacity),
     st(ptrs), read_index(0), own_string(own_string_)
  {
    if(openmode=="w") // truncate it first
      st->clear();

    else if(openmode!="r" && openmode!="a")
      PLERROR("In StringPStreamBuf invalid openmode %s, must be one of r, w, a",openmode.c_str());
  }

  StringPStreamBuf::StringPStreamBuf(const string* ptrs, const string& openmode, bool own_string_,
                   streamsize inbuf_capacity, streamsize unget_capacity)
    :PStreamBuf(true, false, inbuf_capacity, 0, unget_capacity),
     st(const_cast<string*>(ptrs)), read_index(0), own_string(own_string_)
  {
    if(openmode!="r")
      PLERROR("In StringPStreamBuf(const string*, ...) invalid openmode %s, must be one of r if giving a const string* ",openmode.c_str());    
  }

  StringPStreamBuf::~StringPStreamBuf()
  {
    flush();
    
    if(own_string && st!=0)
      delete st;
  }

  StringPStreamBuf::streamsize StringPStreamBuf::read_(char* p, streamsize n)
  {
    string::size_type nread = st->copy(p, string::size_type(n), read_index); 
    read_index += nread;
    return streamsize(nread);
  }

  //! writes exactly n characters from p (unbuffered, must flush)
  void StringPStreamBuf::write_(const char* p, streamsize n)
  {
    while(n--)
      st->operator+=(*p++);
  }
  
} // end of namespace PLearn
