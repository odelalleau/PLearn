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
   * $Id: pl_io.h,v 1.1 2002/09/26 05:06:53 plearner Exp $
   * This file is part of the PLearn library.
   ******************************************************* */


/*! \file PLearnLibrary/PLearnCore/pl_io.h */

#ifndef pl_io_INC
#define pl_io_INC

#include <cctype>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>

#include "PStream.h"

namespace PLearn <%
using namespace std;








/*
template<typename T>
inline void write(ostream& out, const T& x) { PStream myout(&out); myout << x; }
template<typename T>
inline void read(istream& in, T& x) { PStream myin(&in); myin >> x; }
*/







/*
//!  *** LITTLE-ENDIAN / BIG-ENDIAN HELL... ***

  // This is initially set to false.  This flag is useful for reading data
  // that has been written on an architecture with a different endianness
  // than the current architecture. If set to true, then the binread
  // operations will call endianswap after reading their data.  The
  // principle is that binwrite always writes the data as it has it in
  // memory (i.e. with the endianness of the architecture that writes it).
  // save/load will typically store/use endianness information in the file
  // header (typically 'ABCD' for big endian 'DCBA' for little endian), so
  // that load can possibly set swap_endians_on_read before calling read
  extern bool swap_endians_on_read; 

  //! swaps endians for n 2-byte elements (such as short)
  void endianswap2(void* ptr, int n);
  //! swaps endians for n 4-byte elements (such as int or float)
  void endianswap4(void* ptr, int n);
  //! swaps endians for n 8-byte elements (such as double)
  void endianswap8(void* ptr, int n);

  // Versions for short, int, float and double
  inline void endianswap(short* ptr, int n=1) { endianswap2(ptr,n); }
  inline void endianswap(unsigned short* ptr, int n=1) { endianswap2(ptr,n); }
  inline void endianswap(int* ptr, int n=1) { endianswap4(ptr,n); }
  inline void endianswap(unsigned int* ptr, int n=1) { endianswap4(ptr,n); }
  inline void endianswap(float* ptr, int n=1) { endianswap4(ptr,n); }
  inline void endianswap(double* ptr, int n=1) { endianswap8(ptr,n); }
*/  
/*! ******************************************************
    **   SUPPORT FOR GENERIC OPERATIONS ON DATA TYPES   **
    **     (serialization, deep copying, printing)      **
    ******************************************************
*/

/*!     1) Support for generic printing
    Printing is done by overriding operator<<(ostream& out, const YourType&)
    For deep class hierarchies, operator<< should be defined only for the toplevel class
    and call the class's virtual print() method, that each subclass can overload as appropriate.
*/

