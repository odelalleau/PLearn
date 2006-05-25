// -*- C++ -*-

// tostring.cc
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

/*! \file tostring.cc */


#include "tostring.h"
#include <plearn/base/plerror.h>
#include <plearn/math/pl_math.h>    //!< For 'fast_exact_is_equal'.


namespace PLearn {
using namespace std;

PStream& _tostring_static_pstream_(bool lock, 
                                   PStream::mode_t io_formatting)
{
    static bool locked = false;
    static string s;
    static StringPStreamBuf* pbuf = 0;
    static PStream sout;    

    if(!pbuf)
    {
        pbuf = new StringPStreamBuf(&s, "w");
        sout = pbuf;
    }

    if(lock)
    {
        if(locked)
            exitmsg("In %s, already locked! Operations for PStream " 
                    "serialization (operator <<) should NEVER call "
                    "tostring", __FUNCTION__);             
        pbuf->clear();
        sout.setOutMode(io_formatting);
        locked = true;
    }
    else // unlock
    {
        if(!locked)
            exitmsg("In %s, already unlocked! "
                    "This should never happen", __FUNCTION__);
        sout.flush();
        sout.clearOutMap();
        locked = false;
    }

    return sout;
}

string tostring(const double& x, PStream::mode_t io_formatting)
{
    PStream& out = _tostring_static_pstream_(true, io_formatting);
    int ix = int(x);
    if (io_formatting==PStream::raw_ascii && fast_exact_is_equal(ix, x))
        out << ix;
    else
        out << x;
    return static_cast<StringPStreamBuf*>(
        (PStreamBuf*)_tostring_static_pstream_(false))->getString();
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
