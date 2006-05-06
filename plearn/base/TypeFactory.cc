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
#include "stringutils.h"    //!< For addprefix.
#include "TypeFactory.h"

namespace PLearn {
using namespace std;

//#####  TypeFactory  #########################################################

void TypeFactory::register_type(const string& type_name, 
                                const string& parent_class, 
                                NEW_OBJECT constructor, 
                                GETOPTIONLIST_METHOD getoptionlist_method,
                                GET_REMOTE_METHODS get_remote_methods,
                                ISA_METHOD isa_method,
                                const string& one_line_descr,
                                const string& multi_line_help)
{
    TypeMapEntry entry(type_name, 
                       parent_class, 
                       constructor, 
                       getoptionlist_method,
                       get_remote_methods,
                       isa_method,
                       one_line_descr,
                       multi_line_help);

    instance().registerType(entry);
}

void TypeFactory::registerType(const TypeMapEntry& entry)
{
    type_map_.insert( pair<string,TypeMapEntry>(entry.type_name, entry) );
    //cout << "register type " << type_name << endl;
}

void TypeFactory::unregisterType(const string& type_name)
{
    type_map_.erase(type_name);                // ok even if does not exist
}

bool TypeFactory::isRegistered(const string& type_name) const
{
    return type_map_.find(type_name)!=type_map_.end();
}

Object* TypeFactory::newObject(const string& type_name) const
{
    const TypeMapEntry& tme = getTypeMapEntry(type_name);
    if (! tme.constructor)
        PLERROR("In TypeFactory::newObject(\"%s\"): \"%s\" does not have a factory constructor!",
                type_name.c_str(), type_name.c_str());
    return tme.constructor();
}

bool TypeFactory::isAbstract(const string& type_name) const
{
    const TypeMapEntry& tme = getTypeMapEntry(type_name);
    return tme.constructor == 0;
}

const TypeMapEntry& TypeFactory::getTypeMapEntry(const string& type_name) const
{
    TypeMap::const_iterator it = type_map_.find(type_name);
    if (it == type_map_.end())
        PLERROR("TypeFactory: requested type \"%s\" is not registered in type map.",
                type_name.c_str());
    return it->second;
}

TypeFactory& TypeFactory::instance()
{
    static TypeFactory instance_;
    return instance_;
}


//#####  displayObjectHelp  ###################################################

void displayObjectHelp(ostream& out, const string& classname)
{
    const TypeMap& type_map = TypeFactory::instance().getTypeMap();
    TypeMap::const_iterator it = type_map.find(classname);
    TypeMap::const_iterator itend = type_map.end();

    if(it==itend)
        PLERROR("Object type %s unknown.\n"
                "Did you #include it, does it call the IMPLEMENT_NAME_AND_DEEPCOPY macro?\n"
                "and has it indeed been linked with your program?", classname.c_str());

    const TypeMapEntry& entry = it->second;
    Object* obj = 0;

    out << "****************************************************************** \n"
        << "** " << classname << "\n"
        << "****************************************************************** \n" << endl;

    // Display basic help
    out << entry.one_line_descr << endl << endl;
    out << entry.multi_line_help << endl << endl;

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

    for( OptionList::iterator olIt = options.begin(); olIt!=options.end(); ++olIt )
    {
        OptionBase::flag_t flags = (*olIt)->flags();
        if(flags & OptionBase::buildoption)
        {
            string descr = (*olIt)->description();
            string optname = (*olIt)->optionname();
            string opttype = (*olIt)->optiontype();
            string defaultval = "?";
            if(obj) // it's an instantiable class
            {
                defaultval = (*olIt)->defaultval(); 
                if(defaultval=="")
                    defaultval = (*olIt)->writeIntoString(obj);
            }
            // string holderclass = (*olIt)->optionHolderClassName(this);
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
            if( (*entry.isa_method)(o) ) {
                out.width(30);
                out << it->first << " - " << e.one_line_descr << endl;
            }
            if(o)
                delete o;
        }
    }

    out << "\n\n------------------------------------------------------------------ \n" << endl;

}



} // end of namespace PLearn


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
