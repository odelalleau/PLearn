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
   * $Id: MPIStream.cc,v 1.5 2004/02/26 06:49:40 nova77 Exp $
   * AUTHORS: Pascal Vincent & Yoshua Bengio
   * This file is part of the PLearn library.
   ******************************************************* */

#include "MPIStream.h"

#if USING_MPI

namespace PLearn {
using namespace std;


  // ******************
  // ** MPIStreambuf **
  // ******************

  MPIStreambuf::MPIStreambuf(int the_peerrank, int inbufsize) 
    :tag(PLMPI::tag), peerrank(the_peerrank),
    inbuf(0), inbuf_capacity(0)
  {
    if(inbufsize>0)
      reserveInputBuffer(inbufsize);
  }


  MPIStreambuf::~MPIStreambuf()
  {
    if(inbuf)
      delete[] inbuf;
  }

  streambuf* MPIStreambuf::setbuf(char* p, int len)
  {
    if(p && len>0)
      setp(p, p+len-1); // -1 because we want space to put the extra character passed to overflow()
    else
      setp(0,0);
    return this;
  }

  int MPIStreambuf::showmanyc()
  {
    int ready;
    MPI_Status status;
    MPI_Iprobe(peerrank, tag, MPI_COMM_WORLD, &ready, &status);
    if(ready)
      {
        int nready;
        MPI_Get_count(&status, MPI_CHAR, &nready);
        return nready;
      }
    return -1;
  }

  int MPIStreambuf::underflow()
  {
    // cerr<< "[ " << PLMPI::rank << " Entering underflow]";
    int msglength;
    MPI_Status status;
    MPI_Probe(peerrank, tag, MPI_COMM_WORLD, &status);
    MPI_Get_count(&status, MPI_CHAR, &msglength);
    reserveInputBuffer(msglength);    
    MPI_Recv(inbuf, msglength, MPI_CHAR, peerrank, tag, MPI_COMM_WORLD, &status);
    
    setg(inbuf, inbuf, inbuf+msglength);
    // cerr<< "[ " << PLMPI::rank << " msglen=" << msglength << " Leaving underflow]";
    return *inbuf;
  }
  

  // Done
  int MPIStreambuf::overflow(int c)
  {
    // cerr<< "[ " << PLMPI::rank << " Entering overflow]";
    if(c!=EOF)
      {
        if(pbase()) // buffered mode
          {
            streamsize n = pptr() - pbase();
            *pptr() = char(c); // put it in extra space
            if( MPI_Send(pbase(), n+1, MPI_CHAR, peerrank, tag, MPI_COMM_WORLD) != MPI_SUCCESS)
              return EOF;
            pbump(-n);
          }
        else // unbuffered mode
          {
            char tinybuf[1];
            tinybuf[0] = c;
            if( MPI_Send(tinybuf, 1, MPI_CHAR, peerrank, tag, MPI_COMM_WORLD) != MPI_SUCCESS)
              return EOF;
          }
      }
    else // extra char is EOF, we ignore it
      {
        if(pbase()) // buffered mode
          {
            streamsize n = pptr() - pbase();
            if( MPI_Send(pbase(), n, MPI_CHAR, peerrank, tag, MPI_COMM_WORLD) != MPI_SUCCESS)
              return EOF;
            pbump(-n);
          }
      }
    // cerr<< "[ " << PLMPI::rank << " Leaving overflow]";
    return 0;
  }

  int MPIStreambuf::sync()
  {
    // cerr<< "[ " << PLMPI::rank << " Entering sync]";
    streamsize n = pptr() - pbase();
    if(n>0)
      {
	if( MPI_Send(pbase(), n, MPI_CHAR, peerrank, tag, MPI_COMM_WORLD) != MPI_SUCCESS )
	  return EOF;
	pbump(-n);
      }

    // cerr<< "[ " << PLMPI::rank << " Leaving sync]";
    return 0;
  }
  
  // Smart implementation of xsputn:
  // If n is greater than the buffer size we send the message directly
  // Thus, we don't waste time copying stuff into the buffer unnecessarily
  // (ex: whhen sending a long vector for ex.).
  streamsize MPIStreambuf::xsputn(const char* s, streamsize n)
     {      
       // cerr<< "[ " << PLMPI::rank << " Entering xsputn]";
       if(n>epptr()-pptr())  // n greater than buffer size! 
         {
           // Let's not waste time copying stuff into the buffer, send it directly
           sync(); // first make sure we send what's left in the buffer
           if ( MPI_Send((char *)s, n, MPI_CHAR, peerrank, tag, MPI_COMM_WORLD) != MPI_SUCCESS )
            return 0;
         }
       else  // call the default method
         {
           streambuf::xsputn(s,n);
         }
       // cerr<< "[ " << PLMPI::rank << " Leaving xsputn]";
       return n;
     }

  // Smart implementation of xsgetn:
  // If the receive buffer is empty and the length of the next message is exactly n
  // we call MPI_Recv to directly fill s, rather than going through the input buffer
  streamsize MPIStreambuf::xsgetn(char* s, streamsize n)
  {
    // cerr<< "[ " << PLMPI::rank << " Entering xsgetn]";
    int msglength = -1;
    MPI_Status status;
    if(gptr()==eback()) // receive buffer empty
      {
        MPI_Probe(peerrank, tag, MPI_COMM_WORLD, &status);
        MPI_Get_count(&status, MPI_CHAR, &msglength);
      }

    if(msglength == n)
      {
        if( MPI_Recv(s, n, MPI_CHAR, peerrank, tag, MPI_COMM_WORLD, &status) != MPI_SUCCESS )
          return 0;
      }
    else  // call default method that will fill the buffer
      {
        streambuf::xsgetn(s, n);
      }
    // cerr<< "[ " << PLMPI::rank << " Leaving xsgetn]";
    return n;
  }
   

  // ***************
  // ** MPIStream **
  // ***************

    void MPIStream::init(int the_peerrank, int inbufsize, int outbufsize)
    {
      // cerr<< "[ " << PLMPI::rank << " Entering init]";
      rdbuf(new MPIStreambuf(the_peerrank, inbufsize));
      if(outbufsize<=1)
#if __GNUC__ < 3 && !defined(WIN32)
        rdbuf()->setbuf(0,0);
#else
        rdbuf()->pubsetbuf(0,0);
#endif
      else
        {
          outbuffer = new char[outbufsize];
#if __GNUC__ < 3 && !defined(WIN32)
          rdbuf()->setbuf(outbuffer,outbufsize);
#else
          rdbuf()->pubsetbuf(outbuffer,outbufsize);
#endif
        }
      // cerr<< "[ " << PLMPI::rank << " Leaving init]";
    }

    MPIStream::~MPIStream()
    {
      flush();
      delete rdbuf(0); // delete MPIStreambuf
      if(outbuffer)
        delete[] outbuffer;
    }

  // ****************
  // ** MPIStreams **
  // ****************

  MPIStreams::MPIStreams(int inbufsize, int outbufsize)
  {
    if(PLMPI::size==0)
      mpistreams = 0;
    else
      {
        mpistreams = new MPIStream[PLMPI::size];
        for(int k=0; k<PLMPI::size; k++)
          mpistreams[k].init(k, inbufsize, outbufsize);
      }
  }


} // end of namespace PLearn

// end of #if USING_MPI
#endif
