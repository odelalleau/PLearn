// -*- C++ -*-

// PythonObjectWrapper.h
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

/*! \file PythonObjectWrapper.h */


#ifndef PythonObjectWrapper_INC
#define PythonObjectWrapper_INC

// Python stuff must be included first
#include <plearn/python/PythonIncludes.h>

// From C++ stdlib
#include <utility>                           // Pairs
#include <vector>                            // vector<T>
#include <map>                               // map<T,U>

// From PLearn
#include <plearn/math/TVec.h>
#include <plearn/math/TMat.h>
#include <plearn/base/tostring.h>
#include <plearn/base/PMemPool.h>
#include <plearn/base/TypeTraits.h>
#include <plearn/base/tuple.h>

#ifdef USEFLOAT
#define tReal tFloat32
#else
#define tReal tFloat64
#endif

namespace PLearn {

class PythonObjectWrapper;                   // Forward-declare
class Object;
class VMat;

//! Used for error reporting.  If 'print_traceback' is true, a full
//! Python traceback is printed to stderr.  Otherwise, raise PLERROR.
void PLPythonConversionError(const char* function_name, PyObject* pyobj,
                             bool print_traceback);


//#####  ConvertFromPyObject  #################################################

/**
 *  @class  ConvertFromPyObject
 *  @brief  Set of conversion functions from Python to C++.
 *
 *  This cannot be function templates since we cannot partial specialize them.
 *  Note that new C++ objects are created and the original PyObject is not
 *  touched.  In particular, we never manipulate the PyObject reference count.
 *
 *  @note  For performance reasons, these functions DON'T acquire the Python
 *  Global Interpreter Lock (since it is assumed that there is no memory
 *  management involved in simply reading a Python object).  This may be
 *  changed in the future.
 */
template <class T>
struct ConvertFromPyObject
{ 
    static T convert(PyObject*, bool print_traceback)
    {
        PLERROR("Cannot convert this object by value from python (type=%s).",
                TypeTraits<T>::name().c_str());
        return T();//to silence compiler
    }

};

template <>
struct ConvertFromPyObject<PyObject*>
{
    //trivial: no conversion
    static PyObject* convert(PyObject*, bool print_traceback);
};

template <>
struct ConvertFromPyObject<bool>
{
    static bool convert(PyObject*, bool print_traceback);
};

template <>
struct ConvertFromPyObject<int>
{
    static int convert(PyObject*, bool print_traceback);
};

template <>
struct ConvertFromPyObject<unsigned int>
{
    static unsigned int convert(PyObject*, bool print_traceback);
};

template <>
struct ConvertFromPyObject<long>
{
    static long convert(PyObject*, bool print_traceback);
};

template <>
struct ConvertFromPyObject<unsigned long>
{
    static unsigned long convert(PyObject*, bool print_traceback);
};


template <>
struct ConvertFromPyObject<real>
{
    static real convert(PyObject*, bool print_traceback);
};

template <>
struct ConvertFromPyObject<string>
{
    static string convert(PyObject*, bool print_traceback);
};


template <>
struct ConvertFromPyObject<PPath>
{
    static PPath convert(PyObject*, bool print_traceback);
};


template <>
struct ConvertFromPyObject<PPointable*>
{
    static PPointable* convert(PyObject*, bool print_traceback);
};

template <>
struct ConvertFromPyObject<Object*>
{
    static Object* convert(PyObject*, bool print_traceback);
};

template <>
struct ConvertFromPyObject<Vec>
{
    // Return fresh storage
    static Vec convert(PyObject*, bool print_traceback);

    // Convert into pre-allocated Vec; resize it if necessary
    static void convert(PyObject* pyobj, Vec& result,
                        bool print_traceback);
};

template <>
struct ConvertFromPyObject<Mat>
{
    // Return fresh storage
    static Mat convert(PyObject*, bool print_traceback=true);

