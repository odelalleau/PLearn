// -*- C++ -*-

// StringPStreamBuf.h
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

/*! \file StringPStreamBuf.h */


#ifndef StringPStreamBuf_INC
#define StringPStreamBuf_INC

#include "PStreamBuf.h"
#include <string>

namespace PLearn {
using namespace std;

class StringPStreamBuf: public PStreamBuf
{

private:
  
    typedef PStreamBuf inherited;

protected:
    // *********************
    // * protected options *
    // *********************
  
    string* st;
    string::size_type read_index;
    bool own_string; // if true st will be destroyed

public:

    StringPStreamBuf(string* ptrs, const string& openmode="r", bool own_string_=false,
                     streamsize inbuf_capacity=1000, streamsize outbuf_capacity=1000, streamsize unget_capacity=100);

    StringPStreamBuf(const string* ptrs, const string& openmode="r", bool own_string_=false, 
                     streamsize inbuf_capacity=1000, streamsize unget_capacity=100);

    inline const string& getString() const
    { PLASSERT(st); return *st; }

    inline void clear() 
    { 
        flush();        
        st->clear();        
    }

    virtual ~StringPStreamBuf();

protected:

    virtual streamsize read_(char* p, streamsize n);

    //! writes exactly n characters from p (unbuffered, must flush)
    virtual void write_(const char* p, streamsize n);

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
