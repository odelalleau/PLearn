// -*- C++ -*-4 1999/10/29 20:41:34 dugas

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2004 Pascal Vincent and ApSTAT Technologies Inc.

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
   * $Id: pylearn.cc,v 1.1 2004/06/15 19:28:54 plearner Exp $
   * This file is part of the PLearn library.
   ******************************************************* */
// Authors: Pascal Vincent

#include <boost/python.hpp>

#include "Object.h"
#include "getDataSet.h"

using namespace boost::python;
using namespace std;

real (PLearn::VMat::*vmat_get)(int, int) const   = &PLearn::VMat::operator();
PLearn::Object* (*macro_load_object1)(const string&) = &PLearn::macroLoadObject;
PLearn::Object* (*macro_load_object2)(const string&, map<string,string>&) = &PLearn::macroLoadObject;

BOOST_PYTHON_MODULE(pylearn)
{
  def("getDataSetHelp", &PLearn::getDataSetHelp);
  def("getDataSet", &PLearn::getDataSet);

  class_<PLearn::VMat>("VMat")
    .def("length", &PLearn::VMat::length)
    .def("width", &PLearn::VMat::width)
    .def("get", vmat_get)
    ;

  def("newObject", &PLearn::newObject, return_value_policy<manage_new_object>());
  def("loadObject", &PLearn::loadObject, return_value_policy<manage_new_object>());
  def("macroLoadObject", macro_load_object1, return_value_policy<manage_new_object>());
  def("macroLoadObjectWithVars", macro_load_object2, return_value_policy<manage_new_object>());

  class_<PLearn::Object>("Object")
    .def("build", &PLearn::Object::build)
    .def("setOption", &PLearn::Object::setOption)
    .def("getOption", &PLearn::Object::getOption)
    .def("run", &PLearn::Object::run)
    ;
}


