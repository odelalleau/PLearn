// -*- C++ -*-

// MPIPStreamBuf.cc
//
// Copyright (C) 2005 Pascal Vincent 
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
 * $Id: MPIPStreamBuf.cc $ 
 ******************************************************* */

// Authors: Pascal Vincent

/*! \file MPIPStreamBuf.cc */


#include "MPIPStreamBuf.h"

#include <mpi/mpi.h>
#include <plearn/sys/PLMPI.h>

namespace PLearn {
using namespace std;

MPIPStreamBuf::MPIPStreamBuf(int peer_rank, streamsize chunksize)
    : PStreamBuf(true, true, chunksize, chunksize, default_ungetsize), 
      mpibuf(0), 
      mpibuf_capacity(0),
      mpibuf_p(0),
      mpibuf_len(0),
      reached_eof(false),
      max_chunksize(chunksize),
      peerrank(peer_rank)
{}

MPIPStreamBuf::~MPIPStreamBuf()
{    
    if(mpibuf)
        delete[] mpibuf;
}

void MPIPStreamBuf::fill_mpibuf(int msglength)
{
    if (msglength>mpibuf_capacity)
    {
        if(mpibuf)
            delete[] mpibuf;
        // inrease capacity
        mpibuf_capacity = max(msglength,mpibuf_capacity*2);
        mpibuf = new char[mpibuf_capacity];
    }    
    MPI_Status status;
    mpibuf_p = mpibuf;
    MPI_Recv(mpibuf_p, msglength, MPI_CHAR, peerrank, PLMPI::tag, MPI_COMM_WORLD, &status);
    mpibuf_len = msglength;
}

MPIPStreamBuf::streamsize MPIPStreamBuf::read_(char* p, streamsize n)
{
    int msglength = 0;
    MPI_Status status;
    
    streamsize nread = 0;
    if (mpibuf_len>0)
    {
        streamsize m = min(n,mpibuf_len);
        copy(mpibuf_p, mpibuf_p+m, p);
        p += m;
        mpibuf_p += m;
        mpibuf_len -= m;
        n -= m;
        nread += m;
    }
    if (!reached_eof && n>0)
    {
        // MPI_Probe(peerrank, tag, MPI_COMM_WORLD, &status);
        MPI_Get_count(&status, MPI_CHAR, &msglength);        
        if (msglength==0) // we reached EOF
            reached_eof = true;
        else
        {
            if (msglength<=(int)n)
            {
                MPI_Recv(p, msglength, MPI_CHAR, peerrank, PLMPI::tag, MPI_COMM_WORLD, &status);
                nread += msglength;
            }
            else // msglength>n
            {
                fill_mpibuf(msglength);
                copy(mpibuf_p, mpibuf_p+n, p);
                mpibuf_p += n;
                mpibuf_len -= n;
                nread += n;
            }
        }
    }
    return nread; 
}

//! writes exactly n characters from p (unbuffered, must flush)
void MPIPStreamBuf::write_(const char* p, streamsize n)
{
    while (n>0)
    {
        streamsize m = min(max_chunksize,n);
        if( MPI_Send((void *)p, m, MPI_CHAR, peerrank, PLMPI::tag, MPI_COMM_WORLD) != MPI_SUCCESS )
            PLERROR("MPI_Send failed");
        p += m;
        n -= m;
    }
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
