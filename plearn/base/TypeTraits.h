// -*- C++ -*-4 1999/10/29 20:41:34 dugas

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
   * $Id: TypeTraits.h,v 1.1 2002/09/05 05:43:39 plearner Exp $
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

namespace PLearn <%
using namespace std;


template<class T>
class TypeTraits
{
public:
  static string name()
  { return "UNKNOWN_TYPE_NAME"; }
};

template<class T>
class TypeTraits<T*>
{
public:
  static string name() 
  { return TypeTraits<T>::name()+"*"; }
};

#define DECLARE_TYPE_TRAITS(T) \
template<>                     \
class TypeTraits<T>            \
{                              \
public:                        \
  static inline string name()  \
  { return #T; }               \
}

DECLARE_TYPE_TRAITS(bool);
DECLARE_TYPE_TRAITS(char);
DECLARE_TYPE_TRAITS(unsigned char);
DECLARE_TYPE_TRAITS(signed char);
DECLARE_TYPE_TRAITS(int);
DECLARE_TYPE_TRAITS(unsigned int);
DECLARE_TYPE_TRAITS(long);
DECLARE_TYPE_TRAITS(unsigned long);
DECLARE_TYPE_TRAITS(float);
DECLARE_TYPE_TRAITS(double);
DECLARE_TYPE_TRAITS(string);

template<class T>
class TypeTraits< vector<T> >
{
public:
  static inline string name()
  { return string("vector< ") + TypeTraits<T>::name()+" >"; }
};

template<class T>
class TypeTraits< list<T> >
{
public:
  static inline string name()
  { return string("list< ") + TypeTraits<T>::name()+" >"; }
};

template<class T, class U>
class TypeTraits< pair<T,U> >
{
public:
  static inline string name()
  { return string("pair< ") + TypeTraits<T>::name()+", " + TypeTraits<U>::name()+" >"; }
};

template<class T, class U>
class TypeTraits< map<T,U> >
{
public:
  static inline string name()
  { return string("map< ") + TypeTraits<T>::name()+", " + TypeTraits<U>::name()+" >"; }
};

%> // end of namespace PLearn


#endif