    // Convert into pre-allocated Vec; resize it if necessary
    static void convert(PyObject* pyobj, Mat& result,
                        bool print_traceback);
};

template <>
struct ConvertFromPyObject<VMat>
{
    // Return new MemoryVMatrix
    static VMat convert(PyObject*, bool print_traceback);
};

template <typename T>
struct ConvertFromPyObject<PP<T> >
{
    static PP<T> convert(PyObject*, bool print_traceback);
};

template <class T>
struct ConvertFromPyObject< TVec<T> >
{
    static TVec<T> convert(PyObject*, bool print_traceback);
};

template <class T>
struct ConvertFromPyObject< std::vector<T> >
{
    static std::vector<T> convert(PyObject*, bool print_traceback);
};

template <class T, class U>
struct ConvertFromPyObject< std::map<T,U> >
{
    static std::map<T,U> convert(PyObject*, bool print_traceback);
};

template <class T, class U>
struct ConvertFromPyObject< std::pair<T,U> >
{
    static std::pair<T,U> convert(PyObject*, bool print_traceback);
};

//#####  PythonGlobalInterpreterLock  #########################################

/**
 *  @class  PythonGlobalInterpreterLock
 *  @brief  Ensure thread safety by managing the Python Global Interpreter Lock
 *
 *  The Python interpreter is not fully reentrant and must be handled carefully
 *  in the presence of multi-threading.  While this does not affect
 *  multi-threaded pure Python code (for which reentrancy is properly managed),
 *  it does affect extensions that are written in other languages.  To this
 *  end, Python provides a Global Interpreter Lock (GIL) which must be acquired
 *  before calling any of its API within extension code and released
 *  afterwards.
 *
 *  This class provides a simple Resource-Acquisition-is-Initialization (RAII)
 *  idiom to manage the GIL.  The idea is to construct a local variable of this
 *  class at the beginning of a scope which uses Python.  The constructor
 *  acquires the lock, and the destructor automatically releases it.  For
 *  example:
 *
 *  @code
 *  void foo()
 *  {
 *      // Acquire the Python lock.  This blocks if another thread
 *      // already has the lock.
 *      PythonGlobalInterpreterLock gil;
 *
 *      // Code which uses the Python C API comes here
 *      // ...
 *  }   // Destructor releases the lock, so nothing to do.
 *  @endcode
 */
class PythonGlobalInterpreterLock
{
public:
    PythonGlobalInterpreterLock()
        : m_gilstate(PyGILState_Ensure())
    { }
       
    ~PythonGlobalInterpreterLock()
    {
        PyGILState_Release(m_gilstate);
    }

    PyGILState_STATE m_gilstate;
};


//#####  PythonObjectWrapper  #################################################

/**
 *  @class  PythonObjectWrapper
 *  @brief  Very lightweight wrapper over a Python Object that allows conversion
 *          to/from C++ types (including those of PLearn)
 *
 *  A PythonObjectWrapper provides the ability to manage a Python Object in a
 *  fairly lightweight manner.  It supports construction from a number of C++
 *  types, which in turn create new Python objects.  It also supports the
 *  conversion of the Python object back to C++ types.  The PythonObjectWrapper
 *  can either own or not the Python Object.  For owned objects, the Python
 *  reference count is increased with each copy or assignment, and decremented
 *  with each desctruction.
 *
 *  For safety reasons, the Python Global Interpreter Lock is acquired by
 *  default before constructing the Python object.  However, this can be
 *  controlled by a constructor option (useful when you have already acquired
 *  the lock in a bigger context and you have to construct a large number of
 *  PythonObjectWrappers).
 */
class PythonObjectWrapper
{
public:
    /**
     *  Ownership mode of the PythonObject.  If 'control_ownership', full
     *  reference counting is enabled.  If 'transfer_ownership', no counting is
     *  carried out.  The latter is useful when needing to transfer objects to
     *  Python functions that will become owners of the object.
     */
    enum OwnershipMode {
        control_ownership,
        transfer_ownership
    };
    
public:
    //#####  Construction and Utility  ########################################

    // Constructor from various C++ types.  These create a new PyObject (owned
    // by default).

    //! Construct 'None'.  This object is a singleton in Python, but it must be
    //! reference-counted just like any other object.
    PythonObjectWrapper(OwnershipMode o = control_ownership,
                        bool acquire_gil = true /* unused in this overload */);
    
    //! Constructor for pre-existing PyObject
    PythonObjectWrapper(PyObject* pyobj, OwnershipMode o = control_ownership,
                        bool acquire_gil = true /* unused in this overload */);
    
    //! Constructor for general type (forwarded to newPyObject)
    template <class T>
    PythonObjectWrapper(const T& x, OwnershipMode o = control_ownership,
                        bool acquire_gil = true)
        : m_ownership(o)
    {
        if (acquire_gil) {
            PythonGlobalInterpreterLock gil;
            m_object = newPyObject(x);
        }
        else
            m_object = newPyObject(x);
    }

