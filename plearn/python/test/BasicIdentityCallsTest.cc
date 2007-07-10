// -*- C++ -*-

// BasicIdentityCallsTest.cc
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

/*! \file BasicIdentityCallsTest.cc */


#include <plearn/python/PythonCodeSnippet.h>
#include <iostream>
#include <vector>
#include <map>

#include "BasicIdentityCallsTest.h"
#include <plearn/io/openString.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    BasicIdentityCallsTest,
    "Tests the core of PythonCodeSnippet through identity function calls in Python",
    ""
);

////////////////////////////
// BasicIdentityCallsTest //
////////////////////////////
BasicIdentityCallsTest::BasicIdentityCallsTest() 
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
void BasicIdentityCallsTest::build()
{
    inherited::build();
    build_();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void BasicIdentityCallsTest::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields 
    // ### that you wish to be deepCopied rather than 
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("BasicIdentityCallsTest::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

////////////////////
// declareOptions //
////////////////////
void BasicIdentityCallsTest::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify  
    // ### one of OptionBase::buildoption, OptionBase::learntoption or 
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    // ### ex:
    // declareOption(ol, "myoption", &BasicIdentityCallsTest::myoption, OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void BasicIdentityCallsTest::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation. 
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
    // ### You should assume that the parent class' build_() has already been called.
}

string BasicIdentityCallsTest::python_code =
"import sys\n"
"from numarray import *\n"
"\n"
"def nullary():\n"
"    print >>sys.stderr, 'Called nullary()'\n"
"    sys.stderr.flush()\n"
"\n"
"def unary_int(x):\n"
"    assert isinstance(x,int)\n"
"    return x\n"
"\n"
"def unary_long(x):\n"
"    assert isinstance(x,long)\n"
"    return x\n"
"\n"
"def unary_float(x):\n"
"    assert isinstance(x,float)\n"
"    return x\n"
"\n"
"def unary_str(x):\n"
"    assert isinstance(x,str)\n"
"    return x\n"
"\n"
"def unary_vec(x):\n"
"    assert isinstance(x,numarraycore.NumArray) and len(x.shape) == 1\n"
"    return x\n"
"\n"
"def unary_mat(x):\n"
"    print >>sys.stderr, 'Called unary_mat with:\\n',x\n"
"    sys.stderr.flush()\n"
"    assert isinstance(x,numarraycore.NumArray) and len(x.shape) == 2\n"
"    return x\n"
"\n"
"def unary_list_str(x):\n"
"    assert isinstance(x,list)\n"
"    return x\n"
"\n"
"def unary_dict(x):\n"
"    assert isinstance(x,dict)\n"
"    return x\n"
"\n"
"def binary(a,b):\n"
"    return a,b\n"
"\n"
"def ternary(a,b,c):\n"
"    return a,b,c\n"
"\n"
"def quaternary(a,b,c,d):\n"
"    return a,b,c,d\n"
;


void BasicIdentityCallsTest::nullary(const PythonCodeSnippet* python)
{
    cout << "isInvokable(nullary)           : " << python->isInvokable("nullary") << endl;
    cout << "Calling nullary                : " << flush;
    python->invoke("nullary");
}

void BasicIdentityCallsTest::unary(const PythonCodeSnippet* python)
{
    cout << "Calling unary_int(42)          : "
         << python->invoke("unary_int", 42).as<int>() << endl;

    cout << "Calling unary_long(int64_t(42)): "
         << python->invoke("unary_long", int64_t(42)).as<int64_t>() << endl;

    cout << "Calling unary_float(42.01)     : "
         << python->invoke("unary_float", 42.01).as<double>() << endl;

    cout << "Calling unary_str('Hello')     : "
         << python->invoke("unary_str", "Hello").as<string>() << endl;

    Vec v;
    string str_vec = "[2,3,5,7,11,13,17,19,23]";
    PStream is = openString(str_vec, PStream::plearn_ascii);
    is >> v;
    Mat m(3,3);
    m.toVec() << v;

    cout << "Calling unary_vec(v)           : "
         << tostring( python->invoke("unary_vec", v).as<Vec>() )
         << endl;

    // Test full matrix (mod == width)
    cout << "Calling unary_mat(m)           : " << endl;
    cout << tostring( python->invoke("unary_mat", m).as<Mat>() )
         << endl;

    // Test sliced matrix (mod > width)
    cout << "Calling unary_mat(m)           : " << endl;
    cout << tostring( python->invoke("unary_mat", m.subMatColumns(1,2)).as<Mat>() )
         << endl;

    TVec<string> tvs;
    string str_tvs = "[\"Cela\", \"est\", \"juste\", \"et\", \"bon\"]";
    PStream is_tvs = openString(str_tvs, PStream::plearn_ascii);
    is_tvs >> tvs;
    vector<string> vecs(tvs.begin(), tvs.end());

    cout << "Calling unary_list_str(tvs)    : "
         << tostring( python->invoke("unary_list_str", tvs).as< TVec<string> >() )
         << endl;
    cout << "Calling unary_list_str(vecs)   : "
         << tostring( TVec<string>(python->invoke("unary_list_str", vecs)
                                   .as< vector<string> >() ))
         << endl;

    map<string,int32_t> mapsd;
    string str_mapsd = "{ Oui:16 il:32 est:64 juste:128 et:256 bon:512 }";
    PStream is_mapsd = openString(str_mapsd, PStream::plearn_ascii);
    is_mapsd >> mapsd;

    cout << "Calling unary_dict(mapsd)      : "
         << tostring( python->invoke("unary_dict", mapsd).as< map<string,int32_t> >() )
         << endl;
}

void BasicIdentityCallsTest::binary(const PythonCodeSnippet* python)
{
    cout << "Calling binary(2,4)            : "
         << tostring( python->invoke("binary",2,4).as< TVec<int> >())
         << endl;
}

void BasicIdentityCallsTest::ternary(const PythonCodeSnippet* python)
{
    cout << "Calling ternary(2,4,8)         : "
         << tostring( python->invoke("ternary",2,4,8).as< TVec<int> >())
         << endl;
}

void BasicIdentityCallsTest::quaternary(const PythonCodeSnippet* python)
{
    cout << "Calling quaternary(2,4,8,16)   : "
         << tostring( python->invoke("quaternary",2,4,8,16).as< TVec<int> >())
         << endl;
}

/////////////
// perform //
/////////////
void BasicIdentityCallsTest::perform()
{
    // ### The test code should go here.
    cout << "Python code to be executed: " << endl
         << ">>>" << python_code << "<<<" << endl;
    
    PP<PythonCodeSnippet> python = new PythonCodeSnippet(python_code);
    python->build();
    
    nullary(python);
    unary(python);
    binary(python);
    ternary(python);
    quaternary(python);
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
