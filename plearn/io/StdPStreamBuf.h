// -*- C++ -*-

// StdPStreamBuf.h
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
   * $Id: StdPStreamBuf.h,v 1.4 2005/01/14 19:40:49 plearner Exp $ 
   ******************************************************* */

// Authors: Pascal Vincent

/*! \file StdPStreamBuf.h */

//! StdPStreamBuf is is a PStreamBuf that wraps an underlying std::istream and std::ostream

#ifndef StdPStreamBuf_INC
#define StdPStreamBuf_INC

#include "PStreamBuf.h"
#include <iostream>

// It seems we still need to use pl_streambuf hack, even with the new PLStreambuf version
// as std streams not wrapped in a pl_streambuf appear to have unpredictable and buggy behaviour. 
// (EOF showing up at unexpected times!)
// #define DONT_USE_PLSTREAMBUF 0
#define DONT_USE_PLSTREAMBUF 1

#if STREAMBUFVER == 0
#define DONT_USE_PLSTREAMBUF 0
#endif 

#if DONT_USE_PLSTREAMBUF == 0
#include "pl_streambuf.h"
#include "pl_fdstream.h"
#endif

namespace PLearn {
using namespace std;

class StdPStreamBuf: public PStreamBuf
{

private:
  
  typedef PStreamBuf inherited;

protected:
  // *********************
  // * protected options *
  // *********************
  istream* pin;  //<! underlying input stream
  ostream* pout; //<! underlying output stream
  bool own_pin, own_pout; //<! true if {pin|pout} was created internally

#if DONT_USE_PLSTREAMBUF == 0
  PP<pl_streambuf> the_inbuf;   //<! markable input buffer
  PP<pl_fdstreambuf> the_fdbuf; //<! buffer on a POSIX file descriptor

  //! ptrs. to the original buffers;  used to 'reset' the underlying
  //! streams to a valid state when the PStream is destroyed.
  streambuf* original_bufin;
  streambuf* original_bufout;

  //! initInBuf: called by ctors. to ensure that pin's buffer is markable
  void initInBuf();
#endif


public:

  // Default constructor, make sure the implementation in the .cc
  // initializes all fields to reasonable default values.
  StdPStreamBuf();

  //! ctor. from an istream (I)
  StdPStreamBuf(istream* pin_, bool own_pin_=false);

  //! ctor. from an ostream (O)
  StdPStreamBuf(ostream* pout_, bool own_pout_=false);

  //! ctor. from an iostream (IO)
  StdPStreamBuf(iostream* pios_, bool own_pios_=false);

  //! ctor. from an istream and an ostream (IO)
  StdPStreamBuf(istream* pin_, ostream* pout_, bool own_pin_=false, bool own_pout_=false);

  virtual ~StdPStreamBuf();


  void setIn(istream* pin_, bool own_pin_=false);

  void setOut(ostream* pout_, bool own_pout_=false);

#if STREAMBUFVER == 0
  //! attach: "attach" to a POSIX file descriptor.
  void attach(int fd);
#endif

  // ******************
  // * Object methods *
  // ******************

protected: 

  virtual streamsize read_(char* p, streamsize n);

  //! writes exactly n characters from p (unbuffered, must flush)
  virtual void write_(const char* p, streamsize n);

public:

  inline istream* rawin() { return pin; }   //<! access to underlying istream
  inline ostream* rawout() { return pout; }

#if STREAMBUFVER == 0
  //! returns the markable input buffer
  //! DEPRECATED: TO BE REMOVED SOON, DO NOT USE!
  inline pl_streambuf* pl_rdbuf() { return the_inbuf; }
#endif 

  virtual bool good() const;
};

} // end of namespace PLearn

#endif
