// -*- C++ -*-

// openFile.cc
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
   * $Id: openFile.cc,v 1.3 2005/01/13 01:24:13 dorionc Exp $ 
   ******************************************************* */

// Authors: Pascal Vincent

/*! \file openFile.cc */


#include "openFile.h"

#if defined(WIN32)
// TO DO win32 implementation
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <plearn/io/FdPStreamBuf.h>
#endif

namespace PLearn {
using namespace std;

  /// @todo Switch to PrPStreamBuf to get Windows implementation.
  PStream openFile(const string& filepath, const string& openmode, PStream::mode_t io_formatting)
  {
    PStream st;
#if defined(WIN32) || STREAMBUFVER == 0
    PLERROR("openFile not yet implemented for windows");
#else    
//    cerr << "openFile(" << filepath << ", " << openmode << ")" << endl;
    if(openmode=="r")
      {
        int fd = open(filepath.c_str(),O_RDONLY|O_LARGEFILE);
        if(fd<0)
          PLERROR("openFile(\"%s\",\"%s\") failed.",filepath.c_str(), openmode.c_str());
        st = new FdPStreamBuf(fd, -1, true, false);
      }
    else if(openmode=="w")
      {
        int fd = open(filepath.c_str(),O_WRONLY|O_CREAT|O_TRUNC|O_LARGEFILE);
        if(fd<0)
          PLERROR("openFile(\"%s\",\"%s\") failed.",filepath.c_str(), openmode.c_str());
        st = new FdPStreamBuf(-1, fd, false, true);
      }      
    else if(openmode=="a")
      {
        int fd = open(filepath.c_str(),O_WRONLY|O_CREAT|O_APPEND|O_LARGEFILE);
        if(fd<0)
          PLERROR("openFile(\"%s\",\"%s\") failed.",filepath.c_str(), openmode.c_str());
        st = new FdPStreamBuf(-1, fd, false, true);
      }      
    else
      PLERROR("In openFile, invalid openmode=\"%s\" ",openmode.c_str());    
#endif
    
    st.setMode(io_formatting);
    return st;
  }

} // end of namespace PLearn
