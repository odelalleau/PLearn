// -*- C++ -*-

// FilePStreamBuf.h
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
   * $Id: FilePStreamBuf.h,v 1.5 2005/01/14 19:40:49 plearner Exp $ 
   ******************************************************* */

// Authors: Pascal Vincent

/*! \file FilePStreamBuf.h */


#ifndef FilePStreamBuf_INC
#define FilePStreamBuf_INC

#include "PStreamBuf.h"

namespace PLearn {
using namespace std;

class FilePStreamBuf: public PStreamBuf
{

private:
  
  typedef PStreamBuf inherited;

protected:
  // *********************
  // * protected options *
  // *********************
  FILE* in;  //<! input FILE (0 if no input)
  FILE* out; //<! output FILE (0 if no output)
  bool own_in, own_out; //<! true if {in|out} should be closed by this object upon destruction.

public:

  // Default constructor, make sure the implementation in the .cc
  // initializes all fields to reasonable default values.
  FilePStreamBuf(FILE* in_f=0, FILE* out_f=0, 
                 bool own_in=false, bool own_out=false);

  virtual ~FilePStreamBuf();

protected: 

  virtual streamsize read_(char* p, streamsize n);

  //! writes exactly n characters from p (unbuffered, must flush)
  virtual void write_(const char* p, streamsize n);

};

} // end of namespace PLearn

#endif
