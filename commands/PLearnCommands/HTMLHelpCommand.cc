// -*- C++ -*-

// HTMLHelpCommand.cc
//
// Copyright (C) 2004-2006 Nicolas Chapados 
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
 * $Id$ 
 ******************************************************* */

// Authors: Nicolas Chapados

/*! \file HTMLHelpCommand.cc */

#define PL_LOG_MODULE_NAME "HTMLHelpCommand"

#include <time.h>
#include <fstream>
#include <set>
#include <boost/regex.hpp>

#include <plearn/base/general.h>
#include <plearn/base/TypeFactory.h>
#include <plearn/base/Object.h>
#include <plearn/base/stringutils.h>
#include <plearn/io/pl_log.h>
#include "PLearnCommandRegistry.h"
#include "plearn_main.h"                     // For version_string
#include "HTMLHelpCommand.h"

namespace PLearn {
using namespace std;

//#####  HTMLHelpConfig  ######################################################

PLEARN_IMPLEMENT_OBJECT(
    HTMLHelpConfig,
    "Configuration options for HTMLHelpCommand",
    "Specifies generation options for HTML documentation."
    );

void HTMLHelpConfig::declareOptions(OptionList& ol)
{
    declareOption(ol, "output_dir", &HTMLHelpConfig::output_dir,
                  OptionBase::buildoption,
                  "Directory where the .html files should be generated");

    declareOption(ol, "html_index_document", &HTMLHelpConfig::html_index_document,
                  OptionBase::buildoption,
                  "Filename containing the main index file (index.html).  If missing,\n"
                  "a default index is generated.  Note that the prolog and epilog are\n"
                  "always generated for this file.");
    
    declareOption(ol, "html_prolog_document", &HTMLHelpConfig::html_prolog_document,
                  OptionBase::buildoption,
                  "Filename containing the \"top\" of the HTML document (including an\n"
                  "opening <body> tag");
                
    declareOption(ol, "html_epilog_document", &HTMLHelpConfig::html_epilog_document,
                  OptionBase::buildoption,
                  "Filename containing the \"bottom\" of the HTML document (including the\n"
                  "closing </body> tag");
                
    inherited::declareOptions(ol);
}


//#####  HTMLHelpCommand per se  ##############################################
  
//! This allows to register the 'HTMLHelpCommand' command in the command
//! registry
PLearnCommandRegistry HTMLHelpCommand::reg_(new HTMLHelpCommand);

HTMLHelpCommand::HTMLHelpCommand():
    PLearnCommand("htmlhelp",
                  "Output HTML-formatted help for PLearn",
                  "This command provides basic HTML formatting for PLearn\n"
                  "commands and classes.  The command takes a single argument,\n"
                  "a filename containing an HTMLHelpConfig object giving the\n"
                  "configuration options for HTML generation." )
{}

//! The actual implementation of the 'HTMLHelpCommand' command 
void HTMLHelpCommand::run(const vector<string>& args)
{
    if (args.size() != 1) {
        cout << helpmsg << endl;
    }

    const string& fname = args[0];
    config = dynamic_cast<HTMLHelpConfig*>(macroLoadObject(fname));
    if (!config)
        PLERROR("HTMLHelpCommand::run: the file '%s' must contain an "
                "object of type HTMLHelpConfig", fname.c_str());

    helpCommands();
    helpClasses();
    helpIndex();
}

//! Copy the document snippet in filename into the ostream
void HTMLHelpCommand::copySnippet(ostream& os, const string& document)
{
    ifstream f(document.c_str());
    if (f)
        while (f) {
            int c = f.get();
            if (c > 0)
                os.put(c);
        }
    else
        PLERROR("HTMLHelpCommand::copySnippet: cannot open file '%s' for reading",
                document.c_str());
}


//#####  helpIndex  ###########################################################

void HTMLHelpCommand::helpIndex()
{
    assert( config );
    ofstream os((config->output_dir + "/" + "index.html").c_str());
    helpIndex(os, config);
}

void HTMLHelpCommand::helpIndex(ostream& os, const HTMLHelpConfig* config)
{
    copySnippet(os, config->html_prolog_document);

    if (config->html_index_document.empty()) {
        os << "<div class=\"cmdname\">" << endl
           << "Welcome to PLearn User-Level Class and Commands Help" << endl
           << "</div>"
           << "<div class=\"cmdhelp\">" << endl
           << "<ul>" << endl
           << "  <li> <a href=\"class_index.html\">Class Index</a>" << endl
           << "  <li> <a href=\"command_index.html\">Command Index</a>" << endl
           << "</ul></div>" << endl;
    }
    else
        copySnippet(os, config->html_index_document);

    os << generated_by() << endl;

    copySnippet(os, config->html_epilog_document);
}
  

//#####  helpCommands  ######################################################

void HTMLHelpCommand::helpCommands()
{
    assert( config );
    ofstream os((config->output_dir + "/" + "command_index.html").c_str());
    helpCommands(os, config);
    
    for(PLearnCommandRegistry::command_map::iterator
            it  = PLearnCommandRegistry::commands().begin(),
            end = PLearnCommandRegistry::commands().end()
            ; it != end ; ++it)
    {
        // Generate help for this command
        helpOnCommand(it->first);
    }
}

void HTMLHelpCommand::helpCommands(ostream& os, const HTMLHelpConfig* config)
{
    copySnippet(os, config->html_prolog_document);
  
    os << "<div class=\"generaltable\">" << endl
       << "<h2>Index of Available Commands</h2>" << endl
       << "<table cellspacing=\"0\" cellpadding=\"0\">" << endl;

    int i=2;                                   //!< row #1 is header
    vector<string> args(1);
    for(PLearnCommandRegistry::command_map::iterator
            it  = PLearnCommandRegistry::commands().begin(),
            end = PLearnCommandRegistry::commands().end()
            ; it != end ; ++it, ++i)
    {
        string helpurl = string("command_") + it->first + ".html";
        string helphtml = "<a href=\"" + helpurl + "\">" + it->first + "</a>";
        os << string("  <tr class=\"") + (i%2 == 0? "even" : "odd") + "\">" << endl
           << "    <td>" + helphtml + "</td>"
           << "<td>" + it->second->description + "</td></tr>" << endl;
    }
    os << "</table>" << endl
       << "</div>" << endl;

    os << generated_by() << endl;
  
    copySnippet(os, config->html_epilog_document);
}


//#####  HelpOnCommand  #####################################################

void HTMLHelpCommand::helpOnCommand(const string& theCommand)
{
    assert( config );
    ofstream os((config->output_dir + "/" + "command_"+theCommand+".html").c_str());
    helpOnCommand(theCommand, os, config);
}

void HTMLHelpCommand::helpOnCommand(const string& theCommand, ostream& os,
                                    const HTMLHelpConfig* config)
{
    copySnippet(os, config->html_prolog_document);
  
    PLearnCommandRegistry::command_map::iterator
        it  = PLearnCommandRegistry::commands().find(theCommand),
        end = PLearnCommandRegistry::commands().end();
    if (it == end)
        PLERROR("HTMLHelpCommand::helpOnCommand: no help for command '%s'",
                theCommand.c_str());
    else
        os << "<div class=\"cmdname\">"
           << quote(theCommand) << "</div>" << endl
           << "<div class=\"cmddescr\">"
           << quote(it->second->description) << "</div>" << endl
           << "<div class=\"cmdhelp\">"
           << format_free_text(quote(it->second->helpmsg)) << "</div>" << endl;

    os << generated_by() << endl;
  
    copySnippet(os, config->html_epilog_document);
}


//#####  HelpClasses  #######################################################

void HTMLHelpCommand::helpClasses()
{
    assert( config );
    ofstream os((config->output_dir + "/" + "class_index.html").c_str());
    helpClasses(os, config);
    
    for(TypeMap::const_iterator
            it  = TypeFactory::instance().getTypeMap().begin(),
            end = TypeFactory::instance().getTypeMap().end()
            ; it != end ; ++it)
    {
        // Generate help for this class
        helpOnClass(it->first);
    }
}

void HTMLHelpCommand::helpClasses(ostream& os, const HTMLHelpConfig* config)
{
    copySnippet(os, config->html_prolog_document);
  
    os << "<div class=\"generaltable\">" << endl
       << "<h2>Index of Available Classes</h2>" << endl
       << "<table cellspacing=\"0\" cellpadding=\"0\">" << endl;

    vector<string> args(1);
    int i=0;
    for(TypeMap::const_iterator
            it  = TypeFactory::instance().getTypeMap().begin(),
            end = TypeFactory::instance().getTypeMap().end()
            ; it != end ; ++it)
    {
        string helpurl = string("class_") + it->first + ".html";
        string helphtml = "<a href=\"" + helpurl + "\">" + it->first + "</a>";
        os << string("  <tr class=\"") + (i++%2 == 0? "even" : "odd") + "\">" << endl
           << "    <td>" + helphtml + "</td>" << endl
           << "    <td>" + it->second.one_line_descr + "</td></tr>" << endl;
    }
    os << "</table>" << endl
       << "</div>" << endl;

    os << generated_by() << endl;
    copySnippet(os, config->html_epilog_document);
}


//#####  HelpOnClass  #######################################################

void HTMLHelpCommand::helpOnClass(const string& classname)
{
    assert( config );
    ofstream out((config->output_dir + "/class_" + classname + ".html").c_str());
    helpOnClass(classname, out, config);
}

void HTMLHelpCommand::helpOnClass(const string& classname, ostream& out,
                                  const HTMLHelpConfig* config)
{
    copySnippet(out, config->html_prolog_document);

    const TypeMap& type_map = TypeFactory::instance().getTypeMap();
    TypeMap::const_iterator it = type_map.find(classname);
    TypeMap::const_iterator itend = type_map.end();

    if(it==itend)
        PLERROR("Object type %s unknown.\n"
                "Did you #include it, does it call the IMPLEMENT_NAME_AND_DEEPCOPY macro?\n"
                "and has it indeed been linked with your program?", classname.c_str());

    const TypeMapEntry& entry = it->second;
    Object* obj = 0;

    // Determine list of parent classes and print it out as crumbtrail
    TVec<string> parents = parent_classes(classname);
    out << "<div class=\"crumbtrail\">";
    for (int i=0, n=parents.size() ; i<n ; ++i) {
        if (i < n-1)
            out << "<a href=\"class_" + parents[i] + ".html\">"
                << parents[i] << "</a>&nbsp;&gt; ";
        else
            out << parents[i];
    }
    out << "</div>";

    // Output general information
    out << "<div class=\"classname\">"
        << quote(classname)
        << "</div>" << endl
        << "<div class=\"classdescr\">"
        << quote(entry.one_line_descr)
        << "</div>" << endl
        << "<div class=\"classhelp\">"
        << highlight_known_classes(format_free_text(quote(entry.multi_line_help)))
        << "</div>" << endl;
  
    if(entry.constructor) // it's an instantiable class
        obj = (*entry.constructor)();
    else
        out << "<div class=\"classhelp\"><b>Note:</b>"
            << quote(classname)
            << " is a base-class with pure virtual methods that cannot be instantiated directly.\n" 
            << "(default values for build options can only be displayed for instantiable classes, \n"
            << " so you'll only see question marks here.)</div>\n"
            << endl;

    // Output list of options
    out << "<div class=\"generaltable\">" << endl
        << "<h2>List of All Options</h2>" << endl
        << "<table cellspacing=\"0\" cellpadding=\"0\">" << endl;
  
    OptionList& options = (*entry.getoptionlist_method)();    

    int index = 0;
    for( OptionList::iterator olIt = options.begin(); olIt!=options.end();
         ++olIt ) {

        string descr    = (*olIt)->description();
        string optname  = (*olIt)->optionname();
        string opttype  = (*olIt)->optiontype();
        string defclass = "";
        string defaultval = "?";

        // Determine the flag rendering
        string flag_string = join((*olIt)->flagStrings(), " | ");
    
        if(obj) // it's an instantiable class
        {
            defaultval = (*olIt)->defaultval(); 
            if(defaultval=="")
                defaultval = (*olIt)->writeIntoString(obj);
            defclass = (*olIt)->optionHolderClassName(obj);
        }
        out << string("  <tr class=\"") + (index++ % 2 == 0? "even" : "odd")
			                            + "\">" << endl
            << "    <td>"
            << "<div class=\"opttype\">" << highlight_known_classes(quote(opttype))
            << "</div>" << endl
            << "    <div class=\"optname\">" << quote(optname) << "</div>" << endl;
        if (defaultval != "?")
            out << "    <div class=\"optvalue\">= " << quote(defaultval) << "</div>" << endl;
        out << "    <div class=\"opttype\"><i>(" << flag_string << ")</i></div>" << endl;
        out << "    </td>" << endl;
        if (removeblanks(descr) == "")
            descr = "(no description)";
        out << "    <td>"
            << highlight_known_classes(format_free_text(quote(descr)));
        if (defclass != "") {
            out << "    <span class=\"fromclass\">"
                << "(defined&nbsp;by&nbsp;" << highlight_known_classes(defclass) << ")"
                << "</span>" << endl;
        }
        out << "    </td>" << endl
            << "  </tr>" << endl;
    }
    if (index == 0)
        out << "<tr><td>This class does not specify any build options.</td></tr>"
            << endl;

    out << "</table></div>" << endl;

    if(obj)
        delete obj;

    // Output instantiable derived classes
    out << "<div class=\"generaltable\">" << endl
        << "<h2>List of Instantiable Derived Classes</h2>" << endl
        << "<table cellspacing=\"0\" cellpadding=\"0\">" << endl;

    index = 0;
    for(it = type_map.begin(); it!=itend; ++it)
    {
        const TypeMapEntry& e = it->second;
        if(e.constructor && it->first!=classname)
        {
            Object* o = (*e.constructor)();
            if( (*entry.isa_method)(o) ) {
                string helpurl = string("class_") + it->first + ".html";
                out << string("  <tr class=\"") +
					   (index++%2 == 0? "even" : "odd") + "\">" << endl
                    << "    <td><a href=\"" << helpurl << "\">"
                    << quote(it->first) << "</a></td><td>" << quote(e.one_line_descr)
                    << "    </td>"
                    << "  </tr>" << endl;
            }
            if(o)
                delete o;
        }
    }
    if (index == 0)
        out << "<tr><td>This class does not have instantiable derived classes.</td></tr>"
            << endl;

    out << "</table></div>" << endl;

    // Output remote-callable methods
    const RemoteMethodMap* rmm = &(*entry.get_remote_methods)();
    out << "<div class=\"rmitable\">" << endl
        << "<h2>List of Remote-Callable Methods</h2>" << endl
        << "<table cellspacing=\"0\" cellpadding=\"0\">" << endl;
    index = 0;
    while (rmm) {
        RemoteMethodMap::MethodMap::const_iterator
            it = rmm->begin(), end = rmm->end();
        for ( ; it != end ; ++it ) {
            const string& method_name = it->first.first;
            const RemoteTrampoline* t = it->second;
            assert( t );
            const RemoteMethodDoc& doc = t->documentation();

            // Generate the method signature and argument-list table in HTML form
            string return_type = highlight_known_classes(quote(doc.returnType()));
            string args = "";
            string arg_table = "";
            list<ArgDoc>::const_iterator
                adit = doc.argListDoc().begin(), adend = doc.argListDoc().end();
            list<string>::const_iterator
                tyit = doc.argListType().begin(), tyend = doc.argListType().end();

            int j=0;
            for ( ; tyit != tyend ; ++tyit) {
                string arg_type = highlight_known_classes(quote(*tyit));
                string arg_name = "";
                string arg_doc  = "(no documentation)";
                if (adit != adend) {
                    arg_name = quote(adit->m_argument_name);
                    arg_doc  = highlight_known_classes(format_free_text(adit->m_doc));
                    ++adit;
                }
                if (! args.empty())
                    args += ", ";
                args += arg_type + ' ' + arg_name;

                if (! arg_table.empty())
                    arg_table += "</tr><tr><td></td>";

                string td1_class = (++j % 2 == 0? "argnameeven" : "argnameodd");
                string td2_class = (  j % 2 == 0? "argdoceven"  : "argdocodd");
                
                arg_table +=
                      "  <td class=\"" + td1_class + "\">" + arg_type + ' ' + arg_name + "</td>"
                    + "  <td class=\"" + td2_class + "\">" + arg_doc  + "</td>";
            }

            string tr_class = (index++ == 0? "first" : "others");
            out << "<tr class=\"" << tr_class << +  + "\">" << endl
                << "<td colspan=\"3\"><div class=\"rmiprototype\">"
                << return_type
                << " <span class=\"rmifuncname\">" << method_name << "</span>"
                << '(' << args << ')' << "</div>" << endl
                << "</tr><tr><td></td><td colspan=\"2\">" << doc.bodyDoc()
                << "</td></tr>" << endl;

            if (! arg_table.empty())
                out << "<tr>" << endl
                    << "<td class=\"rmititle\">Arguments</td>" << endl
                    << arg_table << "</tr>" << endl;

            string td1_class = (++j % 2 == 0? "argnameeven" : "argnameodd");
            string td2_class = (  j % 2 == 0? "argdoceven"  : "argdocodd");
            
            if (! doc.returnType().empty() || ! doc.returnDoc().empty()) {
                string form_ret = highlight_known_classes(format_free_text(doc.returnDoc()));
                out << "<tr>" << endl
                    << "<td class=\"rmititle\">Returns</td>" << endl
                    << "<td class=\"" + td1_class + "\">" << return_type << "</td>"
                    << "<td class=\"" + td2_class + "\">" << form_ret  << "</td>" << endl
                    << "</tr>"
                    << endl;
            }
        }

        // Inspect base classes
        rmm = rmm->inheritedMethods();
    }
    if (index == 0)
        out << "<tr><td>This class does not define any remote-callable methods.</td></tr>"
            << endl;

    out << "</table></div>" << endl;

    
    out << generated_by() << endl;
    copySnippet(out, config->html_epilog_document);
}


string HTMLHelpCommand::quote(string s) const
{
    search_replace(s, "&", "&amp;");
    search_replace(s, "<", "&lt;");
    search_replace(s, ">", "&gt;");
    search_replace(s, "\"", "&quot;");
    return s;
}


TVec<string> HTMLHelpCommand::parent_classes(string classname) const
{
    const TypeMap& type_map = TypeFactory::instance().getTypeMap();
    TVec<string> parents;
    while (classname != "") {
        TypeMap::const_iterator it = type_map.find(classname);
        if (it == type_map.end()) {
            PLWARNING("HTMLHelpCommand::parent_classes: cannot find class '%s'",
                      classname.c_str());

            static bool printed_parents = false;
            if (! printed_parents) {
                MODULE_LOG << "Registered classes are:" << endl;
                for (TypeMap::const_iterator types_it = type_map.begin(),
                         types_end = type_map.end() ; types_it != types_end ; ++types_it)
                    MODULE_LOG << types_it->first << endl;
                printed_parents = true;
            }
            break;
        }
        parents.insert(0,classname);
        if (classname != it->second.parent_class) // Object is its own parent
            classname = it->second.parent_class;
        else break;
    }
    return parents;
}


string HTMLHelpCommand::highlight_known_classes(string typestr) const
{
    vector<string> tokens = split(typestr, " \t\n\r<>,.';:\"");
    set<string> replaced; // Carry out replacements for a given token only once
    const TypeMap& type_map = TypeFactory::instance().getTypeMap();
    vector<string>::size_type n=tokens.size();
    for (unsigned int i=0; i<n ; ++i) {
        TypeMap::const_iterator it = type_map.find(tokens[i]);
        if (it != type_map.end() && replaced.find(tokens[i]) == replaced.end()) {
            replaced.insert(tokens[i]);
      
            // ensure we only match whole words with the regular expression
            const boost::regex e("\\<" + tokens[i] + "\\>");
            const string repl_str("<a href=\"class_$&.html\">$&</a>");
            typestr = regex_replace(typestr, e, repl_str, boost::match_default | boost::format_default);
        }
    }
    return typestr;
}


string HTMLHelpCommand::format_free_text(string text) const
{
    // sort of DWIM HTML formatting for free-text; cannot use split since it
    // eats up consecutive delimiters
    text = removeblanks(text);
    size_t curpos = 0, curnl = text.find('\n');
    bool ul_active = false;
    bool start_paragraph = false;
    string finallines;
    for ( ; curpos != string::npos ;
          curpos = curnl+(curnl!=string::npos), curnl = text.find('\n', curpos) ) {
        string curline = text.substr(curpos, curnl-curpos);

        // step 1: check if the line starts with a '-': if so, start a new <UL>
        // if not in one, or extend one if so
        if (removeblanks(curline).substr(0,1) == "-" ||
            removeblanks(curline).substr(0,1) == "*" )
        {
            curline = removeblanks(curline).substr(1);
            if (! ul_active)
                curline = "<ul><li>" + curline;
            else
                curline = "<li>" + curline;
            start_paragraph = false;
            ul_active = true;
        }

        // otherwise, a line that starts with some whitespace within a list
        // just extends the previous <li> :: don't touch it
        else if (ul_active && (curline == "" ||
                               curline.substr(0,1) == " " ||
                               curline.substr(0,1) == "\t")) {
            /* do nothing */
        }

        // otherwise, normal processing
        else {
            // any line that is empty or starts with some whitespace gets its own <br>
            if (removeblanks(curline) == "") {
                // Don't start new paragraph right away; wait until we
                // encounter some text that's neither a <ul> or a <pre>
                start_paragraph = true;
                curline = "";
            }
            else if (curline[0] == ' ' || curline[0] == '\t') {
                start_paragraph = false;
                curline = "<pre>" + curline + "</pre>";
            }

            // if we were processing a list, close it first
            if (ul_active) {
                curline = "</ul>" + curline;
                ul_active = 0;
            }
        }

        if (!curline.empty() && start_paragraph) {
            finallines += "<p>";
            start_paragraph = false;
        }
        
        finallines += curline + "\n";
    }

    // Close any pending open blocks
    if (ul_active)
        finallines += "</ul>\n";
  
    // Finally join the lines
    return make_http_hyperlinks(finallines);
}


string HTMLHelpCommand::make_http_hyperlinks(string text) const
{
    // Find elements of the form XYZ://x.y.z/a/b/c and make them into
    // hyperlink. An issue is to determine when
    static const char* recognized_protocols[] = 
        { "http://", "https://", "ftp://", "mailto:" };        // for now...
    static const vector<string> protocols_vector(
        recognized_protocols,
        recognized_protocols + sizeof(recognized_protocols) / sizeof(recognized_protocols[0]));

    // Match everything that starts with the recognized protocol up to the
    // following whitespace, excluding trailing punctuation if any.
    // Make sure the URL is NOT enclosed in quotes
    static const boost::regex e( string("(?!\")") + "(" +
                                 "(?:" + join(protocols_vector, "|") + ")" +
                                 "\\S+(?:\\w|/)" +
                                 ")" + "(?!\")" + "([[:punct:]]*\\s|$)");
    const string repl_str("<a href=\"$1\">$1</a>$2");
    text = regex_replace(text, e, repl_str, boost::match_default | boost::format_default);
    return text;
}


string HTMLHelpCommand::generated_by() const
{
    time_t curtime = time(NULL);
    struct tm *broken_down_time = localtime(&curtime);
    const int SIZE = 100;
    char time_buffer[SIZE];
    strftime(time_buffer,SIZE,"%Y/%m/%d %H:%M:%S",broken_down_time);

    return string("<p>&nbsp;</p><address>Generated on " ) +
        time_buffer + " by " + version_string() + "</address>";
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
