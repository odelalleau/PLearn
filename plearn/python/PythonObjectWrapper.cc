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
#include "PythonExtension.h"

// From C/C++ stdlib
#include <stdio.h>
#include <algorithm>

// From PLearn
#include <plearn/base/plerror.h>
#include <plearn/vmat/VMat.h>
#include <plearn/base/RemoteTrampoline.h>
#include <plearn/base/HelpSystem.h>

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


double ConvertFromPyObject<double>::convert(PyObject* pyobj,
                                            bool print_traceback)
{
    PLASSERT( pyobj );
    if(PyFloat_Check(pyobj))
        return PyFloat_AS_DOUBLE(pyobj);
    if(PyLong_Check(pyobj))
        return PyLong_AsDouble(pyobj);
    if(PyInt_Check(pyobj))
        return (double)PyInt_AS_LONG(pyobj);
    PLPythonConversionError("ConvertFromPyObject<double>", pyobj,
                            print_traceback);
    return 0;//shut up compiler
}

float ConvertFromPyObject<float>::convert(PyObject* pyobj,
                                          bool print_traceback)
{
    PLASSERT( pyobj );
    if(PyFloat_Check(pyobj))
        return (float)PyFloat_AS_DOUBLE(pyobj);
    if(PyLong_Check(pyobj))
        return (float)PyLong_AsDouble(pyobj);
    if(PyInt_Check(pyobj))
        return (float)PyInt_AS_LONG(pyobj);
    PLPythonConversionError("ConvertFromPyObject<float>", pyobj,
                            print_traceback);
    return 0;//shut up compiler
}

string ConvertFromPyObject<string>::convert(PyObject* pyobj,
                                            bool print_traceback)
{
    PLASSERT( pyobj );
    if (! PyString_Check(pyobj))
        PLPythonConversionError("ConvertFromPyObject<string>", pyobj,
                                print_traceback);
    return PyString_AsString(pyobj);
}

PPath ConvertFromPyObject<PPath>::convert(PyObject* pyobj,
                                          bool print_traceback)
{
    PLASSERT( pyobj );
    if (! PyString_Check(pyobj))
        PLPythonConversionError("ConvertFromPyObject<PPath>", pyobj,
                                print_traceback);
    return PPath(PyString_AsString(pyobj));
}

PPointable* ConvertFromPyObject<PPointable*>::convert(PyObject* pyobj,
                                                      bool print_traceback)
{
    PLASSERT(pyobj);
    if (! PyCObject_Check(pyobj))
        PLPythonConversionError("ConvertFromPyObject<PPointable*>", pyobj,
                                print_traceback);
    return static_cast<PPointable*>(PyCObject_AsVoidPtr(pyobj));
}

Object* ConvertFromPyObject<Object*>::convert(PyObject* pyobj,
                                              bool print_traceback)
{
    PLASSERT(pyobj);
    if(pyobj == Py_None)
        return 0;

    if(!PyObject_HasAttrString(pyobj, "_cptr"))
    {
        PLERROR("in ConvertFromPyObject<Object*>::convert : "
                "python object has no attribute '_cptr'");
        return 0;
    }
    PyObject* cptr= PyObject_GetAttrString(pyobj, "_cptr");

    if (! PyCObject_Check(cptr))
        PLPythonConversionError("ConvertFromPyObject<Object*>", pyobj,
                                print_traceback);
    Object* obj= static_cast<Object*>(PyCObject_AsVoidPtr(cptr));

    Py_DECREF(cptr);
    return obj;
}