/*!     2) Support for generic serialization... (read, write, save, load)
    Serialization is achieved by writing functions write and read for your types
    For deep class hierarchies, save and load functions should be defined only for the toplevel class
    and call the class's virtual write and read, that each subclass can overload as appropriate 
    (see the Object hierarchy for an example).
*/
/*
  //!  write and read for a few basic types

  inline void skipBlanks(istream& in) 
    { 
      while(isspace(in.get()));
      in.unget(); 
    }
*/
/*
  inline void write(ostream& out, char x) { out << x << ' '; }
  inline void read(istream& in, char& x) { in >> x; if(!isspace(in.get())) in.unget(); }
  inline void write(ostream& out, int x) { out << x << ' '; }
  inline void read(istream& in, int& x) { in >> x; if(!isspace(in.get())) in.unget(); }
  inline void write(ostream& out, unsigned int x) { out << x << ' '; }
  inline void read(istream& in, unsigned int& x) { in >> x; if(!isspace(in.get())) in.unget(); }
  inline void write(ostream& out, long int x) { out << x << ' '; }
  inline void read(istream& in, long int& x) { in >> x; if(!isspace(in.get())) in.unget(); }
  inline void write(ostream& out, unsigned long x) { out << x << ' '; }
  inline void read(istream& in, unsigned long& x) { in >> x; if(!isspace(in.get())) in.unget(); }
  inline void write(ostream& out, short int x) { out << x << ' '; }
  inline void read(istream& in, short int& x) { in >> x; if(!isspace(in.get())) in.unget(); }
  inline void write(ostream& out, unsigned short x) { out << x << ' '; }
  inline void read(istream& in, unsigned short& x) { in >> x; if(!isspace(in.get())) in.unget(); }

//! writes a boolean as 0 (false) or 1 (true)
  inline void write(ostream& out, bool x) { out << (x?'1':'0') << ' '; }
//! reads a boolean; it understands 0,1 true,false t,f
void read(istream& in, bool& x);

// these also correctly handle the case of "missing value" reals (for which we use nan)
  void write(ostream& out, float x);
  void read(istream& in, float& x);
  void write(ostream& out, double x);
  void read(istream& in, double& x);

  template<class A,class B>
  inline void write(ostream& out, pair<A,B> x) { write(out,x.first); write(out,x.second); }
  template<class A,class B>
  inline void read(istream& in, pair<A,B>& x) { read(in,x.first); read(in,x.second); }
  void write(ostream& out, const char* x);
  void read(istream& in, char*& x);

//! List of characters considered to mark a separation between "words"; 
//! This is a fairly restricted list, meaning that many things can be part of a "word"
//! in this sense (for ex: "this-is_a+single@wor'd"), this is to insure a smooth transition
//! for the new setOption, which calls readOptionVal ... which may call read(istream&, string&)...
static const string wordseparators(" \t\n\r)]};|#"); 

//! If the string does not contain any "wordseparatos" does not start with a digit (it's considered a single "word")
//! then it's written as is. Otherwise the string is written in quotes (and quotes inside it are escaped with a \ )
  void write(ostream& out, const string& x);

//! reads a string from the stream in one of those formats: singleword, "quoted string", <length> <stringval>
  void read(istream& in, string& x) ;
*/
/* These functions have been moved to Array.h (at least for now)
   Below are the old versions...

  template<class T>
  inline void write(ostream& out, const vector<T>& v) {
      int l=v.size(); write(out,l); for (int i=0;i<l;i++) write(out,v[i]);
  }

  template<class T>
  inline void read(istream& in, vector<T>& v) {
      int l; read(in,l); v.resize(l); for (int i=0;i<l;i++) read(in,v[i]);
  }
*/
/*
  template<class K, class V>
  inline void write(ostream& out, const map<K,V>& m) {
    typename map<K,V>::const_iterator i = m.begin();
    typename map<K,V>::const_iterator j = m.end();
    write(out,m.size());
    for (;i!=j;i++) write(out,*i);
  }

  template<class K, class V>
  inline void read(istream& in, map<K,V>& m) {
    pair<K,V> p; int l; read(in,l); m.clear();
    for (int i=0;i<l;i++) { read(in,p); m.insert(p); }
  }
*/
// *************************************
// ***  binary read/write operations ***
// *************************************

// Default behavior
template <class T> inline void
binread(istream &in, T &x)
{ PLERROR("binread() - Not defined for this type"); }

template <class T> inline void
binwrite(ostream &out, const T &x)
{ PLERROR("binwrite() - Not defined for this type"); };

// binread() and binwrite() are defined only for
// certain types.
template <class T> inline void
binread(istream &in, T &x, int n)
{ PLERROR("binread() - Not defined for this type"); };

template <class T> inline void
binwrite(ostream &out, const T &x, int n)
{ PLERROR("binwrite() - Not defined for this type"); };

//!  general purpose (but less efficient)
//!  version for pointers to things that have a binwrite/binread function
template<class T>
inline void binwrite(ostream& out, const T* x, int n)
{ for (int i=0;i<n;i++) binwrite(out,x[i]); }

