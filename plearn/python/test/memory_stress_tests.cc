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
 *  @file   memory_stress_tests
 *  @brief  Perform a large number of Python/C++ conversions to detect memory
 *          leaks (best run under valgrind or other leak detector)
 */


#include <plearn/python/PythonCodeSnippet.h>
#include <iostream>
#include <vector>
#include <map>
#include <plearn/io/openString.h>
#include <plearn/base/ProgressBar.h>

using namespace PLearn;
using namespace std;

string python_code =
"import sys\n"
"from numarray import *\n"
"\n"
"def nullary():\n"
"    pass\n"
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
    python->invoke("nullary");
}

void unary(const PythonCodeSnippet* python)
{
    int i    = python->invoke("unary_int", 42).as<int>();
    long l   = python->invoke("unary_long", 42L).as<long>();
    double d = python->invoke("unary_float", 42.01).as<double>();
    string s = python->invoke("unary_str", "Hello").as<string>();

    i = i;
    l = l;
    d = d;
    
    Vec v;
    string str_v = "[2,3,5,7,11,13,17,19,23]";
    PStream is = openString(str_v, PStream::plearn_ascii);
    is >> v;
    Mat m(3,3);
    m.toVec() << v;

    Vec py_vec = python->invoke("unary_vec", v).as<Vec>();
    Mat py_mat = python->invoke("unary_mat", m).as<Mat>();

    TVec<string> tvs;
    string str_tvs = "[\"Cela\", \"est\", \"juste\", \"et\", \"bon\"]";
    PStream is_tvs = openString(str_tvs, PStream::plearn_ascii);
    is_tvs >> tvs;
    vector<string> vecs(tvs.begin(), tvs.end());

    TVec<string> py_tvs    = python->invoke("unary_list_str", tvs).as< TVec<string> >();
    vector<string> py_vecs = python->invoke("unary_list_str", vecs).as< vector<string> >();

    map<string,long> mapsd;
    string str_mapsd = "{ Oui:16 il:32 est:64 juste:128 et:256 bon:512 }";
    PStream is_mapsd = openString(str_mapsd, PStream::plearn_ascii);
    is_mapsd >> mapsd;

    map<string,long> py_mapsd = python->invoke("unary_dict", mapsd).as< map<string,long> >();
}

void binary(const PythonCodeSnippet* python)
{
    TVec<int> v = python->invoke("binary",2,4).as< TVec<int> >();
}

void ternary(const PythonCodeSnippet* python)
{
    TVec<int> v = python->invoke("ternary",2,4,8).as< TVec<int> >();
}

void quaternary(const PythonCodeSnippet* python)
{
    TVec<int> v = python->invoke("quaternary",2,4,8,16).as< TVec<int> >();
}


int main()
{
    PP<PythonCodeSnippet> python = new PythonCodeSnippet(python_code);
    python->build();

    const int N = 10000;
    ProgressBar pb("Invoking Python Functions", N);
    
    for (int i=0 ; i<N ; ++i) {
        pb.update(i);
        
        nullary(python);
        unary(python);
        binary(python);
        ternary(python);
        quaternary(python);
    }
    
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
 