void ConvertFromPyObject<Vec>::convert(PyObject* pyobj, Vec& v,
                                       bool print_traceback)
{
    // NA_InputArray possibly creates a well-behaved temporary (i.e. not
    // discontinuous is memory)
    PLASSERT( pyobj );
    PyArrayObject* pyarr = NA_InputArray(pyobj, tReal, NUM_C_ARRAY);
    if (! pyarr)
        PLPythonConversionError("ConvertFromPyObject<Vec>", pyobj,
                                print_traceback);
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

void ConvertFromPyObject<Mat>::convert(PyObject* pyobj, Mat& m,
                                       bool print_traceback)
{
    // NA_InputArray possibly creates a well-behaved temporary (i.e. not
    // discontinuous is memory)
    PLASSERT( pyobj );
    PyArrayObject* pyarr = NA_InputArray(pyobj, tReal, NUM_C_ARRAY);
    if (! pyarr)
        PLPythonConversionError("ConvertFromPyObject<Mat>", pyobj,
                                print_traceback);
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

//VMat ConvertFromPyObject<VMat>::convert(PyObject* pyobj,
//                                        bool print_traceback)
PP<VMatrix> ConvertFromPyObject<PP<VMatrix> >::convert(PyObject* pyobj,
                                                       bool print_traceback)
{
    PLASSERT(pyobj);
    if(PyObject_HasAttrString(pyobj, "_cptr"))
        return static_cast<VMatrix*>(
            ConvertFromPyObject<Object*>::convert(pyobj, print_traceback));
    Mat m;
    ConvertFromPyObject<Mat>::convert(pyobj, m, print_traceback);
    return VMat(m);
}

PythonObjectWrapper ConvertFromPyObject<PythonObjectWrapper>::convert(PyObject* pyobj, bool print_traceback)
{
    PLASSERT(pyobj);
    return PythonObjectWrapper(pyobj);
}

CopiesMap ConvertFromPyObject<CopiesMap>::convert(PyObject* pyobj,
                                                  bool print_traceback)
{
    PLASSERT( pyobj );
    if (! PyDict_Check(pyobj))
        PLPythonConversionError("ConvertFromPyObject<CopiesMap>", 
                                pyobj, print_traceback);
#if PL_PYTHON_VERSION>=250
    Py_ssize_t pos = 0;
#else
    int pos = 0;
#endif
    CopiesMap copies;
    PyObject *key, *val;
    while(PyDict_Next(pyobj, &pos, &key, &val)) 
    {
        if(!PyCObject_Check(key))
            PLPythonConversionError("ConvertFromPyObject<CopiesMap> "
                                    "(key is not a cptr)", 
                                    key, print_traceback);
        if(!PyCObject_Check(val))
            PLPythonConversionError("ConvertFromPyObject<CopiesMap> "
                                    "(val is not a cptr)", 
                                    val, print_traceback);
        copies.insert(make_pair(PyCObject_AsVoidPtr(key),
                                PyCObject_AsVoidPtr(val)));
    }
    return copies;
}

//#####  Constructors+Destructors  ############################################

PythonObjectWrapper::PythonObjectWrapper(OwnershipMode o,
                                         // unused in this overload
                                         bool acquire_gil)
    : m_ownership(o),
      m_object(Py_None)
{
    if (m_ownership == control_ownership)
        Py_XINCREF(m_object);
}

//! Constructor for pre-existing PyObject
PythonObjectWrapper::PythonObjectWrapper(PyObject* pyobj, OwnershipMode o,
                                         // unused in this overload
                                         bool acquire_gil)
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
    PythonObjectWrapper s(self);

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
            PythonObjectWrapper(PyTuple_GET_ITEM(args,i));

    // separate self from other params.
    Object* obj= args_tvec[0];
    args_tvec.subVecSelf(1, args_tvec.size()-1);

    gc_collect1();

    //call, catch and send any errors to python
    try
    {
        PythonObjectWrapper returned_value= tramp->call(obj, args_tvec);
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
                        "Caught unknown C++ exception while executing injected function "
                        "inside a PythonObjectWrapper");
        return 0;
    }
}

PyObject* PythonObjectWrapper::python_del(PyObject* self, PyObject* args)
{
    TVec<PyObject*> args_tvec=
        PythonObjectWrapper(args).as<TVec<PyObject*> >();

    Object* obj= PythonObjectWrapper(args_tvec[0]);

    string classname= obj->classname();
    pypl_classes_t::iterator clit= m_pypl_classes.find(classname);
    if(clit == m_pypl_classes.end())
        PLERROR("in PythonObjectWrapper::python_del : "
                "deleting obj. for which no python class exists!");
    --clit->second.nref;

    //perr << "delete " << (void*)obj << " : " << (void*)m_wrapped_objects[obj] << endl;
    //perr << "bef.del o->usage()= " << obj->usage() << endl;
    obj->unref();//python no longer references this obj.
    //perr << "aft.del o->usage()= " << obj->usage() << endl;

    //GC
    if(m_gc_next_object->first == obj)
        m_gc_next_object= m_wrapped_objects.end();

    m_wrapped_objects.erase(obj);

    //printWrappedObjects();

    return newPyObject();//None
}

PyObject* PythonObjectWrapper::newCPPObj(PyObject* self, PyObject* args)
{
    TVec<PyObject*> args_tvec= 
        PythonObjectWrapper(args).as<TVec<PyObject*> >();
    Object* o= newObjectFromClassname(PyString_AsString(args_tvec[1]));

    //perr << "new o->usage()= " << o->usage() << endl;

    return PyCObject_FromVoidPtr(o, 0);
}

