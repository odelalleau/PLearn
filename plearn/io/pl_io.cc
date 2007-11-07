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
 * $Id$
 * AUTHORS: Pascal Vincent
 * This file is part of the PLearn library.
 ******************************************************* */


/*! \file PLearn/plearn/io/pl_io.cc */

//#include <limits>
#include "pl_io.h"
#include <plearn/base/plerror.h>
//#include "pl_math.h"
#include <plearn/base/byte_order.h> //!< For endianswap.

namespace PLearn {
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
        if(fast_exact_is_equal(val, 0.))
        {
            int n=0;
            while(l && fast_exact_is_equal(*data, 0.))
            { ++n; ++data; --l; }
            if(l && fast_exact_is_equal(*data, 1.))
            {
                write_compr_mode_and_size(out, 1, n);
                ++data; --l;
            }
            else
                write_compr_mode_and_size(out, 0, n);              
        }
        else if(fast_exact_is_equal(val,1.))
        {
            write_compr_mode_and_size(out, 1, 0);
            ++data; --l;
        }
        else if( fast_exact_is_equal(double(char(val)),val) )
        {
            const double* start = data;
            int n=0;
            while(l && fast_exact_is_equal(double(char(val=*data)), val)
                    && !fast_exact_is_equal(val, 0)
                    && !fast_exact_is_equal(val, 1))
            { ++n; ++data; --l; }
            write_compr_mode_and_size(out, 2, n);
            while(n--)
                binwrite(out,char(*start++));
        }
        else
        {
            const double* start = data;
            int n=0; 
            while(l && !fast_exact_is_equal((val=*data), 0)
                    && !fast_exact_is_equal(val, 1)
                    && !fast_exact_is_equal(double(char(val)), val))
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
        if(fast_exact_is_equal(val, 0.))
        {
            int n=0;
            while(l && fast_exact_is_equal(*data, 0.))
            { ++n; ++data; --l; }
            if(l && fast_exact_is_equal(*data, 1.))
            {
                write_compr_mode_and_size(out, 1, n);
                ++data; --l;
            }
            else
                write_compr_mode_and_size(out, 0, n);              
        }
        else if(fast_exact_is_equal(val, 1.))
        {
            write_compr_mode_and_size(out, 1, 0);
            ++data; --l;
        }
        else if( fast_exact_is_equal(float(char(val)), val) )
        {
            const float* start = data;
            int n=0;
            while(l && fast_exact_is_equal(float(char(val=*data)), val)
                    && !fast_exact_is_equal(val, 0)
                    && !fast_exact_is_equal(val, 1))
            { ++n; ++data; --l; }
            write_compr_mode_and_size(out, 2, n);
            while(n--)
                binwrite(out,char(*start++));
        }
        else
        {
            const float* start = data;
            int n=0; 
            while(l && !fast_exact_is_equal((val=*data), 0)
                    && !fast_exact_is_equal(val, 1)
                    && !fast_exact_is_equal(float(char(val)), val))
            { ++n; ++data; --l; }
            write_compr_mode_and_size(out, 3, n);
            binwrite(out,start,n);
        }
    }
}

// ********************************************
// *** compressed vector to and from FILE* ***
// ********************************************

inline void read_compr_mode_and_size(FILE* in, unsigned char& mode, int& size)
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

void binread_compressed(FILE* in, double* data, int l)
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

void binwrite_compressed(FILE* out, const double* data, int l)
{
    PLERROR("Not implemented");
}

void binread_compressed(FILE* in, float* data, int l)
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

void binwrite_compressed(FILE* out, const float* data, int l)
{
    PLERROR("Not implemented");
}

// ********************************************
// *** compressed vector to and from memory ***
// ********************************************

