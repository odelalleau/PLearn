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

// From PLearn
#include <plearn/misc/EmbeddedPython.h>
#include <plearn/io/fileutils.h>


namespace PLearn {
using namespace std;


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
    "\"living state\", used to carry information across time-steps.  This state is\n"
    "reset when resetInternalState() is called.\n"
    );
  

PythonCodeSnippet::PythonCodeSnippet(const string& code)
    : inherited(),
      m_code(code),
      m_remap_python_exceptions(false),
      m_compiled_code()
{
    // Compile the code if any
    build();
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
    if (m_code != "")
        m_compiled_code = compileGlobalCode(m_code);
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
}


//#####  Global Environment Interface  ########################################

PythonObjectWrapper
PythonCodeSnippet::getGlobalObject(const string& object_name) const
{
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

bool PythonCodeSnippet::isCallable(const char* function_name) const
{
    PyObject* pFunc = PyDict_GetItemString(m_compiled_code.getPyObject(),
                                           function_name);
    // pFunc: Borrowed reference

    return pFunc && PyCallable_Check(pFunc);
}


// Zero-argument function call
PythonObjectWrapper
PythonCodeSnippet::callFunction(const char* function_name) const
{
    PyObject* pFunc = PyDict_GetItemString(m_compiled_code.getPyObject(),
                                           function_name);
    // pFunc: Borrowed reference

    PyObject* return_value = 0;
    if (pFunc && PyCallable_Check(pFunc)) {
        return_value = PyObject_CallObject(pFunc, NULL);
        if (! return_value)
            handlePythonErrors();
    }
    else
        PLERROR("PythonCodeSnippet::callFunction: cannot call function '%s'",
                function_name);

    return PythonObjectWrapper(return_value);
}


//#####  Miscellaneous Functions  #############################################

PythonObjectWrapper PythonCodeSnippet::compileGlobalCode(const string& code) const
{
    PyObject* globals = PyDict_New();
    PyDict_SetItemString(globals, "__builtins__", PyEval_GetBuiltins());

    if (code != "") {
        PyRun_String(code.c_str(), Py_file_input /* exec code block */,
                     globals, globals);
        if (PyErr_Occurred()) {
            Py_XDECREF(globals);
            PyErr_Print();
            PLERROR("PythonCodeSnippet::resetInternalState: error compiling Python code\n"
                    "contained in the 'init_code' option.");
        }
    }
    return PythonObjectWrapper(globals);
}


void PythonCodeSnippet::handlePythonErrors() const
{
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