PyObject* PythonObjectWrapper::refCPPObj(PyObject* self, PyObject* args)
{
    TVec<PyObject*> args_tvec= 
        PythonObjectWrapper(args).as<TVec<PyObject*> >();
    PyObject* pyo= args_tvec[1];
    Object* o= PythonObjectWrapper(pyo);
    if(args_tvec.length() < 3 || args_tvec[2]==Py_True)
        o->ref();

    //perr << "ref o->usage()= " << o->usage() << endl;
    PythonObjectWrapper::m_wrapped_objects[o]= pyo;
    //perr << "refCPPObj: " << (void*)o << " : " << (void*)pyo << endl;

    addToWrappedObjectsSet(pyo);//Py_INCREF(pyo);
    //printWrappedObjects();

    return newPyObject();//None
}

void PythonObjectWrapper::gc_collect1()
{
    if(m_gc_next_object == m_wrapped_objects.end())
        m_gc_next_object= m_wrapped_objects.begin();
    if(m_gc_next_object != m_wrapped_objects.end())
    {
        wrapped_objects_t::iterator it= m_gc_next_object;
        ++m_gc_next_object;
        if(it->first->usage() == 1 && it->second->ob_refcnt == 1)
        {
            //Py_DECREF(it->second);
            removeFromWrappedObjectsSet(it->second);
            gc_collect1();
        }
    }
}


//#####  newPyObject  #########################################################

//! Return None (increments refcount)
PyObject* PythonObjectWrapper::newPyObject()
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
PyMethodDef PythonObjectWrapper::m_newCPPObj_method_def;
PyMethodDef PythonObjectWrapper::m_refCPPObj_method_def;
PythonObjectWrapper::wrapped_objects_t::iterator 
  PythonObjectWrapper::m_gc_next_object= 
  PythonObjectWrapper::m_wrapped_objects.end();

PyObject* ConvertToPyObject<Object*>::newPyObject(const Object* x)
{
    // void ptr becomes None
    if(!x) return PythonObjectWrapper::newPyObject();

    PythonGlobalInterpreterLock gil;         // For thread-safety
    static PythonEmbedder embedder;
    PythonObjectWrapper::initializePython();

    //see if this obj. is already wrapped
    PythonObjectWrapper::wrapped_objects_t::iterator objit=
        PythonObjectWrapper::m_wrapped_objects.find(x);
    if(objit != PythonObjectWrapper::m_wrapped_objects.end())
    {
        PyObject* o= objit->second;
        Py_INCREF(o);//new ref
        return o;//return ptr to already created pyobj
    }
    // get ptr of object to wrap
    PyObject* plobj= PyCObject_FromVoidPtr(const_cast<Object*>(x), NULL);

    // try to find existing python class
    string classname= x->classname();
    PythonObjectWrapper::pypl_classes_t::iterator clit= 
        PythonObjectWrapper::m_pypl_classes.find(classname);
    if(clit == PythonObjectWrapper::m_pypl_classes.end())
        PLERROR("in ConvertToPyObject<Object*>::newPyObject : "
                "cannot find python class %s",classname.c_str());
    PyObject* the_pyclass= clit->second.pyclass;

    //create the python object itself from the_pyclass
    PyObject* args= PyTuple_New(0);
    PyObject* params= PyDict_New();
    PyDict_SetItemString(params, "_cptr", plobj);
    Py_DECREF(plobj);
    PyObject* the_obj= PyObject_Call(the_pyclass, args, params);
    Py_DECREF(args);
    Py_DECREF(params);
    if(!the_obj)
    {
        if (PyErr_Occurred()) PyErr_Print();
        PLERROR("in PythonObjectWrapper::newPyObject : "
                "can't construct a WrappedPLearnObject.");
    }

    // augment refcount since python now 'points' to this obj.
    x->ref();

    PythonObjectWrapper::m_wrapped_objects[x]= the_obj;

//    perr << "newPyObject: " << (void*)x << " : " << (void*)the_obj << endl;

    addToWrappedObjectsSet(the_obj);//Py_INCREF(the_obj);
    //printWrappedObjects();

    return the_obj;
}

PyObject* ConvertToPyObject<bool>::newPyObject(const bool& x)
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

PyObject* ConvertToPyObject<double>::newPyObject(const double& x)
{
    return PyFloat_FromDouble(x);
}

PyObject* ConvertToPyObject<float>::newPyObject(const float& x)
{
    return PyFloat_FromDouble(double(x));
}

PyObject* ConvertToPyObject<char*>::newPyObject(const char* x)
{
    return PyString_FromString(x);
}

