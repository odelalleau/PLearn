// -*- C++ -*-

// HelpSystem.cc
// Copyright (c) 2006 Pascal Vincent
// Copyright (C) 2007 Xavier Saint-Mleux, ApSTAT Technologies, inc.

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

#include <algorithm>

#include "stringutils.h"    //!< For addprefix.
#include "tostring.h"
#include <plearn/misc/HTMLUtils.h>

#include <plearn/io/openFile.h>
#include <plearn/io/fileutils.h>
#include <plearn/io/openString.h>

#include <plearn/base/TypeFactory.h>
#include <plearn/base/RemoteTrampoline.h>
#include <plearn/base/RemoteDeclareMethod.h>
#include <plearn/base/RemoteMethodDoc.h>
#include <plearn/base/RemoteMethodMap.h>
#include <commands/PLearnCommands/PLearnCommandRegistry.h>
#include <plearn/base/Object.h>


namespace PLearn {
using namespace std;


/*****
 * Commands
 */

vector<string> HelpSystem::listCommands()
{
    vector<string> commands;
    for(PLearnCommandRegistry::command_map::const_iterator
            it  = PLearnCommandRegistry::allCommands().begin();
        it != PLearnCommandRegistry::allCommands().end(); ++it)
        commands.push_back(it->first);
    return commands;
}

string HelpSystem::helpCommands()
{
    vector<string> commands= listCommands();
    string s= "";
    for(vector<string>::iterator it= commands.begin();
        it != commands.end(); ++it)
    {
        PLearnCommand* cmd= PLearnCommandRegistry::getCommand(*it);
        s+= *it + "\t:  " + cmd->description + '\n';
    }
    return s+'\n';
}

string HelpSystem::helpOnCommand(const string& commandname)
{
    PLearnCommand* cmd= PLearnCommandRegistry::getCommand(commandname);
    string help= "*** Help for command '" + commandname + "' ***\n";
    help+= cmd->description + '\n';
    help+= cmd->helpmsg + '\n';        
    return help;
}

string HelpSystem::helpCommandsHTML()
{
    string s;
    s.reserve(16384);
    s+= "<div class=\"generaltable\">\n"
        "<h2>Index of Available Commands</h2>\n"
        "<table cellspacing=\"0\" cellpadding=\"0\">\n";
   
    int i=0;
    vector<string> commands= listCommands();
    for(vector<string>::iterator it= commands.begin();
        it != commands.end(); ++it, ++i)
    {
        PLearnCommand* cmd= PLearnCommandRegistry::getCommand(*it);
        string helpurl= "command_" + *it + ".html";
        string helphtml= "<a href=\"" + helpurl + "\">" + *it + "</a>";
        s+= string("  <tr class=\"") + (i%2 == 0? "even" : "odd") + "\">\n"
            "    <td>" + helphtml + "</td>"
            "<td>" + cmd->description + "</td></tr>\n";
    }
    s+="</table>\n</div>\n";
    return helpPrologueHTML("Command Index") + s + helpEpilogueHTML();
}

string HelpSystem::helpOnCommandHTML(const string& commandname)
{
    PLearnCommand* cmd= PLearnCommandRegistry::getCommand(commandname);
    string s= string("<div class=\"cmdname\">")
        + HTMLUtils::quote(commandname) + "</div>\n"
        "<div class=\"cmddescr\">"
        + HTMLUtils::quote_format_and_highlight(cmd->description) + "</div>\n"
        "<div class=\"cmdhelp\">"
        + HTMLUtils::quote_format_and_highlight(cmd->helpmsg) 
        + "</div>\n";
    return helpPrologueHTML(commandname) + s + helpEpilogueHTML();
}


/*****
 * Functions
 */

vector<pair<string, int> > HelpSystem::listFunctions()
{ 
    return getGlobalFunctionMap().getMethodList(); 
}

vector<string> HelpSystem::listFunctionPrototypes()
{ 
    return getGlobalFunctionMap().getMethodPrototypes(); 
}

string HelpSystem::helpFunctions()
{
    vector<pair<string, int> > funcs= listFunctions();
    string s;
    for(vector<pair<string, int> >::iterator it= funcs.begin();
        it != funcs.end(); ++it)
        s+= it->first +'('+tostring(it->second)+")\t:"
            + helpOnFunction(it->first, it->second) +'\n';
    return s;
}

string HelpSystem::helpOnFunction(const string& functionname, int arity)
{
    return getGlobalFunctionMap().getMethodHelpText(functionname, arity);
}

string HelpSystem::helpFunctionsHTML()
{
    string s= "<div class=\"rmitable\">\n" 
        "<h2>List of Functions</h2>\n"
        "<table cellspacing=\"0\" cellpadding=\"0\">\n";
    s.reserve(32768);

    vector<pair<string, int> > functions= listFunctions();

    int i= 0;
    for(vector<pair<string, int> >::iterator it= functions.begin(); 
        it != functions.end(); ++it)
        s+= string("<tr class=\"") + (i++ == 0? "first" : "others") + "\">\n"
            + helpOnFunctionHTML(it->first, it->second)
            + "</tr>\n";

    if(index == 0)
        s+= "<tr><td>No Remote-Callable Functions.</td></tr>\n";
           
    s+= "</table></div>\n";

    return helpPrologueHTML("Function Index") + s + helpEpilogueHTML();
}

namespace
{
string formatMethodDocHTML(const RemoteMethodDoc& doc, const string& classname= "")
{
    // Generate the function signature and argument-list table in HTML form
    string return_type = HTMLUtils::quote_format_and_highlight(doc.returnType());
    string args = "";
    string arg_table = "";
    list<ArgDoc>::const_iterator adit= doc.argListDoc().begin(), 
        adend= doc.argListDoc().end();
    list<string>::const_iterator tyit= doc.argListType().begin(), 
        tyend= doc.argListType().end();
    int j=0;
    for( ; tyit != tyend ; ++tyit) 
    {
        string arg_type= HTMLUtils::quote_format_and_highlight(*tyit);
        string arg_name= "";
        string arg_doc= "(no documentation)";

        if(adit != adend)
        {
            arg_name= HTMLUtils::quote(adit->m_argument_name);
            arg_doc= HTMLUtils::quote_format_and_highlight(adit->m_doc);
            ++adit;
        }

        if(!args.empty())  args += ", ";

        args+= arg_type + ' ' + arg_name;

        if(!arg_table.empty()) arg_table += "</tr><tr><td></td>";

        string td1_class = (++j % 2 == 0? "argnameeven" : "argnameodd");
        string td2_class = (  j % 2 == 0? "argdoceven"  : "argdocodd");
                
        arg_table+= 
            "  <td class=\"" + td1_class + "\">" + arg_type + ' ' + arg_name + "</td>"
            + "  <td class=\"" + td2_class + "\">" + arg_doc  + "</td>";
    } 

    // Header
    string s= "<td colspan=\"3\"><div class=\"rmiprototype\">";
    s+= return_type
        + " <span class=\"rmifuncname\">" + doc.name() + "</span>"
        + '(' + args + ')' + "</div>\n" 
        "</tr><tr><td></td><td colspan=\"2\">" 
        + HTMLUtils::quote_format_and_highlight(doc.bodyDoc())
        + (classname == "" ? "" 
           : " (defined by " + HTMLUtils::quote_format_and_highlight(classname) + ")")
        + "</td>";

    // Args table
    if(!arg_table.empty())
        s+= "</tr>\n<tr>\n"
            "<td class=\"rmititle\">Arguments</td>\n" + arg_table;

    // Ret. val
    string td1_class = (++j % 2 == 0? "argnameeven" : "argnameodd");
    string td2_class = (  j % 2 == 0? "argdoceven"  : "argdocodd");
            
    if(!doc.returnType().empty() || !doc.returnDoc().empty()) 
        s+= "</tr>\n<tr>\n"
            "<td class=\"rmititle\">Returns</td>\n"
            "<td class=\"" + td1_class + "\">" + return_type + "</td>"
            "<td class=\"" + td2_class + "\">" 
            + HTMLUtils::quote_format_and_highlight(doc.returnDoc()) + "</td>\n";

    //last <tr> s/b closed by calling fn.
    return s;
}
}

string HelpSystem::helpOnFunctionHTML(const string& functionname, int arity)
{
    // the result is a list of table fields (<td>...</td>)
    // should be enclosed in a table row (<table><tr>...</tr></table>)
    const RemoteTrampoline* t= 
        getGlobalFunctionMap().lookup(functionname, arity);
    PLASSERT(t);
    return formatMethodDocHTML(t->documentation());
}

/*****
 * Classes
 */

vector<string> HelpSystem::listClasses()
{
    const TypeMap& type_map = TypeFactory::instance().getTypeMap();
    int nclasses = type_map.size();
    vector<string> class_list(type_map.size());
    TypeMap::const_iterator it = type_map.begin();
    for(int k=0; k<nclasses; ++k, ++it)
        class_list[k] = it->first;
    return class_list;
}

map<string, string> HelpSystem::getClassTree()
{
    const TypeMap& type_map = TypeFactory::instance().getTypeMap();
    map<string, string> class_tree;
    TypeMap::const_iterator it = type_map.begin();
    TypeMap::const_iterator itend = type_map.end();
    for(; it!=itend; ++it)
        class_tree[it->first] = it->second.parent_class;
    return class_tree;
}

string HelpSystem::helpClasses()
{
    vector<string> classes= listClasses();
    string s= "";
    for(vector<string>::iterator it= classes.begin();
        it != classes.end(); ++it)
    {
        const TypeMapEntry& e= 
            TypeFactory::instance().getTypeMapEntry(*it);
        s+= "- " + *it + "\t("+ e.parent_class +")\t:  " 
            + e.one_line_descr + '\n';
    }
    return s+'\n';
}

string HelpSystem::helpOnClass(const string& classname)
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

