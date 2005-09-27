// -*- C++ -*-

// PythonCodeSnippet.h
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
 * $Id: PythonCodeSnippet.h 2797 2005-08-25 14:06:26Z chapados $ 
 ******************************************************* */

// Authors: Nicolas Chapados

/*! \file PythonCodeSnippet.h */

#ifndef PythonCodeSnippet_INC
#define PythonCodeSnippet_INC

// Python stuff must be included first
#include <plearn/python/PythonObjectWrapper.h>

// PLearn stuff
#include <plearn/base/Object.h>
#include <plearn/base/plexceptions.h>


namespace PLearn {
using namespace std;

/**
 *  @class PythonException
 *  @brief C++ Exception object to which Python exceptions are mapped
 *
 *  If a Python exception is encountered during the execution of a function by
 *  the PythonCodeSnippet, a C++ exception of the type is thrown.
 */
class PythonException : public PLearnError
{
    typedef PLearnError inherited;
    
public:
    PythonException(const string& message)
        : inherited(message)
    { }
};


/**
 *  Enables embedded Python code to be called from PLearn/C++ code.
 *
 *  This class enables an embedded Python code snippet to be compiled and
 *  called back later.  It is not designed to be used by itself, but rather in
 *  conjunction with specific PLearn objects that understand the
 *  PythonCodeSnippet calling protocol.
 *
 *  Note that global variables can be used, in the Python code, to keep a
 *  "living state", used to carry information across time-steps.  This state is
 *  reset when resetInternalState() is called.
 */
class PythonCodeSnippet : public Object
{
    typedef Object inherited;

public:
    /**
     *  Python statement list that should be compiled at build time to provide
     *  the desired functions (defined by the client code to PythonCodeSnippet)
     *  and otherwise set up the Python global namespace.  Note that the Python
     *  '__builtins__' module is always injected into the global namespace.
     *  You should also add the statement
     *
     *      from numarray import *'
     *
     *  to manipulate PLearn Vec and Mat.
     */
    string m_code;

    /**
     *  If true, Python exceptions raised during function execution are mapped
     *  to a C++ exception.  If false, then a normal Python stack dump is
     *  output to stderr and a PLERROR is raised.  Default=false
     */
    bool m_remap_python_exceptions;
    
public:
    //! Default constructor
    PythonCodeSnippet(const string& code = "",
                      bool remap_python_exceptions = false);

    //! Default copy ctor, assignment op, dtor

    
    //#####  Global Environment Interface  ####################################

    //! Return an object from the global environment.  Return None if the
    //! object cannot be found.
    PythonObjectWrapper getGlobalObject(const string& object_name) const;

    //! Set an object into the global environment.
    void setGlobalObject(const string& object_name,
                         const PythonObjectWrapper& pow);
    
    
    //#####  Function Call Interface  #########################################

    //! Checks whether the specified function name is callable
    bool isCallable(const char* function_name) const;

    //! Call the specified function taking 0 arguments.
    PythonObjectWrapper callFunction(const char* function_name) const;

    //! Call the specified function with 1 argument.
    template <class T>
    PythonObjectWrapper callFunction(const char* function_name,
                                     const T& arg1) const;
    
    //! Call the specified function with 2 arguments.
    template <class T, class U>
    PythonObjectWrapper callFunction(const char* function_name,
                                     const T& arg1,
                                     const U& arg2) const;
    
    //! Call the specified function with 3 arguments.
    template <class T, class U, class V>
    PythonObjectWrapper callFunction(const char* function_name,
                                     const T& arg1,
                                     const U& arg2,
                                     const V& arg3) const;
    
    //! Call the specified function with 4 arguments.
    template <class T, class U, class V, class W>
    PythonObjectWrapper callFunction(const char* function_name,
                                     const T& arg1,
                                     const U& arg2,
                                     const V& arg3,
                                     const W& arg4) const;
    

    //#####  PLearn::Object Standard Functions  ###############################

    // Declares other standard object methods
    PLEARN_DECLARE_OBJECT(PythonCodeSnippet);

    // simply calls inherited::build() then build_() 
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected: 
    //! Declares this class' options
    static void declareOptions(OptionList& ol);

    //! Compile a code block into a new environment and return it.
    //! Call PLERROR if the code contains an error.
    PythonObjectWrapper compileGlobalCode(const string& code) const;

