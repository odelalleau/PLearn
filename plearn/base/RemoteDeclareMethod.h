// -*- C++ -*-

// RemoteDeclareMethod.h
//
// Copyright (C) 2006 Nicolas Chapados
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

// Authors: Nicolas Chapados

/*! \file RemoteDeclareMethod.h */


#ifndef RemoteDeclareMethod_INC
#define RemoteDeclareMethod_INC

// From C++ stdlib
#include <string>

// From PLearn
#include "RemoteMethodMap.h"
#include "RemoteMethodDoc.h"
#include "RemoteTrampoline.h"

namespace PLearn {

// What follows is a utility cast to bring const methods into non-const
#define METHOD_UNCONST(M) (typename Trampoline::MethodType)(M)

//! This function returns the map in which all remote functions and static
//! methods are to be registered (with declareFunction).
RemoteMethodMap& getGlobalFunctionMap();

// What follows is a bunch of 'declareFunction' overloads, each instantiating the
// appropriate FRemoteTrampoline

//#####  0 Argument  ##########################################################

template <class R>
inline void declareFunction(const string& funcname,
                            R (*func)(),
                            const RemoteMethodDoc& doc)
{
    RemoteMethodMap& rmm = getGlobalFunctionMap();
    typedef FRemoteTrampoline_0<R> Trampoline;
    rmm.insert(funcname, Trampoline::expected_nargs,
               new Trampoline(funcname, doc, func));
}


//#####  1 Argument  ##########################################################

template <class R, class A1>
inline void declareFunction(const string& funcname,
                            R (*func)(A1),
                            const RemoteMethodDoc& doc)
{
    RemoteMethodMap& rmm = getGlobalFunctionMap();
    typedef FRemoteTrampoline_1<R,A1> Trampoline;
    rmm.insert(funcname, Trampoline::expected_nargs,
               new Trampoline(funcname, doc, func));
}


//#####  2 Arguments  #########################################################

template <class R, class A1, class A2>
inline void declareFunction(const string& funcname,
                            R (*func)(A1,A2),
                            const RemoteMethodDoc& doc)
{
    RemoteMethodMap& rmm = getGlobalFunctionMap();
    typedef FRemoteTrampoline_2<R,A1,A2> Trampoline;
    rmm.insert(funcname, Trampoline::expected_nargs,
               new Trampoline(funcname, doc, func));
}


//#####  3 Arguments  #########################################################

template <class R, class A1, class A2, class A3>
inline void declareFunction(const string& funcname,
                            R (*func)(A1,A2,A3),
                            const RemoteMethodDoc& doc)
{
    RemoteMethodMap& rmm = getGlobalFunctionMap();
    typedef FRemoteTrampoline_3<R,A1,A2,A3> Trampoline;
    rmm.insert(funcname, Trampoline::expected_nargs,
               new Trampoline(funcname, doc, func));
}


//#####  4 Arguments  #########################################################

template <class R, class A1, class A2, class A3, class A4>
inline void declareFunction(const string& funcname,
                            R (*func)(A1,A2,A3,A4),
                            const RemoteMethodDoc& doc)
{
    RemoteMethodMap& rmm = getGlobalFunctionMap();
    typedef FRemoteTrampoline_4<R,A1,A2,A3,A4> Trampoline;
    rmm.insert(funcname, Trampoline::expected_nargs,
               new Trampoline(funcname, doc, func));
}

//#####  5 Arguments  #########################################################

template <class R, class A1, class A2, class A3, class A4, class A5>
inline void declareFunction(const string& funcname,
                            R (*func)(A1,A2,A3,A4,A5),
                            const RemoteMethodDoc& doc)
{
    RemoteMethodMap& rmm = getGlobalFunctionMap();
    typedef FRemoteTrampoline_5<R,A1,A2,A3,A4,A5> Trampoline;
    rmm.insert(funcname, Trampoline::expected_nargs,
               new Trampoline(funcname, doc, func));
}

//#####  6 Arguments  #########################################################

template <class R, class A1, class A2, class A3, class A4, class A5, class A6>
inline void declareFunction(const string& funcname,
                            R (*func)(A1,A2,A3,A4,A5,A6),
                            const RemoteMethodDoc& doc)
{
    RemoteMethodMap& rmm = getGlobalFunctionMap();
    typedef FRemoteTrampoline_6<R,A1,A2,A3,A4,A5,A6> Trampoline;
    rmm.insert(funcname, Trampoline::expected_nargs,
               new Trampoline(funcname, doc, func));
}


// What follows is a bunch of 'declareMethod' overloads, each instantiating the
// appropriate RemoteTrampoline

//#####  0 Argument  ##########################################################

// Non-const method
template <class T, class R>
inline void declareMethod(RemoteMethodMap& rmm,
                          const string& methodname,
                          R (T::*method)(),
                          const RemoteMethodDoc& doc)
{
    typedef RemoteTrampoline_0<T,R> Trampoline;
    rmm.insert(methodname, Trampoline::expected_nargs,
               new Trampoline(methodname, doc, METHOD_UNCONST(method)));
}

// Const method
template <class T, class R>
inline void declareMethod(RemoteMethodMap& rmm,
                          const string& methodname,
                          R (T::*method)() const,
                          const RemoteMethodDoc& doc)
{
    typedef RemoteTrampoline_0<T,R> Trampoline;
    rmm.insert(methodname, Trampoline::expected_nargs,
               new Trampoline(methodname, doc, METHOD_UNCONST(method)));
}


//#####  1 Argument  ##########################################################

// Non-const method
template <class T, class R, class A1>
inline void declareMethod(RemoteMethodMap& rmm,
                          const string& methodname,
                          R (T::*method)(A1),
                          const RemoteMethodDoc& doc)
{
    typedef RemoteTrampoline_1<T,R,A1> Trampoline;
    rmm.insert(methodname, Trampoline::expected_nargs,
               new Trampoline(methodname, doc, METHOD_UNCONST(method)));
}

// Const method
template <class T, class R, class A1>
inline void declareMethod(RemoteMethodMap& rmm,
                          const string& methodname,
                          R (T::*method)(A1) const,
                          const RemoteMethodDoc& doc)
{
    typedef RemoteTrampoline_1<T,R,A1> Trampoline;
    rmm.insert(methodname, Trampoline::expected_nargs,
               new Trampoline(methodname, doc, METHOD_UNCONST(method)));
}


//#####  2 Arguments  #########################################################

// Non-const method
template <class T, class R, class A1, class A2>
inline void declareMethod(RemoteMethodMap& rmm,
                          const string& methodname,
                          R (T::*method)(A1,A2),
                          const RemoteMethodDoc& doc)
{
    typedef RemoteTrampoline_2<T,R,A1,A2> Trampoline;
    rmm.insert(methodname, Trampoline::expected_nargs,
               new Trampoline(methodname, doc, METHOD_UNCONST(method)));
}

// Const method
template <class T, class R, class A1, class A2>
inline void declareMethod(RemoteMethodMap& rmm,
                          const string& methodname,
                          R (T::*method)(A1,A2) const,
                          const RemoteMethodDoc& doc)
{
    typedef RemoteTrampoline_2<T,R,A1,A2> Trampoline;
    rmm.insert(methodname, Trampoline::expected_nargs,
               new Trampoline(methodname, doc, METHOD_UNCONST(method)));
}


//#####  3 Arguments  #########################################################

// Non-const method
template <class T, class R, class A1, class A2, class A3>
inline void declareMethod(RemoteMethodMap& rmm,
                          const string& methodname,
                          R (T::*method)(A1,A2,A3),
                          const RemoteMethodDoc& doc)
{
    typedef RemoteTrampoline_3<T,R,A1,A2,A3> Trampoline;
    rmm.insert(methodname, Trampoline::expected_nargs,
               new Trampoline(methodname, doc, METHOD_UNCONST(method)));
}

// Const method
template <class T, class R, class A1, class A2, class A3>
inline void declareMethod(RemoteMethodMap& rmm,
                          const string& methodname,
                          R (T::*method)(A1,A2,A3) const,
                          const RemoteMethodDoc& doc)
{
    typedef RemoteTrampoline_3<T,R,A1,A2,A3> Trampoline;
    rmm.insert(methodname, Trampoline::expected_nargs,
               new Trampoline(methodname, doc, METHOD_UNCONST(method)));
}


//#####  4 Arguments  #########################################################

// Non-const method
template <class T, class R, class A1, class A2, class A3, class A4>
inline void declareMethod(RemoteMethodMap& rmm,
                          const string& methodname,
                          R (T::*method)(A1,A2,A3,A4),
                          const RemoteMethodDoc& doc)
{
    typedef RemoteTrampoline_4<T,R,A1,A2,A3,A4> Trampoline;
    rmm.insert(methodname, Trampoline::expected_nargs,
               new Trampoline(methodname, doc, METHOD_UNCONST(method)));
}

// Const method
template <class T, class R, class A1, class A2, class A3, class A4>
inline void declareMethod(RemoteMethodMap& rmm,
                          const string& methodname,
                          R (T::*method)(A1,A2,A3,A4) const,
                          const RemoteMethodDoc& doc)
{
    typedef RemoteTrampoline_4<T,R,A1,A2,A3,A4> Trampoline;
    rmm.insert(methodname, Trampoline::expected_nargs,
               new Trampoline(methodname, doc, METHOD_UNCONST(method)));
}


//#####  5 Arguments  #########################################################

// Non-const method
template <class T, class R, class A1, class A2, class A3, class A4, class A5>
inline void declareMethod(RemoteMethodMap& rmm,
                          const string& methodname,
                          R (T::*method)(A1,A2,A3,A4,A5),
                          const RemoteMethodDoc& doc)
{
    typedef RemoteTrampoline_5<T,R,A1,A2,A3,A4,A5> Trampoline;
    rmm.insert(methodname, Trampoline::expected_nargs,
               new Trampoline(methodname, doc, METHOD_UNCONST(method)));
}

// Const method
template <class T, class R, class A1, class A2, class A3, class A4, class A5>
inline void declareMethod(RemoteMethodMap& rmm,
                          const string& methodname,
                          R (T::*method)(A1,A2,A3,A4,A5) const,
                          const RemoteMethodDoc& doc)
{
    typedef RemoteTrampoline_5<T,R,A1,A2,A3,A4,A5> Trampoline;
    rmm.insert(methodname, Trampoline::expected_nargs,
               new Trampoline(methodname, doc, METHOD_UNCONST(method)));
}


//#####  6 Arguments  #########################################################

// Non-const method
template <class T, class R, class A1, class A2, class A3, class A4, class A5, class A6>
inline void declareMethod(RemoteMethodMap& rmm,
                          const string& methodname,
                          R (T::*method)(A1,A2,A3,A4,A5,A6),
                          const RemoteMethodDoc& doc)
{
    typedef RemoteTrampoline_6<T,R,A1,A2,A3,A4,A5,A6> Trampoline;
    rmm.insert(methodname, Trampoline::expected_nargs,
               new Trampoline(methodname, doc, METHOD_UNCONST(method)));
}

// Const method
template <class T, class R, class A1, class A2, class A3, class A4, class A5, class A6>
inline void declareMethod(RemoteMethodMap& rmm,
                          const string& methodname,
                          R (T::*method)(A1,A2,A3,A4,A5,A6) const,
                          const RemoteMethodDoc& doc)
{
    typedef RemoteTrampoline_6<T,R,A1,A2,A3,A4,A5,A6> Trampoline;
    rmm.insert(methodname, Trampoline::expected_nargs,
               new Trampoline(methodname, doc, METHOD_UNCONST(method)));
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