    string s= "################################################################## \n";
    s+= "## " + classname + "\n";
    s+= "## " + helpClassParents(classname) + '\n';
    s+= "################################################################## \n\n";

    // Display basic help
    s+= "## " + entry.one_line_descr + "\n\n";
    string ml_help = "# " + entry.multi_line_help;
    search_replace(ml_help, "\n", "\n# ");
    s+= ml_help + "\n\n";

    if(entry.constructor) // it's an instantiable class
        obj = (*entry.constructor)();
    else
        s+= "Note: " + classname 
            + " is a base-class with pure virtual methods that cannot be instantiated directly.\n" 
            "(default values for build options can only be displayed for instantiable classes, \n"
            " so you'll only see question marks here.)\n\n";
      
    s+= "################################################################## \n"
        "##                         Build Options                        ## \n"
        "## (including those inherited from parent and ancestor classes) ## \n"
        "################################################################## \n\n";
    s+= classname + "( \n";
    s+= helpClassOptions(classname);
    s+= ");\n\n";

    if(obj) delete obj;

    s+= "################################################################## \n";
    s+= "## Subclasses of " + classname + " \n"
        "# (only those that can be instantiated) \n"
        "################################################################## \n\n";
    s+= addprefix("# ", helpDerivedClasses(classname));
/*
    s+= "\n\n################################################################## \n";
    s+= "## Remote-callable methods of " + classname + " \n"
        "################################################################## \n\n";
    s+= addprefix("# ", helpMethods(classname));
*/
    s+= "\n\n################################################################## \n\n";

