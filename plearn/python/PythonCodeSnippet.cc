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
#include <plearn/base/tostring.h>


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
      m_instance_params(),
      m_instance(),
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

    declareOption(
        ol, "instance_params", &PythonCodeSnippet::m_instance_params,
        OptionBase::buildoption,
        "If this snippet represents a python object, these are the\n"
        "parameters passed to the object's constructor.");

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

    PyObject* pFunc= 0;
    bool instance_method= false;
    char* fn= new char[strlen(function_name)+1];
    strcpy(fn, function_name);
    if(!m_instance.isNull())
        pFunc= PyObject_GetAttrString(m_instance.getPyObject(),
                                      fn);
    delete[] fn;
    if(pFunc) 
        instance_method= true;
    else
        pFunc= PyDict_GetItemString(m_compiled_code.getPyObject(),
                                    function_name);
    // pFunc: Borrowed reference if not instance_method
    bool ret= pFunc && PyCallable_Check(pFunc);
    if(instance_method) Py_DECREF(pFunc);
    return ret;
}


// Zero-argument function call
PythonObjectWrapper
PythonCodeSnippet::invoke(const char* function_name) const
{
    PythonGlobalInterpreterLock gil;         // For thread-safety

    PyObject* pFunc= 0;
    bool instance_method= false;
    char* fn= new char[strlen(function_name)+1];
    strcpy(fn, function_name);
    if(!m_instance.isNull())
        pFunc= PyObject_GetAttrString(m_instance.getPyObject(),
                                      fn);
    delete[] fn;
    if(pFunc) 
        instance_method= true;
    else
        pFunc= PyDict_GetItemString(m_compiled_code.getPyObject(),
                                    function_name);

    // pFunc: Borrowed reference if not instance_method

    PyObject* return_value = 0;
    if (pFunc && PyCallable_Check(pFunc)) {
        setCurrentSnippet(m_handle);

        return_value = PyObject_CallObject(pFunc, NULL);
        if (! return_value)
        {
            if(instance_method) 
                Py_DECREF(pFunc);
            handlePythonErrors(string("Error while calling function '")
                               + function_name
                               + "' with no params.");
        }

        resetCurrentSnippet();
    }
    else
    {
        if(instance_method) Py_DECREF(pFunc);
        PLERROR("PythonCodeSnippet::invoke: cannot call function '%s' (not callable).",
                function_name);
    }

    if(instance_method) Py_DECREF(pFunc);
    return PythonObjectWrapper(return_value);
}


