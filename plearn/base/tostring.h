// -*- C++ -*-

// tostring.h
//
// Copyright (C) 2005,2006 Christian Dorion, Pascal Vincent 
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

// Authors: Christian Dorion, Pascal Vincent

/*! \file tostring.h */


#ifndef tostring_INC
#define tostring_INC

#include <string>
#include <plearn/io/PStream.h>
#include <plearn/io/StringPStreamBuf.h>

namespace PLearn {
using namespace std;

//! Returns an internal static PStream pointing to a StringPStreamBuf
//! This should first be called with lock=true to acquire the lock on
//! the stream (and set the io_formatting and clear the string buffer)
//! It should then be called with lock=false to flush the stream and 
//! release the lock and clear the outmap.
PStream& _tostring_static_pstream_(bool lock, 
                                   PStream::mode_t io_formatting = PStream::raw_ascii);

string tostring(const double& x,
                PStream::mode_t io_formatting = PStream::raw_ascii);

inline string tostring(const float& x,
                       PStream::mode_t io_formatting = PStream::raw_ascii)
{ return tostring(double(x), io_formatting); }


template<class T> 
string tostring(const T& x, 
                PStream::mode_t io_formatting = PStream::raw_ascii)
{
    _tostring_static_pstream_(true, io_formatting) << x;
    return static_cast<StringPStreamBuf*>(
        (PStreamBuf*)_tostring_static_pstream_(false))->getString();
}

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
