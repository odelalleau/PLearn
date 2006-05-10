// -*- C++ -*-

// InjectionTest.cc
//
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

// Authors: Olivier Delalleau

/*! \file InjectionTest.cc */


#include <plearn/python/PythonCodeSnippet.h>
#include <iostream>

#include "InjectionTest.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    InjectionTest,
    "Ensure that the Python function PyCFunction_NewEx works correctly",
    ""
);

//////////////////
// InjectionTest //
//////////////////
InjectionTest::InjectionTest() 
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
void InjectionTest::build()
{
    inherited::build();
    build_();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void InjectionTest::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields 
    // ### that you wish to be deepCopied rather than 
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("InjectionTest::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

////////////////////
// declareOptions //
////////////////////
void InjectionTest::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify  
    // ### one of OptionBase::buildoption, OptionBase::learntoption or 
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    // ### ex:
    // declareOption(ol, "myoption", &InjectionTest::myoption, OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void InjectionTest::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation. 
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
    // ### You should assume that the parent class' build_() has already been called.
}

string InjectionTest::python_code =
"import sys\n"
"\n"
"def trampoline_function_call(x):\n"
"    y = injected_c_function(x)\n"
"    print >>sys.stderr, 'The C function returned the charming value',y\n"
"    sys.stderr.flush()\n"
"\n"
"def trampoline_method_call(x):\n"
"    y = injected_c_method1(x)\n"
"    z = injected_c_method2(x)\n"
"    print >>sys.stderr, 'The C method 1 returned the charming value',y\n"
"    print >>sys.stderr, 'The C method 2 returned the charming value',z\n"
"    sys.stderr.flush()\n"
;

PythonObjectWrapper InjectionTest_basic_function(const TVec<PythonObjectWrapper>& args)
{
    cout << "basic_function called with arg[0]="
         << args[0].as<int>() << endl;
    return PythonObjectWrapper(42);
}


struct X
{
    X(int value) : i(value) { }
  
    int i;
    PythonObjectWrapper f(const TVec<PythonObjectWrapper>& args);
    PythonObjectWrapper g(const TVec<PythonObjectWrapper>& args) const;
};

PythonObjectWrapper X::f(const TVec<PythonObjectWrapper>& args)
{
    cout << "X::f() called with i=" << i << " and arg[0]="
         << args[0].as<int>() << endl;
    return PythonObjectWrapper(1337);
}

PythonObjectWrapper X::g(const TVec<PythonObjectWrapper>& args) const
{
    cout << "X::g() called with i=" << i << " and arg[0]="
         << args[0].as<int>() << endl;
    return PythonObjectWrapper(1337*2);
}

/////////////
// perform //
/////////////
void InjectionTest::perform()
{
    PP<PythonCodeSnippet> python = new PythonCodeSnippet(python_code);
    python->build();

    X x(13370);
    
    python->inject("injected_c_function", InjectionTest_basic_function);
    python->inject("injected_c_function2", InjectionTest_basic_function);
    python->inject("injected_c_method1", &x, &X::f);
    python->inject("injected_c_method2", &x, &X::g);

    python->invoke("trampoline_function_call", 64);
    python->invoke("trampoline_method_call",   128);
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
