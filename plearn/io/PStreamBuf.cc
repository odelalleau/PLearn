// -*- C++ -*-

// PStreamBuf.cc
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
   * $Id: PStreamBuf.cc,v 1.4 2004/06/26 00:24:14 plearner Exp $ 
   ******************************************************* */

/*! \file PStreamBuf.cc */
#include "PStreamBuf.h"

namespace PLearn {
using namespace std;

PStreamBuf::PStreamBuf(streamsize inbuf_capacity, streamsize outbuf_capacity, 
                       streamsize unget_capacity)
  :ungetsize(unget_capacity),
   inbuf_chunksize(inbuf_capacity),
   inbuf(0), inbuf_p(0), inbuf_end(0),
   outbuf_chunksize(outbuf_capacity),
   outbuf(0), outbuf_p(0), outbuf_end(0),
   is_readable(false),
   is_writable(false),
   is_random_accessible(false)
  {
    build_();
  }

PStreamBuf::~PStreamBuf() 
{
  if(inbuf)
    delete[] inbuf;
  if(outbuf)
    {
      flush();
      delete[] outbuf;
    }
}

  void PStreamBuf::build_()
  {
    if(inbuf)
      delete[] inbuf;
    if(ungetsize+inbuf_chunksize>0)
      {
        inbuf = new char[ungetsize+inbuf_chunksize];
        inbuf_p = inbuf+ungetsize;
        inbuf_end = inbuf_p;
      }

    if(outbuf)
      {
        flush();
        delete[] outbuf;
      }
    if(outbuf_chunksize<0)
      {
        outbuf = new char[outbuf_chunksize];
        outbuf_p = outbuf;
        outbuf_end = outbuf+outbuf_chunksize;
      }
  }

 PStreamBuf::streamsize PStreamBuf::read_(char* p, streamsize n)
 {
   PLERROR("read_ not implemented for this PStremBuf");
   return 0;
 }

  //! writes exactly n characters from p (unbuffered, must flush)
  //! Default version issues a PLERROR
 void PStreamBuf::write_(char* p, streamsize n)
 {
   PLERROR("write_ not implemented for this PStremBuf");
 }

  //! should change the position of the next read/write (seek)
  //! Default version issues a PLERROR
 void PStreamBuf::setpos_(streampos pos)
 {
   PLERROR("setpos_ not implemented for this PStremBuf");
 }
  
  //! should return the position of the next read/write in 
  //! number of bytes from start of file.
  //! Default version issues a PLERROR
 PStreamBuf::streampos PStreamBuf::getpos_()
 {
   PLERROR("getpos_ not implemented for this PStremBuf");
   return 0;
 }




  /*

 // Initially I planned to derive this class from Object. 
 // But when attempting compilation I found out  back in include hell!!!
 // (Object needs PStream which needs PStremBuf which needs Object)
 // Getting out of this mess would not be easy: we need efficiency, so many of 
 // the concerned calls really should stay inline (hence in the .h).
 // Finally I opted to have it derive from PPoitable rather than Object.
 // That's why all typical Object stuff are commented out.
 // (Pascal)

 PLEARN_IMPLEMENT_ABSTRACT_OBJECT(PStreamBuf, "Base class for PLearn strem-buffers", "");

  void PStreamBuf::declareOptions(OptionList& ol)
  {
    declareOption(ol, "inbuf_capacity", &PStreamBuf::inbuf_chunksize, OptionBase::buildoption,
                  "The capacity of the input buffer");
    declareOption(ol, "outbuf_capacity", &PStreamBuf::outbuf_chunksize, OptionBase::buildoption,
                  "The capacity of the output buffer");
    declareOption(ol, "unget_capacity", &PStreamBuf::ungetsize, OptionBase::buildoption,
                  "Maximum number of characters that are guaranteed you can unget");

    declareOption(ol, "is_readable", &PStreamBuf::is_readable, OptionBase::buildoption,
                  "Is this an input stream?");
    declareOption(ol, "is_writable", &PStreamBuf::is_writable, OptionBase::buildoption,
                  "Is this an output stream?");
    declareOption(ol, "is_random_accessible", &PStreamBuf::is_random_accessible, OptionBase::buildoption,
                  "Can we use getpos and setpos on this stream?");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
  }

  // ### Nothing to add here, simply calls build_
  void PStreamBuf::build()
  {
    inherited::build();
    build_();
  }

  void PStreamBuf::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
  {
    inherited::makeDeepCopyFromShallowCopy(copies);
  }

  */




} // end of namespace PLearn
