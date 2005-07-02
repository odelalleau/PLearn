// -*- C++ -*-

// TinyVectorIO.h
//
// Copyright (C) 2005 Nicolas Chapados 
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
   * $Id: .pyskeleton_header,v 1.1 2003/09/01 00:05:31 plearner Exp $ 
   ******************************************************* */

// Authors: Nicolas Chapados

/*! \file TinyVectorIO.h */


#ifndef TinyVectorIO_INC
#define TinyVectorIO_INC

// From C++ standard library
#include <iostream>

// From PLearn
#include <plearn/base/TinyVector.h>
#include <plearn/math/TVec.h>
#include "PStream.h"

namespace PLearn {

/**
 *  This file provides I/O functionality of TinyVectors for PStreams
 */

template <class T, unsigned N, class TTrait>
ostream& operator<<(ostream& os, const TinyVector<T,N,TTrait>& tiny_vec)
{
  os << '[';
  for (int i=0, n=int(tiny_vec.size()); i<n ; ++i) {
    os << tiny_vec[i];
    if (i < n-1)
      os << ',';
  }
  return os << ']';
}

template <class T, unsigned N, class TTrait>
PStream& operator<<(PStream& os, const TinyVector<T,N,TTrait>& tiny_vec)
{
  os << '[';
  for (int i=0, n=int(tiny_vec.size()); i<n ; ++i) {
    os << tiny_vec[i];
    if (i < n-1)
      os << ',';
  }
  return os << ']';
}


template <class T, unsigned N, class TTrait>
PStream& operator>>(PStream& is, TinyVector<T,N,TTrait>& tiny_vec)
{
  TVec<T> v;
  is >> v;
  tiny_vec.resize(v.size);
  copy(v.begin(), v.end(), tiny_vec.begin());
  return is;
}

} // end of namespace PLearn

#endif
