// -*- C++ -*-

// TypeTraits.h
// Copyright (C) 2002 Pascal Vincent
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


/*! \file PLearnLibrary/PLearnCore/TypeTraits.h */

#ifndef TypeTraits_INC
#define TypeTraits_INC

#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <nspr/prlong.h>
#include <plearn/base/pl_stdint.h>

namespace PLearn {
using std::string;

/**
 *  @class TypeTraits
 *  @brief TypeTraits provides a type-information mechanism for C++ types
 *
 *  TypeTraits<some_type> delivers the following information on the type:
 *
 *  @li Its \c name(), returned as a string
 *
 *  @li The "typecode", which is the header-byte used to indicate type of an
 *      element to follow in plearn_binary serialization. \c little_endian_typecode()
 *      and \c big_endian_typecode() respectively return the code to designate
 *      little-endian or big-endian representation.  Only the very basic C++ types
 *      have specific typecodes. For all other more complex types, these functions
 *      should always return 0xFF.
 */
template<class T>
class TypeTraits
{
public:
    //! String representation of type type
    static inline string name()
    { return "UNKNOWN_TYPE_NAME"; }

    //! Type-code for representing the little-endian serialization of an object
    //! in PLearn's binary serialization format
    static inline unsigned char little_endian_typecode()
    { return 0xFF; }

    //! Type-code for representing the big-endian serialization of an object
    //! in PLearn's binary serialization format
    static inline unsigned char big_endian_typecode()
    { return 0xFF; }

};


//#####  Specializations  #####################################################

template<class T>
class TypeTraits<T*>
{
public:
    static inline string name() 
    { return TypeTraits<T>::name()+"*"; }

    static inline unsigned char little_endian_typecode()
    { return 0xFF; }

    static inline unsigned char big_endian_typecode()
    { return 0xFF; }
};

template<class T>
class TypeTraits<T const>
{
public:
    static inline string name() 
    { return TypeTraits<T>::name()+" const"; }

    static inline unsigned char little_endian_typecode()
    { return 0xFF; }

