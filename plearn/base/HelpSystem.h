// -*- C++ -*-

// HelpSystem.h
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

/*! \file HelpSystem.h */

#ifndef HelpSystem_INC
#define HelpSystem_INC

#include <string>
#include <vector>
#include <utility>
#include <map>
#include <plearn/base/OptionBase.h>

namespace PLearn {
using std::string;
using std::vector;
using std::pair;
using std::map;


struct HelpSystem // more or less a namespace
{

    /**
     * Help on Commands
     */

    //! Returns a list of all plearn command names
    static vector<string> listCommands();

    //! Returns a text list of all plearn command names
    static string helpCommands();

    //! Will return full help on the given command
    static string helpOnCommand(const string& commandname);

    //! Returns a list of all plearn commands as an HTML page
    static string helpCommandsHTML();

    //! Will return full HTML help on the given command
    static string helpOnCommandHTML(const string& commandname);

    /**
     * Help on Global Functions
     */

    //! Returns a list of all registered global functions as pairs of (funtionname, nargs)
    static vector<pair<string, int> > listFunctions();

    //! Returns a list of the prototypes of all registered global functions
    static vector<string> listFunctionPrototypes();

    //! Returns a list of all registered global functions in plain text
    static string helpFunctions();

    //! Will return full help on all registered global functions with the given name 
    static string helpOnFunction(const string& functionname, int arity);

    //! Returns a list of all registered global functions as an HTML page
    static string helpFunctionsHTML();

    //! Will return full help on all registered global functions with
    //! the given name, as an HTML string.
    static string helpOnFunctionHTML(const string& functionname, int arity);


    /**
     * Help on Registered Classes
     */

    //! Returns a list of all registered Object classes
    static vector<string> listClasses();

    //! Returns a map, mapping all registered Object classnames to their parentclassname
    static map<string, string> getClassTree();

    //! Returns a list of all registered Object classes as plain text
    static string helpClasses();

    //! Will returns detailed help on registered class with the given name
    //! listing its parent class, and detailed help on all options including inherited ones,
    //! as well as listing all its registered methods.
    static string helpOnClass(const string& classname);

    //! Returns a list of all registered Object classes as an HTML page
    static string helpClassesHTML();

    //! same as helpOnClass, but in HTML
    static string helpOnClassHTML(const string& classname);

    /* Class Parents */

    //! Returns a list of all parent classes of this class
    //! list goes from classname::inherited up to Object
    static vector<string> listClassParents(const string& classname);

    //! Returns a text list of all parent classes of this class
    //! list goes from Object down to classname
    static string helpClassParents(const string& classname);

    //! Returns an HTML list of all parent classes of this class
    //! list goes from Object down to classname
    static string helpClassParentsHTML(const string& classname);

    /* Derived Classes */

    //! Returns a list of all instantiable classes derived from 'classname'
    static vector<string> listDerivedClasses(const string& classname);

    //! Returns a text list of all instantiable classes derived from 'classname'
    static string helpDerivedClasses(const string& classname);

    //! Returns an HTML list of all instantiable classes derived from 'classname'
    static string helpDerivedClassesHTML(const string& classname);

    //! Returns a pair of class descr. and list of build options
    static pair<string, vector<string> > precisOnClass(const string& classname);

    /**
     * Help on Class Options
     */

     //! Returns the list of all options for the class with the given name
    static vector<string> listClassOptions(const string& classname);

     //! Returns the list of build options for the class with the given name
    static vector<string> listBuildOptions(const string& classname);

     //! Returns the list of all options for the class with the given name
    static vector<pair<OptionBase::OptionLevel, string> > 
    listClassOptionsWithLevels(const string& classname,
                               const OptionBase::flag_t& flags= 
                               OptionBase::getCurrentFlags());

     //! Returns the list of build options for the class with the given name
    static vector<pair<OptionBase::OptionLevel, string> > 
    listBuildOptionsWithLevels(const string& classname);

     //! Returns the list of options for the class with the given name, as text
    static string helpClassOptions(const string& classname);

    //! Will return full help on the declared option of the class with the given name 
    static string helpOnOption(const string& classname, const string& optionname);

     //! Returns the list of options for the class with the given name, in HTML
    static string helpClassOptionsHTML(const string& classname);

    //! Will return full help on the declared option of the class
    //! with the given name, as an HTML string.
    static string helpOnOptionHTML(const string& classname, 
                                   const string& optionname);

    //! Returns the default value for this option, or "?" if 
    //! it is from an abstract class
    static string getOptionDefaultVal(const string& classname, 
                                      const string& optionname);

    //! Returns the class that defines this option, or "" if not known
    static string getOptionDefiningClass(const string& classname, 
                                         const string& optionname);

    /**
     * Help on Class Methods
     */

    //! Returns a list of all registered methods for the 
    //! given class as pairs of (methodname, nargs)
    static vector<pair<string, int> > listMethods(const string& classname);

    //! Returns a list of the prototypes of all registered methods for the given class
    static vector<string> listMethodPrototypes(const string& classname);

    //! Returns a list of all registered methods for the 
    //! given class as text
    static string helpMethods(const string& classname);

    //! Will return full help on the registered method of the class with the 
    //! given name and arity
    static string helpOnMethod(const string& classname, 
                               const string& methodname, int arity= -1);

    //! Returns a list of all registered methods for the 
    //! given class as an HTML page
    static string helpMethodsHTML(const string& classname);

    //! Will return full help on the registered method of the class 
    //! with the given name and arity, as an HTML string.
    static string helpOnMethodHTML(const string& classname, 
                                   const string& methodname, int arity= -1);

    /**
     * HTML Help
     */

private:
    //! Directory that holds HTML resources.
    //! These include: 
    //! - index.html (optional)
    //! - help_prolog.html
    //! - help_epilog.html
    //! e.g. {PLEARNDIR}/python_modules/plearn/plide/resources
    static string html_resources_path;
    
public:
    //! Sets the path for resources for HTML help
    static void setResourcesPathHTML(const string& path)
    { html_resources_path= path; }

    //! Returns the path for resources for HTML help
    static string getResourcesPathHTML()
    { return html_resources_path; }

    //! Returns the global help index as an HTML page
    static string helpIndexHTML();

    //! Returns the standard heading for HTML help
    static string helpPrologueHTML(const string& title= 
                                   "PLearn User-Level Documentation");

    //! Returns the standard ending for HTML help
    static string helpEpilogueHTML();



private:
    static PP<OptionBase> getOptionByName(const string& classname, 
                                          const string& optionname);


};


} // end of namespace PLearn

#endif //!<  HelpSystem_INC_


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
