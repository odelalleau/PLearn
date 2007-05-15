// -*- C++ -*-

// plext.cc
// Copyright (C) 2007 Xavier Saint-Mleux, ApSTAT Technologies inc.

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

#include <plearn/python/PythonIncludes.h>
#include <commands/plearn_full_inc.h>

using namespace PLearn;
using namespace std;

PyObject* tramp(PyObject* self, PyObject* args)
{
  RemoteTrampoline* t= 
    static_cast<RemoteTrampoline*>(PyCObject_AsVoidPtr(self));
  int nas= PyTuple_GET_SIZE(args);
  TVec<PythonObjectWrapper> as;
  for(int i= 0; i < nas; ++i)
    as.push_back(PythonObjectWrapper(PyTuple_GET_ITEM(args, i)));
  try
    {
      return t->call(0, as);
    }
  catch(const PLearnError& e) 
    {
      PyErr_SetString(PyExc_Exception, e.message().c_str());
      return 0;
    }
  catch(const std::exception& e) 
    {
      PyErr_SetString(PyExc_Exception, e.what());
      return 0;
    }
  catch(...) 
    {
      PyErr_SetString(PyExc_Exception,
		      "PLearn: Caught unknown C++ exception");
      return 0;
    }
}

static PObjectPool<PyMethodDef> pyfuncs(50);

PyMODINIT_FUNC
initplext(void)
{
  PyObject* plext= Py_InitModule("plext", NULL);

  const RemoteMethodMap::MethodMap& global_funcs= 
    getGlobalFunctionMap().getMap();

  for(RemoteMethodMap::MethodMap::const_iterator it=
	global_funcs.begin();
      it != global_funcs.end(); ++it)
    {
      PyObject* self= 
	PyCObject_FromVoidPtr(it->second, NULL);
    
      PyMethodDef* py_method= pyfuncs.allocate();
      py_method->ml_name= 
	const_cast<char*>(it->first.first.c_str());
      py_method->ml_meth= tramp;
      py_method->ml_flags= METH_VARARGS;
      py_method->ml_doc= "PLearn function";
    
      PyObject* pyfunc= 
	PyCFunction_NewEx(py_method, self, plext);
	    
      if(pyfunc) 
	PyObject_SetAttrString(plext, py_method->ml_name, pyfunc);
      else
	PLERROR("Cannot inject PLearn global function %s into python.",
		py_method->ml_name);
    }
}
