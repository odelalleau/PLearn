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
#include <plearn/python/PythonGlobalInterpreterLock.h>
#include <plearn/python/PythonObjectWrapper.h>

// Boost stuff
#include <boost/function.hpp>
#include <boost/bind.hpp>

// PLearn stuff
#include <plearn/base/Object.h>
#include <plearn/base/plexceptions.h>
#include <plearn/base/PMemPool.h>


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
 *  "living state", used to carry information across calls to Python functions.
 *
 *  A note on exception behavior within the PythonCodeSnippet:
 *
 *  - Exceptions that are raised within executed Python code are handled
 *    according to the 'remap_python_exceptions' option.  Basically, client
 *    code to the PythonCodeSnippet has the choice of either generating a
 *    PLERROR from the Python Exception, or of remapping this exception into
 *    a C++ exception (of class PythonException, subclass of PLearnError).
 *
 *  - C++ exceptions that are thrown from inside injected code functions
 *    are remapped into Python exceptions by the trampoline handler.  These
 *    Python exceptions are then handled according to the behavior in the
 *    previous point.  Note that, for now, all C++ exceptions are turned into
 *    a generic Python 'Exception' (base class for all exceptions).
 *
 *  The current implementation of the PythonCodeSnippet is designed to be
 *  thread-safe, i.e. the Python Global Interpreter Lock is always acquired
 *  before sensitive operations are carried out.
 */
class PythonCodeSnippet : public Object
{
    typedef Object inherited;

public:
    /**
     *  Typedef for an external C function that can be injected into the Python
     *  environment.
     */
    typedef boost::function<PythonObjectWrapper (
        const TVec<PythonObjectWrapper>& args)> StandaloneFunction;

    //! The snippet prepended to 'code' option for the injections to behave properly.
    static const char* InjectSetupSnippet;

    //! Used to (un)set CURRENT_SNIPPET in Python
    static const char* SetCurrentSnippetVar;
    static const char* ResetCurrentSnippetVar;

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

    //! parameters to the python ctor
    map<string, string> m_instance_params;

    //! the python object instance
    PythonObjectWrapper m_instance;

public:
    //! Default constructor.  Note that "build" IS NOT CALLED from the
    //! constructor and must be called manually after all external functions
    //! have been injected (if necessary).
    PythonCodeSnippet(const string& code = "",
                      bool remap_python_exceptions = false);

    PythonCodeSnippet(const PythonObjectWrapper& instance,
                      bool remap_python_exceptions = false);

    //! Default copy ctor, assignment op, dtor


    //#####  Global Environment Interface  ####################################

    //! Return an object from the global environment.  Return None if the
    //! object cannot be found.
    PythonObjectWrapper getGlobalObject(const string& object_name) const;

    //! Set an object into the global environment.
    void setGlobalObject(const string& object_name,
                         const PythonObjectWrapper& pow);
    template<typename T>
    void setGlobalObject(const string& object_name,
                         const T& o)
    {
        setGlobalObject(object_name, PythonObjectWrapper(o));
    }



    //#####  Function Call Interface  #########################################

    //! Checks whether the specified function name is callable
    bool isInvokable(const char* function_name) const;

    //! Call the specified function taking 0 arguments.
    PythonObjectWrapper invoke(const char* function_name) const;

    //! Call the specified function taking n arguments.  NOTE: the
    //! PythonObjectWrapper passed as arguments MUST be created with the
    //! 'transfer_ownership' option.
    PythonObjectWrapper invoke(const char* function_name,
                               const TVec<PythonObjectWrapper>& args) const;

    //! Call the specified function with 1 argument.
    template <class T>
    PythonObjectWrapper invoke(const char* function_name,
                               const T& arg1) const;

    //! Call the specified function with 2 arguments.
    template <class T, class U>
    PythonObjectWrapper invoke(const char* function_name,
                               const T& arg1,
                               const U& arg2) const;

    //! Call the specified function with 3 arguments.
    template <class T, class U, class V>
    PythonObjectWrapper invoke(const char* function_name,
                               const T& arg1,
                               const U& arg2,
                               const V& arg3) const;

    //! Call the specified function with 4 arguments.
    template <class T, class U, class V, class W>
    PythonObjectWrapper invoke(const char* function_name,
                               const T& arg1,
                               const U& arg2,
                               const V& arg3,
                               const W& arg4) const;

