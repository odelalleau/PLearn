// -*- C++ -*-

// PLearnStream.h
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
   * $Id: PLearnStream.h,v 1.1 2002/12/02 08:46:51 plearner Exp $ 
   ******************************************************* */

/*! \file PLearnStream.h */
#ifndef PLearnStream_INC
#define PLearnStream_INC

#include "Object.h"

namespace PLearn <%
using namespace std;

class PLearnStream: public Object
{
protected:
  // *********************
  // * protected options *
  // *********************

  // ### declare protected option fields (such as learnt parameters) here
  // ...
    
public:

  typedef Object inherited;
  typedef unsigned int streamsize; 

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
  PLearnStream();


  // ******************
  // * Object methods *
  // ******************

private: 
  //! This does the actual building. 
  // (Please implement in .cc)
  void build_();

protected: 


  // Input buffer mechanism
  char* inbuf;  //!< beginning of input buffer
  streamsize inbuf_capacity; //!< memory allocated for inbuf
  streamsize inbuf_chunksize; //!< size of chunk to read_ when the buffer runs out
  char* inbuf_p; //!< position of next character to be read
  streamsize inbuf_n; //!< number of valid characters in the buffer starting from inbuf_p

  //! Declares this class' options
  // (Please implement in .cc)
  static void declareOptions(OptionList& ol);
 

  //! reads up to n characters into p
  //! You may overload this call in subclasses. 
  //! In any case, it must return the number of characters successfully read.
  virtual streamsize read_(char* p, streamsize n) =0;
  
  virtual streamsize readNonBlocking_(char* p, streamsize n) =0;






public:
  
inline streamsize read(char* p, streamsize n)
{
  streamsize nleft = n;

  if(inbuf_n) // First copy what's left in the buffer
    {
      streamsize k = nleft<inbuf_n ?nleft :inbuf_n;
      memcpy(p,inbuf_p,k);
      inbuf_p += k;
      p += k;
      nleft -= k;
      inbuf_n -= k;
    }

  if(nleft) // need some more ?
    {
      if(nleft>=inbuf_chunksize) // large block: read it directly
        nleft -= read_(p,nleft);
      else // small block: read it in the buffer first
        {
          inbuf_n = readNonBlocking_(inbuf, inbuf_chunksize);
          streamsize k = nleft<inbuf_n ?nleft :inbuf_n;
          memcpy(p,inbuf_p,k);
          inbuf_p = inbuf+k;
          inbuf_n -= k;
          nleft -= k;
        }
    }
  
  return n-nleft;
}

  inline void get(char& c)
  {
    if(inbuf_n)
      {
        c = *inbuf_p++;
        --inbuf_n;
      }
    else
      read(&c,1);
  }


  //! puts the given characters back in the input buffer
  //! so that they're the next thing read.
  inline unread(const char* p, streamsize n)
  {
    if(n>inbuf_p-inbuf)
      {
        // maximum number of bytes we can shift right in the buffer, while keeping
        // all that's currently in it.
        streamsize shiftamount = (inbuf + inbuf_capacity) - (inbuf_p + inbuf_n);
        if(inbuf_p-inbuf+shiftamount<n) // problem: not enough capacity!
          {
            char* oldinbuf = inbuf;
            inbuf_capacity += n;
            inbuf = new char[inbuf_capacity];
            char* oldinbuf_p = inbuf_p;
            inbuf_p = inbuf + inbuf_capacity - inbuf_n;
            memcpy(inbuf_p, oldinbuf_p, inbuf_n);
            delete[] oldinbuf;
          }
        else // we just need to shift by shiftamount
          {
            memmove(inbuf_p+shiftamount, inbuf_p, inbuf_n);
            inbuf_p += shiftamount;
          }
      }
    inbuf_p -= n;
    memcpy(inbuf_p, p, n);
    inbuf_n += n;
  }

  inline void unget(char c)
  {
    if(inbuf_p>inbuf)
      {
        --inbuf_p;
        *inbuf_p = c;
        ++inbuf_n;
      }
    else
      unread(&c, 1);
  }


  // simply calls inherited::build() then build_() 
  virtual void build();

  //! Provides a help message describing this class
  virtual string help() const;

  //! Transforms a shallow copy into a deep copy
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

  //! Declares name and deepCopy methods
  DECLARE_NAME_AND_DEEPCOPY(PLearnStream);

};

// Declares a few other classes and functions related to this class
  DECLARE_OBJECT_PTR(PLearnStream);
  
%> // end of namespace PLearn

#endif
