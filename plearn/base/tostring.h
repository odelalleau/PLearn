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

#include <map>
#include <string>
#include <plearn/io/PStream.h>
#include <plearn/io/StringPStreamBuf.h>
// #include <plearn/io/openString.h>

namespace PLearn {
using namespace std;

/*
//!  converts anything to a string (same output as with cout << x)
template<class T> string tostring(const T& x);

template<class K, class V, class C, class A>
string tostring(const map<K, V, C, A>& x);
*/

//! Returns an internal static PStream pointing to a StringPStreamBuf
PStream& _tostring_static_pstream_(bool lock);
  
//!  specialised version for char*
/*
string tostring(const char* s,
                PStream::mode_t io_formatting = PStream::raw_ascii)
{ return string(s); }
*/

string tostring(const double& x,
                PStream::mode_t io_formatting = PStream::raw_ascii);

inline string tostring(const float& x,
                       PStream::mode_t io_formatting = PStream::raw_ascii)
{ return tostring(double(x), io_formatting); }


template<class T> 
string tostring2(const T& x, 
                 PStream::mode_t io_formatting = PStream::raw_ascii)
{
    /*
    string s;
    PStream out = openString(s, io_formatting, "w");
    out << x << flush;
    return s;
    */

    // get the PStream and set the lock
    PStream& out = _tostring_static_pstream_(true);
    StringPStreamBuf* pbuf = dynamic_cast<StringPStreamBuf*>((PStreamBuf*)out);
    pbuf->clear();
    out.setOutMode(io_formatting);
    out.clearOutMap();
    out << x;
    out.flush();
    // release the lock
    _tostring_static_pstream_(false);
    return pbuf->getString();
}

template<class T> 
string tostring(const T& x, PStream::mode_t io_formatting = PStream::raw_ascii)
{ return tostring2(x, io_formatting); }

/*! ******************
 * Implementation *
 ******************
 */

/*    
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
*/

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