// N-argument function call
PythonObjectWrapper
PythonCodeSnippet::invoke(const char* function_name,
                          const TVec<PythonObjectWrapper>& args) const
{
    PythonGlobalInterpreterLock gil;         // For thread-safety

    PyObject* pFunc= 0;
    bool instance_method= false;
    char* fn= new char[strlen(function_name)+1];
    strcpy(fn, function_name);
    if(!m_instance.isNull())
        pFunc= PyObject_GetAttrString(m_instance.getPyObject(),
                                      fn);
    delete[] fn;
    if(pFunc) 
        instance_method= true;
    else
        pFunc= PyDict_GetItemString(m_compiled_code.getPyObject(),
                                    function_name);

    // pFunc: Borrowed reference if not instance_method

    PyObject* return_value = 0;
    if (pFunc && PyCallable_Check(pFunc)) {        
        setCurrentSnippet(m_handle);
        
        // Create argument tuple.  Warning: PyTuple_SetItem STEALS references.
        PyObject* pArgs = PyTuple_New(args.size());
        for (int i=0, n=args.size() ; i<n ; ++i)
        {
            PyTuple_SetItem(pArgs, i, args[i].getPyObject());
            Py_INCREF(args[i].getPyObject());
        }
        
        return_value = PyObject_CallObject(pFunc, pArgs);

        Py_DECREF(pArgs);
        if (! return_value)
        {
            if(instance_method) 
                Py_DECREF(pFunc);
            handlePythonErrors(string("Error while calling function '")
                               + function_name
                               + "' with " 
                               + tostring(args.length())
                               + " params.");
        }
        resetCurrentSnippet();        
    }
    else
    {
        if(instance_method) 
            Py_DECREF(pFunc);
        PLERROR("PythonCodeSnippet::invoke: cannot call function '%s'",
                function_name);
    }

    if(instance_method) 
        Py_DECREF(pFunc);
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
            args_tvec[i]= 
                PythonObjectWrapper(PyTuple_GET_ITEM(args,i),
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
        PyErr_SetString(PyExc_Exception, 
                        (string("PLearn Error: ")+e.message()).c_str());
        return NULL;
    }
    // Catch C++ stdlib exceptions
    catch (const std::exception& e) {
        PyErr_SetString(PyExc_Exception, 
                        (string("C++ stdlib error: ")+e.what()).c_str());
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
        if(!m_instance.isNull())
        {
            char* fn= new char[strlen(python_name)+1];
            strcpy(fn, python_name);
            PyObject_SetAttrString(m_instance.getPyObject(), 
                                   fn, py_funcobj);
            delete[] fn;
        }
        else
        {
            // Publish the injection in the '__injected__' dictionary for imported modules
            PythonObjectWrapper inj_dict = this->getGlobalObject("__injected__");
            PyDict_SetItemString(inj_dict.getPyObject(), python_name, py_funcobj);
            
            Py_XDECREF(self);
        }
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

PythonObjectWrapper PythonCodeSnippet::compileGlobalCode(const string& code) //const
{
    PythonGlobalInterpreterLock gil;         // For thread-safety

    PyObject* globals = PyDict_New();
    PyDict_SetItemString(globals, "__builtins__", PyEval_GetBuiltins());

    //always include EmbeddedCodeSnippet to check for an object to instantiate
    string extracode= "\nfrom plearn.pybridge.embedded_code_snippet "
        "import EmbeddedCodeSnippet\n";

    if (code != "") {
#ifdef WIN32
        // Under Windows, it appears the Python code will not execute with
        // Windows carriage returns. Thus we first make a copy of the code and
        // replace any carriage return by a Unix one.
        string code_copy = code;
        PLearn::search_replace(code_copy, "\r\n", "\n");
#else
        const string& code_copy = code;
#endif
        PyRun_String((code_copy+extracode).c_str(),
                     Py_file_input /* exec code block */,
                     globals, globals);
        if (PyErr_Occurred()) {
            Py_XDECREF(globals);
            PyErr_Print();
            PLERROR("in PythonCodeSnippet::compileGlobalCode : error compiling "
                    "Python code contained in the 'code' option.");
        }
    }

    //try to find an EmbeddedCodeSnippet to instantiate
    PythonObjectWrapper wrapped_globals(globals);
    Py_XDECREF(globals);
    map<string, PyObject*> global_map= 
        wrapped_globals.as<map<string, PyObject*> >();

    PyObject* snippet_found= 0;
    map<string, PyObject*>::iterator it_id= 
        global_map.find("pl_embedded_code_snippet_type");

    if(it_id != global_map.end())
        snippet_found= it_id->second;
    else //check for a single class deriving from EmbeddedCodeSnippet
    {
        list<pair<string, PyObject*> > classes_found;

        //iter (find)
        PyTypeObject* embedded_code_snippet_type= 
            (PyTypeObject*)global_map["EmbeddedCodeSnippet"];

        //find all classes deriving from EmbeddedCodeSnippet
        for(map<string, PyObject*>::iterator it= global_map.begin();
            it != global_map.end(); ++it)
        {
            if(PyType_Check(it->second)
               && 0 != PyObject_Compare(it->second, 
                                        (PyObject*)embedded_code_snippet_type)
               && PyType_IsSubtype((PyTypeObject*)it->second, 
                                   embedded_code_snippet_type))
            {
                classes_found.push_back(*it);
            }
        }

        int nclasses= classes_found.size();
        list<pair<string, PyObject*> >::iterator jt= classes_found.begin();
        if(nclasses > 1)
        {
            string classes_list= jt->first;
            for(++jt; jt != classes_found.end(); ++jt)
                classes_list+= string(", ") + jt->first;
            PLERROR("in PythonCodeSnippet::compileGlobalCode : "
                    "more than one class derives from EmbeddedCodeSnippet "
                    "and pl_embedded_code_snippet_type is not defined. "
                    "classes= [%s]",
                    classes_list.c_str());
        }
        if(nclasses == 1)
            snippet_found= jt->second;
    }
    
    if(snippet_found)
    {//instantiate object of appropriate type
        PyObject* pyparams= PyDict_New();
        if(!pyparams)
            handlePythonErrors();
        for(map<string, string>::const_iterator it= m_instance_params.begin();
            it != m_instance_params.end(); ++it)
        {// fill kwargs
            PyObject* val= PyString_FromString(it->second.c_str());
            PyDict_SetItemString(pyparams, it->first.c_str(), val);
            Py_DECREF(val);
        }

        if(!PyCallable_Check(snippet_found))
            PLERROR("in PythonCodeSnippet::compileGlobalCode : "
                    "found something that is not callable [not a class?]");

        PyObject* pargs= PyTuple_New(0);
        PyObject* the_obj= PyObject_Call(snippet_found, pargs, pyparams);
        Py_DECREF(pyparams);
        Py_DECREF(pargs);
        if(!the_obj)
        {
            if (PyErr_Occurred())
                PyErr_Print();
            PLERROR("in PythonCodeSnippet::compileGlobalCode : "
                    "found subclass of EmbeddedCodeSnippet, but can't "
                    "call constructor with given params.  "
                    "class='%s', params=%s", 
                    ((PyTypeObject*)snippet_found)->tp_name, 
                    tostring(m_instance_params).c_str());
        }
        m_instance= PythonObjectWrapper(the_obj);
    }

    return wrapped_globals;
}

void PythonCodeSnippet::run()
{
    if(m_instance.isNull())
        PLERROR("in PythonCodeSnippet::run : this snippet is not "
                "an instance of EmbeddedCodeSnippet");
    if(!PyCallable_Check(m_instance.getPyObject()))
        PLERROR("in PythonCodeSnippet::run : this instance of "
                "EmbeddedCodeSnippet is not callable.");
    PyObject* pargs= PyTuple_New(0);
    PyObject* res= PyObject_Call(m_instance.getPyObject(), pargs, 0);

    Py_DECREF(pargs);
    if(!res) handlePythonErrors();
    Py_XDECREF(res);
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

void PythonCodeSnippet::handlePythonErrors(const string& extramsg) const
{
    PythonGlobalInterpreterLock gil;         // For thread-safety
    if (PyErr_Occurred()) {
        if (m_remap_python_exceptions) {

            // format using cgitb, throw as PythonError (PLearnError)
            PyObject *exception, *v, *traceback;
            PyErr_Fetch(&exception, &v, &traceback);
            PyErr_NormalizeException(&exception, &v, &traceback);

            if(!traceback)
            {
                //perr << "$$$$ before print" << endl;
                PyErr_Print();
                //perr << "$$$$ after print" << endl;
                throw PythonException(string("PythonCodeSnippet: encountered Python "
                                             "exception but there is no traceback.\n")
                                      + extramsg);
            }
            

            PyObject* tbstr= 
                PyString_FromString("plearn.utilities.pltraceback");
            PyObject* tbmod= PyImport_Import(tbstr);
            Py_XDECREF(tbstr);
            if(!tbmod)
                throw PythonException("PythonCodeSnippet::handlePythonErrors :"
                                      " Unable to import cgitb module.");
            PyObject* tbdict= PyModule_GetDict(tbmod);
            Py_XDECREF(tbmod);
            PyObject* formatFunc= PyDict_GetItemString(tbdict, "text");
            if(!formatFunc)
                throw PythonException("PythonCodeSnippet::handlePythonErrors :"
                                      " Can't find cgitb.text");
            PyObject* args= Py_BuildValue("((OOO))", exception, v, traceback);
            if(!args)
                throw PythonException("PythonCodeSnippet::handlePythonErrors :"
                                      " Can't build args for cgitb.text");
            PyObject* pystr= PyObject_CallObject(formatFunc, args);
            Py_XDECREF(args);
            if(!pystr)
                throw PythonException("PythonCodeSnippet::handlePythonErrors :"
                                      " call to cgitb.text failed");
            string str= PyString_AsString(pystr);
            Py_XDECREF(pystr);
            
            PyErr_Clear();

            Py_XDECREF(exception);
            Py_XDECREF(v);
            Py_XDECREF(traceback);
            throw PythonException(str+extramsg);
        }
        else {
            PyErr_Print();
            PLERROR("PythonCodeSnippet: encountered Python exception.\n%s", 
                    extramsg.c_str());
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
