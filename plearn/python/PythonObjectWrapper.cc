// -*- C++ -*-

// PythonObjectWrapper.cc
//
// Copyright (C) 2005-2006 Nicolas Chapados 
// Copyright (C) 2007 Xavier Saint-Mleux, ApSTAT Technologies inc.
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
   * $Id: .pyskeleton_header 544 2003-09-01 00:05:31Z plearner $ 
   ******************************************************* */

// Authors: Nicolas Chapados

/*! \file PythonObjectWrapper.cc */

// Must include Python first...
#include "PythonObjectWrapper.h"
#include "PythonEmbedder.h"

// From C/C++ stdlib
#include <stdio.h>
#include <algorithm>

// From PLearn
#include <plearn/base/plerror.h>
#include <plearn/vmat/VMat.h>
#include <plearn/base/RemoteTrampoline.h>

namespace PLearn {
using namespace std;

// Error-reporting
void PLPythonConversionError(const char* function_name,
                             PyObject* pyobj, bool print_traceback)
{
    if (print_traceback) {
        fprintf(stderr,"For python object: ");
        PyObject_Print(pyobj, stderr, Py_PRINT_RAW);
    }
    PLERROR("Cannot convert Python object using %s", function_name);
}

// Python initialization
void PythonObjectWrapper::initializePython()
{
    static bool numarray_initialized = false;
    if (! numarray_initialized) {
        // must be in each translation unit that makes use of libnumarray;
        // weird stuff related to table of function pointers that's being
        // initialized into a STATIC VARIABLE of the translation unit!
        import_libnumarray();
        numarray_initialized = true;
    }
}


//#####  ConvertFromPyObject  #################################################

PyObject* ConvertFromPyObject<PyObject*>::convert(PyObject* pyobj, bool print_traceback)
{
    PLASSERT( pyobj );
    return pyobj;
}

bool ConvertFromPyObject<bool>::convert(PyObject* pyobj, bool print_traceback)
{
    PLASSERT( pyobj );
    return PyObject_IsTrue(pyobj) != 0;
}

int ConvertFromPyObject<int>::convert(PyObject* pyobj, bool print_traceback)
{
    PLASSERT( pyobj );
    if (! PyInt_Check(pyobj))
        PLPythonConversionError("ConvertFromPyObject<int>", pyobj, print_traceback);
    return int(PyInt_AS_LONG(pyobj));
}

int ConvertFromPyObject<unsigned int>::convert(PyObject* pyobj, bool print_traceback)
{
    PLASSERT( pyobj );
    if (! PyInt_Check(pyobj))
        PLPythonConversionError("ConvertFromPyObject<unsigned int>", pyobj, print_traceback);
    return static_cast<unsigned int>(PyInt_AsUnsignedLongMask(pyobj));
}

long ConvertFromPyObject<long>::convert(PyObject* pyobj, bool print_traceback)
{
    PLASSERT( pyobj );
    if (! PyLong_Check(pyobj))
        PLPythonConversionError("ConvertFromPyObject<long>", pyobj, print_traceback);
    return PyLong_AsLong(pyobj);
}

real ConvertFromPyObject<real>::convert(PyObject* pyobj, bool print_traceback)
{
    PLASSERT( pyobj );
    if (! PyFloat_Check(pyobj))
        PLPythonConversionError("ConvertFromPyObject<real>", pyobj,
                                print_traceback);
    return (real)PyFloat_AS_DOUBLE(pyobj);
}

string ConvertFromPyObject<string>::convert(PyObject* pyobj, bool print_traceback)
{
    PLASSERT( pyobj );
    if (! PyString_Check(pyobj))
        PLPythonConversionError("ConvertFromPyObject<string>", pyobj,
                                print_traceback);
    return PyString_AsString(pyobj);
}

PPath ConvertFromPyObject<PPath>::convert(PyObject* pyobj, bool print_traceback)
{
    PLASSERT( pyobj );
    if (! PyString_Check(pyobj))
        PLPythonConversionError("ConvertFromPyObject<PPath>", pyobj,
                                print_traceback);
    return PPath(PyString_AsString(pyobj));
}

PPointable* ConvertFromPyObject<PPointable*>::convert(PyObject* pyobj, bool print_traceback)
{
    PLASSERT(pyobj);
    if (! PyCObject_Check(pyobj))
        PLPythonConversionError("ConvertFromPyObject<PPointable*>", pyobj,
                                print_traceback);
    return static_cast<PPointable*>(PyCObject_AsVoidPtr(pyobj));
}

Object* ConvertFromPyObject<Object*>::convert(PyObject* pyobj, bool print_traceback)
{
    PLASSERT(pyobj);

    PyObject* cptr= PyObject_GetAttrString(pyobj, "_cptr");

    if (! PyCObject_Check(cptr))
        PLPythonConversionError("ConvertFromPyObject<Object*>", pyobj,
                                print_traceback);
    Object* obj= static_cast<Object*>(PyCObject_AsVoidPtr(cptr));
   
    Py_DECREF(cptr);
    return obj;
}

void ConvertFromPyObject<Vec>::convert(PyObject* pyobj, Vec& v, bool print_traceback)
{
    // NA_InputArray possibly creates a well-behaved temporary (i.e. not
    // discontinuous is memory)
    PLASSERT( pyobj );
    PyArrayObject* pyarr = NA_InputArray(pyobj, tReal, NUM_C_ARRAY);
    if (! pyarr)
        PLPythonConversionError("ConvertFromPyObject<Vec>", pyobj, print_traceback);
    if (pyarr->nd != 1)
        PLERROR("ConvertFromPyObject<Vec>: Dimensionality of the returned array "
                "should be 1; got %d", pyarr->nd);

    v.resize(pyarr->dimensions[0]);
    v.copyFrom((real*)(NA_OFFSETDATA(pyarr)), pyarr->dimensions[0]);
    Py_XDECREF(pyarr);
}

Vec ConvertFromPyObject<Vec>::convert(PyObject* pyobj, bool print_traceback)
{
    Vec v;
    convert(pyobj, v, print_traceback);
    return v;
}

void ConvertFromPyObject<Mat>::convert(PyObject* pyobj, Mat& m, bool print_traceback)
{
    // NA_InputArray possibly creates a well-behaved temporary (i.e. not
    // discontinuous is memory)
    PLASSERT( pyobj );
    PyArrayObject* pyarr = NA_InputArray(pyobj, tReal, NUM_C_ARRAY);
    if (! pyarr)
        PLPythonConversionError("ConvertFromPyObject<Mat>", pyobj, print_traceback);
    if (pyarr->nd != 2)
        PLERROR("ConvertFromPyObject<Mat>: Dimensionality of the returned array "
                "should be 2; got %d", pyarr->nd);

    m.resize(pyarr->dimensions[0], pyarr->dimensions[1]);
    m.toVec().copyFrom((real*)(NA_OFFSETDATA(pyarr)),
                       pyarr->dimensions[0] * pyarr->dimensions[1]);
    Py_XDECREF(pyarr);
}

Mat ConvertFromPyObject<Mat>::convert(PyObject* pyobj, bool print_traceback)
{
    Mat m;
    convert(pyobj, m, print_traceback);
    return m;
}

VMat ConvertFromPyObject<VMat>::convert(PyObject* pyobj, bool print_traceback)
{
    try
    {
        return static_cast<VMatrix*>(
            ConvertFromPyObject<Object*>::convert(pyobj, print_traceback));
    }
    catch(...)
    {
        Mat m;
        ConvertFromPyObject<Mat>::convert(pyobj, m, print_traceback);
        return m;
    }
}


//#####  Constructors+Destructors  ############################################
PythonObjectWrapper::PythonObjectWrapper(OwnershipMode o,
                                         bool acquire_gil /* unused in this overload */)
    : m_ownership(o),
      m_object(Py_None)
{
    if (m_ownership == control_ownership)
        Py_XINCREF(m_object);
}

//! Constructor for pre-existing PyObject
PythonObjectWrapper::PythonObjectWrapper(PyObject* pyobj, OwnershipMode o,
                                         bool acquire_gil /* unused in this overload */)
    : m_ownership(o),
      m_object(pyobj)
{ 
    if (m_ownership == control_ownership)
        Py_XINCREF(m_object);
}


// Copy constructor: increment refcount if controlling ownership.
// No need to manage GIL.
PythonObjectWrapper::PythonObjectWrapper(const PythonObjectWrapper& other)
    : m_ownership(other.m_ownership),
      m_object(other.m_object)
{
    if (m_ownership == control_ownership)
        Py_XINCREF(m_object);
}

// Destructor decrements refcount if controlling ownership.
// Always acquire the Python Global Interpreter Lock before decrementing.
PythonObjectWrapper::~PythonObjectWrapper()
{
    if (m_ownership == control_ownership) {
        // Hack: don't acquire the GIL if we are dealing with Py_None, since
        // this object never moves in memory (no deallocation) and decrementing
        // its refcount should be thread-safe.  It is possible that some empty
        // PythonObjectWrappers exist without build() having been called on
        // them (e.g. type registration for plearn help), i.e. Py_Initialize()
        // has not been called and acquiring the GIL in those cases is iffy.
        if (m_object == Py_None)
            Py_XDECREF(m_object);
        else {
            PythonGlobalInterpreterLock gil;
            Py_XDECREF(m_object);
        }
    }
}

// Assignment: let copy ctor and dtor take care of ownership
PythonObjectWrapper& PythonObjectWrapper::operator=(const PythonObjectWrapper& rhs)
{
    if (&rhs != this) {
        PythonObjectWrapper other(rhs);
        swap(other);
    }
    return *this;
}

// Swap *this with another instance
void PythonObjectWrapper::swap(PythonObjectWrapper& other)
{
    std::swap(this->m_ownership, other.m_ownership);
    std::swap(this->m_object,    other.m_object);
}

// Print out the Python object to stderr for debugging purposes
void PythonObjectWrapper::printDebug() const
{
    PyObject_Print(m_object, stderr, Py_PRINT_RAW);
}

bool PythonObjectWrapper::isNull() const
{
    return ! m_object || m_object == Py_None;
}
    

//##### Trampoline ############################################################
PyObject* PythonObjectWrapper::trampoline(PyObject* self, PyObject* args)
{
    PythonGlobalInterpreterLock gil;         // For thread-safety

    //get object and trampoline from self
    PythonObjectWrapper s(self, transfer_ownership);

    //perr << "refcnt self= " << self->ob_refcnt << endl;

    RemoteTrampoline* tramp= 
        dynamic_cast<RemoteTrampoline*>(s.as<PPointable*>());
    if(!tramp) 
        PLERROR("in PythonObjectWrapper::trampoline : "
                "can't unwrap RemoteTrampoline.");

    //wrap args
    int size = PyTuple_GET_SIZE(args);
    TVec<PythonObjectWrapper> args_tvec(size);
    for(int i= 0; i < size; ++i) 
        args_tvec[i]= 
            PythonObjectWrapper(PyTuple_GET_ITEM(args,i),
                                PythonObjectWrapper::transfer_ownership);

    // separate self from other params.
    Object* obj= args_tvec[0];
    args_tvec.subVecSelf(1, args_tvec.size()-1);

    //call, catch and send any errors to python
    try
    {
        return tramp->call(obj, args_tvec).getPyObject();
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
                        "Caught unknown C++ exception while executing injected function "
                        "inside a PythonObjectWrapper");
        return 0;
    }
}

