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

#if __GNUC__<3 
#  include <hash_set> //to get stl_hash_fun.h ... (template<> class hash)
#  include <hash_map> //to get stl_hash_fun.h ... (template<> class hash)
#else
#  include <ext/hash_set> //to get stl_hash_fun.h ... (template<> class hash)
#  include <ext/hash_map> //to get stl_hash_fun.h ... (template<> class hash)
using namespace __gnu_cxx;
#endif
#include <string>

namespace PLearn <%
using namespace std;


  extern const unsigned int PL_HASH_NOMBRES_MAGIQUES[256];

  //!  **************** Hash tables support *************************

/*!     basic hashing function that can be used in defining the
    hashing functions for objects or any type. This one
    mixes the bits in the byte_length bytes starting at byte_start, 
    and returns an integer between 0 and MAXINT
*/
  unsigned int hashbytes(const char* byte_start, int byte_length);

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
  inline unsigned int hashval(const char* strng)
    { return hashbytes(strng,strlen(strng)); }

  //!  default which will work in many cases but not all
  template <class T>
    inline unsigned int hashval(const T& x) { return hashbytes((char*)&x,sizeof(T)); }

%> // end of namespace PLearn


///////////////////////////////////////////////////////////////////////////

#if __GNUC__==3 && __GNUC_MINOR__>0
namespace __gnu_cxx <%
#else
namespace std <%
#endif
using std::string;


//hash functions for strings
template<>
struct hash<string>
{
  size_t operator()(const string& __s) const { return hash<const char*>()(__s.c_str()); }
};

template<>
struct hash<const string>
{
  size_t operator()(const string& __s) const { return hash<const char*>()(__s.c_str()); }
  //size_t operator()(const string& __s) const { return __stl_hash_string(__s.c_str()); }
};


//for doubles 
template<>
struct hash<double>
{
  size_t operator()(double x) const { return PLearn::hashval(x); }
};

//for floats 
template<>
struct hash<float>
{
  size_t operator()(float x) const { return PLearn::hashval(x); }
};

%> // end of namespace


#endif // pl_hash_fun_H
