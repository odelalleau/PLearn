// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2001 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2002 Pascal Vincent, Julien Keable, Xavier Saint-Mleux
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
   * $Id: FileVMatrix.h,v 1.12 2004/06/29 19:51:32 tihocan Exp $
   ******************************************************* */


/*! \file PLearnLibrary/PLearnCore/VMat.h */

#ifndef FileVMatrix_INC
#define FileVMatrix_INC

#include "RowBufferedVMatrix.h"

namespace PLearn {
using namespace std;
 

//!  A VMatrix that exists in a .pmat file (native plearn matrix format, same as for Mat)
class FileVMatrix: public RowBufferedVMatrix
{

private:

  typedef RowBufferedVMatrix inherited;

protected:

  string filename_;
  FILE* f;
  bool file_is_bigendian;
  bool file_is_float;

private:

  bool build_new_file;

public:

  FileVMatrix();
  FileVMatrix(const string& filename); //!<  opens an existing file
  FileVMatrix(const string& filename, int the_length, int the_width); //!<  create a new matrix file
  FileVMatrix(const string& filename, int the_length, const TVec<string>& fieldnames); //!<  create a new matrix file

  virtual void getNewRow(int i, Vec& v) const;

  //! Re-write the header with all current field values.
  virtual void updateHeader();

  virtual void put(int i, int j, real value);
  virtual void putSubRow(int i, int j, Vec v);
  virtual void appendRow(Vec v);
  virtual void flush();

  virtual void build();

  PLEARN_DECLARE_OBJECT(FileVMatrix);

  static void declareOptions(OptionList & ol);

  //! Transform a shallow copy into a deep copy.
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

  virtual ~FileVMatrix();

private:

  void build_();
};

DECLARE_OBJECT_PTR(FileVMatrix);

} // end of namespcae PLearn
#endif