PyObject* PythonObjectWrapper::python_del(PyObject* self, PyObject* args)
{
    TVec<PyObject*> args_tvec= 
        PythonObjectWrapper(args, transfer_ownership).as<TVec<PyObject*> >();
    Object* obj= PythonObjectWrapper(args_tvec[0], transfer_ownership);

    string classname= obj->classname();
    pypl_classes_t::iterator clit= m_pypl_classes.find(classname);
    if(clit == m_pypl_classes.end())
        PLERROR("in PythonObjectWrapper::python_del : "
                "deleting obj. for which no python class exists!");
    --clit->second.nref;

    /*
    //don't delete python classes
    if(0 == clit->second.nref)
    {
        m_pypl_classes.erase(classname);//cleanup
    }
    */

    obj->unref();//python no longer references this obj.

    m_wrapped_objects.erase(obj);
    return PythonObjectWrapper().getPyObject();//None
}

//#####  newPyObject  #########################################################

PyObject* PythonObjectWrapper::newPyObject()           //!< Return None (increments refcount)
{
    Py_XINCREF(Py_None);
    return Py_None;
}

PythonObjectWrapper::wrapped_objects_t 
    PythonObjectWrapper::m_wrapped_objects;//init.

PythonObjectWrapper::pypl_classes_t 
    PythonObjectWrapper::m_pypl_classes;//init.
