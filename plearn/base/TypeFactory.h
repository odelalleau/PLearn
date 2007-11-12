// -*- C++ -*-

// TypeFactory.cc
// Copyright (c) 2001 by Nicolas Chapados
// Copyright (c) 2003 Pascal Vincent

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

/*! \file TypeFactory.h */

#ifndef TYPEFACTORY_H
#define TYPEFACTORY_H

#include <string>
#include <map>
#include "OptionBase.h"
#include "RemoteMethodMap.h"
#include <plearn/io/PPath.h>

namespace PLearn {
using std::string;

// Predeclarations
class Object;
class RemoteMethodMap;

//!  Typedef for the "new instance" function type, which returns a
//!  default-initialized Object
typedef Object* (*NEW_OBJECT)();
typedef OptionList& (*GETOPTIONLIST_METHOD)();
typedef RemoteMethodMap& (*GET_REMOTE_METHODS)();
typedef bool (*ISA_METHOD)(const Object* o);


//###########################  CLASS  TYPEMAPENTRY  ###########################

/**
 *  Description of a single type within the TypeMap
 */
class TypeMapEntry
{
public:
    //! Name of the "type" (derived from \c PLearn::Object)
    string type_name;

    //! Name of the base class
    string parent_class;

    //! Function pointer which, when called, instantiates a new
    //! object with the default constructor.  This can be null,
    //! in which case the class is considered abstract.
    NEW_OBJECT constructor;

    //! Function pointer which, when called, returns a pointer to
    //! the list of options supported by the class
    GETOPTIONLIST_METHOD getoptionlist_method;

    //! Function pointer which, when called, returns a list of
    //! remote methods supported by the class
    GET_REMOTE_METHODS get_remote_methods;
    
    //! Function pointer which, when called with a pointer to an
    //! object, tests whether the object is dynamic-castable to the
    //! type (class).
    ISA_METHOD isa_method;

    //! Short one-line documentation string
    string one_line_descr;

    //! Detailed documentation for users
    string multi_line_help;

    //! Filename where this type (class) is declared (i.e. to include)
    PPath declaring_file;
  
    TypeMapEntry(const string& the_type_name, 
                 const string& the_parent_class="", 
                 NEW_OBJECT the_constructor=0, 
                 GETOPTIONLIST_METHOD the_getoptionlist_method = 0,
                 GET_REMOTE_METHODS the_get_remote_methods = 0,
                 ISA_METHOD the_isa_method=0,
                 const string& the_one_line_descr = "",
                 const string& the_multi_line_help = "",
                 const PPath& the_declaring_file= PPath(""))
        : type_name(the_type_name),
          parent_class(the_parent_class),
          constructor(the_constructor), 
          getoptionlist_method(the_getoptionlist_method),
          get_remote_methods(the_get_remote_methods),
          isa_method(the_isa_method),
          one_line_descr(the_one_line_descr),
          multi_line_help(the_multi_line_help),
          declaring_file(the_declaring_file)
    { }
};

typedef std::map<string,TypeMapEntry> TypeMap;


//###########################  CLASS  TYPEFACTORY  ############################

/**
 *  Create new objects given their type name (as a string).
 *
 *  The TypeFactory operates as a Singleton class and is shared across the
 *  PLearn system.  It can be used to register types (classes) derived from
 *  \c PLearn::Object.  The typical pattern for instantiating a new object
 *  given its class name is as follows:
 *
 *  @code
 *  string typename = "SomeTypeName";
 *  Object* new_object = TypeFactory::instance().newObject(typename);
 *  @endcode
 */
class TypeFactory
{
protected:
    TypeMap type_map_;

public:
    // Default constructor, destructor, etc.

    //!  Register a type
    static void register_type(const string& type_name, 
                              const string& parent_class, 
                              NEW_OBJECT constructor, 
                              GETOPTIONLIST_METHOD getoptionlist_method,
                              GET_REMOTE_METHODS get_remote_methods,
                              ISA_METHOD isa_method,
                              const string& one_line_descr,
                              const string& multi_line_help,
                              const PPath& declaring_file);

    //! Register a type
    void registerType(const TypeMapEntry& entry);

    //! Unregister a type
    void unregisterType(const string& type_name);

    //! Verify if the type is registered
    bool isRegistered(const string& type_name) const;

    //! Construct a new default-constructed object given its type name
    //! Calls PLERROR (throws an exception) if type_name is not registered
    Object* newObject(const string& type_name) const;

    //! Tells if the given object is a virtual base class (with pure virtual methods)
    //! (This simply checks if it was declared with a constructor or not)
    bool isAbstract(const string& type_name) const;

    //! Return an entry in typemap corresponding to given type_name.  Raise
    //! PLERROR if type_name is not registered
    const TypeMapEntry& getTypeMapEntry(const string& type_name) const;
    
    //! Return a reference to the typemap
    const TypeMap& getTypeMap() const
    { return type_map_; }

    //! Return the singleton (static) instance of the type factory
    static TypeFactory& instance();
};


//! Will display the help message for an object of the given classname
//void displayObjectHelp(ostream& out, const string& classname); //!< DEPRECATED: use HelpSystem


} // end of namespace PLearn

#endif //!<  TYPEFACTORY_H


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