template<class T>
inline void binread(istream& out, T* x, int n)
{ for (int i=0;i<n;i++) binread(out,x[i]); }

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
  inline void binread(istream& in, int& x) { in.read((char*)&x,sizeof(int)); if(swap_endians_on_read) endianswap(&x); }
  inline void binwrite(ostream& out, unsigned int x) { out.write((char*)&x,sizeof(unsigned int)); }
  inline void binread(istream& in, unsigned int& x) { in.read((char*)&x,sizeof(unsigned int)); if(swap_endians_on_read) endianswap(&x); }
  inline void binwrite(ostream& out, short x) { out.write((char*)&x,sizeof(short)); }
  inline void binread(istream& in, short& x) { in.read((char*)&x,sizeof(short)); if(swap_endians_on_read) endianswap(&x); }
  inline void binwrite(ostream& out, unsigned short x) { out.write((char*)&x,sizeof(unsigned short)); }
  inline void binread(istream& in, unsigned short& x) { in.read((char*)&x,sizeof(unsigned short)); if(swap_endians_on_read) endianswap(&x); }
  //!  note that bool are saved as unsigned short
  inline void binwrite(ostream& out, bool x) { binwrite(out,(unsigned short)x); }
  inline void binread(istream& in, bool& x) { unsigned short u; binread(in,u); x=u; }

// The following read/write floats (4 bytes) on disk, regardless of whether the memory-elements are float or double
  inline void binwrite(ostream& out, float x) { out.write((char*)&x,sizeof(float)); }
  inline void binread(istream& in, float& x) { in.read((char*)&x,sizeof(float)); if(swap_endians_on_read) endianswap(&x); }
  inline void binwrite(ostream& out, double x) { binwrite(out, float(x)); }
  inline void binread(istream& in, double& x) { float f; binread(in,f); x = double(f); }

// The following read/write doubles (8 bytes) on disk, regardless of whether the memory-elements are float or double
  inline void binwrite_double(ostream& out, double x) { out.write((char*)&x,sizeof(double)); }
  inline void binread_double(istream& in, double& x) { in.read((char*)&x,sizeof(double)); if(swap_endians_on_read) endianswap(&x); }
  inline void binwrite_double(ostream& out, float x) { binwrite_double(out, double(x)); }
  inline void binread_double(istream& in, float& x) { double d; binread_double(in,d); x = float(d); }


  //!  multi-element versions, giving address and number of elements
  inline void binwrite(ostream& out, const int* x, int n) { out.write((char*)x, n*sizeof(int)); }
  inline void binread(istream& in, int* x, int n) { in.read((char*)x, n*sizeof(int)); if(swap_endians_on_read) endianswap(x,n); }
  inline void binwrite(ostream& out, const unsigned int* x, int n) { out.write((char*)x, n*sizeof(unsigned int)); }
  inline void binread(istream& in, unsigned int* x, int n)  { in.read((char*)x, n*sizeof(unsigned int)); if(swap_endians_on_read) endianswap(x,n); }
  inline void binwrite(ostream& out, const short* x, int n) { out.write((char*)x, n*sizeof(short)); }
  inline void binread(istream& in, short* x, int n) { in.read((char*)x, n*sizeof(short)); if(swap_endians_on_read) endianswap(x,n); }
  inline void binwrite(ostream& out, const unsigned short* x, int n) { out.write((char*)x, n*sizeof(unsigned short)); }
  inline void binread(istream& in, unsigned short* x, int n) { in.read((char*)x, n*sizeof(unsigned short)); if(swap_endians_on_read) endianswap(x,n); }

// The followind read/write 4-byte-floats on disk (whether we pass them a float* or a double*)
  inline void binwrite(ostream& out, const float *x, int n) { out.write((char*)x, n*sizeof(float)); }
  inline void binread(istream& in, float* x, int n) { in.read((char*)x, n*sizeof(float)); if(swap_endians_on_read) endianswap(x,n); }
  inline void binwrite(ostream& out, const double* x, int n) { for(int i=0; i<n; i++) binwrite(out,x[i]); }
  inline void binread(istream& in, double* x, int n) { for(int i=0; i<n; i++) binread(in,x[i]); }

// The followind read/write 8-byte-doubles on disk (whether we pass them a float* or a double*)
  inline void binwrite_double(ostream& out, const double *x, int n) { out.write((char*)x, n*sizeof(double)); }
  inline void binread_double(istream& in, double* x, int n) { in.read((char*)x, n*sizeof(double)); if(swap_endians_on_read) endianswap(x,n); }
  inline void binwrite_double(ostream& out, const float* x, int n) { for(int i=0; i<n; i++) binwrite(out,x[i]); }
  inline void binread_double(istream& in, float* x, int n) { for(int i=0; i<n; i++) binread(in,x[i]); }