    return s;
}

string HelpSystem::helpClassesHTML()
{
    string s= "<div class=\"generaltable\">\n"
        "<h2>Index of Available Classes</h2>\n"
        "<table cellspacing=\"0\" cellpadding=\"0\">\n";
    s.reserve(131072);
    int i=0;
    vector<string> classes= listClasses();
    for(vector<string>::iterator it= classes.begin();
        it != classes.end(); ++it)
    {
        const TypeMapEntry& e= TypeFactory::instance().getTypeMapEntry(*it);
        s+= string("  <tr class=\"") + (i++%2 == 0? "even" : "odd") + "\">\n"
            "    <td>" 
            + HTMLUtils::quote_format_and_highlight(*it)
            + "</td>\n"
            "    <td>" 
            + HTMLUtils::quote_format_and_highlight(e.one_line_descr)
            + '(' + HTMLUtils::quote_format_and_highlight(e.parent_class) + ')'
            + "</td></tr>\n";
    }
    s+= "</table></div>\n";
    return helpPrologueHTML("Class Index") + s + helpEpilogueHTML();
}

string HelpSystem::helpOnClassHTML(const string& classname)
{
    string s= "";
    s.reserve(65536);

    // Output parents heading
    s+= helpClassParentsHTML(classname);

    // Output general information
    const TypeMapEntry& entry= TypeFactory::instance().getTypeMapEntry(classname);
    s+= "<div class=\"classname\">" + HTMLUtils::quote(classname) + "</div>\n"
        "<div class=\"classdescr\">" 
        + HTMLUtils::quote(entry.one_line_descr) + "</div>\n"
        "<div class=\"classhelp\">" 
        + HTMLUtils::quote_format_and_highlight(entry.multi_line_help) 
        + "</div>\n";
  
    if(!entry.constructor) // it's not an instantiable class
        s+= "<div class=\"classhelp\"><b>Note:</b>" + HTMLUtils::quote(classname)
            + " is a base-class with pure virtual methods "
            "that cannot be instantiated directly.\n" 
            "(default values for build options can only be "
            "displayed for instantiable classes, \n"
            " so you'll only see question marks here.)</div>\n";

    // Output list of options
    s+= helpClassOptionsHTML(classname);

    // Output instantiable derived classes
    s+= helpDerivedClassesHTML(classname);

    // Output remote-callable methods
    s+= helpMethodsHTML(classname);

    return helpPrologueHTML(classname) + s + helpEpilogueHTML();
}


vector<string> HelpSystem::listClassParents(const string& classname)
{
    string cl= classname;
    vector<string> parents;
    const TypeMapEntry* e= 
        &TypeFactory::instance().getTypeMapEntry(cl);
    while(e->parent_class != cl)
    {
        cl= e->parent_class;
        parents.push_back(cl);
        e= &TypeFactory::instance().getTypeMapEntry(cl);
    }
    return parents;
}

string HelpSystem::helpClassParents(const string& classname)
{
    vector<string> parents= listClassParents(classname);
    string s= "";
    for(vector<string>::reverse_iterator it= parents.rbegin();
        it != parents.rend(); ++it)
        s+= *it + " > ";
    return s + classname;
}

string HelpSystem::helpClassParentsHTML(const string& classname)
{
    return string("<div class=\"crumbtrail\">")
        + HTMLUtils::quote_format_and_highlight(helpClassParents(classname))
        + "</div>";
}

