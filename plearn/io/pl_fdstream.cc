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


#include <iostream>

// norman: set win32 functions
#ifdef WIN32
#include <io.h>
#define read _read
#define write _write
#else
#include <unistd.h>
#endif
#include "pl_fdstream.h"

namespace PLearn {
using namespace std;


pl_fdstreambuf::pl_fdstreambuf(int fd_, int inbufsize) 
  :fd(fd_), inbuf(0), inbuf_capacity(0)
{
  if(inbufsize > 0)
    reserveInputBuffer(inbufsize);
}


pl_fdstreambuf::~pl_fdstreambuf()
{
  if(inbuf)
    delete[] inbuf;
}

streambuf* pl_fdstreambuf::setbuf(char* p, int len)
{
  if(p && len>0)
    setp(p, p+len-1); // -1 because we want space to put the extra character passed to overflow()
  else
    setp(0,0);
  return this;
}

//! not implemented (yet)
int pl_fdstreambuf::showmanyc()
{ return -1; }

int pl_fdstreambuf::underflow()
{
  int msglength= read(fd, inbuf, inbuf_capacity);
  if(msglength < 1)
    return EOF;
  setg(inbuf, inbuf, inbuf+msglength);
  return *inbuf;
}

int pl_fdstreambuf::overflow(int c)
{
  if(c!=EOF)
    {
      if(pbase()) // buffered mode
	{
	  streamsize n = pptr() - pbase();
	  *pptr()= char(c); // put it in extra space
	  if(write(fd, pbase(), n) < 0)
	    return EOF;
	  pbump(-n);
	}
      else // unbuffered mode
	{
	  char tinybuf[1];
	  tinybuf[0] = c;
	  //cout << "writing " << (char)c << endl;
	  if(write(fd, tinybuf, 1) < 0)
	    return EOF;
	}
    }
  else // extra char is EOF, we ignore it
    {
      if(pbase()) // buffered mode
	{
	  streamsize n = pptr() - pbase();
	  if(write(fd, pbase(), n) < 0)
	    return EOF;
	  pbump(-n);
	}
    }
  return c;
}

int pl_fdstreambuf::sync()
{
  streamsize n = pptr() - pbase();
  if(n>0)
    {
      if(write(fd, pbase(), n) < 0)
	return EOF;
      pbump(-n);
    }
  return 0;
}

// Smart implementation of xsputn:
// If n is greater than the buffer size we send the message directly
// Thus, we don't waste time copying stuff into the buffer unnecessarily
// (ex: whhen sending a long vector for ex.).
streamsize pl_fdstreambuf::xsputn(const char* s, streamsize n)
{      
  if(n>epptr()-pptr())  // n greater than buffer size! 
    {
      // Let's not waste time copying stuff into the buffer, send it directly
      sync(); // first make sure we send what's left in the buffer
      if(write(fd, (char *)s, n) < 0)
	return 0;
    }
  else  // call the default method
    streambuf::xsputn(s,n);

  return n;
}

//use default implementation
streamsize pl_fdstreambuf::xsgetn(char* s, streamsize n)
{ return streambuf::xsgetn(s, n); }

void pl_fdstream::init(int fd, int inbufsize, int outbufsize)
{
  rdbuf(new pl_fdstreambuf(fd, inbufsize));
  if(outbufsize<=1)
#if __GNUC__ < 3 && !defined(WIN32)
    rdbuf()->setbuf(0,0);
#else
    rdbuf()->pubsetbuf(0,0);
#endif
  else
    {
      outbuffer = new char[outbufsize];
#if __GNUC__ < 3 && !defined(WIN32)
      rdbuf()->setbuf(outbuffer,outbufsize);
#else
      rdbuf()->pubsetbuf(outbuffer,outbufsize);
#endif
    }
}

void pl_fdstream::attach(int fd)
{
  rdbuf(new pl_fdstreambuf(fd, pl_dftbuflen));
  outbuffer= new char[pl_dftbuflen];
#if __GNUC__ < 3 && !defined(WIN32)
  rdbuf()->setbuf(outbuffer, pl_dftbuflen);
#else
  rdbuf()->pubsetbuf(outbuffer, pl_dftbuflen);
#endif
}


pl_fdstream::~pl_fdstream()
{
  flush();
  delete rdbuf(0); // delete pl_fdstreambuf
  if(outbuffer)
    delete[] outbuffer;
}

} // end of namespace PLearn