//! version for compressed array (efficient for sparse data, and small integer values)
//! (format is detailed in .cc, see write_compr_mode_and_size function in general.cc)
void binwrite_compressed(ostream& out, const double* data, int l);
void binread_compressed(istream& in, double* data, int l);
void binwrite_compressed(ostream& out, const float* data, int l);
void binread_compressed(istream& in, float* data, int l);

//! compressed vec to and from memory
inline void read_compr_mode_and_size_ptr(char*& in, unsigned char& mode, int& size);
void compress_vec(char* comprbuf, const double* data, int l, bool double_stored_as_float=false);
void uncompress_vec(char* comprbuf, double* data, int l, bool double_stored_as_float=false);
double compressed_dot_product(char* comprbufvec, const double* vecdata, int l, bool double_stored_as_float=false);


// Read operators
//
/*
inline pl_istream &
operator>>(pl_istream &in, char &x)
{
    if (in.pl_stream_flags.test(plf_binary)) {
        in.get(x);
    } else { // plf_plain
        in >> raw >> x;
        if (isspace(in.peek()))
            in.get();
    }
    return in;
}

inline pl_istream &
operator>>(pl_istream &in, unsigned char &x)
{
    if (in.pl_stream_flags.test(plf_binary)) {
        in.get(reinterpret_cast<char &>(x));
    } else { // plf_plain
        in >> raw >> x;
        if (isspace(in.peek()))
            in.get();
    }
    return in;
}

inline pl_istream &
operator>>(pl_istream &in, int &x)
{
    if (in.pl_stream_flags.test(plf_binary)) {
        in.read(reinterpret_cast<char *>(&x), sizeof(int));
        if (in.pl_stream_flags.test(plf_swap_endian))
            endianswap(&x);
    } else { // plf_plain
        in >> raw >> x;
        if (isspace(in.peek()))
            in.get();
    }
    return in;
}

inline pl_istream &
operator>>(pl_istream &in, unsigned int &x)
{
    if (in.pl_stream_flags.test(plf_binary)) {
        in.read(reinterpret_cast<char *>(&x), sizeof(unsigned int));
        if (in.pl_stream_flags.test(plf_swap_endian))
            endianswap(&x);
    } else { // plf_plain
        in >> raw >> x;
        if (isspace(in.peek()))
            in.get();
    }
    return in;
}

inline pl_istream &
operator>>(pl_istream &in, long int &x)
{
    if (in.pl_stream_flags.test(plf_binary)) {
        PLERROR("flags not supported");
    } else { // plf_plain
        in >> raw >> x;
        if (isspace(in.peek()))
            in.get();
    }
    return in;
}

inline pl_istream &
operator>>(pl_istream &in, unsigned long &x)
{
    if (in.pl_stream_flags.test(plf_binary)) {
        PLERROR("flags not supported");
    } else { // plf_plain
        in >> raw >> x;
        if (isspace(in.peek()))
            in.get();
    }
    return in;
}

inline pl_istream &
operator>>(pl_istream &in, short int &x)
{
    if (in.pl_stream_flags.test(plf_binary)) {
        in.read(reinterpret_cast<char *>(&x), sizeof(short int));
        if (in.pl_stream_flags.test(plf_swap_endian))
            endianswap(&x);
    } else { // plf_plain
        in >> raw >> x;
        if (isspace(in.peek()))
            in.get();
    }
    return in;
}

inline pl_istream &
operator>>(pl_istream &in, unsigned short &x)
{
    if (in.pl_stream_flags.test(plf_binary)) {
        in.read(reinterpret_cast<char *>(&x), sizeof(unsigned short));
        if (in.pl_stream_flags.test(plf_swap_endian))
            endianswap(&x);
    } else { // plf_plain        
        in >> raw >> x;
        if (isspace(in.peek()))
            in.get();
    }
    return in;
}

template <class S, class T> inline pl_istream &
operator>>(pl_istream &in, pair<S, T> &x)
{
    in >> x.first;
    in >> x.second;
    return in;
}

template <class S, class T> inline pl_istream &
operator>>(pl_istream &in, map<S, T> &x)
{
    int l;
    pair<S, T> p;
    in >> l;
    x.clear();
    for (int i = 0; i < l; ++i) {
        in >> p;
        x.insert(p);
    }
    return in;
}

pl_istream &operator>>(pl_istream &in, bool &x);
pl_istream &operator>>(pl_istream &in, float &x);
pl_istream &operator>>(pl_istream &in, double &x);
pl_istream &operator>>(pl_istream &in, char * &x);
pl_istream &operator>>(pl_istream &in, string &x);
*/

