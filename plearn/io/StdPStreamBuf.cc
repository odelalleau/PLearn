// -*- C++ -*-

// StdPStreamBuf.cc
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
   * $Id: StdPStreamBuf.cc,v 1.1 2004/06/26 00:24:14 plearner Exp $ 
   ******************************************************* */

// Authors: Pascal Vincent

/*! \file StdPStreamBuf.cc */


#include "StdPStreamBuf.h"

namespace PLearn {
using namespace std;


  StdPStreamBuf::StdPStreamBuf()
    :pin(0), pout(0), own_pin(false), own_pout(false)
#ifdef MARKABLE_STREAM_HACK
    , original_bufin(0), original_bufout(0)
#endif
  {}

  //! ctor. from an istream (I)
  StdPStreamBuf::StdPStreamBuf(istream* pin_, bool own_pin_)
    :pin(pin_), pout(0), own_pin(own_pin_), own_pout(false)
#ifdef MARKABLE_STREAM_HACK
    , original_bufin(pin_->rdbuf()), original_bufout(0)
#endif
  { 
#ifdef MARKABLE_STREAM_HACK
    initInBuf(); 
#endif
  }

  //! ctor. from an ostream (O)
  StdPStreamBuf::StdPStreamBuf(ostream* pout_, bool own_pout_)
    :pin(0), pout(pout_), own_pin(false), own_pout(own_pout_)
#ifdef MARKABLE_STREAM_HACK
    , original_bufin(0), original_bufout(pout_->rdbuf())
#endif
  {}

  //! ctor. from an iostream (IO)
  StdPStreamBuf::StdPStreamBuf(iostream* pios_, bool own_pios_)
    :pin(pios_), pout(pios_), own_pin(own_pios_), own_pout(own_pios_)
#ifdef MARKABLE_STREAM_HACK
    , original_bufin(pios_->rdbuf()), original_bufout(pios_->rdbuf())
#endif
  { 
#ifdef MARKABLE_STREAM_HACK
    initInBuf(); 
#endif
  }

  //! ctor. from an istream and an ostream (IO)
  StdPStreamBuf::StdPStreamBuf(istream* pin_, ostream* pout_, bool own_pin_, bool own_pout_)
    :pin(pin_), pout(pout_), own_pin(own_pin_), own_pout(own_pout_)
#ifdef MARKABLE_STREAM_HACK
    , original_bufin(pin_->rdbuf()), original_bufout(pout_->rdbuf())
#endif
  { 
#ifdef MARKABLE_STREAM_HACK
    initInBuf(); 
#endif
  }

  StdPStreamBuf::~StdPStreamBuf()
  {
#ifdef MARKABLE_STREAM_HACK
    // am I the only PStream using this buffer?
    if(the_inbuf && 1 == the_inbuf->usage())
      {// reset underlying streams's buffers before destroying the_inbuf
        if(pin) pin->rdbuf(original_bufin);
        if(pout) pout->rdbuf(original_bufout);
      }
#endif
    if(own_pin && pin) delete pin; // delete pin if we created it
    if(own_pout && pout) delete pout; // delete pout if we created it
  }

#ifdef MARKABLE_STREAM_HACK
void StdPStreamBuf::initInBuf()
  {
    if(pin)
      {
        the_inbuf= dynamic_cast<pl_streambuf*>(pin->rdbuf());
        if(the_inbuf.isNull())
          the_inbuf= new pl_streambuf(*pin->rdbuf());
        pin->rdbuf(the_inbuf);
      }
  }
#endif


  void StdPStreamBuf::setIn(istream* pin_, bool own_pin_)
  {
    if(pin && original_bufin) pin->rdbuf(original_bufin);
    if(own_pin) delete pin;
    pin = pin_;
    own_pin = own_pin_;
    the_fdbuf = 0;
    if(pin)
      {
        original_bufin = pin->rdbuf();
#ifdef MARKABLE_STREAM_HACK
        initInBuf();
#endif
      }
    else
      original_bufin = 0;
  }

  void StdPStreamBuf::setOut(ostream* pout_, bool own_pout_)
  {
    if(pout && original_bufout) pout->rdbuf(original_bufout);
    if(own_pout) delete pout;
    pout= pout_;
    own_pout = own_pout_;
    if(pout)
      original_bufout= pout->rdbuf();
    else
      original_bufout=0;      
  }

  //! attach: "attach" to a POSIX file descriptor.
  void StdPStreamBuf::attach(int fd)
  {
    the_fdbuf= new pl_fdstreambuf(fd, pl_dftbuflen);
    the_inbuf= new pl_streambuf(*the_fdbuf);
    if(pin)
      pin->rdbuf(the_inbuf);
    else
      {
        own_pin= true;
        pin= new istream(the_inbuf);
      }
    if(pout) 
      pout->rdbuf(the_fdbuf);
    else
      {
        own_pout= true;
        pout= new ostream(the_fdbuf);
      }
  }

  StdPStreamBuf::streamsize StdPStreamBuf::read_(char* p, streamsize n)
  {
    PLERROR("StdPStreamBuf::read_ not yet implemented");
    return 0;
  }

  //! writes exactly n characters from p (unbuffered, must flush)
  void StdPStreamBuf::write_(char* p, streamsize n)
  {
    PLERROR("StdPStreamBuf::write_ not yet implemented");
  }

  //! should change the position of the next read/write (seek)
  void StdPStreamBuf::setpos_(streampos pos)
  {
    PLERROR("StdPStreamBuf::setpos_ not yet implemented");
  }
  
  //! should return the position of the next read/write in 
  //! number of bytes from start of file.
  StdPStreamBuf::streampos StdPStreamBuf::getpos_()
  {    
    PLERROR("StdPStreamBuf::getpos_ not yet implemented");
    return 0;
  }

} // end of namespace PLearn
