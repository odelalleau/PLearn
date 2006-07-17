// -*- C++ -*-

// FdPStreamBuf.cc
//
// Copyright (C) 2004 Pascal Vincent 
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

// Authors: Pascal Vincent

/*! \file FdPStreamBuf.cc */

#include "FdPStreamBuf.h"

#if defined(WIN32) && !defined(__CYGWIN__) && !defined(_MINGW_)
// Dummy implementation to prevent compilation and link errors, as the "normal"
// code does not compiler under Windows.

namespace PLearn {

FdPStreamBuf::FdPStreamBuf(int in_fd, int out_fd,
						   bool own_in_, bool own_out_):
	PStreamBuf(true, false)
{
	PLERROR("The FdPStreamBuf class cannot be used on this system");
}

FdPStreamBuf::~FdPStreamBuf() {}

FdPStreamBuf::streamsize FdPStreamBuf::read_(char* p, streamsize n)
{
	return 0;
}

void FdPStreamBuf::write_(const char* p, streamsize n) {}

}

#else // POSIX

#include <unistd.h>

namespace PLearn {
using namespace std;

FdPStreamBuf::FdPStreamBuf(int in_fd, int out_fd,
                           bool own_in_, bool own_out_)
    :PStreamBuf(in_fd>=0, out_fd>=0, 4096, 4096, default_ungetsize), 
     in(in_fd), out(out_fd), own_in(own_in_), own_out(own_out_)
{}

FdPStreamBuf::~FdPStreamBuf()
{
    const bool in_and_out_equal = (in == out);

    flush();
    if(in>=0 && own_in)
    {
        ::close(in);
        in = -1;
    }
    if(out>=0 && own_out)
    {
        if (!in_and_out_equal)
            ::close(out);
        out = -1;
    }
}

FdPStreamBuf::streamsize FdPStreamBuf::read_(char* p, streamsize n)
{
    return ::read(in, p, n);
}

//! writes exactly n characters from p (unbuffered, must flush)
void FdPStreamBuf::write_(const char* p, streamsize n)
{
    streamsize nwritten = ::write(out, p, n);
    if(nwritten!=n)
        PLERROR("In FdPStreamBuf::write_ failed to write the requested number of bytes");
#ifdef _MINGW_
    PLERROR("In FdPStreamBuf::write_ - The 'fsync' function is not defined "
            "under MinGW (see code)");
#else
    fsync(out);
#endif
}
  
} // end of namespace PLearn

#endif // WIN32 vs POSIX


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
