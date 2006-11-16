// -*- C++ -*-

// HelpSystem.cc
// Copyright (c) 2006 Pascal Vincent

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


#include "HelpSystem.h"
#include "stringutils.h"    //!< For addprefix.
#include <plearn/base/TypeFactory.h>
#include <plearn/base/RemoteTrampoline.h>
#include <plearn/base/RemoteDeclareMethod.h>
#include <plearn/base/RemoteMethodDoc.h>
#include <plearn/base/RemoteMethodMap.h>

namespace PLearn {
using namespace std;


vector< pair<string, int> > listFunctions()
{ 
    return getGlobalFunctionMap().getMethodList(); 
}

vector<string> listFunctionPrototypes()
{ 
    return getGlobalFunctionMap().getMethodPrototypes(); 
}

string helpFunction(const string& functionname)
{
    return getGlobalFunctionMap().getMethodHelpText(functionname);
}

vector< pair<string, int> > listMethods(const string& classname)
{
    const RemoteMethodMap& rmm = TypeFactory::instance().getTypeMapEntry(classname).get_remote_methods();
    return rmm.getMethodList(); 
}

vector<string> listMethodPrototypes(const string& classname)
{
    const RemoteMethodMap& rmm = TypeFactory::instance().getTypeMapEntry(classname).get_remote_methods();
    return rmm.getMethodPrototypes(); 
}

string helpMethod(const string& classname, const string& methodname)
{
    const RemoteMethodMap& rmm = TypeFactory::instance().getTypeMapEntry(classname).get_remote_methods();
    return rmm.getMethodHelpText(methodname);
}

vector<string> listClasses()
{
    const TypeMap& type_map = TypeFactory::instance().getTypeMap();
    int nclasses = type_map.size();
    vector<string> class_list(type_map.size());
    TypeMap::const_iterator it = type_map.begin();
    for(int k=0; k<nclasses; ++k, ++it)
        class_list[k] = it->first;
    return class_list;
}

map<string, string> getClassTree()
{
    const TypeMap& type_map = TypeFactory::instance().getTypeMap();
    map<string, string> class_tree;
    TypeMap::const_iterator it = type_map.begin();
    TypeMap::const_iterator itend = type_map.end();
    for(; it!=itend; ++it)
        class_tree[it->first] = it->second.parent_class;
    return class_tree;
}



BEGIN_DECLARE_REMOTE_FUNCTIONS
    declareFunction("listFunctions", &listFunctions,
                    (BodyDoc("Returns a list of all registered global functions as pairs of (funtionname, nargs)")));

    declareFunction("listFunctionPrototypes", &listFunctionPrototypes,
                    (BodyDoc("Returns a list of the prototypes of all registered global functions")));

    declareFunction("helpFunction", &helpFunction,
                    (BodyDoc("Will return full help on all registered global functions with the given name "),
                     ArgDoc ("functionname", "The name of the function on which to get help")));

    declareFunction("listMethods", &listMethods,
                    (BodyDoc("Returns a list of all registered methods for the given class as pairs of (methodname, nargs)"),
                     ArgDoc ("classname", "The name of the class whose methods you want to list.")));

    declareFunction("listMethodPrototypes", &listMethodPrototypes,
                    (BodyDoc("Returns a list of the prototypes of all registered methods for the given class"),
                     ArgDoc ("classname", "The name of the class whose method prototypes you want to list.")));

    declareFunction("helpMethod", &helpMethod,
                    (BodyDoc("Will return full help on all registered methods of the class with the given name"),
                     ArgDoc ("classname", "The name of the class"),
                     ArgDoc ("methodname", "The name of the method")));
                     
    declareFunction("listClasses", &listClasses,
                    (BodyDoc("Returns a list of all registered Object classes")));

    declareFunction("getClassTree", &getClassTree,
                    (BodyDoc("Returns a map, mapping all registered Object classnames to their parentclassname")));

END_DECLARE_REMOTE_FUNCTIONS


/*


TVec<string> listOptions(const string& classname)
{
    const TypeMapEntry& entry = TypeFactory::instance().getTypeMapEntry(classname);
    OptionList& options = (*entry.getoptionlist_method)();    
    TVec<string> optionnames;
    for( OptionList::iterator it = options.begin(); it!=options.end(); ++it )
        optionnames.append((*it)->optionname());
    return optionnames;
}

string helpOption(const string& classname, const string& optionname)
{
    const TypeMapEntry& entry = TypeFactory::instance().getTypeMapEntry(classname);
    OptionList& options = (*entry.getoptionlist_method)();    
    OptionList::iterator op = options.find(optionname);
    if(op==options.end())
        PLERROR("Class %s has no option named %s", classname.c_str(), optionname.c_str());

    OptionBase::flag_t flags = (*op)->flags();

    string descr = (*op)->description();
    string optname = (*op)->optionname();
    string opttype = (*op)->optiontype();
    string defaultval = "?";
    if(obj) // it's an instantiable class
    {
        defaultval = (*op)->defaultval(); 
        if(defaultval=="")
            defaultval = (*op)->writeIntoString(obj);
    }

    string optflags = "";
    if(flags & OptionBase::buildoption)
        optflags += " buildoption |";
    if(flags & OptionBase::learntoption)
        optflags += " learntoption |";
    if(flags & OptionBase::tuningoption)
        optflags += " tuningoption |";
    if(flags & OptionBase::nosave)
        optflags += " nosave |";
    if(flags & OptionBase::nonparentable)
        optflags += " nonparentable |";
    if(flags & OptionBase::nontraversable)
        optflags += " nontraversable |";

    string helpstring = string("OPTION ")+classname+"::"+optionname+"\n"
        +"Flags: "+optflags+"\n"
        +"Type: "+opttype+"\n"
        +"Description: "+ descr +"\n";
    return helpstring;
}



void printObjectHelp(PStream out, const string& classname)
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

*/

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
