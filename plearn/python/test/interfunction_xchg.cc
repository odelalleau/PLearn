// -*- C++ -*-

// basic_identity_calls.cc
//
// Copyright (C) 2005 Nicolas Chapados 
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

// Authors: Nicolas Chapados


/**
 *  @file   interfunction_xchg
 *  @brief  Test data exchange within a single snippet and exception mapping
 */


#include <plearn/python/PythonCodeSnippet.h>
#include <iostream>

using namespace PLearn;
using namespace std;

string python_code =
"import sys\n"
"\n"
"def set_value(x):\n"
"    global buf\n"
"    buf = x\n"
"\n"
"def get_value():\n"
"    global buf\n"
"    return buf\n"
;


int main()
{
    cout << "Python code to be executed: " << endl
         << ">>>" << python_code << "<<<" << endl;
    
    PP<PythonCodeSnippet> python       = new PythonCodeSnippet(python_code);
    PP<PythonCodeSnippet> python_other = new PythonCodeSnippet(python_code, true);
    
    string survivor = "This string should survive within the Python environment";
    cout << "Setting the string:   '" << survivor << "'" << endl;
    python->callFunction("set_value", survivor);
    cout << "Read back the string: '"
         << python->callFunction("get_value").as<string>()
         << "'" << endl;

    cout << "Trying to read back from second snippet:" << endl;
    try {
        string s = python_other->callFunction("get_value").as<string>();
        cout << "Read back the string: '" << s << "'" << endl;
    }
    catch (const PythonException& e) {
        cout << "Caught Python Exception: '" << e.message() << "'" << endl;
    }
    catch (const PLearnError& e) {
        cout << "Caught PLearn Exception: '" << e.message() << "'" << endl;
    }
    catch (...) {
        cout << "Caught unknown exception..." << endl;
    }

    map<string,long> mapsd;
    PStream is_mapsd = openString("{ Oui:16 il:32 est:64 juste:128 et:256 bon:512 }",
                                  PStream::plearn_ascii);
    is_mapsd >> mapsd;

    python_other->setGlobalObject("some_global_map", PythonObjectWrapper(mapsd));
    cout << "Associated 'some_global_map' with: " << tostring(mapsd) << endl;
    cout << "Read back from Python environment: "
         << tostring(python_other->getGlobalObject("some_global_map").as< map<string,long> >())
         << endl;
    
    return 0;
}


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
 
