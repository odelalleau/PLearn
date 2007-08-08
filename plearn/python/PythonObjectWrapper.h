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
#include <limits>                            // numeric_limits<I>

// From PLearn
#include <plearn/math/TVec.h>
#include <plearn/math/TMat.h>
#include <plearn/base/tostring.h>
#include <plearn/base/PMemPool.h>
#include <plearn/base/TypeTraits.h>
#include <plearn/base/tuple.h>
#include <plearn/base/CopiesMap.h>

// from boost
#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>


#ifdef USEFLOAT
#define tReal tFloat32
#else
#define tReal tFloat64
#endif

namespace PLearn {

class PythonObjectWrapper;                   // Forward-declare
class Object;
class VMatrix;

//! Used for error reporting.  If 'print_traceback' is true, a full
//! Python traceback is printed to stderr.  Otherwise, raise PLERROR.
void PLPythonConversionError(const char* function_name, PyObject* pyobj,
                             bool print_traceback);

//! Used to retrieve integer values from python if possible without precision
//! loss, and convert them to requested type.
//! @TODO: put I's name in error message?
//! @TODO: call PyErr_Print() before PyErr_Clear()?
template <class I>
I integerFromPyObject(PyObject* pyobj, bool print_traceback)
{
    PLASSERT( pyobj );

    I result = (I) 0;
    if (PyInt_Check(pyobj))
    {
        // pyobj is represented in Python as a long,
        // so we are sure it fits into a long, no need to check.
        long x = PyInt_AS_LONG(pyobj);
        result = static_cast<I>(x);

        // Check if x fits into type I (overflow or sign problem)
#ifdef __INTEL_COMPILER
#pragma warning(disable:1682)
// Yes, I know that "implicit conversion of a 64-bit integral type to a smaller
// integral type (potential portability problem)", but the conversion is
// explicit here.
#endif
        if (static_cast<long>(result) != x
            || !(numeric_limits<I>::is_signed) && x<0)
        {
            PLPythonConversionError("integerFromPyObject<I>", pyobj,
                                    print_traceback);
        }
#ifdef __INTEL_COMPILER
#pragma warning(default:1682)
#endif
    }
    else if (PyLong_Check(pyobj))
    {
        if (numeric_limits<I>::is_signed)
        {
            // If I is signed, we have to accept negative values, so we use a
            // signed long long to hold the result.
            // No signed type can hold values greater than a long long anyway.
            long long x = PyLong_AsLongLong(pyobj);

            // Check for possible overflow during conversion
            if (!PyErr_Occurred())
            {
#ifdef __INTEL_COMPILER
#pragma warning(disable:1682)
// Yes, I know that "implicit conversion of a 64-bit integral type to a smaller
// integral type (potential portability problem)", but the conversion is
// explicit here.
#endif
                result = static_cast<I>(x);
#ifdef __INTEL_COMPILER
#pragma warning(default:1682)
#endif

                // Check if x fits into type I (overflow only, there
                // cannot be any sign error because I is signed, too)
                if (static_cast<long long>(result) != x)
                {
                    PLPythonConversionError("integerFromPyObject<I>", pyobj,
                                            print_traceback);
                }
            }
            else if (PyErr_ExceptionMatches(PyExc_OverflowError))
            {
                PyErr_Clear();
                PLPythonConversionError("integerFromPyObject<I>", pyobj,
                                        print_traceback);
            }
            // else?
        }
        else
        {
            // I is unsigned
            unsigned long long x = PyLong_AsUnsignedLongLong(pyobj);

            // Check for possible overflow during conversion
            if (!PyErr_Occurred())
            {
#ifdef __INTEL_COMPILER
#pragma warning(disable:1682)
// Yes, I know that "implicit conversion of a 64-bit integral type to a smaller
// integral type (potential portability problem)", but the conversion is
// explicit here.
#endif
                result = static_cast<I>(x);
#ifdef __INTEL_COMPILER
#pragma warning(default:1682)
#endif

                // Check if x fits into type I (overflow only)
                if (static_cast<unsigned long long>(result) != x)
                {
                    PLPythonConversionError("integerFromPyObject<I>", pyobj,
                                            print_traceback);
                }
            }
            else if (PyErr_ExceptionMatches(PyExc_OverflowError) // too big
                     || PyErr_ExceptionMatches(PyExc_TypeError)) // negative
            {
                PyErr_Clear();
                PLPythonConversionError("integerFromPyObject<I>", pyobj,
                                        print_traceback);
            }
            // else?
        }
    }
    else
        PLPythonConversionError("integerFromPyObject<I>", pyobj,
                                print_traceback);
    return result;
}


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
    static T convert(PyObject* x, bool print_traceback);
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
struct ConvertFromPyObject<short>
{
    static short convert(PyObject* pyobj, bool print_traceback)
    { return integerFromPyObject<short>(pyobj, print_traceback); }
};

template <>
struct ConvertFromPyObject<unsigned short>
{
    static unsigned short convert(PyObject* pyobj, bool print_traceback)
    { return integerFromPyObject<unsigned short>(pyobj, print_traceback); }
};

template <>
struct ConvertFromPyObject<int>
{
    static int convert(PyObject* pyobj, bool print_traceback)
    { return integerFromPyObject<int>(pyobj, print_traceback); }
};

template <>
struct ConvertFromPyObject<unsigned int>
{
    static unsigned int convert(PyObject* pyobj, bool print_traceback)
    { return integerFromPyObject<unsigned int>(pyobj, print_traceback); }
};

template <>
struct ConvertFromPyObject<long>
{
    static long convert(PyObject* pyobj, bool print_traceback)
    { return integerFromPyObject<long>(pyobj, print_traceback); }
};

template <>
struct ConvertFromPyObject<unsigned long>
{
    static unsigned long convert(PyObject* pyobj, bool print_traceback)
    { return integerFromPyObject<unsigned long>(pyobj, print_traceback); }
};

template <>
struct ConvertFromPyObject<long long>
{
    static long long convert(PyObject* pyobj, bool print_traceback)
    { return integerFromPyObject<long long>(pyobj, print_traceback); }
};

template <>
struct ConvertFromPyObject<unsigned long long>
{
    static unsigned long long convert(PyObject* pyobj, bool print_traceback)
    { return integerFromPyObject<unsigned long long>(pyobj, print_traceback); }
};


template <>
struct ConvertFromPyObject<double>
{
    static double convert(PyObject*, bool print_traceback);
};

template <>
struct ConvertFromPyObject<float>
{
    static float convert(PyObject*, bool print_traceback);
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


///***///***
// PARTIAL specialisation from T*.  Assume Object*.
// TODO: fix this assumption
template <class T>
struct ConvertFromPyObject<T*>
{
    static T* convert(PyObject* pyobj, bool print_traceback)
    {
        // Compile-time assertion:
        BOOST_STATIC_ASSERT((boost::is_base_of<Object, typename boost::remove_cv<T>::type>::value)
                            || (boost::is_same<Object, typename boost::remove_cv<T>::type>::value));
        //N.B.: If this assertion fails, it probably means that you are trying
        //      to retrieve a pointer to something that is not an Object from
        //      python.  Only Object pointers are supported.

        Object* obj = ConvertFromPyObject<Object*>::convert(pyobj,
                                                            print_traceback);
        if (T* tobj = dynamic_cast<T*>(obj))
            return tobj;
        else
            PLERROR("Cannot convert object from python (type='%s').",
                    TypeTraits<T*>::name().c_str());
        return 0;                            // Silence compiler
    }
};
///***///***

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
struct ConvertFromPyObject<PP<VMatrix> >
{
    // Return new MemoryVMatrix
    static PP<VMatrix> convert(PyObject*, bool print_traceback);
};

template <>
struct ConvertFromPyObject<PythonObjectWrapper>
{
    static PythonObjectWrapper convert(PyObject*, bool print_traceback);
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
struct ConvertFromPyObject<TMat<T> >
{
    static TMat<T> convert(PyObject*, bool print_traceback);
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

template <>
struct ConvertFromPyObject<CopiesMap>
{
    static CopiesMap convert(PyObject*, bool print_traceback);
};


//! Used to convert integer values to python, using PyInt if possible
template <class I>
PyObject* integerToPyObject(const I& x)
{
    // Try to convert x to a long
#ifdef __INTEL_COMPILER
#pragma warning(disable:1682)
// Yes, I know that "implicit conversion of a 64-bit integral type to a smaller
// integral type (potential portability problem)", but the conversion is
// explicit here.
#endif
    long y = static_cast<long>(x);
#ifdef __INTEL_COMPILER
#pragma warning(default:1682)
#endif

    // Check if we lost value information or sign
    if (static_cast<I>(y) == x && (numeric_limits<I>::is_signed || y >= 0))
        return PyInt_FromLong(y);
    else if (numeric_limits<I>::is_signed)
        return PyLong_FromLongLong(static_cast<long long>(x));
    else
        return PyLong_FromUnsignedLongLong(static_cast<unsigned long long>(x));
}

/////////////////////////////////////
// ConvertToPyObject<>
// Conversions from PLearn to Python
template<typename T>
struct ConvertToPyObject
{
    static PyObject* newPyObject(const T& x);
};

// Specialization for Object*
template<> struct ConvertToPyObject<Object*>
{ static PyObject* newPyObject(const Object* x); };

///***///***
// Other specializations
///***///***

template<> struct ConvertToPyObject<bool>
{ static PyObject* newPyObject(const bool& x); };

template<> struct ConvertToPyObject<short>
{
    static PyObject* newPyObject(const short& x)
    { return integerToPyObject(x); }
};

template<> struct ConvertToPyObject<unsigned short>
{
    static PyObject* newPyObject(const unsigned short& x)
    { return integerToPyObject(x); }
};

template<> struct ConvertToPyObject<int>
{
    static PyObject* newPyObject(const int& x)
    { return integerToPyObject(x); }
};

template<> struct ConvertToPyObject<unsigned int>
{
    static PyObject* newPyObject(const unsigned int& x)
    { return integerToPyObject(x); }
};

template<> struct ConvertToPyObject<long>
{
    static PyObject* newPyObject(const long& x)
    { return integerToPyObject(x); }
};

template<> struct ConvertToPyObject<unsigned long>
{
    static PyObject* newPyObject(const unsigned long& x)
    { return integerToPyObject(x); }
};

template<> struct ConvertToPyObject<long long>
{
    static PyObject* newPyObject(const long long& x)
    { return integerToPyObject(x); }
};

template<> struct ConvertToPyObject<unsigned long long>
{
    static PyObject* newPyObject(const unsigned long long& x)
    { return integerToPyObject(x); }
};

/*
template<> struct ConvertToPyObject<int64_t>
{ static PyObject* newPyObject(const int64_t& x); };
template<> struct ConvertToPyObject<uint64_t>
{ static PyObject* newPyObject(const uint64_t& x); };
*/

template<> struct ConvertToPyObject<double>
{ static PyObject* newPyObject(const double& x); };

template<> struct ConvertToPyObject<float>
{ static PyObject* newPyObject(const float& x); };

template<> struct ConvertToPyObject<char*>
{ static PyObject* newPyObject(const char* x); };

template<size_t N> struct ConvertToPyObject<char[N]>
{ static PyObject* newPyObject(const char x[N]); };

template<> struct ConvertToPyObject<string>
{ static PyObject* newPyObject(const string& x); };

template<> struct ConvertToPyObject<PPath>
{ static PyObject* newPyObject(const PPath& x); };

//! PLearn Vec: use numarray
template<> struct ConvertToPyObject<Vec>
{ static PyObject* newPyObject(const Vec&); };

//! PLearn Mat: use numarray
template<> struct ConvertToPyObject<Mat>
{ static PyObject* newPyObject(const Mat&); };

//! PLearn VMat.  Very inefficient for now: convert to a temporary Mat
//! and then bridge to using numarray.  Fieldnames and other metainfos
//! are lost when converting to Python.
//!
//! @TODO  Must provide a complete Python wrapper over VMatrix objects
template<> struct ConvertToPyObject<PP<VMatrix> >
{ static PyObject* newPyObject(const PP<VMatrix>& vm); };

//! Generic PP: wrap pointed object
template<class T> struct ConvertToPyObject<PP<T> >
{ static PyObject* newPyObject(const PP<T>&); };

//! tuples (1 to 7 elts.)
template<class T>
struct ConvertToPyObject<tuple<T> >
{ static PyObject* newPyObject(const tuple<T>&); };
template <class T, class U>
struct ConvertToPyObject<tuple<T,U> >
{ static PyObject* newPyObject(const tuple<T, U>&); };
template <class T, class U, class V>
struct ConvertToPyObject<tuple<T,U,V> >
{ static PyObject* newPyObject(const tuple<T, U, V>&); };
template <class T, class U, class V, class W>
struct ConvertToPyObject<tuple<T,U,V,W> >
{ static PyObject* newPyObject(const tuple<T, U, V, W>&); };
template <class T, class U, class V, class W, class X>
struct ConvertToPyObject<tuple<T,U,V,W,X> >
{ static PyObject* newPyObject(const tuple<T, U, V, W, X>&); };
template <class T, class U, class V, class W, class X, class Y>
struct ConvertToPyObject<tuple<T,U,V,W,X,Y> >
{ static PyObject* newPyObject(const tuple<T, U, V, W, X, Y>&); };
template <class T, class U, class V, class W, class X, class Y, class Z>
struct ConvertToPyObject<tuple<T,U,V,W,X,Y,Z> >
{ static PyObject* newPyObject(const tuple<T, U, V, W, X, Y, Z>&); };

//! Generic vector: create a Python list of those objects recursively
template <class T> struct ConvertToPyObject<TVec<T> >
{ static PyObject* newPyObject(const TVec<T>&); };

//! Generic matrix: create a Python list of those objects recursively
template <class T> struct ConvertToPyObject<TMat<T> >
{ static PyObject* newPyObject(const TMat<T>&); };

//! C++ stdlib vector<>: create a Python list of those objects recursively
template <class T> struct ConvertToPyObject<std::vector<T> >
{ static PyObject* newPyObject(const std::vector<T>&); };

//! C++ stlib map<>: create a Python dict of those objects
template <class T, class U> struct ConvertToPyObject<std::map<T,U> >
{ static PyObject* newPyObject(const std::map<T,U>&); };

//! C++ stdlib pair<>: create a Python tuple with two elements
template <class T, class U> struct ConvertToPyObject<std::pair<T,U> >
{ static PyObject* newPyObject(const std::pair<T,U>&); };

//! Pointer to vector<>: simply dereference pointer, or None if NULL
//!
//! (NOTE: we don't have conversion from general pointer type since it's
//! not clear that we always want to convert by dereferencing; for some
//! object types, it's possible that we want to preserve object identities).
template <class T> struct ConvertToPyObject<std::vector<T> const* >
{ static PyObject* newPyObject(const std::vector<T>*); };

//! Pointer to map<>: simply dereference pointer, or None if NULL
template <class T, class U> struct ConvertToPyObject<std::map<T,U> const* >
{ static PyObject* newPyObject(const std::map<T,U>*); };

//! For a general PythonObjectWrapper: we simply increment the refcount
//! to the underlying Python object, no matter whether we own it or not.
template<> struct ConvertToPyObject<PythonObjectWrapper>
{ static PyObject* newPyObject(const PythonObjectWrapper& pow); };

template<> struct ConvertToPyObject<CopiesMap>
{ static PyObject* newPyObject(const CopiesMap& copies); };


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
 *  @brief  Very lightweight wrapper over a Python Object that allows
 *          conversion to/from C++ types (including those of PLearn)
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
    explicit PythonObjectWrapper(const T& x,
                                 OwnershipMode o = control_ownership,
                                 bool acquire_gil = true)
        : m_ownership(o)
    {
        if (acquire_gil) {
            PythonGlobalInterpreterLock gil;
            m_object = ConvertToPyObject<T>::newPyObject(x);
        }
        else
            m_object = ConvertToPyObject<T>::newPyObject(x);
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

    static PyObject* newCPPObj(PyObject* self, PyObject* args);

    static PyObject* refCPPObj(PyObject* self, PyObject* args);

    //#####  Low-Level PyObject Creation  #####################################

    /**
     *  @function newPyObject
     *  @brief    Create a raw \c PyObject* from various types
     */

    static PyObject* newPyObject();  //!< Return None (increments refcount)

    /**
     *  This function is called by PythonCodeSnippet to carry out
     *  initializations related to libnumarray.
     */
    static void initializePython();

    //for the unique unref injected method
    static bool m_unref_injected;
    static PyMethodDef m_unref_method_def;
    static PyMethodDef m_newCPPObj_method_def;
    static PyMethodDef m_refCPPObj_method_def;
    typedef map<const string, PLPyClass> pypl_classes_t;
    static pypl_classes_t m_pypl_classes;
    typedef map<const Object*, PyObject*> wrapped_objects_t;
    static wrapped_objects_t m_wrapped_objects; //!< for wrapped PLearn Objects

protected:
    OwnershipMode m_ownership;               //!< Whether we own the PyObject or not
    PyObject* m_object;

    template<class T> friend class ConvertToPyObject;
};

// Specialization for General T*.  Attempt to cast into Object*.  If that works
// we're all set; for specific pointer types (e.g.  map<U,V>* and vector<T>*),
// above, since they are more specialized they should kick in before this one.
// This specialization is not grouped with other specializations because it
// makes explicit use of the 'newPyObject' method in the PythonObjectWrapper
// class, and gcc 4.0.2 does not allow this until that class is properly
// declared.
template <typename T>
struct ConvertToPyObject<T*>
{
    static PyObject* newPyObject(const T* x)
    {
        if(!x) // null ptr. becomes None
            return PythonObjectWrapper::newPyObject();

        if (const Object* objx = dynamic_cast<const Object*>(x))
            return ConvertToPyObject<Object*>::newPyObject(objx);

        PLERROR("Cannot convert type %s by value to python",
                TypeTraits<T*>::name().c_str());
        return 0;//shut up compiler
    }
};


//#####  ConvertFromPyObject Implementations  #################################

template<class U, bool is_enum>
struct StaticConvertEnumFromPyObject
{
    static U convert(PyObject* x, bool print_traceback)
    {
        PLERROR("Cannot convert this object by value from python (type=%s).",
                TypeTraits<U>::name().c_str());
        return U();//to silence compiler
    }
};

template<class U>
struct StaticConvertEnumFromPyObject<U, true>
{
    static U convert(PyObject* x, bool print_traceback)
    {
        return static_cast<U>(
            ConvertFromPyObject<int>::convert(x, print_traceback));
    }
};

template <class T>
T ConvertFromPyObject<T>::convert(PyObject* x, bool print_traceback)
{
    return StaticConvertEnumFromPyObject<T, boost::is_enum<T>::value>
        ::convert(x, print_traceback);
    /*
    if(boost::is_enum<T>::value)
        return ConvertFromPyObject<int>::convert(x, print_traceback);

    PLERROR("Cannot convert this object by value from python (type=%s).",
            TypeTraits<T>::name().c_str());
    return T();//to silence compiler
    */
}


template <class T>
PP<T> ConvertFromPyObject<PP<T> >::convert(PyObject* pyobj,
                                           bool print_traceback)
{
    PLASSERT( pyobj );
    if(pyobj == Py_None)
        return 0;

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
TVec<T> ConvertFromPyObject< TVec<T> >::convert(PyObject* pyobj,
                                                bool print_traceback)
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
TMat<T> ConvertFromPyObject<TMat<T> >::convert(PyObject* pyobj,
                                               bool print_traceback)
{
    PLASSERT( pyobj );

    // Here, we support both Python Tuples and Lists
    if (PyTuple_Check(pyobj)) {
        // Tuple case
        int len= PyTuple_GET_SIZE(pyobj);
        TMat<T> v;
        for(int i= 0; i < len; ++i)
        {
            PyObject* row_i= PyTuple_GET_ITEM(pyobj, i);
            TVec<T> r= ConvertFromPyObject<TVec<T> >::convert(row_i,
                                                              print_traceback);
            if(i == 0)
                v.resize(0, r.size());
            v.appendRow(r);
        }
        return v;
    }
    else if (PyList_Check(pyobj)) {
        // List case
        int len= PyList_GET_SIZE(pyobj);
        TMat<T> v;
        for(int i= 0; i < len; ++i)
        {
            PyObject* row_i= PyList_GET_ITEM(pyobj, i);
            TVec<T> r= ConvertFromPyObject<TVec<T> >::convert(row_i,
                                                              print_traceback);
            if(i == 0)
                v.resize(0, r.size());
            v.appendRow(r);
        }
        return v;
    }
    else
        PLPythonConversionError("ConvertFromPyObject< TMat<T> >", pyobj,
                                print_traceback);

    return TMat<T>();                        // Shut up compiler
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
#if PL_PYTHON_VERSION>=250
    Py_ssize_t pos = 0;
#else
    int pos = 0;
#endif
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
    p.second = ConvertFromPyObject<U>::convert(second, print_traceback);

    return p;
}

//#####  newPyObject Implementations  #########################################
template<typename T, bool is_enum>
struct StaticConvertEnumToPyObject
{
    static PyObject* newPyObject(const T& x)
    {
        PLERROR("Cannot convert type %s by value to python.",
                TypeTraits<T>::name().c_str());
        return 0;//shut up compiler
    }
};

template<typename T>
struct StaticConvertEnumToPyObject<T, true>
{
    static PyObject* newPyObject(const T& x)
    {
        return ConvertToPyObject<int>::newPyObject(x);
    }
};

template<typename T>
PyObject* ConvertToPyObject<T>::newPyObject(const T& x)
{
    return StaticConvertEnumToPyObject<T, boost::is_enum<T>::value>
        ::newPyObject(x);
}


template<size_t N>
PyObject* ConvertToPyObject<char[N]>::newPyObject(const char x[N])
{
    return ConvertToPyObject<char*>::newPyObject(x);
}


template <class T>
PyObject* ConvertToPyObject<PP<T> >::newPyObject(const PP<T>& data)
{
    if(data == 0)
        return PythonObjectWrapper::newPyObject();
    Object* o= dynamic_cast<Object*>(static_cast<T*>(data));
    if(o)
        return ConvertToPyObject<Object*>::newPyObject(o);
    return ConvertToPyObject<T*>::newPyObject(static_cast<T*>(data));
}

template <class T>
PyObject* ConvertToPyObject<TVec<T> >::newPyObject(const TVec<T>& data)
{
    PyObject* newlist = PyList_New(data.size());
    for (int i=0, n=data.size() ; i<n ; ++i) {
        // Since PyList_SET_ITEM steals the reference to the item being set,
        // one does not need to Py_XDECREF the inserted string as was required
        // for the PyArrayObject code above...
        PyList_SET_ITEM(newlist, i, ConvertToPyObject<T>::newPyObject(data[i]));
    }
    return newlist;
}

template <class T>
PyObject* ConvertToPyObject<TMat<T> >::newPyObject(const TMat<T>& data)
{
    PyObject* newlist = PyList_New(data.length());
    for (int i=0, n=data.length() ; i<n ; ++i)
    {
        // Since PyList_SET_ITEM steals the reference to the item being set,
        // one does not need to Py_XDECREF the inserted string as was required
        // for the PyArrayObject code above...
        PyList_SET_ITEM(newlist, i,
                        ConvertToPyObject<TVec<T> >::newPyObject(data(i)));
    }
    return newlist;
}

template <class T>
PyObject* ConvertToPyObject<std::vector<T> >::newPyObject(const std::vector<T>& data)
{
    PyObject* newlist = PyList_New(data.size());
    for (int i=0, n=data.size() ; i<n ; ++i) {
        // Since PyList_SET_ITEM steals the reference to the item being set,
        // one does not need to Py_XDECREF the inserted string as was required
        // for the PyArrayObject code above...
        PyList_SET_ITEM(newlist, i, ConvertToPyObject<T>::newPyObject(data[i]));
    }
    return newlist;
}

template <class T, class U>
PyObject* ConvertToPyObject<std::map<T,U> >::newPyObject(const std::map<T,U>& data)
{
    // From the Python C API Documentation section 1.10.2: (Note that
    // PyDict_SetItem() and friends don't take over ownership -- they are
    // ``normal.'')
    PyObject* newdict = PyDict_New();
    for (typename std::map<T,U>::const_iterator it = data.begin(), end = data.end() ;
         it != end ; ++it)
    {
        PyObject* new_key = ConvertToPyObject<T>::newPyObject(it->first);
        PyObject* new_val = ConvertToPyObject<U>::newPyObject(it->second);
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
PyObject* ConvertToPyObject<std::pair<T,U> >::newPyObject(const std::pair<T,U>& data)
{
    // According to Python Doc, since PyTuple_SET_ITEM steals the reference to
    // the item being set, one does not need to Py_XDECREF the inserted object.
    PyObject* newtuple = PyTuple_New(2);
    PyTuple_SET_ITEM(newtuple, 0, ConvertToPyObject<T>::newPyObject(data.first));
    PyTuple_SET_ITEM(newtuple, 1, ConvertToPyObject<U>::newPyObject(data.second));
    return newtuple;
}

template <class T>
PyObject* ConvertToPyObject<tuple<T> >::newPyObject(const tuple<T>& data)
{
    PyObject* newtuple= PyTuple_New(1);
    PyTuple_SET_ITEM(newtuple, 0, ConvertToPyObject<T>::newPyObject(get<0>(data)));
    return newtuple;
}

template <class T, class U>
PyObject* ConvertToPyObject<tuple<T,U> >::newPyObject(const tuple<T, U>& data)
{
    PyObject* newtuple= PyTuple_New(2);
    PyTuple_SET_ITEM(newtuple, 0, ConvertToPyObject<T>::newPyObject(get<0>(data)));
    PyTuple_SET_ITEM(newtuple, 1, ConvertToPyObject<U>::newPyObject(get<1>(data)));
    return newtuple;
}

template <class T, class U, class V>
PyObject* ConvertToPyObject<tuple<T,U,V> >::newPyObject(const tuple<T, U, V>& data)
{
    PyObject* newtuple= PyTuple_New(3);
    PyTuple_SET_ITEM(newtuple, 0, ConvertToPyObject<T>::newPyObject(get<0>(data)));
    PyTuple_SET_ITEM(newtuple, 1, ConvertToPyObject<U>::newPyObject(get<1>(data)));
    PyTuple_SET_ITEM(newtuple, 2, ConvertToPyObject<V>::newPyObject(get<2>(data)));
    return newtuple;
}

template <class T, class U, class V, class W>
PyObject* ConvertToPyObject<tuple<T,U,V,W> >::newPyObject(const tuple<T, U, V, W>& data)
{
    PyObject* newtuple= PyTuple_New(4);
    PyTuple_SET_ITEM(newtuple, 0, ConvertToPyObject<T>::newPyObject(get<0>(data)));
    PyTuple_SET_ITEM(newtuple, 1, ConvertToPyObject<U>::newPyObject(get<1>(data)));
    PyTuple_SET_ITEM(newtuple, 2, ConvertToPyObject<V>::newPyObject(get<2>(data)));
    PyTuple_SET_ITEM(newtuple, 3, ConvertToPyObject<W>::newPyObject(get<3>(data)));
    return newtuple;
}

template <class T, class U, class V, class W, class X>
PyObject* ConvertToPyObject<tuple<T,U,V,W,X> >::newPyObject(const tuple<T, U, V, W, X>& data)
{
    PyObject* newtuple= PyTuple_New(5);
    PyTuple_SET_ITEM(newtuple, 0, ConvertToPyObject<T>::newPyObject(get<0>(data)));
    PyTuple_SET_ITEM(newtuple, 1, ConvertToPyObject<U>::newPyObject(get<1>(data)));
    PyTuple_SET_ITEM(newtuple, 2, ConvertToPyObject<V>::newPyObject(get<2>(data)));
    PyTuple_SET_ITEM(newtuple, 3, ConvertToPyObject<W>::newPyObject(get<3>(data)));
    PyTuple_SET_ITEM(newtuple, 4, ConvertToPyObject<X>::newPyObject(get<4>(data)));
    return newtuple;
}

template <class T, class U, class V, class W, class X, class Y>
PyObject* ConvertToPyObject<tuple<T,U,V,W,X,Y> >::newPyObject(const tuple<T, U, V, W, X, Y>& data)
{
    PyObject* newtuple= PyTuple_New(6);
    PyTuple_SET_ITEM(newtuple, 0, ConvertToPyObject<T>::newPyObject(get<0>(data)));
    PyTuple_SET_ITEM(newtuple, 1, ConvertToPyObject<U>::newPyObject(get<1>(data)));
    PyTuple_SET_ITEM(newtuple, 2, ConvertToPyObject<V>::newPyObject(get<2>(data)));
    PyTuple_SET_ITEM(newtuple, 3, ConvertToPyObject<W>::newPyObject(get<3>(data)));
    PyTuple_SET_ITEM(newtuple, 4, ConvertToPyObject<X>::newPyObject(get<4>(data)));
    PyTuple_SET_ITEM(newtuple, 5, ConvertToPyObject<Y>::newPyObject(get<5>(data)));
    return newtuple;
}

template <class T, class U, class V, class W, class X, class Y, class Z>
PyObject* ConvertToPyObject<tuple<T,U,V,W,X,Y,Z> >::newPyObject(const tuple<T, U, V, W, X, Y, Z>& data)
{
    PyObject* newtuple= PyTuple_New(7);
    PyTuple_SET_ITEM(newtuple, 0, ConvertToPyObject<T>::newPyObject(get<0>(data)));
    PyTuple_SET_ITEM(newtuple, 1, ConvertToPyObject<U>::newPyObject(get<1>(data)));
    PyTuple_SET_ITEM(newtuple, 2, ConvertToPyObject<V>::newPyObject(get<2>(data)));
    PyTuple_SET_ITEM(newtuple, 3, ConvertToPyObject<W>::newPyObject(get<3>(data)));
    PyTuple_SET_ITEM(newtuple, 4, ConvertToPyObject<X>::newPyObject(get<4>(data)));
    PyTuple_SET_ITEM(newtuple, 5, ConvertToPyObject<Y>::newPyObject(get<5>(data)));
    PyTuple_SET_ITEM(newtuple, 6, ConvertToPyObject<Z>::newPyObject(get<6>(data)));
    return newtuple;
}

template <class T>
PyObject* ConvertToPyObject<std::vector<T> const* >::newPyObject(const std::vector<T>* data)
{
    if (data)
        return ConvertToPyObject<std::vector<T> >::newPyObject(*data);
    else
        return PythonObjectWrapper::newPyObject();
}

template <class T, class U>
PyObject* ConvertToPyObject<std::map<T,U> const* >::newPyObject(const std::map<T,U>* data)
{
    if (data)
        return ConvertToPyObject<std::map<T,U> >::newPyObject(*data);
    else
        return PythonObjectWrapper::newPyObject();
}

PStream& operator>>(PStream& in, PythonObjectWrapper& v);
DECLARE_TYPE_TRAITS(PythonObjectWrapper);

//! for debug purposes
void printWrappedObjects();

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
