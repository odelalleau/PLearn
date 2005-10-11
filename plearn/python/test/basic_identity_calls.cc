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
 *  @file   basic_identity_calls
 *  @brief  Tests the core of PythonCodeSnippet through identity function
 *          calls in Python
 */


#include <plearn/python/PythonCodeSnippet.h>
#include <iostream>
#include <vector>
#include <map>
#include <plearn/io/openString.h>

using namespace PLearn;
using namespace std;

string python_code =
"import sys\n"
"from numarray import *\n"
"\n"
"def nullary():\n"
"    print >>sys.stderr, 'Called nullary()'\n"
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


void nullary(const PythonCodeSnippet* python)
{
    cout << "isInvokable(nullary)        : " << python->isInvokable("nullary") << endl;
    cout << "Calling nullary             : " << flush;
    python->invoke("nullary");
}

void unary(const PythonCodeSnippet* python)
{
    cout << "Calling unary_int(42)       : "
         << python->invoke("unary_int", 42).as<int>() << endl;

    cout << "Calling unary_long(42L)     : "
         << python->invoke("unary_long", 42L).as<long>() << endl;

    cout << "Calling unary_float(42.01)  : "
         << python->invoke("unary_float", 42.01).as<double>() << endl;

    cout << "Calling unary_str('Hello')  : "
         << python->invoke("unary_str", "Hello").as<string>() << endl;

    Vec v;
    PStream is = openString("[2,3,5,7,11,13,17,19,23]", PStream::plearn_ascii);
    is >> v;
    Mat m(3,3);
    m.toVec() << v;

    cout << "Calling unary_vec(v)        : "
         << tostring( python->invoke("unary_vec", v).as<Vec>() )
         << endl;

    cout << "Calling unary_mat(m)        : " << endl
         << tostring( python->invoke("unary_mat", m).as<Mat>() )
         << endl;

    TVec<string> tvs;
    PStream is_tvs = openString("[\"Cela\", \"est\", \"juste\", \"et\", \"bon\"]",
                                PStream::plearn_ascii);
    is_tvs >> tvs;
    vector<string> vecs(tvs.begin(), tvs.end());

    cout << "Calling unary_list_str(tvs) : "
         << tostring( python->invoke("unary_list_str", tvs).as< TVec<string> >() )
         << endl;
    cout << "Calling unary_list_str(vecs): "
         << tostring( TVec<string>(python->invoke("unary_list_str", vecs)
                                   .as< vector<string> >() ))
         << endl;

    map<string,long> mapsd;
    PStream is_mapsd = openString("{ Oui:16 il:32 est:64 juste:128 et:256 bon:512 }",
                                  PStream::plearn_ascii);
    is_mapsd >> mapsd;

    cout << "Calling unary_dict(mapsd)   : "
         << tostring( python->invoke("unary_dict", mapsd).as< map<string,long> >() )
         << endl;
}

void binary(const PythonCodeSnippet* python)
{
    cout << "Calling binary(2,4)         : "
         << tostring( python->invoke("binary",2,4).as< TVec<int> >())
         << endl;
}

void ternary(const PythonCodeSnippet* python)
{
    cout << "Calling ternary(2,4,8)      : "
         << tostring( python->invoke("ternary",2,4,8).as< TVec<int> >())
         << endl;
}

void quaternary(const PythonCodeSnippet* python)
{
    cout << "Calling quaternary(2,4,8,16): "
         << tostring( python->invoke("quaternary",2,4,8,16).as< TVec<int> >())
         << endl;
}


int main()
{
    cout << "Python code to be executed: " << endl
         << ">>>" << python_code << "<<<" << endl;
    
    PP<PythonCodeSnippet> python = new PythonCodeSnippet(python_code);
    python->build();
    
    nullary(python);
    unary(python);
    binary(python);
    ternary(python);
    quaternary(python);
    
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
 
