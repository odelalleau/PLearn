// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999,2000 Pascal Vincent, Yoshua Bengio and University of Montreal
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


/*! \file PLearn/plearn/io/pl_io_deprecated.h */

#ifndef pl_io_deprecated_INC
#define pl_io_deprecated_INC

#include <iostream>                   //!< For stream stuff.
//#include "pl_io.h"
#include <plearn/base/plerror.h>
//#include <cstdio>
#include <plearn/base/byte_order.h>   //!< For endianswap.
#include <plearn/io/PStream.h>

namespace PLearn {
using namespace std;


// *************************************************************************
// *************************************************************************
// *********** ALL THAT FOLLOWS IS DEPRECATED: DO NOT USE!!!! **************
// *************************************************************************
// *************************************************************************

//!  Swap bytes between Big-Endian and Little-Endian representation in memory
//! NOTE: these calls are deprecated, use directly endianswap from base/byte_order.h
inline void reverse_uint(const unsigned int* ptr, int n) { endianswap((unsigned int*) ptr,n); }
inline void reverse_int(const int* ptr, int n) { endianswap((int*)ptr,n); }
inline void reverse_float(const float* ptr, int n) { endianswap((float*)ptr,n); }
inline void reverse_double(const double* ptr, int n) { endianswap((double*)ptr,n); }
inline void reverse_ushort(const unsigned short* ptr, int n) { endianswap((unsigned short*)ptr,n); }
inline void reverse_short(const short* ptr, int n) { endianswap((short*)ptr,n); }

//!  Writes binary data to the file in the specified representation (little or big endian)
//!  regardeless of the endianness used on the current architecture.
void  fwrite_int(FILE *f, const int* ptr, int n, bool is_file_bigendian=true);
void  fwrite_float(FILE *f, const float* ptr, int n, bool is_file_bigendian=true);
void  fwrite_float(FILE *f, const double* ptr, int n, bool is_file_bigendian=true); //!<  writes double array to float file
void  fwrite_double(FILE *f, const double* ptr, int n, bool is_file_bigendian=true);
void  fwrite_double(FILE *f, const float* ptr, int n, bool is_file_bigendian=true); //!<  writes float array to double file

//!  Reads binary data from a file assuming it is in the specified representation (either little or big endian)
//!  If necessary the representation is translated to the endianness on the current architecture.
void  fread_int(FILE *f, int* ptr, int n, bool is_file_bigendian=true);
void  fread_float(FILE *f, float* ptr, int n, bool is_file_bigendian=true);
void  fread_float(FILE *f, double* ptr, int n, bool is_file_bigendian=true); //!<  reads disk floats into double array
void  fread_double(FILE *f, double* ptr, int n, bool is_file_bigendian=true);
void  fread_double(FILE *f, float* ptr, int n, bool is_file_bigendian=true); //!<  reads disk doubles into float array
void  fread_short(FILE *f, unsigned short* ptr, int n, bool is_file_bigendian=true);

/*!   The following calls read a single value from the file, assuming it is in the specified representation
  (either little or big endian) If necessary the representation is translated to the endianness used on
  the current architecture.
*/
inline int fread_int(FILE *f, bool is_file_bigendian=true) 
{ int res; fread_int(f,&res,1,is_file_bigendian); return res; }
inline float fread_float(FILE *f, bool is_file_bigendian=true)
{ float res; fread_float(f,&res,1,is_file_bigendian); return res; }
inline double fread_double(FILE *f, bool is_file_bigendian=true)
{ double res; fread_double(f,&res,1,is_file_bigendian); return res; }

//!  The following calls write a single value to the file in the specified representation,
//!  regardeless of the endianness on the current architecture
inline void fwrite_int(FILE *f, int value, bool is_file_bigendian=true)
{ fwrite_int(f, &value, 1, is_file_bigendian); }
inline void fwrite_float(FILE *f, float value, bool is_file_bigendian=true)
{ fwrite_float(f, &value, 1, is_file_bigendian); }
inline void fwrite_double(FILE *f, double value, bool is_file_bigendian=true)
{ fwrite_double(f, &value, 1, is_file_bigendian); }

//!  Writes binary data to the file in the specified representation (little or big endian)
//!  regardeless of the endianness used on the current architecture.
void write_int(ostream& out, const int* ptr, int n, bool is_file_bigendian);
void write_short(ostream& out, const short* ptr, int n, bool is_file_bigendian);
void write_double(ostream& out, const double* ptr, int n, bool is_file_bigendian);
void write_float(ostream& out, const float* ptr, int n, bool is_file_bigendian);
inline void write_uint(ostream& out, const unsigned int* ptr, int n, bool is_file_bigendian)
{ write_int(out,(int*)ptr,n,is_file_bigendian); }
inline void write_ushort(ostream& out, const unsigned short* ptr, int n, bool is_file_bigendian)
{ write_short(out,(short*)ptr,n,is_file_bigendian); }
void write_bool(ostream& out, const bool* ptr, int n, bool is_file_bigendian);

//!  The following calls write a single value to the file in the specified representation,
//!  regardeless of the endianness on the current architecture
inline void write_int(ostream& out, int value, bool is_file_bigendian=true)
{ write_int(out, &value, 1, is_file_bigendian); }
inline void write_short(ostream& out, short value, bool is_file_bigendian=true)
{ write_short(out, &value, 1, is_file_bigendian); }
inline void write_float(ostream& out, float value, bool is_file_bigendian=true)
{ write_float(out, &value, 1, is_file_bigendian); }
inline void write_double(ostream& out, double value, bool is_file_bigendian=true)
{ write_double(out, &value, 1, is_file_bigendian); }
inline void write_uint(ostream& out, unsigned int value, bool is_file_bigendian=true)
{ write_uint(out, &value, 1, is_file_bigendian); }
inline void write_ushort(ostream& out, unsigned short value, bool is_file_bigendian=true)
{ write_ushort(out, &value, 1, is_file_bigendian); }
inline void write_sbyte(ostream& out, signed char x) { out.put(x); }
inline void write_ubyte(ostream& out, unsigned char x) { out.put(x); }

//!  Reads binary data from a file assuming it is in the specified representation (either little or big endian)
//!  If necessary the representation is translated to the endianness on the current architecture.
void read_int(istream& in, int* ptr, int n, bool is_file_bigendian);
void read_short(istream& in, short* ptr, int n, bool is_file_bigendian);
void read_float(istream& in, float* ptr, int n, bool is_file_bigendian);
void read_double(istream& in, double* ptr, int n, bool is_file_bigendian);
inline void read_uint(istream& in, unsigned int* ptr, int n, bool is_file_bigendian)
{ read_int(in,(int*)ptr,n,is_file_bigendian); }
inline void read_ushort(istream& in, unsigned short* ptr, int n, bool is_file_bigendian)
{ read_short(in,(short*)ptr,n,is_file_bigendian); }
void read_bool(istream& in, bool* ptr, int n, bool is_file_bigendian);

/*!   The following calls read a single value from the file, assuming it is in the specified representation
  (either little or big endian) If necessary the representation is translated to the endianness used on
  the current architecture.
*/
inline int read_int(istream& in, bool is_file_bigendian=true) 
{ int res; read_int(in,&res,1,is_file_bigendian); return res; }
inline short read_short(istream& in, bool is_file_bigendian=true) 
{ short res; read_short(in,&res,1,is_file_bigendian); return res; }
inline float read_float(istream& in, bool is_file_bigendian=true)
{ float res; read_float(in,&res,1,is_file_bigendian); return res; }
inline double read_double(istream& in, bool is_file_bigendian=true)
{ double res; read_double(in,&res,1,is_file_bigendian); return res; }
inline unsigned int read_uint(istream& in, bool is_file_bigendian=true) 
{ unsigned int res; read_uint(in,&res,1,is_file_bigendian); return res; }
inline unsigned short read_ushort(istream& in, bool is_file_bigendian=true) 
{ unsigned short res; read_ushort(in,&res,1,is_file_bigendian); return res; }
inline signed char read_sbyte(istream& in)
{ 
    char res;
    in.read(&res,1);
    return (signed char) res;
}
inline unsigned char read_ubyte(istream& in)
{ 
    char res;
    in.read(&res,1);
    return (unsigned char) res;
}

// ***************************************
// *** DEPRECATED SERIALIZATION SYSTEM ***
// ***************************************

//!  These functions are there to help you write and read object headers and
//!  footers for the persistance mechanism.
void writeHeader(ostream& out, const string& classname, int version=0); //!<  writes "<ClassName:version>\n"
void writeFooter(ostream& out, const string& classname); //!<  writes "</ClassName>\n"
int readHeader(PStream& in, const string& classname);   //!<  consumes "<ClassName:version>\n and returns version"
void readFooter(PStream& in, const string& classname);   //!<  consumes "</ClassName>\n"


//! Writes a single newline character
inline void writeNewline(ostream& out)
{ out << '\n'; }

//! Reads next character and issues an error if it's not a newline
inline void readNewline(istream& in) 
{ if(in.get()!='\n') PLERROR("In readNewline: character read is not a newline char"); }


/*!     writes and reads the given fieldname (should be followed by
  wrtiting or reading of the field's value. The readFieldName method
  checks the read fieldname against the one passed as argument and
  issues an error if they do not match
*/
void writeFieldName(ostream& out, const string& fieldname); //!<  writes "fieldname: "
//!  consumes "fieldname: " if possible, and return true if it does
//!  however if force=true and fieldname is not found then call error.
bool readFieldName(istream& in, const string& fieldname, bool force=false);   

//!  generic field writing and reading
template<class T>
void writeField(ostream& out, const string& fieldname, const T& x)
{ 
// Norman: This gives problems on VSNet when T is a Array<VMFieldStat> or VMFieldStat.
//         Because it is deprecated, well, I have decided to wipe it out! :)
#ifndef WIN32
    writeFieldName(out,fieldname); write(out,x); out << '\n'; 
#endif
}

template<class T>
void readField(istream& in, const string& fieldname, T& x)
{ 
// Norman: This gives problems on VSNet when T is a Array<VMFieldStat> or VMFieldStat.
//         Because it is deprecated, well, I have decided to wipe it out! :)
#ifndef WIN32
    readFieldName(in,fieldname,true); 
    read(in,x); 
    if(!isspace(in.get())) 
        in.unget(); 
#endif
}

//!  generic field BINARY writing and reading
template<class T>
void binwriteField(ostream& out, const string& fieldname, const T& x)
{ writeFieldName(out,fieldname); binwrite(out,x); out << '\n'; }

template<class T>
void binreadField(istream& in, const string& fieldname, T& x)
{ readFieldName(in,fieldname,true); binread(in,x); in.get(); }

template<class T>
void binwriteField_double(ostream& out, const string& fieldname, const T& x)
{ writeFieldName(out,fieldname); binwrite_double(out,x); out << '\n'; }

template<class T>
void binreadField_double(istream& in, const string& fieldname, T& x)
{ readFieldName(in,fieldname,true); binread_double(in,x); in.get(); }

//!  readField with a default value when the field is not found
template<class T>
void readField(istream& in, const string& fieldname, T& x, T default_value)
{ 
    if (readFieldName(in,fieldname)) 
    { 
        read(in,x); 
        if(!isspace(in.get())) 
            in.unget(); 
    } 
    else x=default_value; 
}





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