vector<string> HelpSystem::listDerivedClasses(const string& classname)
{
    const TypeMapEntry& entry= 
        TypeFactory::instance().getTypeMapEntry(classname);
    const TypeMap& the_map= TypeFactory::instance().getTypeMap();
    vector<string> classes;
    for(TypeMap::const_iterator it= the_map.begin();
        it != the_map.end(); ++it)
    {
        const TypeMapEntry& e= it->second;
        if(e.constructor && it->first!=classname)
        {
            Object* o= (*e.constructor)();
            if((*entry.isa_method)(o))
                classes.push_back(it->first);
            if(o) delete o;
        }
    }
    return classes;
}

string HelpSystem::helpDerivedClasses(const string& classname)
{
    vector<string> classes= listDerivedClasses(classname);
    string s= "";
    for(vector<string>::iterator it= classes.begin();
        it != classes.end(); ++it)
        s+= right(*it, 30) + " - " 
            + TypeFactory::instance().getTypeMapEntry(*it).one_line_descr 
            + '\n';
    return s;
}

string HelpSystem::helpDerivedClassesHTML(const string& classname)
{
    // Output instantiable derived classes
    vector<string> classes= listDerivedClasses(classname);

    string s= "<div class=\"generaltable\">\n"
        "<h2>List of Instantiable Derived Classes</h2>\n"
        "<table cellspacing=\"0\" cellpadding=\"0\">\n";

    int i= 0;
    for(vector<string>::iterator it= classes.begin();
        it != classes.end(); ++it)
        s+= string("  <tr class=\"") + (i++%2 == 0? "even" : "odd") + "\">\n"
            "    <td>"
            + HTMLUtils::quote_format_and_highlight(*it) + "</td><td>" 
            + HTMLUtils::quote_format_and_highlight(
                TypeFactory::instance().getTypeMapEntry(*it).one_line_descr)
            + "    </td>  </tr>\n";

    if(i==0)
        s+= "<tr><td>This class does not have instantiable "
            "derived classes.</td></tr>\n";

    return s+"</table></div>\n";
}

pair<string, vector<string> > HelpSystem::precisOnClass(const string& classname)
{
    const TypeMapEntry& tme = TypeFactory::instance().getTypeMapEntry(classname);
    return make_pair(tme.one_line_descr, listBuildOptions(classname));
}


/*****
 * Options
 */

vector<string> HelpSystem::listClassOptions(const string& classname)
{
    vector<pair<OptionBase::OptionLevel, string> > optswl=
        listClassOptionsWithLevels(classname);
    int nopts= optswl.size();
    vector<string> opts(nopts);
    for(int i= 0; i < nopts; ++i)
        opts[i]= optswl[i].second;
    return opts;
}

vector<string> HelpSystem::listBuildOptions(const string& classname)
{
    vector<pair<OptionBase::OptionLevel, string> > optswl=
        listBuildOptionsWithLevels(classname);
    int nopts= optswl.size();
    vector<string> opts(nopts);
    for(int i= 0; i < nopts; ++i)
        opts[i]= optswl[i].second;
    return opts;
}

vector<pair<OptionBase::OptionLevel, string> >
HelpSystem::listClassOptionsWithLevels(const string& classname, 
                                       const OptionBase::flag_t& flags)
{
    const TypeMapEntry& e= TypeFactory::instance().getTypeMapEntry(classname);
    OptionList& optlist= (*e.getoptionlist_method)();
    int nopts= optlist.size();
    vector<pair<OptionBase::OptionLevel, string> > options;
    for(int i= 0; i < nopts; ++i)
        if(optlist[i]->flags() & flags)
            options.push_back(make_pair(optlist[i]->level(), 
                                        optlist[i]->optionname()));
    return options;
}

vector<pair<OptionBase::OptionLevel, string> >
HelpSystem::listBuildOptionsWithLevels(const string& classname)
{ return listClassOptionsWithLevels(classname, OptionBase::buildoption); }

string HelpSystem::helpClassOptions(const string& classname)
{
    string s= "";
    vector<pair<OptionBase::OptionLevel, string> > options= 
        listClassOptionsWithLevels(classname);
    sort(options.begin(), options.end());
    OptionBase::OptionLevel lvl= 0;
    OptionBase::OptionLevel curlvl= OptionBase::getCurrentOptionLevel();
    int nbeyond= 0;
    for(vector<pair<OptionBase::OptionLevel, string> >::iterator 
            it= options.begin(); it != options.end(); ++it)
    {
        if(lvl != it->first)
        {
            lvl= it->first;
            if(lvl <= curlvl)
                s+= "##############################\n"
                    "# Options for level " 
                    + OptionBase::optionLevelToString(lvl) 
                    + "\n\n";
        }
        if(lvl <= curlvl)
            s+= helpOnOption(classname, it->second) + '\n';
        else
            ++nbeyond;
    }
    if(nbeyond > 0)
        s+= "##############################\n"
            "# " + tostring(nbeyond)
            + " hidden options beyond level "
            + OptionBase::optionLevelToString(curlvl)
            + "\n\n";
    return s;
}

