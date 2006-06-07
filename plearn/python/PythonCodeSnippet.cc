// -*- C++ -*-

// PythonCodeSnippet.cc
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

/* *******************************************************      
 * $Id: PythonCodeSnippet.cc 2771 2005-08-11 22:06:13Z chapados $ 
 ******************************************************* */

// Authors: Nicolas Chapados

/*! \file PythonCodeSnippet.cc */

// Python stuff must be included first
#include "PythonCodeSnippet.h"
#include "PythonEmbedder.h"

// From PLearn
#include <plearn/io/fileutils.h>


namespace PLearn {
using namespace std;

// #if sizeof(long) < sizeof(void*)
// #error "Snippets' addresses need to be casted to long"
// #endif

const char* PythonCodeSnippet::InjectSetupSnippet =\
// Redefines import statement behavior
"from plearn.utilities import inject_import as _inject_import_\n"
// The dictionnary in which to inject
"__injected__ = {}\n";

const char* PythonCodeSnippet::SetCurrentSnippetVar =\
// Completed by sprintf using the snippet's address casted to a long
"_inject_import_.setCurrentSnippet(%d)\n";

const char* PythonCodeSnippet::ResetCurrentSnippetVar = \
"_inject_import_.resetCurrentSnippet()\n";

//#####  PythonCodeSnippet  ###################################################

PLEARN_IMPLEMENT_OBJECT(
    PythonCodeSnippet,
    "Enables embedded Python code to be called from PLearn/C++ code.",
    "This class enables an embedded Python code snippet to be compiled and\n"
    "called back later.  It is not designed to be used by itself, but rather in\n"
    "conjunction with specific PLearn objects that understand the\n"
    "PythonCodeSnippet calling protocol.\n"
    "\n"
    "Note that global variables can be used, in the Python code, to keep a\n"
    "\"living state\", used to carry information across calls to Python functions.\n"
    "\n"
    "A note on exception behavior within the PythonCodeSnippet:\n"
    "\n"
    "- Exceptions that are raised within executed Python code are handled\n"
    "  according to the 'remap_python_exceptions' option.  Basically, client\n"
    "  code to the PythonCodeSnippet has the choice of either generating a\n"
    "  PLERROR from the Python Exception, or of remapping this exception into\n"
    "  a C++ exception (of class PythonException, subclass of PLearnError).\n"
    "\n"
    "- C++ exceptions that are thrown from inside injected code functions\n"
    "  are remapped into Python exceptions by the trampoline handler.  These\n"
    "  Python exceptions are then handled according to the behavior in the\n"
    "  previous point.  Note that, for now, all C++ exceptions are turned into\n"
    "  a generic Python 'Exception' (base class for all exceptions).\n"
    "\n"
    "The current implementation of the PythonCodeSnippet is designed to be\n"
    "thread-safe, i.e. the Python Global Interpreter Lock is always acquired\n"
    "before sensitive operations are carried out.\n"
    );
  

PythonCodeSnippet::PythonCodeSnippet(const string& code,
                                     bool remap_python_exceptions)
    : inherited(),
      m_code(code),
      m_remap_python_exceptions(remap_python_exceptions),
      m_handle(long(this)),
      m_compiled_code(),
      m_injected_functions(4),
      m_python_methods(4)
{
    // NOTE: build() not called
}



void PythonCodeSnippet::declareOptions(OptionList& ol)
{
    declareOption(
        ol, "code", &PythonCodeSnippet::m_code,
        OptionBase::buildoption,
        "Python statement list that should be compiled at build time to provide\n"
        "the desired functions (defined by the client code to PythonCodeSnippet)\n"
        "and otherwise set up the Python global namespace.  Note that the Python\n"
        "'__builtins__' module is always injected into the global namespace.\n"
        "You should also add the statement\n"
        "\n"
        "    from numarray import *'\n"
        "\n"
        "to manipulate PLearn Vec and Mat.\n");

    declareOption(
        ol, "remap_python_exceptions", &PythonCodeSnippet::m_remap_python_exceptions,
        OptionBase::buildoption,
        "If true, Python exceptions raised during function execution are mapped\n"
        "to a C++ exception.  If false, then a normal Python stack dump is\n"
        "output to stderr and a PLERROR is raised.  Default=false.");
    
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void PythonCodeSnippet::build_()
{
    static PythonEmbedder python;
    static bool numarray_initialized = false;
    if (! numarray_initialized) {
        // must be in each translation unit that makes use of libnumarray;
        // weird stuff related to table of function pointers that's being
        // initialized into a STATIC VARIABLE of the translation unit!
        import_libnumarray();
        numarray_initialized = true;
        
        PythonObjectWrapper::initializePython();
    }

    // Compile code into global environment
    if (m_code != ""){
        // Here we don't call setCurrentSnippet() because it has to be called
        // between the setup and the m_code... Still have to call
        // resetCurrentSnippet() afterwards though.
        char set_current_snippet[100];
        sprintf(set_current_snippet, SetCurrentSnippetVar, m_handle);
        m_compiled_code = compileGlobalCode(InjectSetupSnippet+
                                            string(set_current_snippet)+m_code);
        resetCurrentSnippet();
    }
    
    // Forget about injected functions
    m_injected_functions.purge_memory();
    m_python_methods.purge_memory();
}

// ### Nothing to add here, simply calls build_
void PythonCodeSnippet::build()
{
    inherited::build();
    build_();
}

void PythonCodeSnippet::makeDeepCopyFromShallowCopy(
    CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // Compile fresh code into global environment
    m_compiled_code = compileGlobalCode(m_code);

    // Forget about injected functions (not necessarily the correct thing to do...)
    m_injected_functions.purge_memory();
    m_python_methods.purge_memory();
}


//#####  Global Environment Interface  ########################################

PythonObjectWrapper
PythonCodeSnippet::getGlobalObject(const string& object_name) const
{
    PythonGlobalInterpreterLock gil;         // For thread-safety
    PyObject* pyobj = PyDict_GetItemString(m_compiled_code.getPyObject(),
                                           object_name.c_str());
    if (pyobj) {
        // pyobj == borrowed reference
        // Increment refcount to keep long-lived reference
        Py_XINCREF(pyobj);
        return PythonObjectWrapper(pyobj);
    }
    return PythonObjectWrapper();            // None
}

void PythonCodeSnippet::setGlobalObject(const string& object_name,
                                        const PythonObjectWrapper& pow)
{
    PythonGlobalInterpreterLock gil;         // For thread-safety

    // Note that PyDict_SetItemString increments the reference count for us
    int non_success = 0;
    if (! pow.isNull())
        non_success = PyDict_SetItemString(m_compiled_code.getPyObject(),
                                           object_name.c_str(),
                                           pow.getPyObject());
    else
        non_success = PyDict_SetItemString(m_compiled_code.getPyObject(),
                                           object_name.c_str(),
                                           Py_None);

    if (non_success)
        PLERROR("PythonCodeSnippet::setGlobalObject: error inserting a global Python \n"
                "object under the name '%s'", object_name.c_str());
}


//#####  Function Call Interface  #############################################

bool PythonCodeSnippet::isInvokable(const char* function_name) const
{
    PythonGlobalInterpreterLock gil;         // For thread-safety
    PyObject* pFunc = PyDict_GetItemString(m_compiled_code.getPyObject(),
                                           function_name);
    // pFunc: Borrowed reference

    return pFunc && PyCallable_Check(pFunc);
}


// Zero-argument function call
PythonObjectWrapper
PythonCodeSnippet::invoke(const char* function_name) const
{
    PythonGlobalInterpreterLock gil;         // For thread-safety
    PyObject* pFunc = PyDict_GetItemString(m_compiled_code.getPyObject(),
                                           function_name);
    // pFunc: Borrowed reference

    PyObject* return_value = 0;
    if (pFunc && PyCallable_Check(pFunc)) {
        setCurrentSnippet(m_handle);

        return_value = PyObject_CallObject(pFunc, NULL);
        if (! return_value)
            handlePythonErrors();

        resetCurrentSnippet();
    }
    else
        PLERROR("PythonCodeSnippet::invoke: cannot call function '%s'",
                function_name);

    return PythonObjectWrapper(return_value);
}


// N-argument function call
PythonObjectWrapper
PythonCodeSnippet::invoke(const char* function_name,
                          const TVec<PythonObjectWrapper>& args) const
{
    PythonGlobalInterpreterLock gil;         // For thread-safety
    PyObject* pFunc = PyDict_GetItemString(m_compiled_code.getPyObject(),
                                           function_name);
    // pFunc: Borrowed reference

    PyObject* return_value = 0;
    if (pFunc && PyCallable_Check(pFunc)) {        
        setCurrentSnippet(m_handle);

        // Create argument tuple.  Warning: PyTuple_SetItem STEALS references.
        PyObject* pArgs = PyTuple_New(args.size());
        for (int i=0, n=args.size() ; i<n ; ++i)
            PyTuple_SetItem(pArgs, i, args[i].getPyObject());
        
        return_value = PyObject_CallObject(pFunc, pArgs);
        Py_XDECREF(pArgs);
        if (! return_value)
            handlePythonErrors();

        resetCurrentSnippet();        
    }
    else
        PLERROR("PythonCodeSnippet::invoke: cannot call function '%s'",
                function_name);

    return PythonObjectWrapper(return_value);
}


//#####  Function Injection Interface  ########################################

// This is the function actually called by Python.  Be careful to remap
// exceptions thrown by C++ into Python exceptions.
PyObject* PythonCodeSnippet::pythonTrampoline(PyObject* self, PyObject* args)
{
    PythonGlobalInterpreterLock gil;         // For thread-safety
    try {
        // Transform the args tuple into a TVec of not-owned PythonObjectWrapper
        if (! PyTuple_Check(args))
            PLERROR("PythonCodeSnippet.cc:python_trampoline: the Python interpreter "
                    "did not pass a Tuple as the arguments object.");

        int size = PyTuple_GET_SIZE(args);
        TVec<PythonObjectWrapper> args_tvec(size);
        for (int i=0 ; i<size ; ++i) {
            args_tvec[i] = PythonObjectWrapper(PyTuple_GET_ITEM(args,i),
                                               PythonObjectWrapper::transfer_ownership);
        }
        
        // Now get the void* stored within the PyCObject of self
        StandaloneFunction* func =
            static_cast<StandaloneFunction*>(PyCObject_AsVoidPtr(self));
        PythonObjectWrapper returned_value = (*func)(args_tvec);
        PyObject* to_return = returned_value.getPyObject();
        Py_XINCREF(to_return);

        return to_return;
    }

    // Catch PLERROR and such
    catch (const PLearnError& e) {
        PyErr_SetString(PyExc_Exception, e.message().c_str());
        return NULL;
    }
    // Catch C++ stdlib exceptions
    catch (const std::exception& e) {
        PyErr_SetString(PyExc_Exception, e.what());
        return NULL;
    }
    // Catch any other unexpected exceptions
    catch (...) {
        PyErr_SetString(PyExc_Exception,
                        "Caught unknown C++ exception while executing injected function "
                        "inside a PythonCodeSnippet");
        return NULL;
    }
}


// Bind "standalone functions" to a Python name
void PythonCodeSnippet::injectInternal(const char* python_name,
                                       StandaloneFunction* function_ptr)
{
    PythonGlobalInterpreterLock gil;         // For thread-safety

    // Wrap the function_ptr into a PyCObject
    PyObject* self = PyCObject_FromVoidPtr(function_ptr, NULL);
    
    // Create a Python Function Object
    PyMethodDef* py_method = m_python_methods.allocate();
    py_method->ml_name  = const_cast<char*>(python_name);
    py_method->ml_meth  = pythonTrampoline;
    py_method->ml_flags = METH_VARARGS;
    py_method->ml_doc   = "injected-function-from-PythonCodeSnippet";
    
    PyObject* py_funcobj = PyCFunction_NewEx(py_method,
                                             self /* info for trampoline */,
                                             NULL /* module */);
    
    if (py_funcobj) {
        // Inject into the running snippet.  Note that when a
        // PythonObjectWrapper is constructed from a PyObject, it steals the
        // refcount, so we don't need to perform a Py_XDECREF on py_funcobj.
        this->setGlobalObject(python_name, py_funcobj);

        // Publish the injection in the '__injected__' dictionary for imported modules
        PythonObjectWrapper inj_dict = this->getGlobalObject("__injected__");
        PyDict_SetItemString(inj_dict.getPyObject(), python_name, py_funcobj);
        
        Py_XDECREF(self);
    }
    else
        PLERROR("PythonCodeSnippet::injectInternal: failed to inject "
                "Python function '%s'", python_name);
}


// High-level injection interface
void PythonCodeSnippet::inject(const char* python_name,
                               StandaloneFunction function_ptr)
{
    StandaloneFunction* pfunc = m_injected_functions.allocate();
    new(pfunc) StandaloneFunction(function_ptr); // In-place copy constructor
    injectInternal(python_name, pfunc);
}


//#####  Miscellaneous Functions  #############################################

PythonObjectWrapper PythonCodeSnippet::compileGlobalCode(const string& code) const
{
    PythonGlobalInterpreterLock gil;         // For thread-safety

    PyObject* globals = PyDict_New();
    PyDict_SetItemString(globals, "__builtins__", PyEval_GetBuiltins());

    if (code != "") {
        PyRun_String(code.c_str(), Py_file_input /* exec code block */,
                     globals, globals);
        if (PyErr_Occurred()) {
            Py_XDECREF(globals);
            PyErr_Print();
            PLERROR("PythonCodeSnippet::resetInternalState: error compiling Python code\n"
                    "contained in the 'code' option.");
        }
    }
    return PythonObjectWrapper(globals);
}

void PythonCodeSnippet::setCurrentSnippet(const long& handle) const
{
    PythonGlobalInterpreterLock gil;         // For thread-safety
    
    char set_current_snippet[100];
    sprintf(set_current_snippet, SetCurrentSnippetVar, handle);    
    PyRun_String(set_current_snippet, Py_file_input /* exec code block */,
                 m_compiled_code.getPyObject(), m_compiled_code.getPyObject());

    if (PyErr_Occurred()) {
        Py_XDECREF(m_compiled_code.getPyObject());
        PyErr_Print();
        PLERROR("PythonCodeSnippet::setCurrentSnippet: error compiling "
                "Python code contained in the 'SetCurrentSnippetVar'.");
    }
}

void PythonCodeSnippet::resetCurrentSnippet() const
{
    PythonGlobalInterpreterLock gil;         // For thread-safety
    
    PyRun_String(ResetCurrentSnippetVar, Py_file_input /* exec code block */,
                 m_compiled_code.getPyObject(), m_compiled_code.getPyObject());

    if (PyErr_Occurred()) {
        Py_XDECREF(m_compiled_code.getPyObject());
        PyErr_Print();
        PLERROR("PythonCodeSnippet::resetCurrentSnippet: error compiling "
                "Python code contained in the 'ResetCurrentSnippetVar'.");
    }
}

void PythonCodeSnippet::handlePythonErrors() const
{
    PythonGlobalInterpreterLock gil;         // For thread-safety
    if (PyErr_Occurred()) {
        if (m_remap_python_exceptions) {
            PyObject *ptype = 0, *pvalue = 0, *ptraceback = 0;
            PyErr_Fetch(&ptype, &pvalue, &ptraceback);

            // Convert the exception type, value and traceback to Python string
            PyObject *ptype_str = 0, *pvalue_str = 0, *ptraceback_str = 0;
            if (ptype)
                ptype_str = PyObject_Str(ptype);
            if (pvalue)
                pvalue_str = PyObject_Str(pvalue);
            if (ptraceback)
                ptraceback_str = PyObject_Str(ptraceback);
            
            // From the strings we got, make a C++ string
            string msg = "Encountered Python Exception";
            if (ptype_str)
                msg += string("\nException Type: ") + PyString_AsString(ptype_str);
            if (pvalue_str)
                msg += string("\nException Value: ") + PyString_AsString(pvalue_str);
            if (ptraceback_str)
                msg += string("\nTraceback: ") + PyString_AsString(ptraceback_str);

            Py_XDECREF(ptype);
            Py_XDECREF(pvalue);
            Py_XDECREF(ptraceback);
            Py_XDECREF(ptype_str);
            Py_XDECREF(pvalue_str);
            Py_XDECREF(ptraceback_str);

            throw PythonException(msg);
        }
        else {
            PyErr_Print();
            PLERROR("PythonCodeSnippet: encountered Python exception.");
        }
    }  
}


void PythonCodeSnippet::dumpPythonEnvironment()
{
    PyObject_Print(m_compiled_code.getPyObject(), stderr, 0);
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
