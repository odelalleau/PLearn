// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999,2000 Pascal Vincent, Yoshua Bengio and University of Montreal
// Copyright (C) 2002 Frederic Morin, Xavier Saint-Mleux, Pascal Vincent
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
   * $Id: pl_io.cc,v 1.2 2003/05/26 04:12:42 plearner Exp $
   * AUTHORS: Pascal Vincent
   * This file is part of the PLearn library.
   ******************************************************* */


/*! \file PLearnLibrary/PLearnCore/pl_io.cc */

#include "pl_io.h"
#include "plerror.h"
#include "pl_math.h"

namespace PLearn <%
using namespace std;


/* compressed array of float format
All data on disk are to be in little-endian format.
bit patterns described from a reading perspective

Attempt to read sizenum as 1 byte
Read 1 byte.
If the bit pattern is 00000000 (0x00) then the sizenum is the following 2-byte unsigned short : read it
If the bit pattern is 11000000 (0xC0)then the sizenum is the following 4-byte unsigned int : read it
Otherwise the sizenum is the unsigned byte just read

The two most significant bits of the sizenum are the mode bits, the remaining bits give a size
Mode 00 (decimal 0) means: insert 'size' zeros
Mode 01 (decimal 1) means: insert 'size' zeros and then a 1
Mode 10 (decimal 2) means: insert 'size' values to be found are in the following 'size' signed bytes
Mode 11 (decimal 3) means: insert 'size' values to be found are in the following 'size' floating points (different implementations for single and for double precision below
*/

inline void write_compr_mode_and_size(ostream& out, unsigned char mode, int size)
{
#ifdef BOUNDCHECK
  if(size<0 || size>=(1<<30))
    PLERROR("In write_compr_mode_and_size: size out of bounds");
#endif
  unsigned int imode = (unsigned int) mode;
  if(size<(1<<6))
    {
      unsigned char sizenum = (unsigned char) size | (unsigned char) (imode<<6);
      binwrite(out, sizenum);
    }
  else if(size<(1<<14))
    {
      unsigned short sizenum = (unsigned short) size | (unsigned short) (imode<<14);
      unsigned char header = 0x00;
      binwrite(out, header);
      binwrite(out, sizenum);
    }
  else
    {
      unsigned int sizenum = (unsigned int) size | (unsigned int) (imode<<30);
      unsigned char header = 0xC0;
      binwrite(out, header);
      binwrite(out, sizenum);
    }
}

inline void read_compr_mode_and_size(istream& in, unsigned char& mode, int& size)
{
  unsigned char sizenum_byte;
  binread(in, sizenum_byte);
  if(sizenum_byte==0x00) // sizenum is an unsigned short
    {
      unsigned short sizenum;
      binread(in, sizenum);
      mode = (unsigned char)(sizenum>>14);
      size = int(sizenum & (unsigned short)0x3FFF);
    }
  else if(sizenum_byte==0xC0) // sizenum is an unsigned int
    {
      unsigned int sizenum;
      binread(in, sizenum);
      mode = (unsigned char)(sizenum>>30);
      size = int(sizenum & (unsigned int)0x3FFFFFFF);
    }
  else // sizenum is the byte we just read
    {
      mode = sizenum_byte>>6;
      size = int(sizenum_byte & (unsigned char)0x3F);
    }
}

void binread_compressed(istream& in, double* data, int l)
{
  unsigned char mode;
  int n;
  double* p = data;
  char cval;
  while(l>0)
    {
      read_compr_mode_and_size(in, mode, n);
      //cerr << "mode: " << int(mode) << " size: " << n << endl;
      l -= n;
      switch(mode)
        {
        case 0:          
          while(n--)
            *p++ = 0;
          break;
        case 1:
          while(n--)
            *p++ = 0;
          *p++ = 1; 
          --l;
          break;
        case 2:
          while(n--)
            {
              binread(in,cval);
              *p++ = double(cval);
            }
          break;
        case 3:
          binread(in,p,n);
          p += n;
          break;
        default:
          PLERROR("BUG IN binread_compressed: mode is only 2 bits, so how can it be other than 0,1,2,3 ?");
        }
    }

  if(l!=0)
    PLERROR("In binread_compressed : l is not 0 at exit of function, wrong data?");
}

