// -*- C++ -*-

// pl_hash_fun  Hash functions for the PLearn library.
// Copyright (C) 2002 Xavier Saint-Mleux
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

#ifndef pl_hash_fun_H
#define pl_hash_fun_H

#include "ms_hash_wrapper.h"
#include <cstring>

namespace PLearn {

using namespace std;

extern const size_t PL_HASH_NOMBRES_MAGIQUES[256];

//!  **************** Hash tables support *************************

/*! basic hashing function that can be used in defining the
  hashing functions for objects or any type. This one
  mixes the bits in the byte_length bytes starting at byte_start, 
  and returns an integer between 0 and MAXINT
*/
size_t hashbytes(const char* byte_start, size_t byte_length);

/*!     hashing function which must be redefined for classes that
  can be used as keys:
    
  unsigned int hash(const T& object);
  or
  unsigned int hash(const T object);
    
  This function returns ANY unsigned int (i.e. between 0 and MAXINT)
  (it is hash(x)%table_size that will be used to choose an address
  in the hash table).
    
  It is defined here for some built-in types:
    
*/
inline size_t hashval(const char* strng)
{ return hashbytes(strng, strlen(strng)); }

//!  default which will work in many cases but not all
template <class T>
inline size_t hashval(const T& x) { return hashbytes((char*)&x,sizeof(T)); }

} // end of namespace PLearn


///////////////////////////////////////////////////////////////////////////

#if defined(__GNUC__) && defined(__INTEL_COMPILER) && __INTEL_COMPILER < 1000
// Intel Compiler (before 10.0) on Linux: we need to define a hash function for
// const char*.
SET_HASH_WITH_FUNCTION_NOCONSTREF(const char*, _s, PLearn::hashval(_s))
#endif

    SET_HASH_WITH_INHERITANCE(std::string, const char*, __s, __s.c_str())

//hash functions for strings
//template<>
//struct hash<string>
//{
//	size_t operator()(const string& __s) const { return hash<const char*>()(__s.c_str()); }
//};

// This has been deactivated because it is useless (and .NET doesn't like it):
//SET_HASH_WITH_INHERITANCE(const std::string, const char*, __s, __s.c_str())
//template<>
//struct hash<const string>
//{
//  size_t operator()(const string& __s) const { return hash<const char*>()(__s.c_str()); }
//  //size_t operator()(const string& __s) const { return __stl_hash_string(__s.c_str()); }
//};


//for doubles 
    SET_HASH_WITH_FUNCTION(double, x, PLearn::hashval(x))
//template<>
//struct hash<double>
//{
//  size_t operator()(double x) const { return PLearn::hashval(x); }
//};

//for floats 
    SET_HASH_WITH_FUNCTION(float, x, PLearn::hashval(x))
//template<>
//struct hash<float>
//{
//  size_t operator()(float x) const { return PLearn::hashval(x); }
//};


#if defined(WIN32) && !defined(_MINGW_) && !defined(__CYGWIN__)

    using namespace stdext;

#if defined(__INTEL_COMPILER)
// Because Intel compiler (in WIN32 only!!) defines hash_map and hash_table both in stdext and std
// if we set that we use both (with "using namespace") it will have an ambiguity.
// To solve this, I force hash_map, hash_multimap and hash_table to be explicit.

#define hash_map stdext::hash_map
#define hash_multimap stdext::hash_multimap
#define hash_set stdext::hash_set
#define hash_multiset stdext::hash_multiset

#endif // __INTEL_COMPILER

#elif defined(__INTEL_COMPILER) && defined(_NAMESPACE_STLPORT)
// STL port version (e.g. Ms computers). There is also a namespace conflict.
// We prefer to use the STL port version because it is easier to debug.

#define hash_map _NAMESPACE_STLPORT::hash_map
#define hash_multimap _NAMESPACE_STLPORT::hash_multimap
#define hash_set _NAMESPACE_STLPORT::hash_set
#define hash_multiset _NAMESPACE_STLPORT::hash_multiset

#endif // WIN32

#endif // pl_hash_fun_H


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
