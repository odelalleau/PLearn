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
   * $Id: PStreamBuf.cc,v 1.2 2003/08/13 08:13:17 plearner Exp $ 
   ******************************************************* */

/*! \file PStreamBuf.cc */
#include "PStreamBuf.h"

namespace PLearn <%
using namespace std;

PStreamBuf::PStreamBuf(streamsize inbuf_capacity, streamsize outbuf_capacity, streamsize unget_capacity) 
  :ungetsize(unget_capacity),
   inbuf_chunksize(inbuf_capacity),
   inbuf(0), inbuf_p(0), inbuf_end(0),
   readpos(0),
   outbuf_chunksize(outbuf_capacity),
   outbuf(0), outbuf_p(0), outbuf_end(0),
   writepos(0)
  {
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


PLEARN_IMPLEMENT_ABSTRACT_OBJECT(PStreamBuf, "ONE LINE DESCR", "NO HELP");

  void PStreamBuf::declareOptions(OptionList& ol)
  {
    declareOption(ol, "inbuf_capacity", &PStreamBuf::inbuf_chunksize, OptionBase::buildoption,
                  "The capacity of the input buffer");
    declareOption(ol, "outbuf_capacity", &PStreamBuf::outbuf_chunksize, OptionBase::buildoption,
                  "The capacity of the output buffer");
    declareOption(ol, "unget_capacity", &PStreamBuf::ungetsize, OptionBase::buildoption,
                  "Maximum number of characters that are guaranteed you can unget");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
  }

  string PStreamBuf::help()
  {
    // ### Provide some useful description of what the class is ...
    return 
      "PStreamBuf is the base class for plearn stream buffers";
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

%> // end of namespace PLearn
