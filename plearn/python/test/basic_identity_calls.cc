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
"    print >>sys.stderr, 'Called unary_mat with:',x\n"
"    assert isinstance(x,numarraycore.NumArray) and len(x.shape) == 2\n"
"    return x\n"
;


void nullary(const PythonCodeSnippet* python)
{
    cout << "isCallable(nullary)         : " << python->isCallable("nullary") << endl;
    cout << "Calling nullary             : " << flush;
    python->callFunction("nullary");
}

void unary(const PythonCodeSnippet* python)
{
    cout << "Calling unary_int(42)       : "
         << python->callFunction("unary_int", 42).as<int>() << endl;

    cout << "Calling unary_long(42L)     : "
         << python->callFunction("unary_long", 42L).as<long>() << endl;

    cout << "Calling unary_float(42.01)  : "
         << python->callFunction("unary_float", 42.01).as<double>() << endl;

    cout << "Calling unary_str('Hello')  : "
         << python->callFunction("unary_str", "Hello").as<string>() << endl;

    Vec v;
    PStream is = openString("[2,3,5,7,11,13,17,19,23]", PStream::plearn_ascii);
    is >> v;
    Mat m(3,3);
    m.toVec() << v;

    cout << "Calling unary_vec(v)        : "
         << tostring( python->callFunction("unary_vec", v).as<Vec>() )
         << endl;

    cout << "Calling unary_mat(m)        : "
         << tostring( python->callFunction("unary_mat", m).as<Mat>() )
         << endl;
}


int main()
{
    cout << "Python code to be executed: " << endl
         << ">>>" << python_code << "<<<" << endl;
    
    PP<PythonCodeSnippet> python = new PythonCodeSnippet(python_code);
    nullary(python);
    unary(python);
  
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
 