//init.
bool PythonObjectWrapper::m_unref_injected= false;
PyMethodDef PythonObjectWrapper::m_unref_method_def;

PyObject* PythonObjectWrapper::newPyObject(const Object* x)
{
    // void ptr becomes None
    if(!x) return newPyObject();

    PythonGlobalInterpreterLock gil;         // For thread-safety

    static PythonEmbedder embedder;
    initializePython();

    //see if this obj. is already wrapped
    wrapped_objects_t::iterator objit= m_wrapped_objects.find(x);
    if(objit != m_wrapped_objects.end())
    {
        PyObject* o= objit->second;
        Py_INCREF(o);//new ref
        return o;//return ptr to already created pyobj
    }

    // import python class for wrapping PLearn objects
    string importcode= "\nfrom plearn.pybridge.wrapped_plearn_object "
        "import *\n";
    PyObject* pyenv= PyDict_New();
    PyDict_SetItemString(pyenv, "__builtins__", PyEval_GetBuiltins());
    PyRun_String(importcode.c_str(), Py_file_input, pyenv, pyenv);
    if (PyErr_Occurred()) 
    {
        Py_DECREF(pyenv);
        PyErr_Print();
        PLERROR("in PythonObjectWrapper::newPyObject : error compiling "
                "WrappedPLearnObject python code.");
    }

    // now find the class in the env.
    typedef map<string, PyObject*> env_t;
    env_t env= PythonObjectWrapper(pyenv, transfer_ownership).as<env_t>();
    env_t::iterator clit= env.find("WrappedPLearnObject");
    if(clit == env.end())
        PLERROR("in PythonObjectWrapper::newPyObject : "
                "class WrappedPLearnObject not defined "
                "in plearn.pybridge.wrapped_plearn_object");
    PyObject* wrapper= clit->second;

    //inject unref method if not already done
    if(!m_unref_injected)
    {
        PyMethodDef* py_method= &m_unref_method_def;
        py_method->ml_name  = "unref";
        py_method->ml_meth  = python_del;
        py_method->ml_flags = METH_VARARGS;
        py_method->ml_doc   = "injected-unref-function-from-PythonObjectWrapper";
        
        PyObject* py_funcobj= PyCFunction_NewEx(py_method, NULL, NULL);
        PyObject* py_methobj= PyMethod_New(py_funcobj, NULL, wrapper);
        Py_XDECREF(py_funcobj);
        if(!py_funcobj || !py_methobj) 
        {
            Py_DECREF(pyenv);
            Py_XDECREF(py_methobj);
            PLERROR("in PythonObjectWrapper::newPyObject : "
                    "can't inject method '%s' (i.e. __del__)", 
                    py_method->ml_name);
        }
        PyObject_SetAttrString(wrapper, py_method->ml_name, py_methobj);
        Py_DECREF(py_methobj);
        m_unref_injected= true;
    }

    if(!PyCallable_Check(wrapper))
        PLERROR("in PythonObjectWrapper::newPyObject : "
                "WrappedPLearnObject is not callable [not a class?]");

    // get ptr of object to wrap
    PyObject* plobj= PyCObject_FromVoidPtr(const_cast<Object*>(x), NULL);

    // try to find existing python class
    string classname= x->classname();
    pypl_classes_t::iterator clit2= m_pypl_classes.find(classname);
    PyObject* the_pyclass= 0;
    if(clit2 == m_pypl_classes.end())
    {
        // create new python type deriving from WrappedPLearnObject
        string derivcode= string("\nclass ")
            + classname + "(WrappedPLearnObject):\n"
            "\tpass\n\n";

        PyRun_String(derivcode.c_str(), Py_file_input, pyenv, pyenv);
        env= PythonObjectWrapper(pyenv, transfer_ownership).as<env_t>();
        clit= env.find(classname);
        if(clit == env.end())
            PLERROR("in PythonObjectWrapper::newPyObject : "
                    "Cannot create new python class deriving from "
                    "WrappedPLearnObject (%s).", classname.c_str());

        //set option names
        OptionList& options= x->getOptionList();
        unsigned int nopts= options.size();
        TVec<string> optionnames(nopts);
        for(unsigned int i= 0; i < nopts; ++i)
            optionnames[i]= options[i]->optionname();

        the_pyclass= clit->second;
        if(-1==PyObject_SetAttrString(the_pyclass, "_optionnames", 
                                      PythonObjectWrapper(optionnames).getPyObject()))
        {
            Py_DECREF(pyenv);
            if (PyErr_Occurred()) 
                PyErr_Print();
            PLERROR("cannot set attr _optionnames");
        }

        // inject all declared methods
        const RemoteMethodMap* methods= &x->getRemoteMethodMap();

        PP<PObjectPool<PyMethodDef> > meth_def_pool= 
            new PObjectPool<PyMethodDef>(methods->size()+1);

        m_pypl_classes.insert(
            make_pair(classname, PLPyClass(the_pyclass, meth_def_pool)));

        while(methods)
        {
            for(RemoteMethodMap::MethodMap::const_iterator it= methods->begin();
                it != methods->end(); ++it)
            {
                //get the RemoteTrampoline
                PyObject* tramp= PyCObject_FromVoidPtr(it->second, NULL);
            
                // Create a Python Function Object
                PyMethodDef* py_method= meth_def_pool->allocate();
                py_method->ml_name  = const_cast<char*>(it->first.first.c_str());
                py_method->ml_meth  = trampoline;
                py_method->ml_flags = METH_VARARGS;
                py_method->ml_doc   = "injected-function-from-PythonObjectWrapper";
    
                PyObject* py_funcobj= PyCFunction_NewEx(py_method, tramp, NULL);

                // create an unbound method from the function
                PyObject* py_methobj= PyMethod_New(py_funcobj, NULL, the_pyclass);

                Py_DECREF(tramp);
                Py_XDECREF(py_funcobj);
                if(!py_funcobj || !py_methobj) 
                {
                    Py_DECREF(pyenv);
                    Py_DECREF(plobj);
                    Py_XDECREF(py_methobj);
                    PLERROR("in PythonObjectWrapper::newPyObject : "
                            "can't inject method '%s'", py_method->ml_name);
                }

                PyObject_SetAttrString(the_pyclass, py_method->ml_name, py_methobj);
                Py_DECREF(py_methobj);
            }
            methods= methods->inheritedMethods();//get parent class methods
        }
    }
    else
    {
        the_pyclass= clit2->second.pyclass;
        ++clit2->second.nref;
    }
    
    //create the python object itself from the_pyclass
    PyObject* args= PyTuple_New(1);
    Py_INCREF(plobj);//keep it after it is 'stolen'
    PyTuple_SetItem(args, 0, plobj);
    
    PyObject* params= PyDict_New();
    PyObject* the_obj= PyObject_Call(the_pyclass, args, params);
    Py_DECREF(args);
    Py_DECREF(params);
    if(!the_obj)
    {
        Py_DECREF(pyenv);
        Py_DECREF(plobj);
        if (PyErr_Occurred()) PyErr_Print();
        PLERROR("in PythonObjectWrapper::newPyObject : "
                "can't construct a WrappedPLearnObject.");
    }

    //finalize
    Py_DECREF(pyenv);
    Py_DECREF(plobj);

    // augment refcount since python now 'points' to this obj.
    x->ref();

    m_wrapped_objects[x]= the_obj;

    return the_obj;
}

