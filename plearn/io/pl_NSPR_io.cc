// -*- C++ -*-

// pl_NSPR_io.cc
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
 * AUTHORS: Pascal Vincent
 * This file is part of the PLearn library.
 ******************************************************* */

/*! \file PLearn/plearn/io/pl_NSPR_io.cc */

#include "pl_NSPR_io.h"
#include <plearn/base/byte_order.h>   //!< For endianswap.

namespace PLearn {
using namespace std;

// Functions to read from a file written in any representation

void PR_Read_int(PRFileDesc *f, int* ptr, int n, bool is_file_bigendian)
{
    PR_Read(f,ptr,sizeof(int)*n);
#ifdef LITTLEENDIAN
    if(is_file_bigendian)
        endianswap(ptr,n);
#endif
#ifdef BIGENDIAN
    if(!is_file_bigendian)
        endianswap(ptr,n);
#endif
}

void PR_Read_float(PRFileDesc *f, float* ptr, int n, bool is_file_bigendian)
{
    PR_Read(f,ptr,sizeof(float)*n);
#ifdef LITTLEENDIAN
    if(is_file_bigendian)
        endianswap(ptr,n);
#endif
#ifdef BIGENDIAN
    if(!is_file_bigendian)
        endianswap(ptr,n);
#endif
}

void PR_Read_float(PRFileDesc *f, double* ptr, int n, bool is_file_bigendian)
{
    float* fptr = new float[n];
    PR_Read_float(f,fptr,n,is_file_bigendian);
    for(int i=0; i<n; i++)
        ptr[i] = double(fptr[i]);
    delete[] fptr;
}

void PR_Read_double(PRFileDesc *f, double* ptr, int n, bool is_file_bigendian)
{
    PR_Read(f,ptr,sizeof(double)*n);
#ifdef LITTLEENDIAN
    if(is_file_bigendian)
        endianswap(ptr,n);
#endif
#ifdef BIGENDIAN
    if(!is_file_bigendian)
        endianswap(ptr,n);
#endif
}

void PR_Read_double(PRFileDesc *f, float* ptr, int n, bool is_file_bigendian)
{
    double* dptr = new double[n];
    PR_Read_double(f,dptr,n,is_file_bigendian);
    for(int i=0; i<n; i++)
        ptr[i] = float(dptr[i]);
    delete[] dptr;
}

void PR_Read_short(PRFileDesc *f, unsigned short* ptr, int n, bool is_file_bigendian)
{
    PR_Read(f,ptr,sizeof(unsigned short)*n);
#ifdef LITTLEENDIAN
    if(is_file_bigendian)
        endianswap(ptr,n);
#endif
#ifdef BIGENDIAN
    if(!is_file_bigendian)
        endianswap(ptr,n);
#endif
}


// Functions to write a file in any representation

void PR_Write_int(PRFileDesc *f, const int* ptr, int n, bool is_file_bigendian)
{
#ifdef LITTLEENDIAN
    if(is_file_bigendian)
    {
        endianswap(const_cast<int*>(ptr),n);
        PR_Write(f,ptr,sizeof(int)*n);
        endianswap(const_cast<int*>(ptr),n);
    }
    else
        PR_Write(f,ptr,sizeof(int)*n);
#endif
#ifdef BIGENDIAN
    if(is_file_bigendian)
        PR_Write(f,ptr,sizeof(int)*n);
    else
    {
        endianswap(const_cast<int*>(ptr),n);
        PR_Write(f,ptr,sizeof(int)*n);
        endianswap(const_cast<int*>(ptr),n);
    }
#endif
}

void PR_Write_float(PRFileDesc *f, const float* ptr, int n, bool is_file_bigendian)
{
#ifdef LITTLEENDIAN
    if(is_file_bigendian)
    {
        endianswap(const_cast<float*>(ptr),n);
        PR_Write(f,ptr,sizeof(float)*n);
        endianswap(const_cast<float*>(ptr),n);
    }
    else
        PR_Write(f,ptr,sizeof(float)*n);
#endif
#ifdef BIGENDIAN
    if(is_file_bigendian)
        PR_Write(f,ptr,sizeof(float)*n);
    else
    {
        endianswap(const_cast<float*>(ptr),n);
        PR_Write(f,ptr,sizeof(float)*n);
        endianswap(const_cast<float*>(ptr),n);
    }
#endif
}

void PR_Write_float(PRFileDesc *f, const double* ptr, int n, bool is_file_bigendian)
{
    float* fptr = new float[n];
    for(int i=0; i<n; i++)
        fptr[i] = float(ptr[i]);
    PR_Write_float(f,fptr,n,is_file_bigendian);
    delete[] fptr;
}

void PR_Write_double(PRFileDesc *f, const double* ptr, int n, bool is_file_bigendian)
{
#ifdef LITTLEENDIAN
    if(is_file_bigendian)
    {
        endianswap(const_cast<double*>(ptr),n);
        PR_Write(f,ptr,sizeof(double)*n);
        endianswap(const_cast<double*>(ptr),n);
    }
    else
        PR_Write(f,ptr,sizeof(double)*n);
#endif
#ifdef BIGENDIAN
    if(is_file_bigendian)
        PR_Write(f,ptr,sizeof(double)*n);
    else
    {
        endianswap(const_cast<double*>(ptr),n);
        PR_Write(f,ptr,sizeof(double)*n);
        endianswap(const_cast<double*>(ptr),n);
    }
#endif
}

void PR_Write_double(PRFileDesc *f, const float* ptr, int n, bool is_file_bigendian)
{
    double* dptr = new double[n];
    for(int i=0; i<n; i++)
        dptr[i] = double(ptr[i]);
    PR_Write_double(f,dptr,n,is_file_bigendian);
    delete[] dptr;
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
