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


#include "Object.h"
#include "TypeFactory.h"

namespace PLearn <%
using namespace std;



//#####  TypeRegistrar  #######################################################

TypeRegistrar::TypeRegistrar(const string& type_name, const TypeMapEntry& entry)
{
  TypeFactory::instance().registerType(type_name, entry);
}


//#####  TypeFactory  #########################################################

void TypeFactory::registerType(const string& type_name, const TypeMapEntry& entry)
{
  type_map_[type_name] = entry;
  //cout << "register type " << type_name << endl;
}

void TypeFactory::unregisterType(string type_name)
{
  type_map_.erase(type_name);                // ok even if does not exist
}

bool TypeFactory::isRegistered(string type_name) const
{
  return type_map_.find(type_name)!=type_map_.end();
}

Object* TypeFactory::newObject(string type_name) const
{
  TypeMap::const_iterator it = type_map_.find(type_name);
  if (it == type_map_.end())
    PLERROR("In TypeFactory::newObject(\"%s\"): %s not registered in type map.", type_name.c_str(), type_name.c_str());
  return (*it->second.constructor)();
}

string TypeFactory::help(string type_name) const
{
  TypeMap::const_iterator it = type_map_.find(type_name);
  if (it == type_map_.end())
    PLERROR("In TypeFactory::help(\"%s\"): %s not registered in type map.", type_name.c_str(), type_name.c_str());
  return (*it->second.help_method)();
}

bool TypeFactory::isAbstract(string type_name) const
{
  TypeMap::const_iterator it = type_map_.find(type_name);
  if (it == type_map_.end())
    PLERROR("In TypeFactory::help(\"%s\"): %s not registered in type map.", type_name.c_str(), type_name.c_str());
  return it->second.constructor == 0;
}

TypeFactory& TypeFactory::instance()
{
  static TypeFactory instance_;
  return instance_;
}


void displayObjectHelp(ostream& out, const string& classname)
{
  const TypeMap& type_map = TypeFactory::instance().getTypeMap();
  TypeMap::const_iterator it = type_map.find(classname);
  TypeMap::const_iterator itend = type_map.end();

  if(it==itend)
    PLERROR("Learner type %s unknown.\n"
            "Did you #include it, does it call the IMPLEMENT_NAME_AND_DEEPCOPY macro?\n"
            "and has it indeed been linked with your program?", classname.c_str());

  const TypeMapEntry& entry = it->second;
  Object* obj = 0;

  out << "****************************************************************** \n"
      << "** " << classname << "\n"
      << "****************************************************************** \n" << endl;

  // Display basic help
  out << (*entry.help_method)() << endl << endl;

  if(entry.constructor) // it's an instantiable class
    obj = (*entry.constructor)();
  else
    out << "Note: " << classname << " is a base-class with pure virtual methods that cannot be instantiated directly.\n" 
        << "(default values for build options can only be displayed for instantiable classes, \n"
        << " so you'll only see question marks here.)\n" << endl;
      
  out << "****************************************************************** \n"
      << "**                         Build Options                        ** \n"
      << "** (including those inherited from parent and ancestor classes) ** \n"
      << "****************************************************************** \n" << endl;

  out << classname + "( \n";
  OptionList& options = (*entry.getoptionlist_method)();    

  for( OptionList::iterator it = options.begin(); it!=options.end(); ++it )
    {
      OptionBase::flag_t flags = (*it)->flags();
      if(flags & OptionBase::buildoption)
        {
          string descr = (*it)->description();
          string optname = (*it)->optionname();
          string opttype = (*it)->optiontype();
          string defaultval = "?";
          if(obj) // it's an instantiable class
            {
              defaultval = (*it)->defaultval(); 
              if(defaultval=="")
                defaultval = (*it)->writeIntoString(obj);
            }
          // string holderclass = (*it)->optionHolderClassName(this);
          out << addprefix("# ", opttype + ": " + descr);
          out << optname + " = " + defaultval + " ;\n\n";
        }
    }
  out << ");\n\n";

  if(obj)
    delete obj;

  out << "****************************************************************** \n"
      << "** Subclasses of " << classname << " \n"
      << "** (only those that can be instantiated) \n"
      << "****************************************************************** \n" << endl;
  for(it = type_map.begin(); it!=itend; ++it)
    {
      // cerr << "Attempting to instantiate: " << it->first << endl;
      const TypeMapEntry& e = it->second;
      if(e.constructor && it->first!=classname)
        {
          Object* o = (*e.constructor)();
          if( (*entry.isa_method)(o) )
            out << it->first << " ";
          if(o)
            delete o;
        }
    }

  out << "\n\n------------------------------------------------------------------ \n" << endl;

}



%> // end of namespace PLearn
