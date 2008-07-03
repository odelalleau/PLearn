// -*- C++ -*-

// PrPStreamBuf.cc
//
// Copyright (C) 2004 Christian Hudon 
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
 * $Id$ 
 ******************************************************* */

// Authors: Christian Hudon

/*! \file PrPStreamBuf.cc */


#include "PrPStreamBuf.h"
#include <plearn/base/PrUtils.h>
#include <nspr/prio.h>
#include <stdio.h>
#include "PStream.h"

namespace PLearn {
using namespace std;

PrPStreamBuf::PrPStreamBuf(PRFileDesc* in_, PRFileDesc* out_,
                           bool own_in_, bool own_out_)
    : PStreamBuf(in_ != 0, out_ != 0, 4096, 4096, default_ungetsize), 
      in(in_), out(out_), own_in(own_in_), own_out(own_out_)
{}

PrPStreamBuf::~PrPStreamBuf()
{
    const bool in_and_out_equal = (in == out);
    
    try {
        flush();
    }
    catch(...)
    {
        fprintf(stderr,"Could not properly clean up (flush) in destructor of PrPStreamBuf\n");
    }
    if (in && own_in)
    {
        PR_Close(in);
        in = 0;
    }
    if (out && own_out)
    {
        // If "in" and "out" were pointing to the same PRFileDesc,
        // don't close it a second time.
        if (!in_and_out_equal)
            PR_Close(out);
        out = 0;
    }
}

PrPStreamBuf::streamsize PrPStreamBuf::read_(char* p, streamsize n)
{
    PRInt32 nr= PR_Read(in, p, PRInt32(n));
    if(nr < 0)
        PLERROR((string("in PrPStreamBuf::read_ : no chars read: ") + getPrErrorString()).c_str());
    return nr;
}

//! writes exactly n characters from p (unbuffered, must flush)
void PrPStreamBuf::write_(const char* p, streamsize n)
{
    streamsize nwritten = ::PR_Write(out, p, PRInt32(n));
    if (nwritten != n)
        PLERROR("In PrPStreamBuf::write_ failed to write the requested number "
                "of bytes: wrote %ld instead of %ld",long(nwritten),long(n));
}
  
} // end of namespace PLearn


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
