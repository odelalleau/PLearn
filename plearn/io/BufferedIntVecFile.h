// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
// Copyright (C) 2006 Xavier Saint-Mleux

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


#ifndef BufferedIntVecFile_INC
#define BufferedIntVecFile_INC

#include <plearn/io/IntVecFile.h>

namespace PLearn {
using namespace std;

class BufferedIntVecFile : public IntVecFile
{
protected:

  static const int DEFAULT_BUFLEN= 65536;

  typedef IntVecFile inherited;
  
  mutable int bufstart;
  const int buflen;
  mutable int* buf;
  mutable bool bufmod;

public:
    //!  Default constructor, you must then call open
    BufferedIntVecFile(int buflen_= DEFAULT_BUFLEN)
      : IntVecFile(), bufstart(-1), buflen(buflen_), bufmod(false)
      { buf= new int[buflen]; }
    BufferedIntVecFile(const string& the_filename, bool readwrite=false, int buflen_= DEFAULT_BUFLEN)
      : IntVecFile(the_filename, readwrite), bufstart(-1), buflen(buflen_), bufmod(false)
      { buf= new int[buflen]; }

    //! The copy constructor opens the file a second time in readonly mode only
    //BufferedIntVecFile(const IntVecFile& other);

    virtual void open(const string& the_filename, bool readwrite=false);
    virtual void close();

    virtual int get(int i) const;
    virtual void put(int i, int value);
  
    virtual inline int operator[](int i) const { return get(i); }
    virtual inline void append(int value) { put(length(),value); }

    virtual ~BufferedIntVecFile();

 protected:
    virtual void getBuf(int bufstart_);
    virtual void flush();

};


} // end of namespace PLearn

#endif


/*
  Local Variables:
  mode:c++
  c-basic-offset:4
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:79
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=79 :