    //! If no Python error, do nothing.  If an error occurred, convert the
    //! Python Exception into a C++ exception if required, or otherwise print a
    //! traceback and abort
    void handlePythonErrors() const;

protected:
    //! Compiled Python code module and global environment
    PythonObjectWrapper m_compiled_code;

private: 
    //! This does the actual building.  This is where the Python code
    //! is in fact compiled
    void build_();
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(PythonCodeSnippet);


//#####  Implementation of callFunction  ######################################

template <class T>
PythonObjectWrapper
PythonCodeSnippet::callFunction(const char* function_name,
                                const T& arg1) const
{
    PyObject* pFunc = PyDict_GetItemString(m_compiled_code.getPyObject(),
                                           function_name);
    // pFunc: Borrowed reference

    PyObject* return_value = 0;
    if (pFunc && PyCallable_Check(pFunc)) {
        // Create argument tuple.  Warning: PyTuple_SetItem STEALS references.
        PyObject* pArgs = PyTuple_New(1);
        PyObject* py_arg1 = PythonObjectWrapper::newPyObject(arg1);

        if (! (py_arg1)) {
            Py_XDECREF(py_arg1);
            PLERROR("PythonCodeSnippet::callFunction: error during argument conversion "
                    "from C++ to Python for function '%s'", function_name);
        }
        
        PyTuple_SetItem(pArgs, 0, py_arg1);
        
        return_value = PyObject_CallObject(pFunc, pArgs);
        Py_XDECREF(pArgs);
        if (! return_value)
            handlePythonErrors();
    }
    else
        PLERROR("PythonCodeSnippet::callFunction: cannot call function '%s'",
                function_name);

    return PythonObjectWrapper(return_value);
}


template <class T, class U>
PythonObjectWrapper
PythonCodeSnippet::callFunction(const char* function_name,
                                const T& arg1,
                                const U& arg2) const
{
    PyObject* pFunc = PyDict_GetItemString(m_compiled_code.getPyObject(),
                                           function_name);
    // pFunc: Borrowed reference

    PyObject* return_value = 0;
    if (pFunc && PyCallable_Check(pFunc)) {
        // Create argument tuple.  Warning: PyTuple_SetItem STEALS references.
        PyObject* pArgs = PyTuple_New(2);
        PyObject* py_arg1 = PythonObjectWrapper::newPyObject(arg1);
        PyObject* py_arg2 = PythonObjectWrapper::newPyObject(arg2);

        if (! (py_arg1 && py_arg2)) {
            Py_XDECREF(py_arg1);
            Py_XDECREF(py_arg2);
            PLERROR("PythonCodeSnippet::callFunction: error during argument conversion "
                    "from C++ to Python for function '%s'", function_name);
        }
        
        PyTuple_SetItem(pArgs, 0, py_arg1);
        PyTuple_SetItem(pArgs, 1, py_arg2);
        
        return_value = PyObject_CallObject(pFunc, pArgs);
        Py_XDECREF(pArgs);
        if (! return_value)
            handlePythonErrors();
    }
    else
        PLERROR("PythonCodeSnippet::callFunction: cannot call function '%s'",
                function_name);

    return PythonObjectWrapper(return_value);
}


template <class T, class U, class V>
PythonObjectWrapper
PythonCodeSnippet::callFunction(const char* function_name,
                                const T& arg1,
                                const U& arg2,
                                const V& arg3) const
{
    PyObject* pFunc = PyDict_GetItemString(m_compiled_code.getPyObject(),
                                           function_name);
    // pFunc: Borrowed reference

    PyObject* return_value = 0;
    if (pFunc && PyCallable_Check(pFunc)) {
        // Create argument tuple.  Warning: PyTuple_SetItem STEALS references.
        PyObject* pArgs = PyTuple_New(3);
        PyObject* py_arg1 = PythonObjectWrapper::newPyObject(arg1);
        PyObject* py_arg2 = PythonObjectWrapper::newPyObject(arg2);
        PyObject* py_arg3 = PythonObjectWrapper::newPyObject(arg3);

        if (! (py_arg1 && py_arg2 && py_arg3)) {
            Py_XDECREF(py_arg1);
            Py_XDECREF(py_arg2);
            Py_XDECREF(py_arg3);
            PLERROR("PythonCodeSnippet::callFunction: error during argument conversion "
                    "from C++ to Python for function '%s'", function_name);
        }
        
        PyTuple_SetItem(pArgs, 0, py_arg1);
        PyTuple_SetItem(pArgs, 1, py_arg2);
        PyTuple_SetItem(pArgs, 2, py_arg3);
        
        return_value = PyObject_CallObject(pFunc, pArgs);
        Py_XDECREF(pArgs);
        if (! return_value)
            handlePythonErrors();
    }
    else
        PLERROR("PythonCodeSnippet::callFunction: cannot call function '%s'",
                function_name);

    return PythonObjectWrapper(return_value);
}


template <class T, class U, class V, class W>
PythonObjectWrapper
PythonCodeSnippet::callFunction(const char* function_name,
                                const T& arg1,
                                const U& arg2,
                                const V& arg3,
                                const W& arg4) const
{
    PyObject* pFunc = PyDict_GetItemString(m_compiled_code.getPyObject(),
                                           function_name);
    // pFunc: Borrowed reference

    PyObject* return_value = 0;
    if (pFunc && PyCallable_Check(pFunc)) {
        // Create argument tuple.  Warning: PyTuple_SetItem STEALS references.
        PyObject* pArgs = PyTuple_New(4);
        PyObject* py_arg1 = PythonObjectWrapper::newPyObject(arg1);
        PyObject* py_arg2 = PythonObjectWrapper::newPyObject(arg2);
        PyObject* py_arg3 = PythonObjectWrapper::newPyObject(arg3);
        PyObject* py_arg4 = PythonObjectWrapper::newPyObject(arg4);

        if (! (py_arg1 && py_arg2 && py_arg3 && py_arg4)) {
            Py_XDECREF(py_arg1);
            Py_XDECREF(py_arg2);
            Py_XDECREF(py_arg3);
            Py_XDECREF(py_arg4);
            PLERROR("PythonCodeSnippet::callFunction: error during argument conversion "
                    "from C++ to Python for function '%s'", function_name);
        }
        
        PyTuple_SetItem(pArgs, 0, py_arg1);
        PyTuple_SetItem(pArgs, 1, py_arg2);
        PyTuple_SetItem(pArgs, 2, py_arg3);
        PyTuple_SetItem(pArgs, 3, py_arg4);
        
        return_value = PyObject_CallObject(pFunc, pArgs);
        Py_XDECREF(pArgs);
        if (! return_value)
            handlePythonErrors();
    }
    else
        PLERROR("PythonCodeSnippet::callFunction: cannot call function '%s'",
                function_name);

    return PythonObjectWrapper(return_value);
}

} // end of namespace PLearn

#endif


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