inline void write_compr_mode_and_size_ptr(char*& out, unsigned char mode, int size)
{
    union {unsigned short s;char cs[2];} unis;
    union {unsigned int i;char ci[4];} unii;

    if(sizeof(unsigned int)!=4)
        PLERROR("Function write_compr_mode_and_size_ptr is not ready for 64 bit.");

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
        if(fast_exact_is_equal(val, 0.))
        {
            int n=0;
            while(l && fast_exact_is_equal(*data, 0.))
            { ++n; ++data; --l; }
            if(l && fast_exact_is_equal(*data, 1.))
            {
                write_compr_mode_and_size_ptr(comprbuf, 1, n);
                ++data; --l;
            }
            else
                write_compr_mode_and_size_ptr(comprbuf, 0, n);              
        }
        else if(fast_exact_is_equal(val, 1.))
        {
            write_compr_mode_and_size_ptr(comprbuf, 1, 0);
            ++data; --l;
        }
        else if( fast_exact_is_equal(double(char(val)), val) )
        {
            const double* start = data;
            int n=0;
            while(l && fast_exact_is_equal(double(char(val=*data)), val)
                    && !fast_exact_is_equal(val, 0)
                    && !fast_exact_is_equal(val, 1))
            { ++n; ++data; --l; }
            write_compr_mode_and_size_ptr(comprbuf, 2, n);
            while(n--)
                (*comprbuf++) = char(*start++);
        }
        else
        {
            const double* start = data;
            int n=0; 
            while(l && !fast_exact_is_equal((val=*data), 0)
                    && !fast_exact_is_equal(val, 1)
                    && !fast_exact_is_equal(double(char(val)), val))
            { ++n; ++data; --l; }
            write_compr_mode_and_size_ptr(comprbuf, 3, n);
            memcpy(comprbuf,start,n*sizeof(double));
            comprbuf += n*sizeof(double);
        }
    }
}


// ********************************************************
// ********************************************************
// *********       NEW COMPRESSION FORMAT        **********
// ********************************************************
// ********************************************************

/*

Format description:

A succession of [ mode-byte, optionally followed by specificaitons of length N, followed by data ]

The way N is encoded will be explained later.

The bits of the mode-byte are interpreted as follows:
* Most significant bit: 
0 : insert the following N values of type T
1 : insert N zeroes and then the following single value of type T

* Next 2 bits indicate the data type T
00 (=0): ones (that's just 1.0, no further data is given to provide the value) 
01 (=1): small 1 byte signed integers in the range [-127, +127], or missing values (indicated by -128: bit pattern 0x80)
10 (=2): 4 byte float  
11 (=3): 8 byte double

In all but the 00 case, 1 or N corresponding values of type T are 
to be read in the stream (after possibly reading N)

* Next 5 bits (values between 0 .. 31) indicate how to get the number N, 

1..29: N is that particular value (between 1 and 29)
0 : N is given in the following 1 byte unsigned char
30: N is given in the following 2 byte unsigned short
31: N is given in the following 4 byte unsigned int
 

*/


size_t new_read_compressed(FILE* in, real* vec, int l, bool swap_endians)
{
    size_t nbytes = 0; // number of bytes read
    unsigned char mode; // the mode byte
    unsigned int N = 0; // N (number of 0s or values to insert) 

    while(l)
    {
        if(l<0)
            PLERROR("Big problem in new_read_compressed: l=%d", l);
        int i = getc(in);
        if(i == EOF)
            PLERROR("Got EOF while expecting more data in the file!");
        mode = (unsigned char)(i); 
            
        ++nbytes;
        unsigned char N1 = (mode & 0x1F);
        switch(N1)
        {
        case 0:  // N is the 1 byte to follow
            N1 = (unsigned char)(getc(in));
            ++nbytes;
            N = N1;
            break;
        case 30: // N is the 2 bytes to follow
            unsigned short N2;
            fread(&N2,2,1,in);
            if(swap_endians)
                endianswap(&N2);
            nbytes += 2;
            N = N2;
            break;
        case 31: // N is the 4 bytes to follow
            fread(&N,4,1,in);
            if(swap_endians)
                endianswap(&N);
            nbytes += 4;
            break;
        default: // N is N1
            N = N1;
        }

        if(mode & (unsigned char)(0x80)) // most significant bit is on
        { // insert N zeros
            l -= N;
            while(N--)            
                *vec++ = 0;
            N = 1;
        }

        if(!l)  // vec ends with zeroes, so there's no extra single value to append. We're done!
            break;

        l -= N;
        mode = ((mode & ~0x80) >> 5); // get the 2 bits we're interested in
        switch(mode)
        {
        case 0: // type ones
        {
            while(N--)
                *vec++ = 1;
        }
        break;
        case 1: // type signed char (or missing value if -128)
        {
            signed char val;
            nbytes += N;
            while(N--)
            {
                val = (signed char)(getc(in));
                if(val==-128)
                    *vec++ = MISSING_VALUE;
                else
                    *vec++ = val;
            }
        }
        break;
        case 2: // type float
        {
            float val;
            nbytes += N<<2;
            while(N--)
            {
                fread(&val,sizeof(float),1,in);
                if(swap_endians)
                    endianswap(&val);
                *vec++ = val;
            }
        }
        break;
        case 3: // type double
        {
            nbytes += N<<3;
            fread(vec,sizeof(double),N,in);
            if(swap_endians)
                endianswap(vec,N);
            vec += N;
        }
        } 
    }
    return nbytes;
}

