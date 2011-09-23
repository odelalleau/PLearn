// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2002 Xavier Saint-Mleux <saintmlx@iro.umontreal.ca>
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

/*!
 * pl_streambuf.{h|cc}
 * Defines a stream buffer that allows marking the current input position and
 * seeking back to that position after some extraction.  The marking is done 
 * using stream marker objects (class pl_streammarker).
 */


#ifndef pl_streambuf_INC
#define pl_streambuf_INC

//#include <iosfwd>
#include <iostream>
#include <cstdio> //Needed by g++ 4.5.1
//#include "plerror.h"
#include <plearn/base/PP.h>

namespace PLearn {

using namespace std;

class ioassignstream; //fwd decl.
class pl_streambuf; // fwd decl.
class pl_streammarker; // fwd decl.

/*!
 * pl_streambuf: stream buffer that allows marking.
 * This buffer makes a second layer of buffering over another stream buffer.
 */
class pl_streambuf : public streambuf, public PPointable
{
#if __GNUC__ < 3 && !defined(WIN32)
    typedef int int_type;
    typedef char char_type;
#endif

    static const int_type eof = EOF;

    friend class pl_streammarker;
    friend class ioassignstream;
  
private:
    typedef streambuf inherited;

protected:
  
    streambuf& original_buf; //!< original buffer that needs marking
    char* inbuf;             //!< buffer for marking: keeps all chars since first active mark
    int inbuflen;            //!< current length of inbuf
    static const int pback_size= 4;     //!< length of default put back area
    static const int min_buf_size= 16;  //!< minimum buffer size

    pl_streammarker* first_marker;  //!< ptr. to the head of a linked list of active markers

    virtual int_type underflow();   //!< underflow redefined
    virtual int_type uflow();       //!< uflow redefined


    virtual streamsize xsgetn(char* s, streamsize n);
    virtual streamsize xsputn(const char* s, streamsize n);


    virtual int_type overflow(int_type meta = pl_streambuf::eof); //!< overflow redefined
    virtual int_type sync();        //!< sync redefined
    virtual int_type pbackfail(int_type c= eof);  //!< pbackfail redefined

    //inline int curpos() const { return gptr() - inbuf; }  //!< return current position within the input buffer
    // norman: explicit cast:
    inline int curpos() const { return (int)(gptr() - inbuf); }  //!< return current position within the input buffer

public:

    //! ctor: needs a stream buffer
    pl_streambuf(streambuf& _original_buf, int_type _inbuflen= 0);
    virtual ~pl_streambuf();

    void seekmark(const pl_streammarker& mark);  //!< reposition input to the marked position

};

/*!
 * pl_streammarker: used to mark a position on a pl_streambuf
 */
class pl_streammarker
{
    friend class pl_streambuf;

protected:

    pl_streambuf* buf;  //!< marked buffer
    pl_streammarker* next_marker;  //!< next marker for the marked buffer (linked list)
    int pos;            //!< marked position

public:

    pl_streammarker(pl_streambuf* _buf);  //!< ctor. to mark a pl_streambuf
    pl_streammarker(streambuf* _buf);  //!< ctor. to mark an STL streambuf (...only if it's really a pl_streambuf)

    virtual ~pl_streammarker(); 

};


} // namespace PLearn

#endif //ndef pl_streambuf_INC


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