/*V************************************************************V*/

// Read operators
//
/*
inline pl_stream &
operator>>(pl_stream &in, char &x)
{
    if (in.pl_stream_flags_in.test(plf_binary)) {
        in.get(x);
    } else { // plf_plain
        in >> raw >> x;
        if (isspace(in.peek()))
            in.get();
    }
    return in;
}

inline pl_stream &
operator>>(pl_stream &in, unsigned char &x)
{
    if (in.pl_stream_flags_in.test(plf_binary)) {
        in.get(reinterpret_cast<char &>(x));
    } else { // plf_plain
        in >> raw >> x;
        if (isspace(in.peek()))
            in.get();
    }
    return in;
}

inline pl_stream &
operator>>(pl_stream &in, int &x)
{
    if (in.pl_stream_flags_in.test(plf_binary)) {
        in.read(reinterpret_cast<char *>(&x), sizeof(int));
        if (in.pl_stream_flags_in.test(plf_swap_endian))
            endianswap(&x);
    } else { // plf_plain
        in >> raw >> x;
        if (isspace(in.peek()))
            in.get();
    }
    return in;
}

inline pl_stream &
operator>>(pl_stream &in, unsigned int &x)
{
    if (in.pl_stream_flags_in.test(plf_binary)) {
        in.read(reinterpret_cast<char *>(&x), sizeof(unsigned int));
        if (in.pl_stream_flags_in.test(plf_swap_endian))
            endianswap(&x);
    } else { // plf_plain
        in >> raw >> x;
        if (isspace(in.peek()))
            in.get();
    }
    return in;
}

inline pl_stream &
operator>>(pl_stream &in, long int &x)
{
    if (in.pl_stream_flags_in.test(plf_binary)) {
        PLERROR("flags not supported");
    } else { // plf_plain
        in >> raw >> x;
        if (isspace(in.peek()))
            in.get();
    }
    return in;
}

inline pl_stream &
operator>>(pl_stream &in, unsigned long &x)
{
    if (in.pl_stream_flags_in.test(plf_binary)) {
        PLERROR("flags not supported");
    } else { // plf_plain
        in >> raw >> x;
        if (isspace(in.peek()))
            in.get();
    }
    return in;
}

inline pl_stream &
operator>>(pl_stream &in, short int &x)
{
    if (in.pl_stream_flags_in.test(plf_binary)) {
        in.read(reinterpret_cast<char *>(&x), sizeof(short int));
        if (in.pl_stream_flags_in.test(plf_swap_endian))
            endianswap(&x);
    } else { // plf_plain
        in >> raw >> x;
        if (isspace(in.peek()))
            in.get();
    }
    return in;
}

inline pl_stream &
operator>>(pl_stream &in, unsigned short &x)
{
    if (in.pl_stream_flags_in.test(plf_binary)) {
        in.read(reinterpret_cast<char *>(&x), sizeof(unsigned short));
        if (in.pl_stream_flags_in.test(plf_swap_endian))
            endianswap(&x);
    } else { // plf_plain        
        in >> raw >> x;
        if (isspace(in.peek()))
            in.get();
    }
    return in;
}

template <class S, class T> inline pl_stream &
operator>>(pl_stream &in, pair<S, T> &x)
{
    in >> x.first;
    in >> y.second;
    return in;
}

template <class S, class T> inline pl_stream &
operator>>(pl_stream &in, map<S, T> &x)
{
    int l;
    pair<S, T> p;
    in >> l;
    x.clear();
    for (int i = 0; i < l; ++i) {
        in >> p;
        x.insert(p);
    }
    return in;
}

pl_stream &operator>>(pl_stream &in, bool &x);
pl_stream &operator>>(pl_stream &in, float &x);
pl_stream &operator>>(pl_stream &in, double &x);
pl_stream &operator>>(pl_stream &in, char * &x);
pl_stream &operator>>(pl_stream &in, string &x);
*/
/*^************************************************************^*/