string HelpSystem::helpOnOption(const string& classname, const string& optionname)
{
    string s= "";
    PP<OptionBase> opt= getOptionByName(classname, optionname);
    OptionBase::flag_t flags = opt->flags();
    if(flags & OptionBase::buildoption 
       && opt->level() <= OptionBase::getCurrentOptionLevel())
        s+= addprefix("# ", opt->optiontype() + ": " + opt->description())
            //+ addprefix("# ", "*OptionLevel: " + opt->levelString())
            + opt->optionname() + " = " 
            + getOptionDefaultVal(classname, optionname) + " ;\n\n";

    return s;
}

string HelpSystem::helpClassOptionsHTML(const string& classname)
{
    string s= "<div class=\"generaltable\">\n" 
        "<h2>List of All Options</h2>\n" 
        "<table cellspacing=\"0\" cellpadding=\"0\">\n";
    s.reserve(32768);

    vector<string> options= listClassOptions(classname);
    int i= 0;
    for(vector<string>::iterator it= options.begin();
        it != options.end(); ++it)
    {
        PP<OptionBase> opt= getOptionByName(classname, *it);
        if(opt->flags() & OptionBase::buildoption 
           && opt->level() <= OptionBase::getCurrentOptionLevel())
        {
            s+= string("  <tr class=\"") + (i % 2 == 0? "even" : "odd") + "\">\n"
                + helpOnOptionHTML(classname, *it) + "</tr>\n";
            ++i;
        }
    }
    
    if (i==0)
        s+= "<tr><td>This class does not specify any build options.</td></tr>\n";
    
    s+= "</table></div>\n";
    return s;
}

string HelpSystem::helpOnOptionHTML(const string& classname, const string& optionname)
{
    // the result is a list of table fields (<td>...</td>)
    // should be enclosed in a table row (<table><tr>...</tr></table>)
    string s= "";
    PP<OptionBase> opt= getOptionByName(classname, optionname);
    s+= "    <td><div class=\"opttype\">" 
        + HTMLUtils::quote_format_and_highlight(opt->optiontype()) + "</div>\n"
        "    <div class=\"optname\">" 
        + HTMLUtils::quote(opt->optionname()) + "</div>\n";

    string defaultval= getOptionDefaultVal(classname, optionname);
    if (defaultval != "?")
        s+= "    <div class=\"optvalue\">= " 
            + HTMLUtils::quote(defaultval) + "</div>\n";

    string flag_string = join(opt->flagStrings(), " | ");
    s+= string("    <div class=\"opttype\"><i>(")
        + join(opt->flagStrings(), " | ") + ")</i></div>\n"
        + "    <div class=\"optlevel\"><i>(OptionLevel: " 
        + opt->levelString() + ")</i></div>\n"
        "    </td>\n";

    string descr= opt->description();
    if (removeblanks(descr) == "") descr = "(no description)";
    s+= string("    <td>")+HTMLUtils::quote_format_and_highlight(descr);

    string defclass= getOptionDefiningClass(classname, optionname);
    if (defclass != "") 
        s+= string("    <span class=\"fromclass\">")
            + "(defined&nbsp;by&nbsp;" 
            + HTMLUtils::highlight_known_classes(defclass) + ")"
            "</span>\n";
    s+= "    </td>\n";
    return s;
}

string HelpSystem::getOptionDefaultVal(const string& classname, 
                                       const string& optionname)
{
    PP<OptionBase> opt= getOptionByName(classname, optionname);
    const TypeMapEntry& entry= TypeFactory::instance().getTypeMapEntry(classname);
    Object* obj= 0;
    if(entry.constructor) obj= (*entry.constructor)();

    string defaultval= "?";
    if(obj) // it's an instantiable class
    {
        defaultval= opt->defaultval(); 
        if(defaultval=="") defaultval= opt->writeIntoString(obj);
        delete obj;
    }
    return defaultval;
}

string HelpSystem::getOptionDefiningClass(const string& classname, 
                                          const string& optionname)
{
    PP<OptionBase> opt= getOptionByName(classname, optionname);
    const TypeMapEntry& entry= TypeFactory::instance().getTypeMapEntry(classname);
    string defclass= "";
    if(entry.constructor)
    {
        Object* obj= (*entry.constructor)();
        defclass= opt->optionHolderClassName(obj);
        delete obj;
    }
    return defclass;
}

/*****
 * Methods
 */

vector<pair<string, int> > HelpSystem::listMethods(const string& classname)
{
    return TypeFactory::instance().getTypeMapEntry(classname).
        get_remote_methods().getMethodList();
}

vector<string> HelpSystem::listMethodPrototypes(const string& classname)
{
    return TypeFactory::instance().getTypeMapEntry(classname).
        get_remote_methods().getMethodPrototypes();
}

string HelpSystem::helpMethods(const string& classname)
{
    vector<pair<string, int> > methods= listMethods(classname);
    string s= "";
    for(vector<pair<string, int> >::iterator it= methods.begin();
        it != methods.end(); ++it)
        s+= string("\n##########\n") 
            + helpOnMethod(classname, it->first, it->second) + '\n';
    return s;
}

string HelpSystem::helpOnMethod(const string& classname, 
                                const string& methodname, int arity)
{
    return TypeFactory::instance().getTypeMapEntry(classname)
        .get_remote_methods().getMethodHelpText(methodname, arity);
}

