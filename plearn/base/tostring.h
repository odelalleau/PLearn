// -*- C++ -*-

// tostring.h
//
// Copyright (C) 2005 Christian Dorion 
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
   * $Id: tostring.h,v 1.2 2005/02/04 17:42:04 plearner Exp $ 
   ******************************************************* */

// Authors: Christian Dorion

/*! \file tostring.h */


#ifndef tostring_INC
#define tostring_INC

#include <map>
#include <string>
#include <plearn/io/PStream.h>
#include <plearn/io/openString.h>

namespace PLearn {
using namespace std;
  
//!  converts anything to a string (same output as with cout << x)
template<class T> string tostring(const T& x);

template<class K, class V, class C, class A>
string tostring(const map<K, V, C, A>& x);
  
//!  specialised version for char*
inline string tostring(const char* s) { return string(s); }

string tostring(const double& x);

string tostring(const float& x);


  template<class T> string tostring2(const T& x, 
                                     PStream::mode_t io_formatting = PStream::raw_ascii)
  {
    string s;
    PStream out = openString(s, io_formatting, "w");
    out << x << flush;
    return s;
  }
  
/*! ******************
    * Implementation *
    ******************
*/
    
  template<class T> string tostring(const T& x)
    {
      ostringstream out;
      out << x;
      return out.str();
    }

  template<class K, class V, class C, class A>
  string tostring(const map<K, V, C, A>& x)
    {
      ostringstream out;
      PStream pout(&out);
      pout << x;
      return out.str();
    }

} // end of namespace PLearn

#endif