// Write operators
//
/*
inline pl_ostream &
operator<<(pl_ostream &out, char x)
{
    if (out.pl_stream_flags.test(plf_binary)) {
        out.put(x);
    } else { // plf_plain
        out << raw << x << ' ';
    }
    return out;
}

inline pl_ostream &
operator<<(pl_ostream &out, unsigned char x)
{
    if (out.pl_stream_flags.test(plf_binary)) {
        out.put(x);
    } else { // plf_plain
        out << raw << x << ' ';
    }
    return out;
}

inline pl_ostream &
operator<<(pl_ostream &out, int x)
{
    if (out.pl_stream_flags.test(plf_binary)) {
        out.write(reinterpret_cast<char *>(&x), sizeof(int));
    } else { // plf_plain
        out << raw << x << ' ';
    }
    return out;
}

inline pl_ostream &
operator<<(pl_ostream &out, unsigned int x)
{
    if (out.pl_stream_flags.test(plf_binary)) {
        out.write(reinterpret_cast<char *>(&x), sizeof(unsigned int));
    } else { // plf_plain
        out << raw << x << ' ';
    }
    return out;
}

inline pl_ostream &
operator<<(pl_ostream &out, long int x)
{
    if (out.pl_stream_flags.test(plf_binary)) {
        PLERROR("flags not supported");
    } else { // plf_plain
        out << raw << x << ' ';
    }
    return out;
}

inline pl_ostream &
operator<<(pl_ostream &out, unsigned long x)
{
    if (out.pl_stream_flags.test(plf_binary)) {
        PLERROR("flags not supported");
    } else { // plf_plain
        out << raw << x << ' ';
    }
    return out;
}

inline pl_ostream &
operator<<(pl_ostream &out, short int x)
{
    if (out.pl_stream_flags.test(plf_binary)) {
        out.write(reinterpret_cast<char *>(&x), sizeof(short int));
    } else { // plf_plain
        out << raw << x << ' ';
    }
    return out;
}

inline pl_ostream &
operator<<(pl_ostream &out, unsigned short x)
{
    if (out.pl_stream_flags.test(plf_binary)) {
        out.write(reinterpret_cast<char *>(&x), sizeof(unsigned short));
    } else { // plf_plain
        out << raw << x << ' ';
    }
    return out;
}

inline pl_ostream &
operator<<(pl_ostream &out, bool x)
{
    if (out.pl_stream_flags.test(plf_binary)) {
        out << static_cast<unsigned short>(x);
    } else { // plf_plain
        out << raw << (x ? '1' : '0') << ' ';
    }
    return out;
}

template <class S, class T> inline pl_ostream &
operator<<(pl_ostream &out, const pair<S, T> &x)
{
    out << x.first;
    out << x.second;
    return out;
}

template <class S, class T> inline pl_ostream &
operator<<(pl_ostream &out, const map<S, T> &x)
{
    typename map<S, T>::const_iterator i = x.begin();
    typename map<S, T>::const_iterator j = x.end();
    out << x.size();
    for ( ; i != j; ++i)
        out << *i;
    return out;
}

pl_ostream &operator<<(pl_ostream &out, float x);
pl_ostream &operator<<(pl_ostream &out, double x);
pl_ostream &operator<<(pl_ostream &out, const char *x);
pl_ostream &operator<<(pl_ostream &out, const string &x);
*/
/*v**************************************************v*/

