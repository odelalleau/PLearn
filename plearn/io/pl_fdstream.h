// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
// Copyright (C) 2002 Xavier Saint-Mleux <saintmlx@iro.umontreal.ca>
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

/*!
 * pl_fdstream.{h|cc}
 * Defines a stream buffer than can be created from a POSIX file descriptor,
 * along with a stream to use that buffer.
 */


#ifndef pl_fdstream_INC
#define pl_fdstream_INC

#include <iostream>
#include "PP.h"

namespace PLearn <%
using namespace std;

const int pl_dftbuflen= 4096;

//! pl_fdstreambuf: stream buffer that acts on a POSIX file descriptor
class pl_fdstreambuf : public streambuf, public PPointable
{
protected:  
  int fd;  //<! the file descriptor
  char* inbuf; //<! input buffer
  int inbuf_capacity; //<! length of inbuf
  
protected:
  
  //!  inline for efficiency
  inline void reserveInputBuffer(int buflen)
  {
    if(buflen>inbuf_capacity)
      {
	if(inbuf)
	  delete[] inbuf;
	inbuf = new char[buflen];
	inbuf_capacity = buflen;
      }
  }
  
  virtual int overflow(int c=EOF);
  virtual int underflow();
  virtual int showmanyc();
  virtual streamsize xsputn(const char* s, streamsize n);
  virtual streamsize xsgetn(char* s, streamsize n);
  virtual streambuf* setbuf(char* p, int len);
  virtual int sync();
  
public:
  pl_fdstreambuf(int fd_, int inbufsize);
  virtual ~pl_fdstreambuf();
  
};

class pl_fdstream: public iostream
{
protected:
  char* outbuffer;
  
public:
  
  pl_fdstream()
    :iostream(0), outbuffer(0)
  {}
  
  pl_fdstream(int fd, int inbufsize= pl_dftbuflen, int outbufsize= pl_dftbuflen)
    :iostream(0), outbuffer(0)
  { init(fd, inbufsize, outbufsize); }
  
  void init(int fd, int inbufsize= pl_dftbuflen, int outbufsize= pl_dftbuflen);
  
  //! for compatibility with old non-standard stl fstream
  void attach(int fd);

  ~pl_fdstream();
};

%> // end of namespace PLearn

#endif //ndef pl_fdstream_INC