PyObject* PythonObjectWrapper::newPyObject(const bool& x)
{
    if (x) {
        Py_XINCREF(Py_True);
        return Py_True;
    }
    else {
        Py_XINCREF(Py_False);
        return Py_False;
    }
}


PyObject* PythonObjectWrapper::newPyObject(const int& x)
{
    return PyInt_FromLong(long(x));
}
    
PyObject* PythonObjectWrapper::newPyObject(const long& x)
{
    return PyLong_FromLong(x);
}
    
PyObject* PythonObjectWrapper::newPyObject(const double& x)
{
    return PyFloat_FromDouble(x);
}

PyObject* PythonObjectWrapper::newPyObject(const char* x)
{
    return PyString_FromString(x);
}
    
PyObject* PythonObjectWrapper::newPyObject(const string& x)
{
    return PyString_FromString(x.c_str());
}


PyObject* PythonObjectWrapper::newPyObject(const Vec& data)
{
    PyArrayObject* pyarr = 0;
    if (data.isNull() || data.isEmpty())
        pyarr = NA_NewArray(NULL, tReal, 1, 0);
    else
        pyarr = NA_NewArray(data.data(), tReal, 1, data.size());
        
    return (PyObject*)pyarr;
}

PyObject* PythonObjectWrapper::newPyObject(const Mat& data)
{
    PyArrayObject* pyarr = 0;
    if (data.isNull() || data.isEmpty())
        pyarr = NA_NewArray(NULL, tReal, 2, data.length(), data.width());
    else if (data.mod() == data.width())
        pyarr = NA_NewArray(data.data(), tReal, 2, data.length(), data.width());
    else {
        // static PyObject* NA_NewAll( int ndim, maybelong *shape, NumarrayType
        // type, void *buffer, maybelong byteoffset, maybelong bytestride, int
        // byteorder, int aligned, int writable)
        //
        // numarray from C data buffer. The new array has type type, ndim
        // dimensions, and the length of each dimensions must be given in
        // shape[ndim]. byteoffset, bytestride specify the data-positions in
        // the C array to use. byteorder and aligned specify the corresponding
        // parameters. byteorder takes one of the values NUM_BIG_ENDIAN or
        // NUM_LITTLE_ENDIAN. writable defines whether the buffer object
        // associated with the resuling array is readonly or writable. Data is
        // copied from buffer into the memory object of the new array.

        // maybelong shape[2];
        // shape[0] = data.length();
        // shape[1] = data.width();
        // pyarr = NA_NewAll(2, shape, tReal, data.data(), 0, data.mod()*sizeof(real),
        //                   NA_ByteOrder(), 1, 1);

        // NOTE (NC) -- I could not get the above function to work; for now,
        // simply copy the matrix to new storage before converting to Python.
        Mat new_data = data.copy();
        pyarr = NA_NewArray(new_data.data(), tReal, 2,
                            new_data.length(), new_data.width());
    }

    return (PyObject*)pyarr;
}

PyObject* PythonObjectWrapper::newPyObject(const VMat& vm)
{
    if (vm.isNull())
        return newPyObject(Mat());
    else
        return newPyObject(vm.toMat());
}

PyObject* PythonObjectWrapper::newPyObject(const PythonObjectWrapper& pow)
{
    Py_XINCREF(pow.m_object);
    return pow.m_object;
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