// Write operators
//
/*
inline pl_stream &
operator<<(pl_stream &out, char x)
{
    if (out.pl_stream_flags_out.test(plf_binary)) {
        out.put(x);
    } else { // plf_plain
        out << raw << x << ' ';
    }
    return out;
}

inline pl_stream &
operator<<(pl_stream &out, unsigned char x)
{
    if (out.pl_stream_flags_out.test(plf_binary)) {
        out.put(x);
    } else { // plf_plain
        out << raw << x << ' ';
    }
    return out;
}

inline pl_stream &
operator<<(pl_stream &out, int x)
{
    if (out.pl_stream_flags_out.test(plf_binary)) {
        out.write(reinterpret_cast<char *>(&x), sizeof(int));
    } else { // plf_plain
        out << raw << x << ' ';
    }
    return out;
}

inline pl_stream &
operator<<(pl_stream &out, unsigned int x)
{
    if (out.pl_stream_flags_out.test(plf_binary)) {
        out.write(reinterpret_cast<char *>(&x), sizeof(unsigned int));
    } else { // plf_plain
        out << raw << x << ' ';
    }
    return out;
}

inline pl_stream &
operator<<(pl_stream &out, long int x)
{
    if (out.pl_stream_flags_out.test(plf_binary)) {
        PLERROR("flags not supported");
    } else { // plf_plain
        out << raw << x << ' ';
    }
    return out;
}

inline pl_stream &
operator<<(pl_stream &out, unsigned long x)
{
    if (out.pl_stream_flags_out.test(plf_binary)) {
        PLERROR("flags not supported");
    } else { // plf_plain
        out << raw << x << ' ';
    }
    return out;
}

inline pl_stream &
operator<<(pl_stream &out, short int x)
{
    if (out.pl_stream_flags_out.test(plf_binary)) {
        out.write(reinterpret_cast<char *>(&x), sizeof(short int));
    } else { // plf_plain
        out << raw << x << ' ';
    }
    return out;
}

inline pl_stream &
operator<<(pl_stream &out, unsigned short x)
{
    if (out.pl_stream_flags_out.test(plf_binary)) {
        out.write(reinterpret_cast<char *>(&x), sizeof(unsigned short));
    } else { // plf_plain
        out << raw << x << ' ';
    }
    return out;
}

inline pl_stream &
operator<<(pl_stream &out, bool x)
{
    if (out.pl_stream_flags_out.test(plf_binary)) {
        out << static_cast<unsigned short>(x);
    } else { // plf_plain
        out << raw << (x ? '1' : '0') << ' ';
    }
    return out;
}

template <class S, class T> inline pl_stream &
operator<<(pl_stream &out, const pair<S, T> &x)
{
    out << x.first;
    out << x.second;
    return out;
}

template <class S, class T> inline pl_stream &
operator<<(pl_stream &out, const map<S, T> &x)
{
    typename map<S, T>::const_iterator i = x.begin();
    typename map<S, T>::const_iterator j = x.end();
    out << x.size();
    for ( ; i != j; ++i)
        out << *i;
    return out;
}

pl_stream &operator<<(pl_stream &out, float x);
pl_stream &operator<<(pl_stream &out, double x);
pl_stream &operator<<(pl_stream &out, const char *x);
pl_stream &operator<<(pl_stream &out, const string &x);
*/
/*^**************************************************^*/

// ********************************
// *** save and load operations ***
// ********************************

template <class T> inline void
load(const string &filename, T &x, OBflag_t flags = dft_option_flag)
{
    ifstream in_(filename.c_str());
    if (!in_)
        PLERROR("Could not open file \"%s\" for reading", filename.c_str());
    PStream in(&in_);
    in.option_flags_in = flags;
    int c = in.peek();
    if (c == '~') // File is big endian
      PLERROR("RELOADING OF BIG ENDIAN RAW BORDEL NOT YET SUPPORTED");
    /*
#ifdef BIGENDIAN
        in.pl_stream_flags_in.reset(plf_swap_endian);
#else
        in.pl_stream_flags_in.set(plf_swap_endian) = 1;
#endif
    } else {
#ifdef LITTLEENDIAN
        in.pl_stream_flags_in.reset(plf_swap_endian);
#else
        in.pl_stream_flags_in.set(plf_swap_endian) = 1;
#endif
    }
    */

    in >> x;
}

template<class T> 
inline void save(const string& filename, const T& x, OBflag_t flags = dft_option_flag)
{ 
    ofstream out_(filename.c_str()); 
    if(!out_)
        PLERROR("Could not open file %s for writing",filename.c_str());

    PStream out(&out_);
    out << option_flags(flags);
    out << x;
}