    static inline unsigned char big_endian_typecode()
    { return 0xFF; }
};

#define DECLARE_TYPE_TRAITS_FOR_BASETYPE(T,LITTLE_ENDIAN_TYPECODE,BIG_ENDIAN_TYPECODE)  \
template<>                                                                              \
class TypeTraits<T>                                                                     \
{                                                                                       \
public:                                                                                 \
  static inline string name()                                                           \
  { return #T; }                                                                        \
                                                                                        \
  static inline unsigned char little_endian_typecode()                                  \
  { return LITTLE_ENDIAN_TYPECODE; }                                                    \
                                                                                        \
  static inline unsigned char big_endian_typecode()                                     \
  { return BIG_ENDIAN_TYPECODE; }                                                       \
}

#define DECLARE_TYPE_TRAITS_FOR_INTTYPE(T)                  \
template<>                                                  \
class TypeTraits<T>                                         \
{                                                           \
public:                                                     \
    static inline string name()                             \
    { return #T; }                                          \
                                                            \
    static inline unsigned char little_endian_typecode()    \
    {                                                       \
        switch(sizeof(T))                                   \
        {                                                   \
        case sizeof(int8_t):                                \
            return 0x01;                                    \
        case sizeof(int16_t):                               \
            return 0x03;                                    \
        case sizeof(int32_t):                               \
            return 0x07;                                    \
        case sizeof(int64_t):                               \
            return 0x16;                                    \
        default:                                            \
            return 0xFF;                                    \
        }                                                   \
    }                                                       \
                                                            \
    static inline unsigned char big_endian_typecode()       \
    {                                                       \
        switch(sizeof(T))                                   \
        {                                                   \
        case sizeof(int8_t):                                \
            return 0x01;                                    \
        case sizeof(int16_t):                               \
            return 0x04;                                    \
        case sizeof(int32_t):                               \
            return 0x08;                                    \
        case sizeof(int64_t):                               \
            return 0x17;                                    \
        default:                                            \
            return 0xFF;                                    \
        }                                                   \
    }                                                       \
}

#define DECLARE_TYPE_TRAITS_FOR_UINTTYPE(T)                 \
template<>                                                  \
class TypeTraits<T>                                         \
{                                                           \
public:                                                     \
    static inline string name()                             \
    { return #T; }                                          \
                                                            \
    static inline unsigned char little_endian_typecode()    \
    {                                                       \
        switch(sizeof(T))                                   \
        {                                                   \
        case sizeof(uint8_t):                               \
            return 0x02;                                    \
        case sizeof(uint16_t):                              \
            return 0x05;                                    \
        case sizeof(uint32_t):                              \
            return 0x0B;                                    \
        case sizeof(uint64_t):                              \
            return 0x18;                                    \
        default:                                            \
            return 0xFF;                                    \
        }                                                   \
    }                                                       \
                                                            \
    static inline unsigned char big_endian_typecode()       \
    {                                                       \
        switch(sizeof(T))                                   \
        {                                                   \
        case sizeof(uint8_t):                               \
            return 0x02;                                    \
        case sizeof(uint16_t):                              \
            return 0x06;                                    \
        case sizeof(uint32_t):                              \
            return 0x0C;                                    \
        case sizeof(uint64_t):                              \
            return 0x19;                                    \
        default:                                            \
            return 0xFF;                                    \
        }                                                   \
    }                                                       \
}

#define DECLARE_TYPE_TRAITS(T)                          \
template<>                                              \
class TypeTraits<T>                                     \
{                                                       \
public:                                                 \
  static inline string name()                           \
  { return #T; }                                        \
                                                        \
  static inline unsigned char little_endian_typecode()  \
  { return 0xFF; }                                      \
                                                        \
  static inline unsigned char big_endian_typecode()     \
  { return 0xFF; }                                      \
}

// DECLARE_TYPE_TRAITS_FOR_BASETYPE(bool, ??, ??);
DECLARE_TYPE_TRAITS_FOR_BASETYPE(void,               0xFF, 0xFF);
DECLARE_TYPE_TRAITS_FOR_BASETYPE(float,              0x0E, 0x0F);
DECLARE_TYPE_TRAITS_FOR_BASETYPE(double,             0x10, 0x11);
DECLARE_TYPE_TRAITS_FOR_BASETYPE(bool,               0x30, 0x30);

DECLARE_TYPE_TRAITS_FOR_INTTYPE(char);
DECLARE_TYPE_TRAITS_FOR_INTTYPE(signed char);
DECLARE_TYPE_TRAITS_FOR_INTTYPE(short);
DECLARE_TYPE_TRAITS_FOR_INTTYPE(int);
DECLARE_TYPE_TRAITS_FOR_INTTYPE(long);
DECLARE_TYPE_TRAITS_FOR_INTTYPE(long long);

DECLARE_TYPE_TRAITS_FOR_UINTTYPE(unsigned char);
DECLARE_TYPE_TRAITS_FOR_UINTTYPE(unsigned short);
DECLARE_TYPE_TRAITS_FOR_UINTTYPE(unsigned int);
DECLARE_TYPE_TRAITS_FOR_UINTTYPE(unsigned long);
DECLARE_TYPE_TRAITS_FOR_UINTTYPE(unsigned long long);

DECLARE_TYPE_TRAITS(string);

template<class T>
class TypeTraits< std::vector<T> >
{
public:
    static inline string name()
    { return string("vector< ") + TypeTraits<T>::name()+" >"; }

    static inline unsigned char little_endian_typecode()
    { return 0xFF; }

    static inline unsigned char big_endian_typecode()
    { return 0xFF; }
};

template<class T>
class TypeTraits< std::list<T> >
{
public:
    static inline string name()
    { return string("list< ") + TypeTraits<T>::name()+" >"; }

    static inline unsigned char little_endian_typecode()
    { return 0xFF; }

    static inline unsigned char big_endian_typecode()
    { return 0xFF; }
};

template<class T, class U>
class TypeTraits< std::pair<T,U> >
{
public:
    static inline string name()
    { return string("pair< ") + TypeTraits<T>::name()+", " + TypeTraits<U>::name()+" >"; }

    static inline unsigned char little_endian_typecode()
    { return 0xFF; }

    static inline unsigned char big_endian_typecode()
    { return 0xFF; }
};

template<class T, class U>
class TypeTraits< std::map<T,U> >
{
public:
    static inline string name()
    { return string("map< ") + TypeTraits<T>::name()+", " + TypeTraits<U>::name()+" >"; }

    static inline unsigned char little_endian_typecode()
    { return 0xFF; }

    static inline unsigned char big_endian_typecode()
    { return 0xFF; }
};

template<class T>
class TypeTraits< std::set<T> >
{
public:
    static inline string name()
    { return string("set< ") + TypeTraits<T>::name()+" >"; }

    static inline unsigned char little_endian_typecode()
    { return 0xFF; }

    static inline unsigned char big_endian_typecode()
    { return 0xFF; }
};

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