void binwrite_compressed(ostream& out, const double* data, int l)
{
  double val = 0.;
  while(l)
    {
      val = *data;
      if(val==0.)
        {
          int n=0;
          while(l && *data==0.)
            { ++n; ++data; --l; }
          if(l && *data==1.)
            {
              write_compr_mode_and_size(out, 1, n);
              ++data; --l;
            }
          else
            write_compr_mode_and_size(out, 0, n);              
        }
      else if(val==1.)
        {
          write_compr_mode_and_size(out, 1, 0);
          ++data; --l;
        }
      else if( double(char(val))==val )
        {
          const double* start = data;
          int n=0;
          while(l && double(char(val=*data))==val && val!=0 && val!=1)
            { ++n; ++data; --l; }
          write_compr_mode_and_size(out, 2, n);
          while(n--)
            binwrite(out,char(*start++));
        }
      else
        {
          const double* start = data;
          int n=0; 
          while(l && (val=*data)!=0 && val!=1 && double(char(val))!=val)
            { ++n; ++data; --l; }
          write_compr_mode_and_size(out, 3, n);
          binwrite(out,start,n);
        }
    }
}
void binread_compressed(istream& in, float* data, int l)
{
  unsigned char mode;
  int n;
  float* p = data;
  while(l>0)
    {
      read_compr_mode_and_size(in, mode, n);
      //cerr << "mode: " << int(mode) << " size: " << n << endl;
      if(mode==0 || mode==1)
        {
          while(n--)
            { *p++ = 0; --l; }
          if(mode==1)
            { *p++ = 1; --l; }
        }
      else if(mode==2)
        {
          char val; 
          while(n--)
            {
              binread(in,val);
              *p++ = float(val);
              --l;
            }
        }
      else if(mode==3)
        {
          binread(in,p,n);
          p += n;
          l -= n;
        }
      else 
        PLERROR("BUG IN binread_compressed: mode is only 2 bits, so how can it be other than 0,1,2,3 ?");
    }

  if(l!=0)
    PLERROR("In binread_compressed : l is not 0 at exit of function, wrong data?");
}

void binwrite_compressed(ostream& out, const float* data, int l)
{
  float val = 0.;
  while(l)
    {
      val = *data;
      if(val==0.)
        {
          int n=0;
          while(l && *data==0.)
            { ++n; ++data; --l; }
          if(l && *data==1.)
            {
              write_compr_mode_and_size(out, 1, n);
              ++data; --l;
            }
          else
            write_compr_mode_and_size(out, 0, n);              
        }
      else if(val==1.)
        {
          write_compr_mode_and_size(out, 1, 0);
          ++data; --l;
        }
      else if( float(char(val))==val )
        {
          const float* start = data;
          int n=0;
          while(l && float(char(val=*data))==val && val!=0 && val!=1)
            { ++n; ++data; --l; }
          write_compr_mode_and_size(out, 2, n);
          while(n--)
            binwrite(out,char(*start++));
        }
      else
        {
          const float* start = data;
          int n=0; 
          while(l && (val=*data)!=0 && val!=1 && float(char(val))!=val)
            { ++n; ++data; --l; }
          write_compr_mode_and_size(out, 3, n);
          binwrite(out,start,n);
        }
    }
}

// ********************************************
// *** compressed vector to and from memory ***
// ********************************************

inline void write_compr_mode_and_size_ptr(char*& out, unsigned char mode, int size)
{
  union {unsigned short s;char cs[2];} unis;
  union {unsigned int i;char ci[2];} unii;
#ifdef BOUNDCHECK
  if(size<0 || size>=(1<<30))
    PLERROR("In write_compr_mode_and_size: size out of bounds");
#endif
  unsigned int imode = (unsigned int) mode;
  if(size<(1<<6))
    {
      unsigned char sizenum = (unsigned char) size | (unsigned char) (imode<<6);
      (*out++) = sizenum;
    }
  else if(size<(1<<14))
    {
      unis.s = (unsigned short) size | (unsigned short) (imode<<14);
      unsigned char header = 0x00;
      (*out++) = header;
      (*out++) = unis.cs[0];
      (*out++) = unis.cs[1];
    }
  else
    {
      unii.i = (unsigned int) size | (unsigned int) (imode<<30);
      unsigned char header = 0xC0;
      (*out++) = header;
      (*out++) = unii.ci[0];
      (*out++) = unii.ci[1];
      (*out++) = unii.ci[2];
      (*out++) = unii.ci[3];
    }
}