string HelpSystem::helpMethodsHTML(const string& classname)
{
    string s= "<div class=\"rmitable\">\n" 
        "<h2>List of Remote-Callable Methods</h2>\n"
        "<table cellspacing=\"0\" cellpadding=\"0\">\n";
    
    vector<string> parents= listClassParents(classname);
    parents.insert(parents.begin(), classname);
    vector<pair<string, int> > methods;
    map<pair<string, int>, string> definingclass;
    for(vector<string>::iterator it= parents.begin();
        it != parents.end(); ++it)
    {
        vector<pair<string, int> > ms= listMethods(*it);
        for(vector<pair<string, int> >::iterator jt= ms.begin();
            jt != ms.end(); ++jt)
        {
            if(definingclass.find(*jt) == definingclass.end())
                methods.push_back(*jt);
            definingclass[*jt]= *it;
        }
    }

    int i= 0;
    for(vector<pair<string, int> >::iterator it= methods.begin(); 
        it != methods.end(); ++it)
        s+= string("<tr class=\"") + (i++ == 0? "first" : "others") + "\">\n"
            + helpOnMethodHTML(definingclass[*it], it->first, it->second)
            + "</tr>\n";

    if(index == 0)
        s+= "<tr><td>This class does not define any remote-callable methods.</td></tr>\n";
           
    s+= "</table></div>\n";

    return s;
}

string HelpSystem::helpOnMethodHTML(const string& classname, 
                                    const string& methodname, int arity)
{
    // the result is a list of table fields (<td>...</td>)
    // should be enclosed in a table row (<table><tr>...</tr></table>)
    const RemoteTrampoline* t= TypeFactory::instance().getTypeMapEntry(classname)
        .get_remote_methods().lookup(methodname, arity);
    PLASSERT(t);
    return formatMethodDocHTML(t->documentation(), classname);
}


/***
 * General Stuff
 */

string HelpSystem::html_resources_path; // init.

string HelpSystem::helpIndexHTML()
{
    static PPath from_dir= "";
    static string the_index= string("<div class=\"cmdname\">\n")
        + "Welcome to PLearn User-Level Class and Commands Help\n" 
        "</div>"
        "<div class=\"cmdhelp\">\n"
        "<ul>\n"
        "  <li> <a href=\"classes_index.html\">Class Index</a>\n"
        "  <li> <a href=\"commands_index.html\">Command Index</a>\n"
        "  <li> <a href=\"functions_index.html\">Function Index</a>\n"
        "</ul></div>\n";

    if(from_dir != html_resources_path)
    {
        from_dir= html_resources_path;
        PPath indexfile= from_dir/"index.html";
        if(pathexists(indexfile))
        {
            PStream f= openFile(indexfile,
                                PStream::raw_ascii);
            the_index= f.readAll();
        }
    }

    return helpPrologueHTML() + the_index + helpEpilogueHTML();
}

string HelpSystem::helpPrologueHTML(const string& title)
{
    static PPath from_dir= "";
    static string the_prologue= "<html><body>";

    if(from_dir != html_resources_path)
    {
        from_dir= html_resources_path;
        PStream f= openFile(from_dir/"help_prolog.html",
                            PStream::raw_ascii);
        the_prologue= f.readAll();
    }

    map<string, string> dic;
    dic["PAGE_TITLE"]= title;
    PStream stm= openString(the_prologue, PStream::raw_ascii);
    return readAndMacroProcess(stm, dic, false);
}

string HelpSystem::helpEpilogueHTML()
{
    static PPath from_dir= "";
    static string the_epilogue= "</body></html>";

    if(from_dir != html_resources_path)
    {
        from_dir= html_resources_path;
        PStream f= openFile(from_dir/"help_epilog.html",
                            PStream::raw_ascii);
        the_epilogue= f.readAll();
    }

    return HTMLUtils::generated_by() + the_epilogue;
}



BEGIN_DECLARE_REMOTE_FUNCTIONS

// Commands

    declareFunction("listCommands", &HelpSystem::listCommands,
                    (BodyDoc("Returns a list of all registered "
                             "commands as strings."),
                     RetDoc ("vector of command names")));

    declareFunction("helpCommands", &HelpSystem::helpCommands,
                    (BodyDoc("Returns a plain text list of all "
                             "registered commands."),
                     RetDoc ("plain text list of commands")));

    declareFunction("helpOnCommand", &HelpSystem::helpOnCommand,
                    (BodyDoc("Will return full help for the "
                             "command with the given name "),
                     ArgDoc ("commandname", 
                             "The name of the command on which to get help"),
                     RetDoc ("help text for the command")));

    // HTML
    declareFunction("helpCommandsHTML", &HelpSystem::helpCommandsHTML,
                    (BodyDoc("Returns an HTML list of all "
                             "registered commands."),
                     RetDoc ("HTML list of commands")));

    declareFunction("helpOnCommandHTML", &HelpSystem::helpOnCommandHTML,
                    (BodyDoc("Will return full help for the "
                             "command with the given name, in HTML"),
                     ArgDoc ("commandname", 
                             "The name of the command on which to get help"),
                     RetDoc ("help text for the command, in HTML")));

