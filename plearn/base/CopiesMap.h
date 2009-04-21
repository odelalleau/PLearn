// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1999-2006 University of Montreal and individual contributors

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


#ifndef CopiesMap_INC
#define CopiesMap_INC

#include <deque>
#include <map>
#include <plearn/base/pl_hash_fun.h>
#include <set>
#include <list>
#include <vector>
#include <string>
#include <time.h>	//!< For definition of 'clock_t'.
#include <utility>      //!< For pair.
#include "plerror.h"    //!< For PLWARNING.


//! Macro to define deep copy for types that actually do not require
//! any deep copy (such as int, real, etc.).
//! Since the copy constructor of an array does copy the content of
//! its storage, deep copying an array of such types is not necessary
//! either.
#define NODEEPCOPY(TYPE)                                      \
      inline void deepCopyField(TYPE&, CopiesMap&) {}         \
      inline void deepCopyField(Array<TYPE>&, CopiesMap&) {}

namespace PLearn {
using std::string;
using std::deque;
using std::map;
using std::multimap;
using std::set;
using std::multiset;
using std::list;
using std::vector;
using std::pair;

class PPath;
class VMField;
class VMFieldStat;
template <class T> class Array;

//!  Global typedef to make the map of copied objects (needed by the deep
//!  copy mechanism in Object) more palatable
typedef map<const void*,void*> CopiesMap;

//! Some typedefs to use the NODEEPCOPY macro with.
typedef map<string, float> map_string_float;
typedef map<string, double> map_string_double;
typedef map<double, string> map_double_string;
typedef map<float, string> map_float_string;
typedef map<string, string> map_string_string;
typedef map<float, float> map_float_float;
typedef map<double, double> map_double_double;
typedef map<string, int> map_string_int;
typedef map<int, string> map_int_string;

/**
 *  Support for generic deep copying
 *      
 *  Deep copying is defined for objects in the following manner:
 *
 *  - copy constructors should always do a shallow copy
 *
 *  - a public method OBJTYPE* deepCopy(CopiesMap& copies) const 
 *    should be defined to allow deepCopying
 *
 *  - the deepCopy method should be virtual for classes that are designed to be
 *    subclassed Take a close look at the Object class in Object.h to see how
 *    this is done.
 */

//! Types that do not require deep copy.
NODEEPCOPY(double)
NODEEPCOPY(const double)
NODEEPCOPY(float)
NODEEPCOPY(const float)
NODEEPCOPY(int)
NODEEPCOPY(int8_t)
NODEEPCOPY(const int)
NODEEPCOPY(unsigned int)
NODEEPCOPY(const unsigned int)
NODEEPCOPY(short)
NODEEPCOPY(const short)
NODEEPCOPY(unsigned short)
NODEEPCOPY(const unsigned short)
NODEEPCOPY(char)
NODEEPCOPY(const char)
NODEEPCOPY(unsigned char)
NODEEPCOPY(const unsigned char)
NODEEPCOPY(const string)
NODEEPCOPY(clock_t)
NODEEPCOPY(bool)
NODEEPCOPY(map_string_float)
NODEEPCOPY(map_string_double)
NODEEPCOPY(map_float_string)
NODEEPCOPY(map_double_string)
NODEEPCOPY(map_string_string)
NODEEPCOPY(map_float_float)
NODEEPCOPY(map_double_double)
NODEEPCOPY(map_string_int)
NODEEPCOPY(map_int_string)
NODEEPCOPY(string)
NODEEPCOPY(PPath)
NODEEPCOPY(VMField)
NODEEPCOPY(VMFieldStat)
NODEEPCOPY(FILE*)   //!< There is currently no proper way to deep copy these.


//#####  Some Standard STL Containers  ########################################

//! Pairs handle deepCopying by distributing it to each element
template <class T, class U>
inline void deepCopyField(pair<T,U>& p, CopiesMap& copies)
{
    deepCopyField(p.first, copies);
    deepCopyField(p.second, copies);
}

//! Standard containers handle deepcopying by distributing it to each element
template <class T, class Alloc>
void deepCopyField(deque<T,Alloc>& c, CopiesMap& copies)
{
    for (typename deque<T,Alloc>::iterator it = c.begin(), end=c.end()
             ; it != end ; ++it)
        deepCopyField(*it, copies);
}

template <class T, class U, class Compare, class Alloc>
void deepCopyField(map<T,U,Compare,Alloc>& c, CopiesMap& copies)
{
    for (typename map<T,U,Compare,Alloc>::iterator it = c.begin(), end=c.end()
             ; it != end ; ++it)
        deepCopyField(*it, copies);
}

template <class T, class U, class Compare, class Alloc>
void deepCopyField(hash_map<T,U,Compare,Alloc>& c, CopiesMap& copies)
{
    for (typename hash_map<T,U,Compare,Alloc>::iterator it = c.begin(), end=c.end()
             ; it != end ; ++it)
        deepCopyField(*it, copies);
}

template <class T, class U, class Compare, class Alloc>
void deepCopyField(multimap<T,U,Compare,Alloc>& c, CopiesMap& copies)
{
    for (typename multimap<T,U,Compare,Alloc>::iterator it = c.begin(), end=c.end()
             ; it != end ; ++it)
        deepCopyField(*it, copies);
}

template <class T, class Compare, class Alloc>
void deepCopyField(set<T,Compare,Alloc>& c, CopiesMap& copies)
{
    for (typename set<T,Compare,Alloc>::iterator it = c.begin(), end=c.end()
             ; it != end ; ++it)
        deepCopyField(*it, copies);
}

template <class T, class Compare, class Alloc>
void deepCopyField(multiset<T,Compare,Alloc>& c, CopiesMap& copies)
{
    for (typename multiset<T,Compare,Alloc>::iterator it = c.begin(), end=c.end()
             ; it != end ; ++it)
        deepCopyField(*it, copies);
}

template <class T, class Alloc>
void deepCopyField(list<T,Alloc>& c, CopiesMap& copies)
{
    for (typename list<T,Alloc>::iterator it = c.begin(), end=c.end()
             ; it != end ; ++it)
        deepCopyField(*it, copies);
}

template <class T, class Alloc>
void deepCopyField(vector<T,Alloc>& c, CopiesMap& copies)
{
    for (typename vector<T,Alloc>::iterator it = c.begin(), end=c.end()
             ; it != end ; ++it)
        deepCopyField(*it, copies);
}

//#####  Fallbacks  ###########################################################

//!  Any type not handled below: do nothing
template <class T>
inline void deepCopyField(T&, CopiesMap&)
{
    /*! no op */
    PLWARNING(
        "In CopiesMap.h - deepCopyField not handled for the type '%s'. "
        "If it actually doesn't need deep copy, edit CopiesMap.h and add"
        " NODEEPCOPY(your_type) to remove this warning.",
        TypeTraits<T>().name().c_str()
        );
}

template <class T>
inline void deepCopyField(T*& field, CopiesMap& copies)
{
    if (field)
        field = field->deepCopy(copies);
}

//!  A simple template function that calls the method
template<class T>
T* deepCopy(const T* source, CopiesMap& copies)
{ return source->deepCopy(copies); }

//!  This function simply calls the previous one with an initially empty map
template<class T>
inline T* deepCopy(const T* source)
{ 
    CopiesMap copies; //!<  create empty map
    return deepCopy(source, copies);
}


}

#endif //ndef CopiesMap_INC


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
