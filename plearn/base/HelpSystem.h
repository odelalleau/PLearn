// -*- C++ -*-

// HelpSystem.h
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

/*! \file PLearnLibrary/PLearnCore/HelpSystem.h */

#ifndef HelpSystem_INC
#define HelpSystem_INC

#include <string>
#include <vector>
#include <utility>
#include <map>

namespace PLearn {
using std::string;
using std::vector;
using std::pair;
using std::map;

//! Returns a list of all registered global functions as pairs of (funtionname, nargs)
vector< pair<string, int> > listFunctions();

//! Returns a list of the prototypes of all registered global functions
vector<string> listFunctionPrototypes();

//! Will return full help on all registered global functions with the given name 
string helpFunction(const string& functionname);

//! Returns a list of all registered methods for the given class as pairs of (methodname, nargs)
vector< pair<string, int> > listMethods(const string& classname);

//! Returns a list of the prototypes of all registered methods for the given class
vector<string> listMethodPrototypes(const string& classname);

//! Will return full help on all registered methods of the class with the given name 
string helpMethod(const string& classname, const string& methodname);

//! Returns a list of all registered Object classes
vector<string> listClasses();

//! Returns a map, mapping all registered Object classnames to their parentclassname
map<string, string> getClassTree();

/*

//! Returns a list of all direct subclasses of classname
//! Throws an exception if classname is not registered.
vector<string> childrenOf(const string& classname);

//! Returns a list of all descendents of the given class
//! Throws an exception if classname is not registered.
vector<string> descendantsOf(const string& classname);

//! Returns the parent class of classname (empty string if no parent)
//! Throws an exception if classname is not registered.
string parentOf(const string& classname);

//! Returns the structured FunctionHelp object describing the specified function
//! Throws an exception if no function is registered with that name and number of arguments
//FunctionHelp getFunctionDoc(const string& functionname, int nargs);

//! Returns the structured FunctionHelp object describing the specified method
//! Throws an exception if classname is not registered or if it has no
//! registered method with that name and number of arguments.
//FunctionHelp getMethodDoc(const string& classname, const string& methodname, int nargs);


//! Returns the list of options
// vector<string> listClassOptions(const string& classname);

//! Will returns detailed help on registered class with the given name
//! listing its parent class, and detailed help on all options including inherited ones,
//! as well as listing all its registered methods.
string helpClass(const string& classname);


*/


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