    //! Copy constructor: increment refcount if controlling ownership.
    PythonObjectWrapper(const PythonObjectWrapper& other);
    
    //! Destructor: decrement refcount if controlling ownership.
    //! Always acquire the Python Global Interpreter Lock before decrementing.
    ~PythonObjectWrapper();

    //! Assignment operator: manage refcount if necessary
    PythonObjectWrapper& operator=(const PythonObjectWrapper& rhs);

    //! implicit conversion
    template<typename T> operator T() const { return as<T>(); }

    //! Swap *this with another instance
    void swap(PythonObjectWrapper& other);
    
    //! Return the bare PyObject managed by the wrapper
    PyObject* getPyObject() const
    {
        return m_object;
    }
    
    //! Return true if m_object is either NULL or stands for Python's None
    bool isNull() const;
    
    //! Print out the Python object to stderr for debugging purposes
    void printDebug() const;

    

    //#####  Conversion Back to C++  ##########################################

    //! General version that relies on ConvertFromPyOBject
    template <class T>
    T as() const
    {
        return ConvertFromPyObject<T>::convert(m_object, true);
    }

    /**
     *  Conversion that does not print any traceback to stderr if there is a
     *  conversion error.  This should be used in the rare cases where multiple
     *  return types are expected from Python and one wants to attempt several
     *  (e.g. from more specific to more general).
     */
    template <class T>
    T asNoTraceback() const
    {
        return ConvertFromPyObject<T>::convert(m_object, false);
    }
    

    //##### Trampoline for PLearn objects #####################################

    static PyObject* trampoline(PyObject* self, PyObject* args);

    static PyObject* python_del(PyObject* self, PyObject* args);


    //#####  Low-Level PyObject Creation  #####################################

    /**
     *  @function newPyObject
     *  @brief    Create a raw \c PyObject* from various types
     */
    static PyObject* newPyObject();  //!< Return None (increments refcount)

    static PyObject* newPyObject(const Object* x);

    static PyObject* newPyObject(const bool& x);
    
    static PyObject* newPyObject(const int& x);
    static PyObject* newPyObject(const unsigned int& x);
    
    static PyObject* newPyObject(const long& x);
    static PyObject* newPyObject(const unsigned long& x);
    
    static PyObject* newPyObject(const double& x);

    static PyObject* newPyObject(const char* x);
    
    static PyObject* newPyObject(const string& x);
  
    //! PLearn Vec: use numarray
    static PyObject* newPyObject(const Vec&);

    //! PLearn Mat: use numarray
    static PyObject* newPyObject(const Mat&);

    //! PLearn VMat.  Very inefficient for now: convert to a temporary Mat
    //! and then bridge to using numarray.  Fieldnames and other metainfos
    //! are lost when converting to Python.
    //!
    //! @TODO  Must provide a complete Python wrapper over VMatrix objects
    static PyObject* newPyObject(const VMat& vm);
    
    //! Generic PP: wrap pointed object
    template <class T>
    static PyObject* newPyObject(const PP<T>&);
    
    //! tuples (1 to 7 elts.)
    template <class T>
    static PyObject* newPyObject(const tuple<T>&);
    template <class T, class U>
    static PyObject* newPyObject(const tuple<T, U>&);
    template <class T, class U, class V>
    static PyObject* newPyObject(const tuple<T, U, V>&);
    template <class T, class U, class V, class W>
    static PyObject* newPyObject(const tuple<T, U, V, W>&);
    template <class T, class U, class V, class W, class X>
    static PyObject* newPyObject(const tuple<T, U, V, W, X>&);
    template <class T, class U, class V, class W, class X, class Y>
    static PyObject* newPyObject(const tuple<T, U, V, W, X, Y>&);
    template <class T, class U, class V, class W, class X, class Y, class Z>
    static PyObject* newPyObject(const tuple<T, U, V, W, X, Y, Z>&);

    //! Generic vector: create a Python list of those objects recursively
    template <class T>
    static PyObject* newPyObject(const TVec<T>&);

    //! C++ stdlib vector<>: create a Python list of those objects recursively
    template <class T>
    static PyObject* newPyObject(const std::vector<T>&);

    //! C++ stlib map<>: create a Python dict of those objects
    template <class T, class U>
    static PyObject* newPyObject(const std::map<T,U>&);

