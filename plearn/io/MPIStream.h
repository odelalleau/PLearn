// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
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
 * AUTHORS: Pascal Vincent 
 * This file is part of the PLearn library.
 ******************************************************* */


/*! \file PLearn/plearn/io/MPIStream.h */

#ifndef MPIStream_INC
#define MPIStream_INC

#if USING_MPI

#include <iostream>
#include <plearn/sys/PLMPI.h>

namespace PLearn {
using namespace std;

class MPIStreambuf : public streambuf
{
protected:
    int tag; //!<  the tag used for all MPI send/receive
    int peerrank; //!<  rank number of peer processor in the COMM_WORLD (the to/from which to send/receive)
  
    char* inbuf;
    int inbuf_capacity;
  
protected:

    //!  inline for efficiency
    inline void reserveInputBuffer(int buflen)
    {
        if(buflen>inbuf_capacity)
        {
            if(inbuf)
                delete[] inbuf;
            inbuf = new char[buflen];
            inbuf_capacity = buflen;
        }
    }

    //!   virtual void imbue(const locale& loc)
    virtual int overflow(int c=EOF);
    virtual int underflow();
    // virtual int uflow();
    //!   virtual int pbackfail(int c);
    virtual int showmanyc();
    virtual streamsize xsputn(const char* s, streamsize n);
    virtual streamsize xsgetn(char* s, streamsize n);
    //!  virtual streampos seekoff(streamoff, _seek_dir, int mode=ios::in|ios::out);
    //!  virtual streampos seekpos(streampos pos, int mode = ios::in|ios::out);
    virtual streambuf* setbuf(char* p, int len);
    virtual int sync();

public:
    MPIStreambuf(int the_peerrank, int inbufsize);
    virtual ~MPIStreambuf();

};
  
class MPIStream: public iostream
{
protected:
    char* outbuffer;

public:

    MPIStream()
        :iostream(0), outbuffer(0)
    {}

    //!  The peer number of the other node, and initial size of input-buffer and output buffers
    MPIStream(int the_peerrank, int inbufsize=200, int outbufsize=200)
        :iostream(0), outbuffer(0)
    { init(the_peerrank, inbufsize, outbufsize); }

    //!  This function may be called only once, if the stream was built with the default constructor.
    void init(int the_peerrank, int inbufsize= 0, int outbufsize= 0);

    ~MPIStream();
};


// implements an "array" with streams open to all nodes
class MPIStreams
{
protected:
    MPIStream* mpistreams;

public:
    MPIStreams(int inbufsize=200, int outbufsize=200);

    int size() const { return PLMPI::size; } 
    int length() const { return PLMPI::size; } 

    iostream& operator[](int i)
    {
#ifdef BOUNDCHECK
        if(i<0 || i>=PLMPI::size)
            PLERROR("In MPIStreams OUT OF BOUND ACCESS");
#endif      
        return mpistreams[i];
    }

    ~MPIStreams() 
    { if(mpistreams) delete[] mpistreams; }
};

} // end of namespace PLearn

//!  end of #if USING_MPI
#endif 

//!  end of #ifndef MPIStream_INC
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
