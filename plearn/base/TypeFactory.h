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
typedef string (*HELP_METHOD)();
typedef bool (*ISA_METHOD)(Object* o);

class TypeMapEntry
{
public:
  string parent_class; // name of parent class
  NEW_OBJECT constructor;
  GETOPTIONLIST_METHOD getoptionlist_method;
  HELP_METHOD help_method;
  ISA_METHOD isa_method;
  
  TypeMapEntry(const string& the_parent_class="", 
               NEW_OBJECT the_constructor=0, 
               GETOPTIONLIST_METHOD the_getoptionlist_method=0,
               HELP_METHOD the_help_method=0,
               ISA_METHOD the_isa_method=0)
    :parent_class(the_parent_class),
     constructor(the_constructor), 
     getoptionlist_method(the_getoptionlist_method),
     help_method(the_help_method),
     isa_method(the_isa_method)
  {}
};

typedef map<string,TypeMapEntry> TypeMap;

//##########################  CLASS  TYPEREGISTRAR  ###########################
/*!   
   This object, upon construction, registers a name and a construction
   function with the static type factory
*/

class TypeRegistrar
{
public:
  TypeRegistrar(const string& type_name, const TypeMapEntry& entry);
};


//###########################  CLASS  TYPEFACTORY  ############################
//!  

class TypeFactory
{
protected:
  TypeMap type_map_;

public:
  // Default constructor, destructor, etc.

  //!  Register a type
  void registerType(const string& type_name, const TypeMapEntry& entry);

  //!  Unregister a type
  void unregisterType(string type_name);

  //!  Verify if the type is registered
  bool isRegistered(string type_name) const;

  //!  Construct a new default-constructed object given its type name
  //!  Calls PLERROR (throws an exception) if type_name is not registered
  Object* newObject(string type_name) const;

  //! Returns help for registered object
  string help(string type_name) const;

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
