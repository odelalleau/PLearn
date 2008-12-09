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


#include "pl_streambuf.h"


using namespace PLearn;
using namespace std;

/*****
 * pl_streambuf
 */

//! ctor: create an input buffer
pl_streambuf::pl_streambuf(streambuf& _original_buf, int _inbuflen) 
    :original_buf(_original_buf), inbuf(0), inbuflen(_inbuflen), first_marker(0)
{
    //set minimum size for buffer
    if(inbuflen < pback_size+1)
        inbuflen= min_buf_size + pback_size;

    inbuf= new char[inbuflen];
    setg(inbuf+pback_size, inbuf+pback_size, inbuf+pback_size); // reserve a few bytes for a putback area.
}

//! destructor: delete the allocated input buffer
pl_streambuf::~pl_streambuf()
{
    if(inbuf)
        delete[] inbuf;
}

//! underflow: If no mark, get from underlying buffer.
//! If buffer is marked, grow/fill buffer as necessary and return next char.
pl_streambuf::int_type pl_streambuf::underflow()
{
    //no marker: return get from original buffer.
    if(first_marker == 0 && gptr() == egptr())
    {
        //delete buffer if we have a long one that is not used anymore
        if(inbuflen > min_buf_size + pback_size)
	{ //don't keep copy of last char: underlying buf. will be used for putbacks.
            delete[] inbuf;
            inbuflen= min_buf_size + pback_size;
            inbuf= new char[inbuflen];
            setg(inbuf+pback_size, inbuf+pback_size, inbuf+pback_size);
	}
        return original_buf.sgetc();
    }
  
    //marked buffer:

    //return a buffered char if any is available (is this necessary? -xsm)
    if(gptr() < egptr())
        return *gptr(); 

    int oldbuflen = int(egptr()-inbuf); //< current length used
    //if at end of buffer, make it twice as long as before
    if(egptr() == inbuf+inbuflen)
    {
        //create a new longer buffer
        int newbuflen= inbuflen*2;
        char* newbuf= new char[newbuflen];
      
        //copy from current buf. to new one
        for(int i= 0; i < inbuflen; ++i)
            newbuf[i]= inbuf[i];

        //reposition get pointers
        setg(newbuf+pback_size, newbuf+(gptr()-inbuf), newbuf+inbuflen);
        delete[] inbuf; //< delete prev. buffer
        inbuf= newbuf;  //< point to new buffer
        inbuflen= newbuflen; //< adjust current buffer length
    }
      
    char* the_egptr= 0;  //ptr. to actual end of buf. (not known yet)

    //fill buffer from underlying streambuf
    for(int i= oldbuflen; i < inbuflen; ++i)
        if((original_buf.sgetc() != pl_streambuf::eof && original_buf.in_avail()) || i == oldbuflen)
            inbuf[i]= original_buf.sbumpc();  //< get a char from underlying streambuf and advance it's pos.
        else
        { //no input available: stop filling buffer (set egptr at current pos)
            the_egptr= inbuf+i;
            break;
        }

    if(the_egptr == 0) //buf. all filled: set egptr at end of buf.
        the_egptr= inbuf + inbuflen;

    //set pointers into buffer
    setg(eback(), gptr(), the_egptr);
    if(gptr() < egptr()) //< got some new stuff?
        return *gptr();    //< return next char.
    return pl_streambuf::eof; //< at eof: return eof.
}


pl_streambuf::int_type pl_streambuf::uflow()
{
    int c= underflow(); //< get char. at current pos.
    if(first_marker == 0 && gptr() == egptr()) 
        original_buf.sbumpc();  //< no mark: advance pos. of original streambuf
    else
        gbump(1);               //< mark(s): advance pos. in our buffer
    return c;
}

streamsize pl_streambuf::xsgetn(char_type* s, streamsize n)
{
    int_type c= uflow();
    int i;
    for(i= 0; i < n && c != pl_streambuf::eof; ++i)
    {
        s[i]= static_cast<char_type>(c);
        c= uflow();
    }
    return i;
}

streamsize pl_streambuf::xsputn(const char_type* s, streamsize n)
{
    int_type c = 1;
    int i;
    for(i= 0; i < n && c != pl_streambuf::eof; ++i)
        c= overflow(static_cast<int_type>(s[i]));
    return i;
}


pl_streambuf::int_type pl_streambuf::overflow(int_type meta) //trivial overflow
{ return original_buf.sputc(meta); }           //(no marking on output == no buffering)

void pl_streambuf::seekmark(const pl_streammarker& mark)
{ setg(eback(), inbuf+mark.pos, egptr()); } //< set get ptr. to mark position

pl_streambuf::int_type pl_streambuf::sync()
{ // no marking on output: sync underlying streambuf
#if __GNUC__ < 3 && !defined(WIN32)
    return original_buf.sync();
#else
    return original_buf.pubsync();
#endif
}

pl_streambuf::int_type pl_streambuf::pbackfail(int_type c)
{ return original_buf.sungetc(); } //< pback before beginning of our buf: pback in underlying streambuf




/*****
 * pl_streammarker
 */

//ctor. from pl_streambuf: insert self in buf's list of markers and remember current pos.
pl_streammarker::pl_streammarker(pl_streambuf* _buf)
    :buf(_buf), next_marker(buf->first_marker), pos(buf->curpos())
{ buf->first_marker= this; }

//ctor. from any streambuf: just make sure it's a pl_streambuf and construct normally
pl_streammarker::pl_streammarker(streambuf* _buf)
    :buf(dynamic_cast<pl_streambuf*>(_buf))
{
    if(buf == 0)
        PLERROR("Cannot put a pl_streammarker on a streambuf that is not a pl_streambuf...");
    next_marker= buf->first_marker;
    pos= buf->curpos();
  
    buf->first_marker= this; 
}

//destructor: simply remove self from buf's list of markers.
pl_streammarker::~pl_streammarker()
{
    pl_streammarker* prev= 0; 
    for(pl_streammarker* it= buf->first_marker; it != 0; prev= it, it!=0?it= it->next_marker:it= 0)
        if(it == this)
        {
            if(prev == 0)
                buf->first_marker= it->next_marker;
            else
                prev->next_marker= it->next_marker;
            it= 0;
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
