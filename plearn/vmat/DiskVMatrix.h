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
   * $Id: DiskVMatrix.h,v 1.3 2003/07/24 00:48:53 ducharme Exp $
   ******************************************************* */


/*! \file PLearnLibrary/PLearnCore/VMat.h */

#ifndef DiskVMatrix_INC
#define DiskVMatrix_INC

#include "RowBufferedVMatrix.h"

namespace PLearn <%
using namespace std;
 

//!  A VMatrix whose (compressed) data resides in a directory and can span several files.
//!  Each row is compressed/decompressed through the methods of VecCompressor
class DiskVMatrix: public RowBufferedVMatrix
{
protected:
  mutable fstream * indexf;
  mutable Array<fstream*> dataf;
  bool readwritemode;
  bool freshnewfile;
public:
  string dirname;
  
  DiskVMatrix(){};

/*!     Opens an existing one. If directory does not exist or has missing files, an error is issued.
    If readwrite is true, then the files are opened in read/write mode and appendRow can be called.
    If readwrite is false (the default), then the files are opened in read only mode, and calling appendRow 
    will issue an error.
*/
  DiskVMatrix(const string& the_dirname, bool readwrite=false); 

/*!     Create a new one. 
    If directory already exist an error is issued
    (you may consider calling force_rmdir prior to this.)
    Howver if it is a file then the file is erased and replaced by a new directory
    (this was to allow TmpFilenames to be used with this class).
    Files are opened in read/write mode so appendRow can be called.
*/
  DiskVMatrix(const string& the_dirname, int the_width, bool write_double_as_float=false);

  virtual void getRow(int i, Vec v) const;
  virtual void putRow(int i, Vec v);
  virtual void appendRow(Vec v);
  virtual void flush();

  static void writeRow(ostream& out, const Vec& v);
  static void readRow(istream& in, const Vec& v);

  virtual void build();
  void build_();

  PLEARN_DECLARE_OBJECT_METHODS(DiskVMatrix, "DiskVMatrix", RowBufferedVMatrix);

  static void declareOptions(OptionList & ol);
  
  virtual ~DiskVMatrix();
};

DECLARE_OBJECT_PTR(DiskVMatrix);

%> // end of namespace PLearn
#endif
