// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
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
   * $Id: IntVecFile.cc,v 1.1 2002/07/30 09:01:27 plearner Exp $
   * AUTHORS: Pascal Vincent
   * This file is part of the PLearn library.
   ******************************************************* */

#include "IntVecFile.h"
#include "fileutils.h"

namespace PLearn <%
using namespace std;

  void IntVecFile::open(const string& the_filename, bool readwrite)
  {
    if(f)
      close();

    filename = the_filename;
    if(readwrite)
      {
        f = fopen(filename.c_str(),"a+");
        if(!f)
          PLERROR("Couldn't open file %s for read/write",filename.c_str());
      }
    else
      {
        f = fopen(filename.c_str(),"r");
        if(!f)
          PLERROR("Couldn't open file %s for reading",filename.c_str());
      }
    length_ = file_size(filename)/sizeof(int);
  }

  int IntVecFile::get(int i) const
  {
#ifdef BOUNDCHECK
    if(i<0 || i>=length())
      PLERROR("Out Of Bounds in IntVecFile::get");
#endif
    fseek(f,i*sizeof(int),SEEK_SET);
    return fread_int(f,false);
  }

  void IntVecFile::put(int i, int value)
  {
    fseek(f,i*sizeof(int),SEEK_SET);
    fwrite_int(f,value,false);
    if(i>=length_)
      length_ = i+1;
  }

  void IntVecFile::close()
  {
    if(f)
      fclose(f);
    f=NULL;
  }

  IntVecFile::~IntVecFile()
  {
    close();
  }

  TVec<int> IntVecFile::getVec() const
  {
    int tt;
    TVec<int> res(length());
    fseek(f,0,SEEK_SET);
    if((tt=fread(res.data(), sizeof(int), length(), f))!=length())
      PLERROR("fread error in IntVecFile::getVec()");
    return res;
  }
  void IntVecFile::append(const TVec<int>& vec)
  {
    fseek(f,0,SEEK_END);
    fwrite(vec.data(), sizeof(int), vec.length(), f);
  }

%> // end of namespace PLearn



