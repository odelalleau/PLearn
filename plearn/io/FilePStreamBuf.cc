// -*- C++ -*-

// FilePStreamBuf.cc
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
   * $Id: FilePStreamBuf.cc,v 1.2 2004/08/31 17:22:40 plearner Exp $ 
   ******************************************************* */

// Authors: Pascal Vincent

/*! \file FilePStreamBuf.cc */


#include "FilePStreamBuf.h"

namespace PLearn {
using namespace std;

  string FilePStreamBuf::getFilePathFromURL(string fileurl)
  {    
    return fileurl; // (for now. Will do sth more fancy later)
  }

FilePStreamBuf::FilePStreamBuf() 
  : f(0)
/* ### Initialise all fields to their default value */
  {
    // ...

    // ### You may or may not want to call build_() to finish building the object
    // build_();
  }

  FilePStreamBuf::~FilePStreamBuf()
  {
    if(f)
      fclose(f);      
  }


  PLEARN_IMPLEMENT_OBJECT(FilePStreamBuf, "ONE LINE DESCRIPTION", "MULTI LINE\nHELP");

  void FilePStreamBuf::declareOptions(OptionList& ol)
  {
    declareOption(ol, "url", &FilePStreamBuf::url, OptionBase::buildoption,
                  "Uniform resource location of file");

    declareOption(ol, "openmode", &FilePStreamBuf::openmode, OptionBase::buildoption,
                  "Uniform resource location of file");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
  }

  void FilePStreamBuf::build_()
  {
    string filepath = getFilePathFromURL(url);

    is_random_accessible = true;
    if(openmode=="r")
      {
        is_readable = true;
        is_writable = false;
      }
    else if(openmode=="w" || openmode=="a")
      {
        is_readable = false;
        is_writable = true;
      }
    else if(openmode=="r+" || openmode=="w+" || openmode=="a+")
      {
        is_readable = true;
        is_writable = true;
      }
    else
      PLERROR("In FilePStreamBuf::build Invalid openmode. Must be one of r, w, a, r+, w+, a+. The 'b' character is not used to denote 'binary mode'. All files are opened in binary mode anyway.");

    string binaryopenmode = openmode+'b'; // binary for windows
    f = fopen(filepath.c_str(), binaryopenmode.c_str());
    if(!f)
      PLERROR("Failed to open file %s in mode %s",filepath.c_str(), openmode.c_str());
  }

  // ### Nothing to add here, simply calls build_
  void FilePStreamBuf::build()
  {
    inherited::build();
    build_();
  }

  FilePStreamBuf::streamsize FilePStreamBuf::read_(char* p, streamsize n)
  {
    return fread(p, 1, n, f);
  }

  //! writes exactly n characters from p (unbuffered, must flush)
  void FilePStreamBuf::write_(const char* p, streamsize n)
  {
    streamsize nwritten = fwrite(p, 1, n, f);
    if(nwritten!=n)
      PLERROR("In FilePStreamBuf::write_ failed to write the requested number of bytes");
    fflush(f);
  }

  void FilePStreamBuf::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
  {
    inherited::makeDeepCopyFromShallowCopy(copies);
    // deepCopyField(trainvec, copies);
    f = 0;
    build();
  }

} // end of namespace PLearn
