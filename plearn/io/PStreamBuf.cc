// -*- C++ -*-

// PStreamBuf.cc
// 
// Copyright (C) 2003 Pascal Vincent
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

/*! \file PStreamBuf.cc */
#include "PStreamBuf.h"
#include <plearn/io/pl_log.h>

#define PSTREAMBUF_NO_GET (-1000)

namespace PLearn {
using namespace std;

PStreamBuf::PStreamBuf(bool is_readable_, bool is_writable_,
                       streamsize inbuf_capacity, streamsize outbuf_capacity, 
                       streamsize unget_capacity)
    :is_readable(is_readable_),
     is_writable(is_writable_),
     last_get(PSTREAMBUF_NO_GET),
     ungetsize(0),
     inbuf_chunksize(0),
     inbuf(0), inbuf_p(0), inbuf_end(0),
     outbuf_chunksize(0),
     outbuf(0), outbuf_p(0), outbuf_end(0)
{
    setBufferCapacities(inbuf_capacity, outbuf_capacity, unget_capacity);
}

PStreamBuf::~PStreamBuf() 
{
    if (inbuf)
        delete[] inbuf;
    if (outbuf)
        delete[] outbuf;
}

void PStreamBuf::setBufferCapacities(streamsize inbuf_capacity,
                                     streamsize outbuf_capacity, streamsize unget_capacity)
{
    if (inbuf_capacity < 1)
        inbuf_capacity = 1;
    if (unget_capacity < 1)
        unget_capacity = 1;
    ungetsize = unget_capacity;
    inbuf_chunksize = inbuf_capacity;
    outbuf_chunksize = outbuf_capacity;
    
    if (inbuf)
        delete[] inbuf;
    if (ungetsize + inbuf_chunksize <= 0)
        inbuf = inbuf_p = inbuf_end = 0;
    else
    {
        inbuf = new char[ungetsize+inbuf_chunksize];
        inbuf_p = inbuf+ungetsize;
        inbuf_end = inbuf_p;
    }

    if (outbuf)
    {
        flush();
        delete[] outbuf;
    }
    if (outbuf_chunksize <= 0)
        outbuf = outbuf_p = outbuf_end = 0;
    else
    {
        outbuf = new char[outbuf_chunksize];
        outbuf_p = outbuf;
        outbuf_end = outbuf+outbuf_chunksize;
    }
}

PStreamBuf::streamsize PStreamBuf::read_(char* p, streamsize n)
{
    PLERROR("read_ not implemented for this PStreamBuf");
    return 0;
}

//! writes exactly n characters from p (unbuffered, must flush)
//! Default version issues a PLERROR
void PStreamBuf::write_(const char* p, streamsize n)
{
    PLERROR("write_ not implemented for this PStreamBuf");
}
 
PStreamBuf::streamsize PStreamBuf::refill_in_buf()
{
#ifdef BOUNDCHECK
    if(!isReadable())
        PLERROR("Called PStreamBuf::refill_in_buf on a buffer not marked as readable");
#endif

    inbuf_p = inbuf + ungetsize;
    inbuf_end= inbuf_p; //buf empty until read_ finished
    streamsize n = read_(inbuf_p, inbuf_chunksize);
    inbuf_end = inbuf_p + n;
    return n;
}

PStreamBuf::streamsize PStreamBuf::read(char* p, streamsize n)
{
#ifdef BOUNDCHECK
    if(!isReadable())
        PLERROR("Called PStreamBuf::read on a buffer not marked as readable");
#endif

    streamsize nleft = n;

    streamsize inbuf_n = (streamsize)(inbuf_end-inbuf_p);
    if(inbuf_n) // First copy what's left in the buffer
    {
        streamsize k = nleft<inbuf_n ?nleft :inbuf_n;
        memcpy(p,inbuf_p,k);
        inbuf_p += k;
        p += k;
        nleft -= k;
    }

    if(nleft) // need some more ?
    {
        if(nleft>=inbuf_chunksize) // large block: read it directly
        {
            streamsize nr= read_(p,nleft);
            nleft-= nr;
            p+= nr;
            while(nleft > 0 && nr > 0) // need some more and not eof?
            {
                nr= read_(p,nleft);
                nleft-= nr;
                p+= nr;
            }
        }
        else // small block: read it in the buffer first
        {
            inbuf_n = refill_in_buf();
            while(nleft > 0 && inbuf_n > 0) // need some more and not eof?
            {
                streamsize k = nleft<inbuf_n ?nleft :inbuf_n;
                memcpy(p,inbuf_p,k);
                inbuf_p += k;
                nleft -= k;
                p+= k;
                if(nleft > 0)
                    inbuf_n = refill_in_buf();
            }
        }
    }

    streamsize nread = n-nleft;

    if (nread > 0)
        last_get = (unsigned char) p[-1];//p has advanced and now points one after the end
    else
        last_get = EOF;

    return nread;
}

void PStreamBuf::unread(const char* p, streamsize n)
{
    if(streamsize(inbuf_p-inbuf)<n)
        PLERROR("Cannot unread that many characters: %d, input buffer bound reached (you may want to increase the unget_capacity)", n);
  
    inbuf_p -= n;
    memcpy(inbuf_p,p,n);
}

void PStreamBuf::putback(char c)
{
    if(inbuf_p<=inbuf)
        PLERROR("Cannot putback('%c') Input buffer bound reached (you may want to increase the unget_capacity)",c);
  
    inbuf_p--;
    *inbuf_p = c;
}

void PStreamBuf::unget()
{
    if (last_get == PSTREAMBUF_NO_GET)
        PLERROR("In PStreamBuf::unget - You called unget() more than once or you did not call get() first");
    if (last_get != EOF)
        putback((char) last_get);
    last_get = PSTREAMBUF_NO_GET;
}


void PStreamBuf::flush()
{
    streamsize n = (streamsize)(outbuf_p-outbuf);
    if(n)
    {
        write_(outbuf, n);
        outbuf_p = outbuf;
    }
}


void PStreamBuf::write(const char* p, streamsize n)
{
#ifdef BOUNDCHECK
    if(!isWritable())
        PLERROR("Called PStreamBuf::write on a buffer not marked as writable");
#endif
    if(outbuf_chunksize>0) // buffered
    {
        streamsize bufrem = (streamsize)(outbuf_end-outbuf_p);
        if(n<=bufrem)
        {
            memcpy(outbuf_p, p, n);
            outbuf_p += n;
        }
        else // n>bufrem
        {
            memcpy(outbuf_p, p, bufrem);
            outbuf_p += bufrem;
            flush();
            p += bufrem;
            n -= bufrem;
            if(n>outbuf_chunksize)
                write_(p, n);
            else
            {
                memcpy(outbuf_p, p, n);
                outbuf_p += n;
            }
        }
    }
    else // unbuffered
        write_(p,n);
}

bool PStreamBuf::good() const
{
    if (is_readable)
        return !eof();
    else
        return is_writable;
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