// Functions

    declareFunction("listFunctions", &HelpSystem::listFunctions,
                    (BodyDoc("Returns a list of all registered global "
                             "functions as pairs of (funtionname, nargs)"),
                     RetDoc ("vector of function names, arity")));

    declareFunction("listFunctionPrototypes", 
                    &HelpSystem::listFunctionPrototypes,
                    (BodyDoc("Returns a list of the prototypes "
                             "of all registered global functions"),
                     RetDoc ("vector of function prototypes as strings")));

    declareFunction("helpFunctions", &HelpSystem::helpFunctions,
                    (BodyDoc("Returns a list of all registered global "
                             "functions as plain text"),
                     RetDoc ("plain text list of functions")));

    declareFunction("helpOnFunction", &HelpSystem::helpOnFunction,
                    (BodyDoc("Will return full help on all registered "
                             "global functions with the given name "),
                     ArgDoc ("functionname", 
                             "The name of the function on which to get help"),
                     ArgDoc ("arity", "The number of params"),
                     RetDoc ("help text for the function")));

    // HTML
    declareFunction("helpFunctionsHTML", &HelpSystem::helpFunctionsHTML,
                    (BodyDoc("Returns a list of all registered global "
                             "functions as an HTML page."),
                     RetDoc ("HTML list of functions")));

    declareFunction("helpOnFunctionHTML", &HelpSystem::helpOnFunctionHTML,
                    (BodyDoc("Will return full HTML help on all registered "
                             "global functions with the given name "),
                     ArgDoc ("functionname", 
                             "The name of the function on which to get help"),
                     ArgDoc ("arity", "The number of params"),
                     RetDoc ("HTML help text for the function")));

// Classes

    declareFunction("listClasses", &HelpSystem::listClasses,
                    (BodyDoc("Returns a list of all registered Object classes"),
                     RetDoc ("vector of class names")));

    declareFunction("getClassTree", &HelpSystem::getClassTree,
                    (BodyDoc("Returns a map, mapping all registered "
                             "Object classnames to their parentclassname"),
                     RetDoc ("map of class names to class names")));

    declareFunction("helpClasses", &HelpSystem::helpClasses,
                    (BodyDoc("Returns a plain text list of all registered Object classes"),
                     RetDoc ("plain text list of class names")));

    declareFunction("helpOnClass", &HelpSystem::helpOnClass,
                    (BodyDoc("Will return full help for "
                             "the class with the given name"),
                     ArgDoc ("classname", 
                             "The name of the class on which to get help"),
                     RetDoc ("help text for the class")));

    declareFunction("precisOnClass", &HelpSystem::precisOnClass,
                    (BodyDoc("Will return short class descr. and list of build options"),
                     ArgDoc ("classname", 
                             "The name of the class on which to get help"),
                     RetDoc ("pair of classname and list of options")));

    // HTML
    declareFunction("helpClassesHTML", &HelpSystem::helpClassesHTML,
                    (BodyDoc("Returns a list of all registered Object "
                             "classes as an HTML page."),
                     RetDoc ("HTML list of class names")));

    declareFunction("helpOnClassHTML", &HelpSystem::helpOnClassHTML,
                    (BodyDoc("Will return full HTML help for "
                             "the class with the given name"),
                     ArgDoc ("classname", 
                             "The name of the class on which to get help"),
                     RetDoc ("HTML help text for the class")));
    // Parents
    declareFunction("listClassParents", &HelpSystem::listClassParents,
                    (BodyDoc("List of parent classes."),
                     ArgDoc ("classname", 
                             "The name of the class on which to get parents"),
                     RetDoc ("vector of parent class names")));

    declareFunction("helpClassParents", &HelpSystem::helpClassParents,
                    (BodyDoc("Text list of parent classes."),
                     ArgDoc ("classname", 
                             "The name of the class on which to get parents"),
                     RetDoc ("text list of parent class names")));

    declareFunction("helpClassParentsHTML", &HelpSystem::helpClassParentsHTML,
                    (BodyDoc("HTML list of parent classes."),
                     ArgDoc ("classname", 
                             "The name of the class on which to get parents"),
                     RetDoc ("HTML list of parent class names")));
    // Children
    declareFunction("listDerivedClasses", &HelpSystem::listDerivedClasses,
                    (BodyDoc("List of derived classes."),
                     ArgDoc ("classname", 
                             "The name of the class on which to get children"),
                     RetDoc ("List of derived class names")));

    declareFunction("helpDerivedClasses", &HelpSystem::helpDerivedClasses,
                    (BodyDoc("Text list of derived classes."),
                     ArgDoc ("classname", 
                             "The name of the class on which to get children"),
                     RetDoc ("Text list of derived class names")));

    declareFunction("helpDerivedClassesHTML", &HelpSystem::helpDerivedClassesHTML,
                    (BodyDoc("HTML list of derived classes."),
                     ArgDoc ("classname", 
                             "The name of the class on which to get children"),
                     RetDoc ("HTML list of derived class names")));