    //! Call the specified function with 5 arguments.
    template <class T, class U, class V, class W, class X>
    PythonObjectWrapper invoke(const char* function_name,
                               const T& arg1,
                               const U& arg2,
                               const V& arg3,
                               const W& arg4,
                               const X& arg5) const;

    //! Call the specified function with 6 arguments.
    template <class T, class U, class V, class W, class X, class Y>
    PythonObjectWrapper invoke(const char* function_name,
                               const T& arg1,
                               const U& arg2,
                               const V& arg3,
                               const W& arg4,
                               const X& arg5,
                               const Y& arg6) const;

    //! Call the specified function with 7 arguments.
    template <class T, class U, class V, class W, class X, class Y, class Z>
    PythonObjectWrapper invoke(const char* function_name,
                               const T& arg1,
                               const U& arg2,
                               const V& arg3,
                               const W& arg4,
                               const X& arg5,
                               const Y& arg6,
                               const Z& arg7) const;


    //#####  Function Injection Interface  ####################################

    /**
     *  Inject into the Python code the specified stand-alone function object
     *  under the given name.  Note that the function must take accept a TVec
     *  of PythonObjectWrapper (the arguments) and return a
     *  PythonObjectWrapper.  The return value needs to have controlled
     *  ownership (the default).  You can use the Boost bind library in order
     *  to transform a class member function into such a stand-alone function.
     *  Note that the PythonCodeSnippet must have been compiled (with build)
     *  BEFORE injecting the desired functions.  A typical idiom would be :
     *
     *      @code
     *      PP<PythonCodeSnippet> python = new PythonCodeSnippet(my_code);
     *      python->build();
     *      python->inject("func1", my_func1);
     *      python->inject("func2", my_func2);
     *      // ...
     *      @endcode
     *
     *  Note that a new call to build() would have the effect of "forgetting"
     *  the injections, so they have to be carried out again.
     */
    void inject(const char* python_name, StandaloneFunction function_ptr);

    /**
     *  Inject a bound C++ member function into the Python code under the given
     *  name (const version).
     */
    template <class T>
    void inject(const char* python_name, const T* object,
                PythonObjectWrapper (T::*)(const TVec<PythonObjectWrapper>&) const);

    /**
     *  Inject a bound C++ member function into the Python code under the given
     *  name (non-const version).
     */
    template <class T>
    void inject(const char* python_name, T* object,
                PythonObjectWrapper (T::*)(const TVec<PythonObjectWrapper>&));


    /**
     *  Produces a dump of the Python compiled code object to stderr; for
     *  debugging purposes.
     */
    void dumpPythonEnvironment();


    //#####  PLearn::Object Standard Functions  ###############################

    // Declares other standard object methods
    PLEARN_DECLARE_OBJECT(PythonCodeSnippet);

    // simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    virtual void run();

protected:
    //! Declares this class' options
    static void declareOptions(OptionList& ol);

    //! Compile a code block into a new environment and return it.
    //! Call PLERROR if the code contains an error.
    PythonObjectWrapper compileGlobalCode(const string& code); //const;

    void setCurrentSnippet(const void* handle) const;
    void resetCurrentSnippet() const;

    //! If no Python error, do nothing.  If an error occurred, convert the
    //! Python Exception into a C++ exception if required, or otherwise print a
    //! traceback and abort
    void handlePythonErrors(const string& extramsg= "") const;

    //! This is the trampoline function actually called by Python
    static PyObject* pythonTrampoline(PyObject* self, PyObject* args);

    //! This performs the low-level injection into the compiled Python code.
    //! Note that the pointer to StandaloneFunction must remain valid for the
    //! entire duration of the compiled code validity.
    void injectInternal(const char* python_name, StandaloneFunction* function_ptr);

protected:
    //! The Python handle for *this* instance
    void* m_handle;

public:
    //! Compiled Python code module and global environment
    PythonObjectWrapper m_compiled_code;

protected:
    //! Functions to be injected into the compiled Python code
    PObjectPool<StandaloneFunction> m_injected_functions;

