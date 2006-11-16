// -*- C++ -*-

// RemoteMethodDoc.h
//
// Copyright (C) 2006 Nicolas Chapados, Pascal Vincent
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

/*! \file RemoteMethodDoc.h */


#ifndef RemoteMethodDoc_INC
#define RemoteMethodDoc_INC

// From C++ stdlib
#include <string>
#include <list>
#include <utility>                           // For pair
#include <vector>
#include "plearn/base/tuple.h"

namespace PLearn {

using std::string;
using std::pair;
using std::list;

/**
 *  Documentation for remote method body
 */
struct BodyDoc
{
    BodyDoc(const char* s)
        : m_doc(s)
    { }

    string m_doc;
};

/**
 *  Documentation for a single remote method argument
 */
struct ArgDoc
{
    ArgDoc(const string& argument_name, const string& doc)
        : m_argument_name(argument_name),
          m_doc(doc)
    { }

    string m_argument_name;
    string m_doc;
};

/**
 *  Documentation for a remote method return value
 */
struct RetDoc
{
    RetDoc(const char* s)
        : m_doc(s)
    { }

    string m_doc;
};

/**
 *  Documentation for a method argument type (just contains the type as a
 *  string)
 */
struct ArgTypeDoc
{
    ArgTypeDoc(const string& typestr)
        : m_typestr(typestr)
    { }

    string m_typestr;
};

/**
 *  Documentation for a method return type (just contains the type as a string)
 */
struct RetTypeDoc
{
    RetTypeDoc(const string& typestr)
        : m_typestr(typestr)
    { }

    string m_typestr;
};


/**
 *  Documentation holder for a remote method.
 *
 *  Method documentation consists of the following:
 *
 *  - Main body
 *  - Argument list (in the form of pairs argument-name:documentation)
 *  - Return value
 *
 *  (Added later by trampolines: type information for the return value and
 *  arguments)
 */
class RemoteMethodDoc
{
public:
    //! Constructors
    RemoteMethodDoc()
    { }
    
    RemoteMethodDoc(const BodyDoc& doc)
        : m_body_doc(doc.m_doc)
    { }
    
    RemoteMethodDoc(const RetDoc& doc)
        : m_return_doc(doc.m_doc)
    { }
    
    RemoteMethodDoc(const ArgDoc& doc)
        : m_args_doc(1, doc)
    { }

    void setName(const string& methodname) const
    { m_name = methodname; }

    //! Access documentation components

    const string& name() const
    {
        return m_name;
    }

    //! Will perform consistency checks 
    //! (such as verifying if margs_doc and m_args_type have the same size)
    //! and launch an exception if inconsistencies are detected.
    void checkConsistency() const;

    int nArgs() const
    {        
        checkConsistency();
        return m_args_type.size();
    }

    const string& bodyDoc() const
    {
        return m_body_doc;
    }
    
    const string& returnDoc() const
    {
        return m_return_doc;
    }
    
    const string& returnType() const
    {
        return m_return_type;
    }
    
    const list<ArgDoc>& argListDoc() const
    {
        return m_args_doc;
    }

    const list<string>& argListType() const
    {
        return m_args_type;
    }

    //! Set a body
    const RemoteMethodDoc& operator,(const BodyDoc& doc) const
    {
        m_body_doc += doc.m_doc;
        return *this;
    }

    //! Set a return value
    const RemoteMethodDoc& operator,(const RetDoc& doc) const
    {
        m_return_doc += doc.m_doc;
        return *this;
    }

    //! Set a return type
    const RemoteMethodDoc& operator,(const RetTypeDoc& doc) const
    {
        m_return_type = doc.m_typestr;
        return *this;
    }
    
    //! Add a new argument
    const RemoteMethodDoc& operator,(const ArgDoc& doc) const
    {
        m_args_doc.push_back(doc);
        return *this;
    }

    //! Add type information to the corresponding argument (must add type
    //! information in the same order and number as argument documentation)
    const RemoteMethodDoc& operator,(const ArgTypeDoc& doc) const
    {
        m_args_type.push_back(doc.m_typestr);
        return *this;
    }
    
    //! Returns a string repretenting the "prototype" (signature) of the function in the doc.
    //! Argsep is used a sthe separator between arguments (typically ", " or ",\n")
    string getPrototypeString(string argsep=", ") const;

    //! return full help text for the function in the doc.
    string getFullHelpText() const;


protected:
    mutable string m_name;                   //!< Function name
    mutable string m_body_doc;               //!< Function body documentation
    mutable string m_return_doc;             //!< Return value documentation
    mutable string m_return_type;            //!< Type string for return value
    mutable list<ArgDoc> m_args_doc;         //!< Arguments documentation
    mutable list<string> m_args_type;        //!< Type string for each argument
};

//! Global operator, to start off the list of RemoteMethodDoc chaining
inline RemoteMethodDoc operator,(const BodyDoc& body, const ArgDoc& arg)
{
    RemoteMethodDoc doc(body);
    return doc.operator,(arg);
}

//! Global operator, to start off the list of RemoteMethodDoc chaining
inline RemoteMethodDoc operator,(const BodyDoc& body, const RetDoc& ret)
{
    RemoteMethodDoc doc(body);
    return doc.operator,(ret);
}

//! Global operator, to start off the list of RemoteMethodDoc chaining
inline RemoteMethodDoc operator,(const ArgDoc& arg, const BodyDoc& body)
{
    RemoteMethodDoc doc(body);
    return doc.operator,(arg);
}

//! Global operator, to start off the list of RemoteMethodDoc chaining
inline RemoteMethodDoc operator,(const ArgDoc& arg, const RetDoc& ret)
{
    RemoteMethodDoc doc(ret);
    return doc.operator,(arg);
}

//! Global operator, to start off the list of RemoteMethodDoc chaining
inline RemoteMethodDoc operator,(const RetDoc& ret, const BodyDoc& body)
{
    RemoteMethodDoc doc(body);
    return doc.operator,(ret);
}

//! Global operator, to start off the list of RemoteMethodDoc chaining
inline RemoteMethodDoc operator,(const RetDoc& ret, const ArgDoc& arg)
{
    RemoteMethodDoc doc(arg);
    return doc.operator,(ret);
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
