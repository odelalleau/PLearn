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
// If I don't add a definition of the namespace std .NET it does not compile
// (weird..)!
#ifdef WIN32
#include <string>
#endif

#include <stdlib.h>

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

//! calls endianswap2, 4, or 8 depending on elemsize
//! (an elemsize of 1 is also valid and does nothing)
inline void endianswap(void* ptr, int nelems, int elemsize)
{
    switch(elemsize)
    {
    case 1:
        break;
    case 2:
        endianswap2(ptr,nelems);
        break;
    case 4:
        endianswap4(ptr,nelems);
        break;
    case 8:
        endianswap8(ptr,nelems);
        break;
    default:
        abort();
    }
}


template<class T>
inline void endianswap(T* ptr, int n=1)
{ endianswap(ptr,n,sizeof(T)); }

}

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
