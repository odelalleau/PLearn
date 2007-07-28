// -*- C++ -*-

// InterfunctionXchgTest.cc
//
// Copyright (C) 2005 Nicolas Chapados 
// Copyright (C) 2005-2006 Olivier Delalleau 
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
   * $Id: .pyskeleton_header 544 2003-09-01 00:05:31Z plearner $ 
   ******************************************************* */

// Authors: Nicolas Chapados, Olivier Delalleau

/*! \file InterfunctionXchgTest.cc */


#include <plearn/python/PythonCodeSnippet.h>
#include <boost/regex.hpp>
#include <iostream>

#include "InterfunctionXchgTest.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    InterfunctionXchgTest,
    "Test data exchange within a single snippet and exception mapping",
    ""
);

//////////////////
// InterfunctionXchgTest //
//////////////////
InterfunctionXchgTest::InterfunctionXchgTest() 
    /* ### Initialize all fields to their default value */
{
    // ...

    // ### You may (or not) want to call build_() to finish building the object
    // ### (doing so assumes the parent classes' build_() have been called too
    // ### in the parent classes' constructors, something that you must ensure)
}

///////////
// build //
///////////
void InterfunctionXchgTest::build()
{
    inherited::build();
    build_();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void InterfunctionXchgTest::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields 
    // ### that you wish to be deepCopied rather than 
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("InterfunctionXchgTest::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

////////////////////
// declareOptions //
////////////////////
void InterfunctionXchgTest::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify  
    // ### one of OptionBase::buildoption, OptionBase::learntoption or 
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    // ### ex:
    // declareOption(ol, "myoption", &InterfunctionXchgTest::myoption, OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void InterfunctionXchgTest::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation. 
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
    // ### You should assume that the parent class' build_() has already been called.
}

string InterfunctionXchgTest::python_code =
"import sys\n"
"\n"
"def set_value(x):\n"
"    global buf\n"
"    buf = x\n"
"\n"
"def get_value():\n"
"    global buf\n"
"    return buf\n"
"\n"
"def print_global_map():\n"
"    print 'Printing some_global_map within Python:', some_global_map\n"
"    sys.stdout.flush()\n"
;


/////////////
// perform //
/////////////
void InterfunctionXchgTest::perform()
{
    cout << "Python code to be executed: " << endl
         << ">>>" << python_code << "<<<" << endl;
    
    PP<PythonCodeSnippet> python       = new PythonCodeSnippet(python_code);
    PP<PythonCodeSnippet> python_other = new PythonCodeSnippet(python_code, true);
    python->build();
    python_other->build();
    
    string survivor = "This string should survive within the Python environment";
    cout << "Setting the string:   '" << survivor << "'" << endl;
    python->invoke("set_value", survivor);
    cout << "Read back the string: '"
         << python->invoke("get_value").as<string>()
         << "'" << endl;

    cout << "Trying to read back from second snippet:" << endl;
    try {
        string s = python_other->invoke("get_value").as<string>();
        cout << "Read back the string: '" << s << "'" << endl;
    }
    catch (const PythonException& e) {
        string exception_msg = e.message();
        // Remove memory addresses from the exception message, as they may
        // differ from one computer to another, causing the test to fail.
        boost::regex memory_adr("0x[abcdef0123456789]{6,}",
                                boost::regex::perl|boost::regex::icase);
        string msg_without_sys_dependent_data =
            regex_replace(exception_msg, memory_adr,
                    "0x[memory_address]");
        boost::regex python_ver("Python 2\\.[0-9]\\.[0-9]",
                                boost::regex::perl|boost::regex::icase);
        msg_without_sys_dependent_data =
            regex_replace(msg_without_sys_dependent_data, python_ver,
                    "Python 2.X.Y");
        cout << "Caught Python Exception: '" << msg_without_sys_dependent_data
             << "'" << endl;
    }
    catch (const PLearnError& e) {
        cout << "Caught PLearn Exception: '" << e.message() << "'" << endl;
    }
    catch (...) {
        cout << "Caught unknown exception..." << endl;
    }


    try {
        map<string,int> mapsd;
        string str_mapsd = "{ Oui:16 il:32 est:64 juste:128 et:256 bon:512 }";
        PStream is_mapsd = openString(str_mapsd, PStream::plearn_ascii);
        is_mapsd >> mapsd;

        python_other->setGlobalObject("some_global_map", PythonObjectWrapper(mapsd));
        cout << "Associated 'some_global_map' with: " << tostring(mapsd) << endl;
        cout << "Read back from Python environment: "
             << tostring(python_other->getGlobalObject("some_global_map").as< map<string,int> >())
             << endl;
        python_other->invoke("print_global_map");

        cout << "Dump of the 'python_other' compiled environment" << endl;
    }
    catch(const PLearnError& e)
    {
        cerr << "FATAL ERROR: " << e.message() << endl;
    }
    catch (...) 
    {
        cerr << "FATAL ERROR: uncaught unknown exception "
             << "(ex: out-of-memory when allocating a matrix)" << endl;
    }
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
