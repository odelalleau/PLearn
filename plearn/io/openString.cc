// -*- C++ -*-

// openString.cc
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
   * $Id: openString.cc,v 1.4 2005/01/14 23:27:18 plearner Exp $ 
   ******************************************************* */

// Authors: Pascal Vincent

/*! \file openString.cc */


#include "openString.h"
#include <plearn/io/StringPStreamBuf.h>


namespace PLearn {
using namespace std;

  /**  Returns a PStream attached to the given string.
   *
   * @param s The string used as storage for stuff read from or written to
   * the PStream.
   *
   * @param io_formatting The type of PStream formatting that will be used
   * when reading/writing to the string. Common modes include
   * PStream::raw_ascii (for a normal ascii text file) and
   * PStream::plearn_ascii (for files in the PLearn serialization format).
   *   
   * @param openmode The mode (read/write/append) to open the string in. 
   * Use "r" for opening the string for reading, "w" for writing
   * (overwrites the string), or "a" for appending to the string. The
   * default is to open the string for reading ("r").
   */
  PStream openString(string& s, PStream::mode_t io_formatting,
                     const string& openmode)
  {
    PStream st = new StringPStreamBuf(&s, openmode);
    st.setMode(io_formatting);
    return st;
  }

  PStream openString(const string& s, PStream::mode_t io_formatting)
  {
    PStream st = new StringPStreamBuf(&s, "r");
    st.setMode(io_formatting);
    return st;
  }

} // end of namespace PLearn