inline void read_compr_mode_and_size_ptr(char*& in, unsigned char& mode, int& size)
{
  union {unsigned short s;char cs[2];} unis;
  union {unsigned int i;char ci[4];} unii;

  unsigned char sizenum_byte;
  sizenum_byte = (*in++);
  if(sizenum_byte==0x00) // sizenum is an unsigned short
    {
      unis.cs[0] = (*in++);
      unis.cs[1] = (*in++);
      mode = (unsigned char)(unis.s>>14);
      size = int(unis.s & (unsigned short)0x3FFF);
    }
  else if(sizenum_byte==0xC0) // sizenum is an unsigned int
    {
      unii.ci[0] = (*in++);
      unii.ci[1] = (*in++);
      unii.ci[2] = (*in++);
      unii.ci[3] = (*in++);
      mode = (unsigned char)(unii.i>>30);
      size = int(unii.i & (unsigned int)0x3FFFFFFF);
    }
  else // sizenum is the byte we just read
    {
      mode = sizenum_byte>>6;
      size = int(sizenum_byte & (unsigned char)0x3F);
    }
}


void uncompress_vec(char* comprbuf, double* data, int l, bool double_stored_as_float)
{
  unsigned char mode;
  int n;
  double* p = data;
  while(l>0)
    {
      read_compr_mode_and_size_ptr(comprbuf, mode, n);
      //cerr << "mode: " << int(mode) << " size: " << n << endl;
      if(mode==0 || mode==1)
        {
          while(n--)
            { *p++ = 0; --l; }
          if(mode==1)
            { *p++ = 1; --l; }
        }
      else if(mode==2)
        {
          char val; 
          while(n--)
            {
              val=(*comprbuf++);
              *p++ = double(val);
              --l;
            }
        }
      else if(mode==3)
        {
          memcpy(p,comprbuf,sizeof(double)*n);
          comprbuf+=sizeof(double)*n;
          p += n;
          l -= n;
        }
      else 
        PLERROR("BUG IN binread_compressed: mode is only 2 bits, so how can it be other than 0,1,2,3 ?");
    }

  if(l!=0)
    PLERROR("In binread_compressed : l is not 0 at exit of function, wrong data?");
}

void compress_vec(char* comprbuf, const double* data, int l, bool double_stored_as_float)
{
  //  char* comprbufold=comprbuf;
  double val = 0.;
  while(l)
    {
      val = *data;
      if(val==0.)
        {
          int n=0;
          while(l && *data==0.)
            { ++n; ++data; --l; }
          if(l && *data==1.)
            {
              write_compr_mode_and_size_ptr(comprbuf, 1, n);
              ++data; --l;
            }
          else
            write_compr_mode_and_size_ptr(comprbuf, 0, n);              
        }
      else if(val==1.)
        {
          write_compr_mode_and_size_ptr(comprbuf, 1, 0);
          ++data; --l;
        }
      else if( double(char(val))==val )
        {
          const double* start = data;
          int n=0;
          while(l && double(char(val=*data))==val && val!=0 && val!=1)
            { ++n; ++data; --l; }
          write_compr_mode_and_size_ptr(comprbuf, 2, n);
          while(n--)
            (*comprbuf++) = char(*start++);
        }
      else
        {
          const double* start = data;
          int n=0; 
          while(l && (val=*data)!=0 && val!=1 && double(char(val))!=val)
            { ++n; ++data; --l; }
          write_compr_mode_and_size_ptr(comprbuf, 3, n);
          memcpy(comprbuf,start,n*sizeof(double));
          comprbuf += n*sizeof(double);
        }
    }
}

double compressed_dot_product(char* comprbufvec, const double* vecdata, int l, bool double_stored_as_float)
{
  union { double d; char c[8]; };
  unsigned char mode;
  int n;
  double res=0;
  const double *p=vecdata;
  while(l>0)
    {
      read_compr_mode_and_size_ptr(comprbufvec, mode, n);
      if(mode==0 || mode==1)
        {
          p+=n;
          for(int j=0;j<n;j++)cout<<"0"<<endl;
          l-=n;
          if(mode==1)
            {         
              cout<<1<<endl;            
              --l; res+=*p++; 
            }
        }
      else if(mode==2)
        {
          while(n--)
            {
              cout<<(double)*comprbufvec<<endl;
              res+= *p++ * double(*comprbufvec++);
              --l;
            }
        }
      else if(mode==3)
        {
          while(n--)
            {
              memcpy(c,comprbufvec,sizeof(double));
              comprbufvec+=8;
              res+= *p++ * d;
              --l;
            }
        }
      else 
        PLERROR("BUG IN binread_compressed: mode is only 2 bits, so how can it be other than 0,1,2,3 ?");
    }

  if(l!=0)
    PLERROR("In compressed_dot_product : l is not 0 at exit of function, wrong data?");
  return res;
}


%> // end of namespace PLearn
