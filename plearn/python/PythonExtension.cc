// -*- C++ -*-

// PythonExtension.h
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

#include <plearn/python/PythonExtension.h>
#include <plearn/base/RemoteDeclareMethod.h>
#include <plearn/base/HelpSystem.h>

namespace PLearn {

// Trampoline for global PLearn 'remote' functions
PyObject* pythonGlobalFuncTramp(PyObject* self, PyObject* args)
{
    RemoteTrampoline* t= 
        static_cast<RemoteTrampoline*>(PyCObject_AsVoidPtr(self));
    int nas= PyTuple_GET_SIZE(args);
    TVec<PythonObjectWrapper> as;
    for(int i= 0; i < nas; ++i)
        as.push_back(PythonObjectWrapper(PyTuple_GET_ITEM(args, i)));
    try
    {
        PythonObjectWrapper returned_value= t->call(0, as);
        PyObject* to_return= returned_value.getPyObject();
        Py_XINCREF(to_return);
        return to_return;
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
                        "Caught unknown C++ exception");
        return 0;
    }
}

// the global funcs (storage never reclaimed)
static PObjectPool<PyMethodDef> pyfuncs(50);
static TVec<string> funcs_help;


void injectPLearnGlobalFunctions(PyObject* env)
{
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
        py_method->ml_meth= pythonGlobalFuncTramp;
        py_method->ml_flags= METH_VARARGS;
        funcs_help.push_back(
            HelpSystem::helpOnFunction(it->first.first.c_str(), 
                                       it->first.second));
        py_method->ml_doc= const_cast<char*>(funcs_help.last().c_str());
    
        /* module= env if env is a module; NULL otherwise */
        PyObject* module= 0;
        if(PyModule_Check(env))
            module= env;

        // N.B.: module == NULL works on python2.3, 2.4 and 2.5, but is not
        // documented
        PyObject* pyfunc= 
            PyCFunction_NewEx(py_method, self, module);
	    
        if(pyfunc) 
            PyObject_SetAttrString(env, 
                                   py_method->ml_name, 
                                   pyfunc);
        else
            PLERROR("Cannot inject global function "
                    "'%s' into python.",
                    py_method->ml_name);
    }
}


// Init func for python module.
// init module, then inject global funcs
void initPythonExtensionModule(char* module_name)
{
    PythonObjectWrapper::initializePython();
    PyObject* plext= Py_InitModule(module_name, NULL);
    injectPLearnGlobalFunctions(plext);
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
