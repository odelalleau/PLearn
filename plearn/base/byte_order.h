// -*- C++ -*-

// byte_order.h
// Copyright (C) 1998-2002 Pascal Vincent
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


#ifndef byte_order_INC
#define byte_order_INC

// norman:
// If I don't add a definition of the namespace std .NET it does not compile (weird..)!
#ifdef WIN32
#include <string>
#endif

namespace PLearn {
using namespace std;

//!                WELCOME TO:
//!  *** LITTLE-ENDIAN / BIG-ENDIAN HELL... ***

#define LITTLE_ENDIAN_ORDER 'L'
#define BIG_ENDIAN_ORDER 'B'

inline char byte_order()
{
#ifdef BIGENDIAN
  return BIG_ENDIAN_ORDER; 
#else
  return LITTLE_ENDIAN_ORDER;
#endif
}

//! swaps endians for n 2-byte elements (such as short)
void endianswap2(void* ptr, int n);
//! swaps endians for n 4-byte elements (such as int or float)
void endianswap4(void* ptr, int n);
//! swaps endians for n 8-byte elements (such as double)
void endianswap8(void* ptr, int n);

// Version for char and unsigned char (I know this is useless, but some code relies on it being defined)
inline void endianswap(char* ptr, int n=1) {}
inline void endianswap(signed char* ptr, int n=1) {}
inline void endianswap(unsigned char* ptr, int n=1) {}

// Versions for short, int, long, float and double
inline void endianswap(short* ptr, int n=1) { endianswap2(ptr,n); }
inline void endianswap(unsigned short* ptr, int n=1) { endianswap2(ptr,n); }
inline void endianswap(int* ptr, int n=1) { endianswap4(ptr,n); }
inline void endianswap(unsigned int* ptr, int n=1) { endianswap4(ptr,n); }
inline void endianswap(long* ptr, int n=1) { endianswap4(ptr,n); }
inline void endianswap(unsigned long* ptr, int n=1) { endianswap4(ptr,n); }
inline void endianswap(float* ptr, int n=1) { endianswap4(ptr,n); }
inline void endianswap(double* ptr, int n=1) { endianswap8(ptr,n); }


}

#endif
