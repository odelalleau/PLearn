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

/*! \file PLearnLibrary/PLearnCore/TypeFactory.h */

#ifndef TYPEFACTORY_H
#define TYPEFACTORY_H

#include <string>
#include <map>
#include <vector>
#include "OptionBase.h"

namespace PLearn <%
using namespace std;

// Predeclarations
class Object;

//!  Typedef for the "new instance" function type, which returns a
//!  default-initialized Object
typedef Object* (*NEW_OBJECT)();
typedef OptionList& (*GETOPTIONLIST_METHOD)();
typedef bool (*ISA_METHOD)(Object* o);

class TypeMapEntry
{
public:
  string type_name;
  string parent_class; // name of parent class
  NEW_OBJECT constructor;
  GETOPTIONLIST_METHOD getoptionlist_method;
  ISA_METHOD isa_method;
  string one_line_descr;
  string multi_line_help;
  
  TypeMapEntry(const string& the_type_name, 
               const string& the_parent_class="", 
               NEW_OBJECT the_constructor=0, 
               GETOPTIONLIST_METHOD the_getoptionlist_method=0,
               ISA_METHOD the_isa_method=0,
               const string& the_one_line_descr = "",
               const string& the_multi_line_help = "")
    :type_name(the_type_name),
     parent_class(the_parent_class),
     constructor(the_constructor), 
     getoptionlist_method(the_getoptionlist_method),
     isa_method(the_isa_method),
     one_line_descr(the_one_line_descr),
     multi_line_help(the_multi_line_help)
  {}
};

typedef map<string,TypeMapEntry> TypeMap;


//###########################  CLASS  TYPEFACTORY  ############################
//!  

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
                            ISA_METHOD isa_method,
                            const string& one_line_descr,
                            const string& multi_line_help);  

  //!  Register a type
  void registerType(const TypeMapEntry& entry);

  //!  Unregister a type
  void unregisterType(string type_name);

  //!  Verify if the type is registered
  bool isRegistered(string type_name) const;

  //!  Construct a new default-constructed object given its type name
  //!  Calls PLERROR (throws an exception) if type_name is not registered
  Object* newObject(string type_name) const;

  //! Tells if the given object is a virtual base class (with pure virtual methods)
  //! (This simply checks if it was declared with a constructor or not)
  bool isAbstract(string type_name) const;

  const TypeMap& getTypeMap() const
  { return type_map_; }

  //!  Return the singleton (static) instance of the type factory
  static TypeFactory& instance();
  
};


//! Will display the help message for an object of the given classname
void displayObjectHelp(ostream& out, const string& classname);


%> // end of namespace PLearn

#endif //!<  TYPEFACTORY_H