/*
template<class T> 
inline void load(const string& filename, T& x, OBflag_t option_flag= dft_option_flag) 
{ 
  ifstream in(filename.c_str()); 
  if(!in)
    PLERROR("could not open file %s for reading",filename.c_str());
  // store endianswap status
  bool old_swap_endians_on_read = swap_endians_on_read;
  int c = in.peek();
  if(c=='~') // File is big endian
    {
      in.get();
#ifdef BIGENDIAN
          swap_endians_on_read = false;
#else
          swap_endians_on_read = true;
#endif
    }
  else // File is little endian
    {
#ifdef LITTLEENDIAN
          swap_endians_on_read = false;
#else
          swap_endians_on_read = true;
#endif
    }

  // For now
  SerialOptions opts(option_flag);
  read(in, x, opts); 

  // restore endianswap status
  swap_endians_on_read = old_swap_endians_on_read;
}
*/

/*
template <class T> 
inline void read(istream& in, const T& x, OBflag_t option_flag)
{ read(in, const_cast<T &>(x)); }
*/

// **********************************************************
// **** DEEPWRITE AND DEEPREAD (use only if necessary!) *****
// ****      THIS SYSTEM IS LIKELY TO CHANGE SOON       *****
// **********************************************************

  //!  deepWrite and deepRead for a few basic types
  typedef map<unsigned long, void*> DeepReadMap;
  typedef set<void*> DeepWriteSet;
  inline void deepWrite(ostream& out, DeepWriteSet& already_saved, char x) { write(out,x); }
  inline void deepRead(istream& in, DeepReadMap& old2new, char& x) { read(in,x); }
  inline void deepWrite(ostream& out, DeepWriteSet& already_saved, int x) { write(out,x); }
  inline void deepRead(istream& in, DeepReadMap& old2new, int& x) { read(in,x); }
  inline void deepWrite(ostream& out, DeepWriteSet& already_saved, unsigned int x) { write(out,x); }
  inline void deepRead(istream& in, DeepReadMap& old2new, unsigned int& x) { read(in,x); }
  inline void deepWrite(ostream& out, DeepWriteSet& already_saved, unsigned long x) { write(out,x); }
  inline void deepRead(istream& in, DeepReadMap& old2new, unsigned long& x) { read(in,x); }
  inline void deepWrite(ostream& out, DeepWriteSet& already_saved, long int x) { write(out,x); }
  inline void deepRead(istream& in, DeepReadMap& old2new, long int& x) { read(in,x); }
  inline void deepWrite(ostream& out, DeepWriteSet& already_saved, bool x) { write(out,x); }
  inline void deepRead(istream& in, DeepReadMap& old2new, bool& x) { read(in,x); }
  inline void deepWrite(ostream& out, DeepWriteSet& already_saved, float x) { write(out,x); }
  inline void deepRead(istream& in, DeepReadMap& old2new, float& x) { read(in,x); }
  inline void deepWrite(ostream& out, DeepWriteSet& already_saved, double x) { write(out,x); }
  inline void deepRead(istream& in, DeepReadMap& old2new, double& x) { read(in,x); }
  template<class A,class B>
  inline void deepWrite(ostream& out, DeepWriteSet& already_saved, pair<A,B> x) { write(out,x); }
  template<class A,class B>
  inline void deepRead(istream& in, DeepReadMap& old2new, pair<A,B>& x) { read(in,x); }
  inline void deepWrite(ostream& out, DeepWriteSet& already_saved, const char* x) { write(out,x); }
  inline void deepRead(istream& in, DeepReadMap& old2new, char*& x) { read(in,x); }
  inline void deepWrite(ostream& out, DeepWriteSet& already_saved, const string& x) { write(out,x); }
  inline void deepRead(istream& in, DeepReadMap& old2new, string& x) { read(in,x); }

  template<class T>
  inline void deepWrite(ostream& out, T& x)
  { DeepWriteSet already_saved; deepWrite(out, already_saved, x); out.flush(); }

  template<class T>
  inline void deepRead(istream& in, T& x)
  { DeepReadMap old2new; deepRead(in, old2new, x); }



%> // end of namespace PLearn


#endif
