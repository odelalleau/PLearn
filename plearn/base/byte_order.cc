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

#include "byte_order.h"

namespace PLearn {
using namespace std;

// *** LITTLE-ENDIAN / BIG-ENDIAN HELL... ***

// Functions to swap representation in memory

void endianswap2(void* ptr, int n)
{
    char *mptr = (char *) ptr;
    char tmp;
    while(n--)
    {
        tmp = mptr[0]; mptr[0]=mptr[1]; mptr[1]=tmp;
        mptr+=2;
    }
}

void endianswap4(void* ptr, int n)
{
    char *mptr = (char *) ptr;
    char tmp;
    while(n--)
    {
        tmp = mptr[0]; mptr[0]=mptr[3]; mptr[3]=tmp;
        tmp = mptr[1]; mptr[1]=mptr[2]; mptr[2]=tmp;
        mptr+=4;
    }
}

void endianswap8(void* ptr, int n)
{
    char *mptr = (char *) ptr;
    char tmp;
    while(n--)
    {
        tmp = mptr[0]; mptr[0]=mptr[7]; mptr[7]=tmp;
        tmp = mptr[1]; mptr[1]=mptr[6]; mptr[6]=tmp;
        tmp = mptr[2]; mptr[2]=mptr[5]; mptr[5]=tmp;
        tmp = mptr[3]; mptr[3]=mptr[4]; mptr[4]=tmp;
        mptr+=8;
    }
}


}


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
