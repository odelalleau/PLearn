// -*- C++ -*-

// PStreamBuf.h
// 
// Copyright (C) 2003 Pascal Vincent
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
   * $Id: PStreamBuf.h,v 1.3 2003/08/13 08:13:17 plearner Exp $ 
   ******************************************************* */

/*! \file PStreamBuf.h */
#ifndef PStreamBuf_INC
#define PStreamBuf_INC

#include "Object.h"

namespace PLearn <%
using namespace std;

class PStreamBuf: public Object
{    
public:

  typedef Object inherited;
  typedef unsigned int streamsize; 
  typedef unsigned long streampos; 

  // ************************
  // * public build options *
  // ************************

  // ### declare public option fields (such as build options) here
  // ...

  // ****************
  // * Constructors *
  // ****************

  // Default constructor, make sure the implementation in the .cc
  // initializes all fields to reasonable default values.
  PStreamBuf(streamsize inbuf_capacity=1000, streamsize outbuf_capacity=1000, streamsize unget_capacity=100);
  virtual ~PStreamBuf();


  // ******************
  // * Object methods *
  // ******************

private: 
  //! This does the actual building. 
  void build_();

  // Input buffer mechanism
  // ungetsize+inbuf_chunksize characters are allocated in total for the buffer.
  // Calls to read_ are always made as read_(inbuf+ungetsize, inbuf_chunksize);
  // The first ungetsize characters of the buffer are reserved for ungets
  streamsize ungetsize;
  streamsize inbuf_chunksize;
  char* inbuf;  //!< beginning of input buffer
  char* inbuf_p; //!< position of next character to be read
  char* inbuf_end; //!< one after last available character
  streampos readpos;  //!< current position in read stream

  // Output buffer
  streamsize outbuf_chunksize;
  char* outbuf; //!< beginning of output buffer
  char* outbuf_p; //!< position of next character to be written
  char* outbuf_end; //!< one after last reserved character in outbuf
  streampos writepos; //!< currnet position in write stream
  

protected:

  //! Declares this class' options
  // (Please implement in .cc)
  static void declareOptions(OptionList& ol);
 

  //! reads up to n characters into p
  //! You should overload this call in subclasses. 
  /*!
    On success, the number of bytes  read  is  returned  (zero
    indicates  end of file).
    It is not an  error  if  this  number  is
    smaller  than the number of bytes requested; this may hap­
    pen for example because fewer bytes are actually available
    right  now (maybe because we were close to end-of-file, or
    because we are reading from a pipe, or from  a  terminal).
    If an error occurs, an exception should be thrown.
  */
  virtual streamsize read_(char* p, streamsize n) =0;

  // virtual streamsize nonBlockingRead_(char* p, streamsize n) =0;

  //! Input seek. Default version issues a PLERROR
  virtual streampos seekg_(streampos pos) =0;
  
  //! writes n characters from p (unbuffered, must flush)
  virtual void write_(char* p, streamsize n) =0;

  //! Output seek. Default version issues a PLERROR
  virtual streampos seekp_(streampos pos) =0;
  
private:
  // refills the inbuf
  inline streamsize refill_in_buf()
  {
    inbuf_p = inbuf+ungetsize;
    streamsize n = read_(inbuf_p, inbuf_chunksize);
    inbuf_end = inbuf_p+n;
    return n;
  }

public:
  
  inline int get()
  {
    if(inbuf_p<inbuf_end || refill_in_buf())
      {
        readpos++;
        return *inbuf_p++;
      }
    else
      return -1;    
  }

  inline int peek()
  {
    if(inbuf_p<inbuf_end || refill_in_buf())
      return *inbuf_p;
    else
      return -1;
  }

  
inline streamsize read(char* p, streamsize n)
{
  streamsize nleft = n;

  streamsize inbuf_n = inbuf_end-inbuf_p;
  if(inbuf_n) // First copy what's left in the buffer
    {
      streamsize k = nleft<inbuf_n ?nleft :inbuf_n;
      memcpy(p,inbuf_p,k);
      inbuf_p += k;
      p += k;
      nleft -= k;
    }

  if(nleft) // need some more ?
    {
      if(nleft>=inbuf_chunksize) // large block: read it directly
        nleft -= read_(p,nleft);
      else // small block: read it in the buffer first
        {
          inbuf_n = refill_in_buf();
          if(inbuf_n)
            {
              streamsize k = nleft<inbuf_n ?nleft :inbuf_n;
              memcpy(p,inbuf_p,k);
              inbuf_p += k;
              nleft -= k;
            }
        }
    }
  
  streamsize nread = n-nleft;
  readpos += nread;
  return nread;
}



  //! puts the given characters back in the input buffer
  //! so that they're the next thing read.
  inline void unread(const char* p, streamsize n)
  {
    inbuf_p -= n;

    if(inbuf_p<inbuf)
      PLERROR("Cannot unread that many characters: %d, input buffer bound reached", n);

    memcpy(inbuf_p,p,n);
    readpos -= n;
  }


  inline void unget(char c)
  {
    if(inbuf_p<=inbuf)
      PLERROR("Cannot unget that many characters, input buffer bound reached");
    
    inbuf_p--;
    *inbuf_p = c;
    readpos--;
  }


  //! Input seek. Default version issues a PLERROR
  inline streampos seekg(streampos pos)
  { 
    // reset input buffer
    inbuf_p = inbuf+ungetsize;
    inbuf_end = inbuf_p;
    // seek
    readpos = seekg_(pos);
    return readpos;
  }
  
  //! Input tell.
  inline streampos tellg() { return readpos; }


  inline void flush()
  {
    streamsize n = outbuf_p-outbuf;
    if(n)
      {
        write_(outbuf, n);
        outbuf_p = outbuf;
      }
  }

  inline void put(char c)
  {
    *outbuf_p++ = c;
    if(outbuf_p==outbuf_end)
      flush();
  }

  inline void write(char* p, streamsize n)
  {
    streamsize bufrem = outbuf_end-outbuf_p;
    streamsize nn = n<bufrem ?n :bufrem;
    memcpy(outbuf_p, p, nn);
    p += nn;
    outbuf_p += nn;
    flush();
    n -= nn;
    if(n>outbuf_chunksize)
      write_(p, n);
    else
      {
        memcpy(outbuf_p, p, n);
        outbuf_p += n;
      }
  }
  
  //! Output seek.
  streampos seekp(streampos pos)
  { 
    // flush output buffer
    flush();
    // seek
    writepos = seekp_(pos);
    return writepos;
  }

  //! Output tell.
  inline streampos tellp() { return writepos; }

  // simply calls inherited::build() then build_() 
  virtual void build();

  //! Provides a help message describing this class
  static string help();

  //! Transforms a shallow copy into a deep copy
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

  //! Declares name and deepCopy methods
  PLEARN_DECLARE_ABSTRACT_OBJECT(PStreamBuf);

};

// Declares a few other classes and functions related to this class
  DECLARE_OBJECT_PTR(PStreamBuf);
  
%> // end of namespace PLearn

#endif