    //! C++ stdlib pair<>: create a Python tuple with two elements
    template <class T, class U>
    static PyObject* newPyObject(const std::pair<T,U>&);
    
    //! Pointer to vector<>: simply dereference pointer, or None if NULL
    //!
    //! (NOTE: we don't have conversion from general pointer type since it's
    //! not clear that we always want to convert by dereferencing; for some
    //! object types, it's possible that we want to preserve object identities).
    template <class T>
    static PyObject* newPyObject(const std::vector<T>*);

    //! Pointer to map<>: simply dereference pointer, or None if NULL
    template <class T, class U>
    static PyObject* newPyObject(const std::map<T,U>*);

    //! For a general PythonObjectWrapper: we simply increment the refcount
    //! to the underlying Python object, no matter whether we own it or not.
    static PyObject* newPyObject(const PythonObjectWrapper& pow);
    
    /**
     *  This function is called by PythonCodeSnippet to carry out
     *  initializations related to libnumarray.
     */
    static void initializePython();
    
protected:
    OwnershipMode m_ownership;               //!< Whether we own the PyObject or not
    PyObject* m_object;
    
    struct PLPyClass
    {
        // holds info about a PLearn class
        // injected into python
        PLPyClass(PyObject* pyclass_,
                  PP<PObjectPool<PyMethodDef> >& methods_)
            :pyclass(pyclass_),
             methods(methods_),
             methods_help(0),
             nref(1)
        {}
        PyObject* pyclass;
        PP<PObjectPool<PyMethodDef> > methods;
        TVec<string> methods_help;
        int nref;
    };

    //for the unique unref injected method
    static bool m_unref_injected;
    static PyMethodDef m_unref_method_def;

    typedef map<const string, PLPyClass> pypl_classes_t;
    static pypl_classes_t m_pypl_classes;

    typedef map<const Object*, PyObject*> wrapped_objects_t;