PyObject* ConvertToPyObject<string>::newPyObject(const string& x)
{
    return PyString_FromString(x.c_str());
}

PyObject* ConvertToPyObject<PPath>::newPyObject(const PPath& x)
{
    return PyString_FromString(x.c_str());
}


PyObject* ConvertToPyObject<Vec>::newPyObject(const Vec& data)
{
    PyArrayObject* pyarr = 0;
    if (data.isNull() || data.isEmpty())
        pyarr = NA_NewArray(NULL, tReal, 1, 0);
    else
        pyarr = NA_NewArray(data.data(), tReal, 1, data.size());

    return (PyObject*)pyarr;
}

PyObject* ConvertToPyObject<Mat>::newPyObject(const Mat& data)
{
    PyArrayObject* pyarr = 0;
    if (data.isNull() || data.isEmpty())
        pyarr = NA_NewArray(NULL, tReal, 2, data.length(), data.width());
    else if (data.mod() == data.width())
        pyarr = NA_NewArray(data.data(), tReal, 2, data.length(),
                            data.width());
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
        // pyarr = NA_NewAll(2, shape, tReal, data.data(), 0,
        //                   data.mod()*sizeof(real), NA_ByteOrder(), 1, 1);

        // NOTE (NC) -- I could not get the above function to work; for now,
        // simply copy the matrix to new storage before converting to Python.
        Mat new_data = data.copy();
        pyarr = NA_NewArray(new_data.data(), tReal, 2,
                            new_data.length(), new_data.width());
    }

    return (PyObject*)pyarr;
}


bool PythonObjectWrapper::VMatAsPtr= false;//numpy array by default

//PyObject* ConvertToPyObject<VMat>::newPyObject(const VMat& vm)
PyObject* ConvertToPyObject<PP<VMatrix> >::newPyObject(const PP<VMatrix>& vm)
{
    if(PythonObjectWrapper::VMatAsPtr)
        return ConvertToPyObject<Object*>::newPyObject(static_cast<Object*>(vm));
    else// as a numpy array
        if (vm.isNull())
            return ConvertToPyObject<Mat>::newPyObject(Mat());
        else
            return ConvertToPyObject<Mat>::newPyObject(vm->toMat());
}

PyObject* ConvertToPyObject<PythonObjectWrapper>::newPyObject(const PythonObjectWrapper& pow)
{
    Py_XINCREF(pow.m_object);
    return pow.m_object;
}

PyObject* ConvertToPyObject<CopiesMap>::newPyObject(const CopiesMap& copies)
{
    PyObject* pyobj= PyDict_New();
    for(CopiesMap::const_iterator it= copies.begin();
        it != copies.end(); ++it)
    {
        PyObject* key= PyCObject_FromVoidPtr(const_cast<void*>(it->first), 0);
        PyObject* val= PyCObject_FromVoidPtr(it->second, 0);
        int non_success = PyDict_SetItem(pyobj, key, val);
        Py_XDECREF(key);
        Py_XDECREF(val);
        if(non_success)
            PLERROR("ConvertToPyObject<CopiesMap>::newPyObject: cannot insert element "
                    "into Python dict");
    }
    return pyobj;
}

PStream& operator>>(PStream& in, PythonObjectWrapper& v)
{
    PLERROR("operator>>(PStream&, PythonObjectWrapper&) : "
            "not supported (yet).");
/*
    string s;
    in >> s;
    string sub= "PythonObjectWrapper(ownership=";
    if(s.substr(0,sub.length()) != sub)
        PLERROR("in operator>>(PStream& in, PythonObjectWrapper& v) : "
                "expected '%s' but got '%s'.",
                sub.c_str(), s.c_str());
    s= s.substr(sub.length());
    v.m_ownership= static_cast<PythonObjectWrapper::OwnershipMode>(s[0]-'0');
    s= s.substr(1);
    sub= ", object=";
    if(s.substr(0,sub.length()) != sub)
        PLERROR("in operator>>(PStream& in, PythonObjectWrapper& v) : "
                "expected '%s' but got '%s'.",
                sub.c_str(), s.c_str());
    s= s.substr(sub.length());
    PStream sin= openString(s, PStream::plearn_ascii, "r");
    string pickle;
    sin >> pickle;

    PyObject* pypickle= PyString_FromString(pickle.c_str());
    PyObject* env= PyDict_New();
    if(0 != PyDict_SetItemString(env, "__builtins__", PyEval_GetBuiltins()))
        PLERROR("in operator>>(PStream&, PythonObjectWrapper& v) : "
                "cannot insert builtins in env.");
    if(0 != PyDict_SetItemString(env, "the_pickle", pypickle))
        PLERROR("in operator>>(PStream&, PythonObjectWrapper& v) : "
                "cannot insert the_pickle in env.");
    Py_DECREF(pypickle);
    PyObject* res= PyRun_String("\nfrom cPickle import *\nresult= loads(the_pickle)\n", 
                                Py_file_input, env, env);
    if(!res)
    {
        Py_DECREF(env);
        if(PyErr_Occurred()) PyErr_Print();
        PLERROR("in operator<<(PStream&, const PythonObjectWrapper& v) : "
                "cannot unpickle python object '%s'.",pickle.c_str());
    }
    Py_DECREF(res);
    v.m_object= 
        PythonObjectWrapper(env).as<std::map<string, PyObject*> >()["result"];
    Py_INCREF(v.m_object);
    Py_DECREF(env);
*/
    return in;
}