// Options

    declareFunction("listClassOptions", &HelpSystem::listClassOptions,
                    (BodyDoc("Returns a list of all options "
                             "for the given class."),
                     ArgDoc ("classname", "The name of the class "
                             "on which to get option help"),
                     RetDoc ("vector of option names")));

    declareFunction("listBuildOptions", &HelpSystem::listBuildOptions,
                    (BodyDoc("Returns a list of build options "
                             "for the given class."),
                     ArgDoc ("classname", "The name of the class "
                             "on which to get option help"),
                     RetDoc ("vector of option names")));

    declareFunction("helpOnOption", &HelpSystem::helpOnOption,
                    (BodyDoc("Will return full help for the option with "
                             "the given name within the given class"),
                     ArgDoc ("classname", "The name of the class "
                             "on which to get option help"),
                     ArgDoc ("optionname", 
                             "The name of the option on which to get help"),
                     RetDoc ("help text for the option")));
    // HTML
    declareFunction("helpClassOptionsHTML", &HelpSystem::helpClassOptionsHTML,
                    (BodyDoc("Returns a list of all options "
                             "for the given class as an HTML page."),
                     ArgDoc ("classname", "The name of the class "
                             "on which to get option help"),
                     RetDoc ("HTML list of option names")));

    declareFunction("helpOnOptionHTML", &HelpSystem::helpOnOptionHTML,
                    (BodyDoc("Will return full HTML help for the option "
                             "with the given name within the given class"),
                     ArgDoc ("classname", "The name of the class "
                             "on which to get option help"),
                     ArgDoc ("optionname", 
                             "The name of the option on which to get help"),
                     RetDoc ("HTML help text for the option")));

// Methods

    declareFunction("listMethods", &HelpSystem::listMethods,
                    (BodyDoc("Returns a list of all registered methods "
                             "for the given class as pairs of (methodname, nargs)"),
                     ArgDoc ("classname", 
                             "The name of the class whose methods you want to list."),
                     RetDoc ("vector of method names")));

    declareFunction("listMethodPrototypes", 
                    &HelpSystem::listMethodPrototypes,
                    (BodyDoc("Returns a list of the prototypes of "
                             "all registered methods for the given class"),
                     ArgDoc ("classname", "The name of the class "
                             "whose method prototypes you want to list."),
                     RetDoc ("vector of prototypes as strings")));

    declareFunction("helpMethods", &HelpSystem::helpMethods,
                    (BodyDoc("Returns a list of all registered methods "
                             "for the given class as text."),
                     ArgDoc ("classname", 
                             "The name of the class whose methods you want to list."),
                     RetDoc ("Text list of method names")));

    declareFunction("helpOnMethod", &HelpSystem::helpOnMethod,
                    (BodyDoc("Will return full help on all registered "
                             "methods of the class with the given name"),
                     ArgDoc ("classname", "The name of the class"),
                     ArgDoc ("methodname", "The name of the method"),
                     ArgDoc ("arity", "The number of params"),
                     RetDoc ("help text")));

    // HTML
    declareFunction("helpMethodsHTML", &HelpSystem::helpMethodsHTML,
                    (BodyDoc("Returns a list of all registered methods "
                             "for the given class as an HTML page."),
                     ArgDoc ("classname", 
                             "The name of the class whose methods you want to list."),
                     RetDoc ("HTML list of method names")));

    declareFunction("helpOnMethodHTML", &HelpSystem::helpOnMethodHTML,
                    (BodyDoc("Will return full help on all registered "
                             "methods of the class with the given name"),
                     ArgDoc ("classname", "The name of the class"),
                     ArgDoc ("methodname", "The name of the method"),
                     ArgDoc ("arity", "The number of params"),
                     RetDoc ("help text in HTML")));

// HTML-only

    declareFunction("helpIndexHTML", &HelpSystem::helpIndexHTML,
                    (BodyDoc("Returns the global help index in HTML."),
                     RetDoc ("HTML global help index")));

    declareFunction("setResourcesPathHTML", 
                    &HelpSystem::setResourcesPathHTML,
                    (BodyDoc("Sets the help resource path "
                             "for HTML resources."),
                     ArgDoc ("path","HTML help resource path")));

    declareFunction("getResourcesPathHTML", 
                    &HelpSystem::getResourcesPathHTML,
                    (BodyDoc("Gets the help resource path "
                             "for HTML resources."),
                     RetDoc ("path of HTML resources")));

END_DECLARE_REMOTE_FUNCTIONS


PP<OptionBase> HelpSystem::getOptionByName(const string& classname, 
                                           const string& optionname)
{
    const TypeMapEntry& e= TypeFactory::instance().getTypeMapEntry(classname);
    OptionList& optlist= (*e.getoptionlist_method)();
    for(OptionList::iterator it= optlist.begin();
        it != optlist.end(); ++it)
        if((*it)->optionname() == optionname)
            return *it;
    return 0;//not found...
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
