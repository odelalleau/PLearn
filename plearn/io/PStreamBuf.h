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
   * $Id: PStreamBuf.h,v 1.6 2004/06/26 00:24:14 plearner Exp $ 
   ******************************************************* */

/*! \file PStreamBuf.h */
#ifndef PStreamBuf_INC
#define PStreamBuf_INC

#include "PP.h"

namespace PLearn {
using namespace std;

class PStreamBuf: public PPointable
{    
public:

  // typedef Object inherited;
  typedef PPointable inherited;
  typedef size_t streamsize; 
  typedef off_t streampos; 

  // ************************
  // * public build options *
  // ************************

  // ### declare public option fields (such as build options) here
  // ...

  // ****************
  // * Constructors *
  // ****************

  //! Default constructor
  //! SUBCLASS WRITING:subclasse should also correctly set is_readable, is_writable, is_random_accessible 
  PStreamBuf(streamsize inbuf_capacity=1000, streamsize outbuf_capacity=1000, streamsize unget_capacity=100);

  virtual ~PStreamBuf();


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

  // Output buffer
  streamsize outbuf_chunksize;
  char* outbuf; //!< beginning of output buffer
  char* outbuf_p; //!< position of next character to be written
  char* outbuf_end; //!< one after last reserved character in outbuf

protected:

  bool is_readable;
  bool is_writable;
  bool is_random_accessible; //!< is random access supported? (setpos and getpos)

 
  //! reads up to n characters into p
  //! You should overload this call in subclasses. 
  //! Default version issues a PLERROR
  /*!  On success, the number of bytes read is returned.  Zero indicates
    end of file. If we are not at end of file, at least one character
    should be returned (the call must block until at least one char is
    available).  It is not an error if the number returned is smaller than
    the number of bytes requested; this may happen for example because
    fewer bytes are actually available right now (maybe because we were
    close to end-of-file, or because we are reading from a pipe, or from a
    terminal).  If an error occurs, an exception should be thrown.
  */
  virtual streamsize read_(char* p, streamsize n);

  //! writes exactly n characters from p (unbuffered, must flush)
  //! Default version issues a PLERROR
  virtual void write_(char* p, streamsize n);

  //! should change the position of the next read/write (seek)
  //! Default version issues a PLERROR
  virtual void setpos_(streampos pos);
  
  //! should return the position of the next read/write in 
  //! number of bytes from start of file.
  //! Default version issues a PLERROR
  virtual streampos getpos_();

private:
  // refills the inbuf
  inline streamsize refill_in_buf()
  {
#ifdef BOUNDCHECK
    if(!isReadable())
      PLERROR("Called PStreamBuf::refill_in_buf on a buffer not marked as readable");
#endif

    inbuf_p = inbuf+ungetsize;
    streamsize n = read_(inbuf_p, inbuf_chunksize);
    inbuf_end = inbuf_p+n;
    return n;
  }

public:

  inline bool isReadable() const
  { return is_readable; }

  inline bool isWritable() const
  { return is_writable; }

  inline bool isRandomAccessible() const
  { return is_random_accessible; }  
  
  inline int get()
  {
    if(inbuf_p<inbuf_end || refill_in_buf())
      return *inbuf_p++;
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
#ifdef BOUNDCHECK
  if(!isReadable())
    PLERROR("Called PStreamBuf::read on a buffer not marked as readable");
#endif

  streamsize nleft = n;

  streamsize inbuf_n = (streamsize)(inbuf_end-inbuf_p);
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
  return nread;
}



  //! puts the given characters back in the input buffer
  //! so that they're the next thing read.
  inline void unread(const char* p, streamsize n)
  {
    if(inbuf_p<inbuf)
      PLERROR("Cannot unread that many characters: %d, input buffer bound reached", n);

    inbuf_p -= n;
    memcpy(inbuf_p,p,n);
  }


  inline void unget(char c)
  {
    if(inbuf_p<=inbuf)
      PLERROR("Cannot unget that many characters, input buffer bound reached");
    
    inbuf_p--;
    *inbuf_p = c;
  }


  inline void flush()
  {
    streamsize n = (streamsize)(outbuf_p-outbuf);
    if(n)
      {
        write_(outbuf, n);
        outbuf_p = outbuf;
      }
  }

  inline void setpos(streampos pos)
  { 
#ifdef BOUNDCHECK
    if(!isRandomAccessible())
      PLERROR("Call to PStreamBuf::setpos() on a stream that is not random accessible isRandomAccessible()==false");
#endif
    // flush output buffer
    flush();
    // reset input buffer
    inbuf_p = inbuf+ungetsize;
    inbuf_end = inbuf_p;
    // set the position
    setpos_(pos);
  }
  
  inline streampos getpos() 
  { 
#ifdef BOUNDCHECK
    if(!isRandomAccessible())
      PLERROR("Call to PStreamBuf::getpos() on a stream that is not random accessible isRandomAccessible()==false");
#endif
    streamsize in_off = inbuf_end-inbuf_p;
    streamsize out_off = outbuf_p-outbuf;
    if(in_off && out_off)  // We both read and wrote to the stream without calling setpos in between!
      PLERROR("In PStreamBuf::getpos() For seekable i/o streams, you must call setpos (ex: setpos(getpos())) "
              "before switching between read and write calls. "
              "You probably didn't do this properly. That's why you get this error.");
    return getpos_() - in_off + out_off;
  }


  inline void put(char c)
  {
#ifdef BOUNDCHECK
  if(!isWritable())
    PLERROR("Called PStreamBuf::put on a buffer not marked as writable");
#endif
    *outbuf_p++ = c;
    if(outbuf_p==outbuf_end)
      flush();
  }

  inline void write(char* p, streamsize n)
  {
#ifdef BOUNDCHECK
  if(!isWritable())
    PLERROR("Called PStreamBuf::write on a buffer not marked as writable");
#endif

    streamsize bufrem = (streamsize)(outbuf_end-outbuf_p);
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
  
 // Initially I planned to derive this class from Object. 
 // But when attempting compilation I found out  back in include hell!!!
 // (Object needs PStream which needs PStremBuf which needs Object)
 // Getting out of this mess would not be easy: we need efficiency, so many of 
 // the concerned calls really should stay inline (hence in the .h).
 // Finally I opted to have it derive from PPoitable rather than Object.
 // That's why all typical Object stuff are commented out.
 // (Pascal)

  // protected:
  // static void declareOptions(OptionList& ol);

  // public:
  // simply calls inherited::build() then build_() 
  //  virtual void build();

  //! Transforms a shallow copy into a deep copy
  // virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

  //! Declares name and deepCopy methods
  // PLEARN_DECLARE_ABSTRACT_OBJECT(PStreamBuf);

};

// Declares a few other classes and functions related to this class
//  DECLARE_OBJECT_PTR(PStreamBuf);
  
} // end of namespace PLearn

#endif
