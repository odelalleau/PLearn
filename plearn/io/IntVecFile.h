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
   * $Id: IntVecFile.h,v 1.1 2002/07/30 09:01:27 plearner Exp $
   * AUTHORS: Pascal Vincent
   * This file is part of the PLearn library.
   ******************************************************* */


/*! \file PLearnLibrary/PLearnCore/IntVecFile.h */

#ifndef IntVecFile_INC
#define IntVecFile_INC

#include <cstdlib>
#include "PP.h"
#include "Mat.h"

namespace PLearn <%
using namespace std;

  /*! 
IntVecFile is a class to handle a simple linear file of integers with random access.

Integers are stored in the file in plain little-endian binary
representation, with no header at all.  The "length" of the vector is
inferred directly from the size of the file.

Random access in both read and write are possible. And it is possible to
write beyond length(), in which case length() will be updated to reflect
the change
  */

class IntVecFile
{
protected:
  string filename;
  FILE* f;
  int length_;

public:
  //!  Default constructor, you must then call open
  IntVecFile(): filename(""), f(0), length_(-1) 
  {}

  IntVecFile(const string& the_filename, bool readwrite=false): f(0)
  { open(the_filename, readwrite); }

  void open(const string& the_filename, bool readwrite=false);
  void close();

  int get(int i) const;
  void put(int i, int value);
  
  TVec<int> getVec() const;
  

  inline int length() const { return length_; }
  inline int operator[](int i) const { return get(i); }
  inline void append(int value) { put(length(),value); }
  void append(const TVec<int>& vec);

  ~IntVecFile();
};


%> // end of namespace PLearn

#endif