PStream& operator<<(PStream& out, const PythonObjectWrapper& v)
{
    PLERROR("operator<<(PStream&, const PythonObjectWrapper&) : "
            "not supported (yet).");
/*
    PyObject* env= PyDict_New();
    if(0 != PyDict_SetItemString(env, "__builtins__", PyEval_GetBuiltins()))
        PLERROR("in operator<<(PStream&, const PythonObjectWrapper& v) : "
                "cannot insert builtins in env.");
    if(0 != PyDict_SetItemString(env, "the_obj", v.m_object))
        PLERROR("in operator<<(PStream&, const PythonObjectWrapper& v) : "
                "cannot insert the_obj in env.");
    PyObject* res= PyRun_String("\nfrom cPickle import *\nresult= dumps(the_obj)\n", 
                                Py_file_input, env, env);
    if(!res)
    {
        Py_DECREF(env);
        if(PyErr_Occurred()) PyErr_Print();
        PLERROR("in operator<<(PStream&, const PythonObjectWrapper& v) : "
                "cannot pickle python object.");
    }
    Py_DECREF(res);
    string pickle= 
        PythonObjectWrapper(env).as<std::map<string, PythonObjectWrapper> >()["result"];
    Py_DECREF(env);
    string toout= string("PythonObjectWrapper(ownership=") + tostring(v.m_ownership) + ", object=\"" + pickle + "\")";
    out << toout;
*/
    return out; // shut up compiler
}


PStream& operator>>(PStream& in, PyObject* v)
{
    PLERROR("operator>>(PStream& in, PyObject* v) not supported yet");
    return in;
}

PStream& operator<<(PStream& out, const PyObject* v)
{
    PyObject* pystr= PyObject_Str(const_cast<PyObject*>(v));
    if(!pystr)
    {
        if (PyErr_Occurred()) PyErr_Print();
        PLERROR("in PythonTableVMatrix::build_ : "
                "access to underlying table's 'weightsize' failed.");
    }
    out << PythonObjectWrapper(pystr).as<string>();
    Py_DECREF(pystr);
    return out;
}


//! debug
void printWrappedObjects()
{
    perr << "wrapped_objects= " << endl;
    for(PythonObjectWrapper::wrapped_objects_t::iterator it= 
            PythonObjectWrapper::m_wrapped_objects.begin();
        it != PythonObjectWrapper::m_wrapped_objects.end(); ++it)
        perr << '\t' << it->first->classname() << ' ' << (void*)it->first 
             << ' ' << it->first->usage() << " : " 
             << (void*)it->second << ' ' << it->second->ob_refcnt << endl;
}

void ramassePoubelles()
{
    size_t sz= 0;
    while(sz != PythonObjectWrapper::m_wrapped_objects.size())
    {
        sz= PythonObjectWrapper::m_wrapped_objects.size();
        PythonObjectWrapper::wrapped_objects_t::iterator it= 
            PythonObjectWrapper::m_wrapped_objects.begin();
        while(it != PythonObjectWrapper::m_wrapped_objects.end())
        {
            PythonObjectWrapper::wrapped_objects_t::iterator jt= it;
            ++it;
            if(jt->second->ob_refcnt == 1 && jt->first->usage() == 1)
                removeFromWrappedObjectsSet(jt->second);
        }
    }
}

BEGIN_DECLARE_REMOTE_FUNCTIONS
    declareFunction("printWrappedObjects", &printWrappedObjects,
                    (BodyDoc("Prints PLearn objects wrapped into python.\n")));
    declareFunction("ramassePoubelles", &ramassePoubelles,
                    (BodyDoc("GC for wrapped objects.\n")));
END_DECLARE_REMOTE_FUNCTIONS



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
