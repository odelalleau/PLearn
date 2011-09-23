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
 * This file is part of the PLearn library.
 ******************************************************* */


/*! \file PLearn/plearn/io/pl_io.h */

#ifndef pl_io_INC
#define pl_io_INC

#include <cstdio> // needed by g++ 4.5.1
#include <iostream>
#include <map>
#include <plearn/math/pl_math.h>        //!< For 'real'.

namespace PLearn {
using std::map;
using std::pair;
using std::istream;
using std::ostream;

// Support for outputing std::map on an ostream.
template <class Key, class Value>
ostream& operator<<(ostream& out, const map<Key, Value>& m)
{
    out << "{" << flush;
    typename map<Key,Value>::const_iterator it  = m.begin();

    if ( m.size() > 0 )
    {
        for ( unsigned int elem = 0; elem < m.size()-1; elem++, it++ ) 
            out << it->first << " : " << it->second << ", " << flush;

        PLASSERT( it != m.end() );
        out << it->first << " : " << it->second << flush;
    }
  
    out << "}" << flush;
    return out;
}


//*** Binary read and write to/from std::stream ***

//!  general purpose (but less efficient)
//!  version for pointers to things that have a binwrite/binread function
template<class T>
inline void binwrite(ostream& out, const T* x, int n)
{ for (int i=0;i<n;i++) binwrite(out,x[i]); }

template<class T>
inline void binread(istream& in, T* x, int n)
{ for (int i=0;i<n;i++) binread(in,x[i]); }

template<class A,class B>
inline void binwrite(ostream& out, const pair<A,B> x)
{ binwrite(out,x.first); binwrite(out,x.second); }

template<class A,class B>
inline void binread(istream& in, pair<A,B>& x)
{ binread(in,x.first); binread(in,x.second); }


//!  binwrite and binread for a few basic types
inline void binwrite(ostream& out, char x) { out.put(x); }
inline void binread(istream& in, char& x) { in.get(x); }
inline void binwrite(ostream& out, unsigned char x) { out.put(x); }
inline void binread(istream& in, unsigned char& x) { in.get((char&)x); }
inline void binwrite(ostream& out, int x) { out.write((char*)&x,sizeof(int)); }
inline void binread(istream& in, int& x) { in.read((char*)&x,sizeof(int));  }
inline void binwrite(ostream& out, unsigned int x) { out.write((char*)&x,sizeof(unsigned int)); }
inline void binread(istream& in, unsigned int& x) { in.read((char*)&x,sizeof(unsigned int));  }
inline void binwrite(ostream& out, short x) { out.write((char*)&x,sizeof(short)); }
inline void binread(istream& in, short& x) { in.read((char*)&x,sizeof(short));  }
inline void binwrite(ostream& out, unsigned short x) { out.write((char*)&x,sizeof(unsigned short)); }
inline void binread(istream& in, unsigned short& x) { in.read((char*)&x,sizeof(unsigned short));  }
//!  note that bool are saved as unsigned short
inline void binwrite(ostream& out, bool x) { binwrite(out,(unsigned short)x); }

// norman: usigned short to boolean?? Performance hit!!
//inline void binread(istream& in, bool& x) { unsigned short u; binread(in,u); x=u; }
inline void binread(istream& in, bool& x) { 
    unsigned short u; binread(in,u); 
    u == 0 ? x = false : x = true;
}

// The following read/write floats (4 bytes) on disk, regardless of whether the memory-elements are float or double
inline void binwrite(ostream& out, float x) { out.write((char*)&x,sizeof(float)); }
inline void binread(istream& in, float& x) { in.read((char*)&x,sizeof(float));  }
inline void binwrite(ostream& out, double x) { binwrite(out, float(x)); }
inline void binread(istream& in, double& x) { float f; binread(in,f); x = double(f); }

// The following read/write doubles (8 bytes) on disk, regardless of whether the memory-elements are float or double
inline void binwrite_double(ostream& out, double x) { out.write((char*)&x,sizeof(double)); }
inline void binread_double(istream& in, double& x) { in.read((char*)&x,sizeof(double));  }
inline void binwrite_double(ostream& out, float x) { binwrite_double(out, double(x)); }
inline void binread_double(istream& in, float& x) { double d; binread_double(in,d); x = float(d); }


//!  multi-element versions, giving address and number of elements
inline void binwrite(ostream& out, const int* x, int n)
    { out.write((char*)x, int(n*sizeof(int))); }
inline void binread(istream& in, int* x, int n)
    { in.read((char*)x, int(n*sizeof(int))); }
inline void binwrite(ostream& out, const unsigned int* x, int n)
    { out.write((char*)x, int(n*sizeof(unsigned int))); }
inline void binread(istream& in, unsigned int* x, int n)
    { in.read((char*)x, int(n*sizeof(unsigned int))); }
inline void binwrite(ostream& out, const short* x, int n)
    { out.write((char*)x, int(n*sizeof(short))); }
inline void binread(istream& in, short* x, int n)
    { in.read((char*)x, int(n*sizeof(short))); }
inline void binwrite(ostream& out, const unsigned short* x, int n)
    { out.write((char*)x, int(n*sizeof(unsigned short))); }
inline void binread(istream& in, unsigned short* x, int n)
    { in.read((char*)x, int(n*sizeof(unsigned short))); }

// The followind read/write 4-byte-floats on disk (whether we pass them a float* or a double*)
inline void binwrite(ostream& out, const float *x, int n)
    { out.write((char*)x, int(n*sizeof(float))); }
inline void binread(istream& in, float* x, int n)
    { in.read((char*)x, int(n*sizeof(float))); }
inline void binwrite(ostream& out, const double* x, int n) { for(int i=0; i<n; i++) binwrite(out,x[i]); }
inline void binread(istream& in, double* x, int n) { for(int i=0; i<n; i++) binread(in,x[i]); }

// The followind read/write 8-byte-doubles on disk (whether we pass them a float* or a double*)
inline void binwrite_double(ostream& out, const double *x, int n)
    { out.write((char*)x, int(n*sizeof(double))); }
inline void binread_double(istream& in, double* x, int n)
    { in.read((char*)x, int(n*sizeof(double))); }
inline void binwrite_double(ostream& out, const float* x, int n) { for(int i=0; i<n; i++) binwrite(out,x[i]); }
inline void binread_double(istream& in, float* x, int n) { for(int i=0; i<n; i++) binread(in,x[i]); }

//! version for compressed array (efficient for sparse data, and small integer values)
//! (format is detailed in .cc, see write_compr_mode_and_size function in general.cc)
void binwrite_compressed(ostream& out, const double* data, int l);
void binread_compressed(istream& in, double* data, int l);
void binwrite_compressed(ostream& out, const float* data, int l);
void binread_compressed(istream& in, float* data, int l);


//*** Binary read and write to/from FILE* ***

//!  general purpose (but less efficient)
//!  version for pointers to things that have a binwrite/binread function
template<class T>
inline void binwrite(FILE* out, const T* x, int n)
{ for (int i=0;i<n;i++) binwrite(out,x[i]); }

template<class T>
inline void binread(FILE* in, T* x, int n)
{ for (int i=0;i<n;i++) binread(in,x[i]); }

template<class A,class B>
inline void binwrite(FILE* out, const pair<A,B> x)
{ binwrite(out,x.first); binwrite(out,x.second); }

template<class A,class B>
inline void binread(FILE* in, pair<A,B>& x)
{ binread(in,x.first); binread(in,x.second); }


//!  binwrite and binread for a few basic types
inline void binwrite(FILE* out, char x) { putc((unsigned char)x, out); }
inline void binread(FILE* in, char& x) { x = (char)getc(in); }
inline void binwrite(FILE* out, unsigned char x) { putc(x,out); }
inline void binread(FILE* in, unsigned char& x) { x = (unsigned char)getc(in); }
inline void binwrite(FILE* out, int x) { fwrite(&x,sizeof(int),1,out); }
inline void binread(FILE* in, int& x) { fread(&x,sizeof(int),1,in);  }
inline void binwrite(FILE* out, unsigned int x) { fwrite(&x,sizeof(unsigned int),1,out); }
inline void binread(FILE* in, unsigned int& x) { fread(&x,sizeof(unsigned int),1,in);  }
inline void binwrite(FILE* out, short x) { fwrite(&x,sizeof(short),1,out); }
inline void binread(FILE* in, short& x) { fread(&x,sizeof(short),1,in);  }
inline void binwrite(FILE* out, unsigned short x) { fwrite(&x,sizeof(unsigned short),1,out); }
inline void binread(FILE* in, unsigned short& x) { fread(&x,sizeof(unsigned short),1,in);  }
//!  note that bool are saved as unsigned short
inline void binwrite(FILE* out, bool x) { binwrite(out,(unsigned short)x); }
// norman: usigned short to boolean?? At least use a cast
//inline void binread(FILE* in, bool& x) { unsigned short u; binread(in,u); x=u; }
inline void binread(FILE* in, bool& x) { 
    unsigned short u; binread(in,u); 
    u == 0 ? x = false : x = true;
}

// The following read/write floats (4 bytes) on disk, regardless of whether the memory-elements are float or double
inline void binwrite(FILE* out, float x) { fwrite(&x,sizeof(float),1,out); }
inline void binread(FILE* in, float& x) { fread(&x,sizeof(float),1,in);  }
inline void binwrite(FILE* out, double x) { binwrite(out, float(x)); }
inline void binread(FILE* in, double& x) { float f; binread(in,f); x = double(f); }

// The following read/write doubles (8 bytes) on disk, regardless of whether the memory-elements are float or double
inline void binwrite_double(FILE* out, double x) { fwrite(&x,sizeof(double),1,out); }
inline void binread_double(FILE* in, double& x) { fread(&x,sizeof(double),1,in);  }
inline void binwrite_double(FILE* out, float x) { binwrite_double(out, double(x)); }
inline void binread_double(FILE* in, float& x) { double d; binread_double(in,d); x = float(d); }


//!  multi-element versions, giving address and number of elements
inline void binwrite(FILE* out, const int* x, int n) { fwrite(x, sizeof(int),n,out); }
inline void binread(FILE* in, int* x, int n) { fread(x, sizeof(int),n,in);  }
inline void binwrite(FILE* out, const unsigned int* x, int n) { fwrite(x, sizeof(unsigned int),n,out); }
inline void binread(FILE* in, unsigned int* x, int n)  { fread(x, sizeof(unsigned int),n,in);  }
inline void binwrite(FILE* out, const short* x, int n) { fwrite(x, sizeof(short),n,out); }
inline void binread(FILE* in, short* x, int n) { fread(x, sizeof(short),n,in);  }
inline void binwrite(FILE* out, const unsigned short* x, int n) { fwrite(x, sizeof(unsigned short),n,out); }
inline void binread(FILE* in, unsigned short* x, int n) { fread(x, sizeof(unsigned short),n,in);  }

// The followind read/write 4-byte-floats on disk (whether we pass them a float* or a double*)
inline void binwrite(FILE* out, const float *x, int n) { fwrite(x, sizeof(float),n,out); }
inline void binread(FILE* in, float* x, int n) { fread(x, sizeof(float),n,in);  }
inline void binwrite(FILE* out, const double* x, int n) { for(int i=0; i<n; i++) binwrite(out,x[i]); }
inline void binread(FILE* in, double* x, int n) { for(int i=0; i<n; i++) binread(in,x[i]); }

// The followind read/write 8-byte-doubles on disk (whether we pass them a float* or a double*)
inline void binwrite_double(FILE* out, const double *x, int n) { fwrite(x, sizeof(double),n,out); }
inline void binread_double(FILE* in, double* x, int n) { fread(x, sizeof(double),n,in);  }
inline void binwrite_double(FILE* out, const float* x, int n) { for(int i=0; i<n; i++) binwrite(out,x[i]); }
inline void binread_double(FILE* in, float* x, int n) { for(int i=0; i<n; i++) binread(in,x[i]); }

void binwrite_compressed(FILE* out, const double* data, int l);
void binread_compressed(FILE* in, double* data, int l);
void binwrite_compressed(FILE* out, const float* data, int l);
void binread_compressed(FILE* in, float* data, int l);

//! DEPRECATED DO NOT USE! compressed vec to and from memory
inline void read_compr_mode_and_size_ptr(char*& in, unsigned char& mode, int& size);
void compress_vec(char* comprbuf, const double* data, int l, bool double_stored_as_float=false);
void uncompress_vec(char* comprbuf, double* data, int l, bool double_stored_as_float=false);


// *********       NEW COMPRESSION FORMAT        **********
// (see its description in .cc)

//! Writes the l doubles in new compressed format to out.
//! Returns the number of bytes written
//! tolerance is the maximum allowed error tolerance to store doubles as floats.
//! Set swap_endians to true if the data is to be written in the different byte-order from this machine's 
size_t new_write_compressed(FILE* out, real* vec, int l, double tolerance=1e-6, bool swap_endians=false);

//! Reads the l doubles in the new compressed formtat from in
//! Returns the number of bytes read.
//! Set swap_endians to true if the data was written on a machine with a different 
//! endianness than the current one, so that the endians get swapped.
size_t new_read_compressed(FILE* in, real* vec, int l, bool swap_endians=false);



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
