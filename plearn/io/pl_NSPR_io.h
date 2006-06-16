// -*- C++ -*-

// pl_NSPR_io.h
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


/*! \file PLearnLibrary/PLearnCore/pl_NSPR_io.h */

#ifndef pl_NSPR_io_INC
#define pl_NSPR_io_INC

#include <firefox/nspr/prio.h>

namespace PLearn {
using namespace std;

// Functions to read from a file written in any representation

void PR_Read_int(PRFileDesc *f, int* ptr, int n, bool is_file_bigendian);
void PR_Read_float(PRFileDesc *f, float* ptr, int n, bool is_file_bigendian);
void PR_Read_float(PRFileDesc *f, double* ptr, int n, bool is_file_bigendian);
void PR_Read_double(PRFileDesc *f, double* ptr, int n, bool is_file_bigendian);
void PR_Read_double(PRFileDesc *f, float* ptr, int n, bool is_file_bigendian);
void PR_Read_short(PRFileDesc *f, unsigned short* ptr, int n, bool is_file_bigendian);


// Functions to write to a file in any representation

void PR_Write_int(PRFileDesc *f, const int* ptr, int n, bool is_file_bigendian);
void PR_Write_float(PRFileDesc *f, const float* ptr, int n, bool is_file_bigendian);
void PR_Write_float(PRFileDesc *f, const double* ptr, int n, bool is_file_bigendian);
void PR_Write_double(PRFileDesc *f, const double* ptr, int n, bool is_file_bigendian);
void PR_Write_double(PRFileDesc *f, const float* ptr, int n, bool is_file_bigendian);


/*!   The following calls read a single value from the file, assuming it is in the specified representation
  (either little or big endian) If necessary the representation is translated to the endianness used on
  the current architecture.
*/
inline int PR_Read_int(PRFileDesc *f, bool is_file_bigendian=true) 
{ int res; PR_Read_int(f,&res,1,is_file_bigendian); return res; }
inline float PR_Read_float(PRFileDesc *f, bool is_file_bigendian=true)
{ float res; PR_Read_float(f,&res,1,is_file_bigendian); return res; }
inline double PR_Read_double(PRFileDesc *f, bool is_file_bigendian=true)
{ double res; PR_Read_double(f,&res,1,is_file_bigendian); return res; }

//!  The following calls write a single value to the file in the specified representation,
//!  regardeless of the endianness on the current architecture
inline void PR_Write_int(PRFileDesc *f, int value, bool is_file_bigendian=true)
{ PR_Write_int(f, &value, 1, is_file_bigendian); }
inline void PR_Write_float(PRFileDesc *f, float value, bool is_file_bigendian=true)
{ PR_Write_float(f, &value, 1, is_file_bigendian); }
inline void PR_Write_double(PRFileDesc *f, double value, bool is_file_bigendian=true)
{ PR_Write_double(f, &value, 1, is_file_bigendian); }


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