    //! Injected Python method definitions
    PObjectPool<PyMethodDef> m_python_methods;

private:
    //! This does the actual building.  This is where the Python code
    //! is in fact compiled
    void build_();
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(PythonCodeSnippet);


//#####  Implementation of call  ##############################################

template <class T>
PythonObjectWrapper
PythonCodeSnippet::invoke(const char* function_name,
                          const T& arg1) const
{
    TVec<PythonObjectWrapper> args(1);
    args[0]= PythonObjectWrapper(arg1);
    return invoke(function_name, args);
}


template <class T, class U>
PythonObjectWrapper
PythonCodeSnippet::invoke(const char* function_name,
                          const T& arg1,
                          const U& arg2) const
{
    TVec<PythonObjectWrapper> args(2);
    args[0]= PythonObjectWrapper(arg1);
    args[1]= PythonObjectWrapper(arg2);
    return invoke(function_name, args);
}


template <class T, class U, class V>
PythonObjectWrapper
PythonCodeSnippet::invoke(const char* function_name,
                          const T& arg1,
                          const U& arg2,
                          const V& arg3) const
{
    TVec<PythonObjectWrapper> args(3);
    args[0]= PythonObjectWrapper(arg1);
    args[1]= PythonObjectWrapper(arg2);
    args[2]= PythonObjectWrapper(arg3);
    return invoke(function_name, args);
}


template <class T, class U, class V, class W>
PythonObjectWrapper
PythonCodeSnippet::invoke(const char* function_name,
                          const T& arg1,
                          const U& arg2,
                          const V& arg3,
                          const W& arg4) const
{
    TVec<PythonObjectWrapper> args(4);
    args[0]= PythonObjectWrapper(arg1);
    args[1]= PythonObjectWrapper(arg2);
    args[2]= PythonObjectWrapper(arg3);
    args[3]= PythonObjectWrapper(arg4);
    return invoke(function_name, args);
}

template <class T, class U, class V, class W, class X>
PythonObjectWrapper
PythonCodeSnippet::invoke(const char* function_name,
                          const T& arg1,
                          const U& arg2,
                          const V& arg3,
                          const W& arg4,
                          const X& arg5) const
{
    TVec<PythonObjectWrapper> args(5);
    args[0]= PythonObjectWrapper(arg1);
    args[1]= PythonObjectWrapper(arg2);
    args[2]= PythonObjectWrapper(arg3);
    args[3]= PythonObjectWrapper(arg4);
    args[4]= PythonObjectWrapper(arg5);
    return invoke(function_name, args);
}

template <class T, class U, class V, class W, class X, class Y>
PythonObjectWrapper
PythonCodeSnippet::invoke(const char* function_name,
                          const T& arg1,
                          const U& arg2,
                          const V& arg3,
                          const W& arg4,
                          const X& arg5,
                          const Y& arg6) const
{
    TVec<PythonObjectWrapper> args(6);
    args[0]= PythonObjectWrapper(arg1);
    args[1]= PythonObjectWrapper(arg2);
    args[2]= PythonObjectWrapper(arg3);
    args[3]= PythonObjectWrapper(arg4);
    args[4]= PythonObjectWrapper(arg5);
    args[5]= PythonObjectWrapper(arg6);
    return invoke(function_name, args);
}


template <class T, class U, class V, class W, class X, class Y, class Z>
PythonObjectWrapper
PythonCodeSnippet::invoke(const char* function_name,
                          const T& arg1,
                          const U& arg2,
                          const V& arg3,
                          const W& arg4,
                          const X& arg5,
                          const Y& arg6,
                          const Z& arg7) const
{
    TVec<PythonObjectWrapper> args(7);
    args[0]= PythonObjectWrapper(arg1);
    args[1]= PythonObjectWrapper(arg2);
    args[2]= PythonObjectWrapper(arg3);
    args[3]= PythonObjectWrapper(arg4);
    args[4]= PythonObjectWrapper(arg5);
    args[5]= PythonObjectWrapper(arg6);
    args[6]= PythonObjectWrapper(arg7);
    return invoke(function_name, args);
}


//#####  Implementation of inject  ############################################

template <class T>
void PythonCodeSnippet::inject(
    const char* python_name, const T* object,
    PythonObjectWrapper (T::*member_function)(const TVec<PythonObjectWrapper>&) const)
{
    StandaloneFunction func = boost::bind(member_function, object, _1);
    inject(python_name, func);
}

template <class T>
void PythonCodeSnippet::inject(
    const char* python_name, T* object,
    PythonObjectWrapper (T::*member_function)(const TVec<PythonObjectWrapper>&))
{
    StandaloneFunction func = boost::bind(member_function, object, _1);
    inject(python_name, func);
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