unsigned char new_get_compr_data_type(double x, double tolerance)
{
    if(is_missing(x))
        return 1;
    else if(fast_exact_is_equal(x, 1.))
        return 0;
    else if(fast_exact_is_equal(double(char(x)), x) &&
            !fast_exact_is_equal(x, -128)) // -128 codes for missing value
        return 1;
    else if(fabs(double(float(x))-x)<=tolerance)
        return 2;
    return 3;
}

unsigned char new_get_compr_data_type(float x)
{
    if(is_missing(x))
        return 1;
    else if(fast_exact_is_equal(x, 1.))
        return 0;
    else if(fast_exact_is_equal(float(char(x)), x) && 
            !fast_exact_is_equal(x, -128)) // -128 codes for missing value
        return 1;
    return 2;
}

//! returns number of bytes written
size_t new_write_mode_and_size(FILE* out, bool insert_zeroes, unsigned int N, unsigned char data_type)
{
    size_t nbytes = 0; // nbytes written
    unsigned char mode = data_type<<5;
    if(insert_zeroes)
        mode |= (unsigned char)0x80;
    if(N<30)
    {
        mode |= (unsigned char)N;
        putc(mode,out);
        nbytes = 1;
    }
    else if(N<=UCHAR_MAX)
    {
        putc(mode,out);
        putc((unsigned char)N,out);
        nbytes = 2;
    }
    else if(N<=USHRT_MAX)
    {
        mode |= (unsigned char)30;
        putc(mode,out);
        unsigned short N2 = (unsigned short)N;
        fwrite(&N2,sizeof(unsigned short),1,out);
        nbytes = 3;
    }
    else // (N<=UINT_MAX)
    {
        mode |= (unsigned char)31;
        putc(mode,out);
        unsigned int N4 = (unsigned int)N;
        fwrite(&N4,sizeof(unsigned int),1,out);
        nbytes = 5;
    }
    return nbytes;
}

size_t new_write_raw_data_as(FILE* out, real *vec, int l, unsigned char data_type)
{
    size_t nbytes = 0; // nbytes written
    switch(data_type)
    {
    case 1:
        nbytes = l;
        while(l--)
        {
            real val = *vec++;
            if(is_missing(val))
                putc(0x80,out);
            else
                putc((unsigned char)static_cast<signed char>(val),out);
        }
        break;
    case 2:
        nbytes = l*sizeof(float);
        while(l--)
        {
            float val = static_cast<float>(*vec++);
            fwrite(&val,sizeof(float),1,out);
        }
        break;      
    case 3:
        nbytes = l*sizeof(double);
        while(l--)
        {
            double val = static_cast<double>(*vec++);
            fwrite(&val,sizeof(double),1,out);
        }
        break;
    }
    return nbytes;
}

// Warning: this is low-level code written for efficiency
size_t new_write_compressed(FILE* out, real* vec, int l, double tolerance, bool swap_endians)
{
    if(swap_endians)
        PLERROR("swap_endians in new_write_compressed not yet supported (currently only supported by new_read_compresed");

    size_t nbytes = 0; // number of bytes written

    while(l)
    {
        int nzeroes = 0;
        while(l && fast_exact_is_equal(*vec, 0.))
        {
            ++nzeroes;
            ++vec;
            --l;
        }

        int nvals = 0;
        unsigned char data_type = 0;
        if(l)
        {
            real* ptr = vec;
            data_type = new_get_compr_data_type(*ptr, tolerance);
            ++nvals;
            ++ptr;
            --l;
            while(l && !fast_exact_is_equal(*ptr, 0.) && new_get_compr_data_type(*ptr, tolerance)==data_type)
            {
                ++nvals;
                ++ptr;
                --l;
            }
        }

        // Now we know nzeroes, nvals, and data_type
        // So let's encode it:

        if(nzeroes) // we have zeroes
        {
            // write the code for zeroes followed by a single value
            nbytes += new_write_mode_and_size(out, true, nzeroes, data_type);
            if(nvals) // write the following single value
            {
                nbytes += new_write_raw_data_as(out, vec, 1, data_type);
                ++vec;
                --nvals;
            }
        }

        if(nvals) // we have some remaining values
        {
            nbytes += new_write_mode_and_size(out, false, nvals, data_type);
            nbytes += new_write_raw_data_as(out, vec, nvals, data_type);
            vec += nvals;
        }

    } // end of for(;;)
    return nbytes;
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