    static wrapped_objects_t m_wrapped_objects; //!< for wrapped PLearn Objects
};


//#####  ConvertFromPyObject Implementations  #################################

template <class T>
PP<T> ConvertFromPyObject<PP<T> >::convert(PyObject* pyobj, bool print_traceback)
{
    PPointable* o= 0;
    if(PyCObject_Check(pyobj))
        o= ConvertFromPyObject<PPointable*>::convert(pyobj, print_traceback);
    else
        o= ConvertFromPyObject<Object*>::convert(pyobj, print_traceback);
    PP<T> p(dynamic_cast<T*>(o));
    if(!p)
        PLPythonConversionError("ConvertFromPyObject<PP<T> >", pyobj,
                                print_traceback);
    return p;
}

template <class T>
TVec<T> ConvertFromPyObject< TVec<T> >::convert(PyObject* pyobj, bool print_traceback)
{
    PLASSERT( pyobj );
    
    // Here, we support both Python Tuples and Lists
    if (PyTuple_Check(pyobj)) {
        // Tuple case
        int size = PyTuple_GET_SIZE(pyobj);
        TVec<T> v(size);
        for (int i=0 ; i<size ; ++i) {
            PyObject* elem_i = PyTuple_GET_ITEM(pyobj, i);
            v[i] = ConvertFromPyObject<T>::convert(elem_i, print_traceback);
        }
        return v;
    }
    else if (PyList_Check(pyobj)) {
        // List case
        int size = PyList_GET_SIZE(pyobj);
        TVec<T> v(size);
        for (int i=0 ; i<size ; ++i) {
            PyObject* elem_i = PyList_GET_ITEM(pyobj, i);
            v[i] = ConvertFromPyObject<T>::convert(elem_i, print_traceback);
        }
        return v;
    }
    else
        PLPythonConversionError("ConvertFromPyObject< TVec<T> >", pyobj,
                                print_traceback);

    return TVec<T>();                        // Shut up compiler
}

template <class T>
std::vector<T> ConvertFromPyObject< std::vector<T> >::convert(PyObject* pyobj,
                                                              bool print_traceback)
{
    PLASSERT( pyobj );
    
    // Simple but inefficient implementation: create temporary TVec and copy
    // into a vector
    TVec<T> v = ConvertFromPyObject< TVec<T> >::convert(pyobj, print_traceback);
    return std::vector<T>(v.begin(), v.end());
}

template <class T, class U>
std::map<T,U> ConvertFromPyObject< std::map<T,U> >::convert(PyObject* pyobj,
                                                            bool print_traceback)
{
    PLASSERT( pyobj );
    if (! PyDict_Check(pyobj))
        PLPythonConversionError("ConvertFromPyObject< std::map<T,U> >", pyobj,
                                print_traceback);
    
    PyObject *key, *value;
    int pos = 0;
    std::map<T,U> result;

    while (PyDict_Next(pyobj, &pos, &key, &value)) {
        T the_key = ConvertFromPyObject<T>::convert(key, print_traceback);
        U the_val = ConvertFromPyObject<U>::convert(value, print_traceback);
        result.insert(make_pair(the_key,the_val));
    }
    return result;
}

template <class T, class U>
std::pair<T,U> ConvertFromPyObject< std::pair<T,U> >::convert(PyObject* pyobj,
                                                              bool print_traceback)
{
    PLASSERT( pyobj );
    // Here, we support both Python Tuples and Lists
    if (! PyTuple_Check(pyobj) && PyTuple_GET_SIZE(pyobj) != 2)
        PLPythonConversionError("ConvertFromPyObject< std::pair<T,U> >", pyobj,
                                print_traceback);

    std::pair<T,U> p;

    PyObject* first = PyTuple_GET_ITEM(pyobj, 0);
    p.first = ConvertFromPyObject<T>::convert(first, print_traceback);

    PyObject* second = PyTuple_GET_ITEM(pyobj, 1);
    p.second = ConvertFromPyObject<T>::convert(second, print_traceback);

    return p;
}



//#####  newPyObject Implementations  #########################################

template <class T>
PyObject* PythonObjectWrapper::newPyObject(const PP<T>& data)
{
    return newPyObject(static_cast<T*>(data));
}

template <class T>
PyObject* PythonObjectWrapper::newPyObject(const TVec<T>& data)
{
    PyObject* newlist = PyList_New(data.size());
    for (int i=0, n=data.size() ; i<n ; ++i) {
        // Since PyList_SET_ITEM steals the reference to the item being set,
        // one does not need to Py_XDECREF the inserted string as was required
        // for the PyArrayObject code above...
        PyList_SET_ITEM(newlist, i, newPyObject(data[i]));
    }
    return newlist;
}

template <class T>
PyObject* PythonObjectWrapper::newPyObject(const std::vector<T>& data)
{
    PyObject* newlist = PyList_New(data.size());
    for (int i=0, n=data.size() ; i<n ; ++i) {
        // Since PyList_SET_ITEM steals the reference to the item being set,
        // one does not need to Py_XDECREF the inserted string as was required
        // for the PyArrayObject code above...
        PyList_SET_ITEM(newlist, i, newPyObject(data[i]));
    }
    return newlist;
}
    
template <class T, class U>
PyObject* PythonObjectWrapper::newPyObject(const std::map<T,U>& data)
{
    // From the Python C API Documentation section 1.10.2: (Note that
    // PyDict_SetItem() and friends don't take over ownership -- they are
    // ``normal.'')
    PyObject* newdict = PyDict_New();
    for (typename std::map<T,U>::const_iterator it = data.begin(), end = data.end() ;
         it != end ; ++it)
    {
        PyObject* new_key = newPyObject(it->first);
        PyObject* new_val = newPyObject(it->second);
        int non_success = PyDict_SetItem(newdict, new_key, new_val);
        Py_XDECREF(new_key);
        Py_XDECREF(new_val);

        if (non_success)
            PLERROR("PythonObjectWrapper::newPyObject: cannot insert element '%s':'%s' "
                    "into Python dict",
                    tostring(it->first).c_str(),
                    tostring(it->second).c_str());
    }

    return newdict;
}

template <class T, class U>
PyObject* PythonObjectWrapper::newPyObject(const std::pair<T,U>& data)
{
    // According to Python Doc, since PyTuple_SET_ITEM steals the reference to
    // the item being set, one does not need to Py_XDECREF the inserted object.
    PyObject* newtuple = PyTuple_New(2);
    PyTuple_SET_ITEM(newtuple, 0, newPyObject(data.first));
    PyTuple_SET_ITEM(newtuple, 1, newPyObject(data.second));
    return newtuple;
}

template <class T>
PyObject* PythonObjectWrapper::newPyObject(const tuple<T>& data)
{
    PyObject* newtuple= PyTuple_New(1);
    PyTuple_SET_ITEM(newtuple, 0, newPyObject(get<0>(data)));
    return newtuple;
}

template <class T, class U>
PyObject* PythonObjectWrapper::newPyObject(const tuple<T, U>& data)
{
    PyObject* newtuple= PyTuple_New(2);
    PyTuple_SET_ITEM(newtuple, 0, newPyObject(get<0>(data)));
    PyTuple_SET_ITEM(newtuple, 1, newPyObject(get<1>(data)));
    return newtuple;
}

template <class T, class U, class V>
PyObject* PythonObjectWrapper::newPyObject(const tuple<T, U, V>& data)
{
    PyObject* newtuple= PyTuple_New(3);
    PyTuple_SET_ITEM(newtuple, 0, newPyObject(get<0>(data)));
    PyTuple_SET_ITEM(newtuple, 1, newPyObject(get<1>(data)));
    PyTuple_SET_ITEM(newtuple, 2, newPyObject(get<2>(data)));
    return newtuple;
}

template <class T, class U, class V, class W>
PyObject* PythonObjectWrapper::newPyObject(const tuple<T, U, V, W>& data)
{
    PyObject* newtuple= PyTuple_New(4);
    PyTuple_SET_ITEM(newtuple, 0, newPyObject(get<0>(data)));
    PyTuple_SET_ITEM(newtuple, 1, newPyObject(get<1>(data)));
    PyTuple_SET_ITEM(newtuple, 2, newPyObject(get<2>(data)));
    PyTuple_SET_ITEM(newtuple, 3, newPyObject(get<3>(data)));
    return newtuple;
}

template <class T, class U, class V, class W, class X>
PyObject* PythonObjectWrapper::newPyObject(const tuple<T, U, V, W, X>& data)
{
    PyObject* newtuple= PyTuple_New(5);
    PyTuple_SET_ITEM(newtuple, 0, newPyObject(get<0>(data)));
    PyTuple_SET_ITEM(newtuple, 1, newPyObject(get<1>(data)));
    PyTuple_SET_ITEM(newtuple, 2, newPyObject(get<2>(data)));
    PyTuple_SET_ITEM(newtuple, 3, newPyObject(get<3>(data)));
    PyTuple_SET_ITEM(newtuple, 4, newPyObject(get<4>(data)));
    return newtuple;
}

template <class T, class U, class V, class W, class X, class Y>
PyObject* PythonObjectWrapper::newPyObject(const tuple<T, U, V, W, X, Y>& data)
{
    PyObject* newtuple= PyTuple_New(6);
    PyTuple_SET_ITEM(newtuple, 0, newPyObject(get<0>(data)));
    PyTuple_SET_ITEM(newtuple, 1, newPyObject(get<1>(data)));
    PyTuple_SET_ITEM(newtuple, 2, newPyObject(get<2>(data)));
    PyTuple_SET_ITEM(newtuple, 3, newPyObject(get<3>(data)));
    PyTuple_SET_ITEM(newtuple, 4, newPyObject(get<4>(data)));
    PyTuple_SET_ITEM(newtuple, 5, newPyObject(get<5>(data)));
    return newtuple;
}

template <class T, class U, class V, class W, class X, class Y, class Z>
PyObject* PythonObjectWrapper::newPyObject(const tuple<T, U, V, W, X, Y, Z>& data)
{
    PyObject* newtuple= PyTuple_New(7);
    PyTuple_SET_ITEM(newtuple, 0, newPyObject(get<0>(data)));
    PyTuple_SET_ITEM(newtuple, 1, newPyObject(get<1>(data)));
    PyTuple_SET_ITEM(newtuple, 2, newPyObject(get<2>(data)));
    PyTuple_SET_ITEM(newtuple, 3, newPyObject(get<3>(data)));
    PyTuple_SET_ITEM(newtuple, 4, newPyObject(get<4>(data)));
    PyTuple_SET_ITEM(newtuple, 5, newPyObject(get<5>(data)));
    PyTuple_SET_ITEM(newtuple, 6, newPyObject(get<6>(data)));
    return newtuple;
}

template <class T>
PyObject* PythonObjectWrapper::newPyObject(const std::vector<T>* data)
{
    if (data)
        return newPyObject(*data);
    else
        return newPyObject();
}

template <class T, class U>
PyObject* PythonObjectWrapper::newPyObject(const std::map<T,U>* data)
{
    if (data)
        return newPyObject(*data);
    else
        return newPyObject();
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
