// -*- C++ -*-

// ObjectConversions.h
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
   * $Id: .pyskeleton_header 544 2003-09-01 00:05:31Z plearner $ 
   ******************************************************* */

// Authors: Nicolas Chapados

/*! \file ObjectConversions.h */


#ifndef ObjectConversions_INC
#define ObjectConversions_INC

// From PLearn
#include <plearn/base/Array.h>
#include <plearn/base/TypeTraits.h>

// From Boost
#include <boost/type_traits.hpp>

namespace PLearn {

class Object;


//#####  isConvertibleToObjectPtr  ############################################

/**
 * @brief Return true if \c toObjectPtr() or \c toIndexedObjectPtr would
 * succeed.
 */
template <class T>
inline bool isConvertibleToObjectPtr(const T&)
{
    return boost::is_convertible< typename boost::remove_cv<T>::type,
                                  const Object* >::value
        || boost::is_convertible< typename boost::remove_cv<T>::type*,
                                  const Object* >::value ;
}

template <class T>
inline bool isConvertibleToObjectPtr(const PP<T>& x)
{
    return isConvertibleToObjectPtr((T*)0);
}

template <class T>
inline bool isConvertibleToObjectPtr(const Array<T>& x)
{
    return isConvertibleToObjectPtr((T*)0);
}

template <class T>
inline bool isConvertibleToObjectPtr(const TVec<T>& x)
{
    return isConvertibleToObjectPtr((T*)0);
}

template <class T>
inline bool isConvertibleToObjectPtr(const Array< PP<T> >& x)
{
    return isConvertibleToObjectPtr((T*)0);
}

template <class T>
inline bool isConvertibleToObjectPtr(const TVec< PP<T> >& x)
{
    return isConvertibleToObjectPtr((T*)0);
}


//#####  indexableObjectSize  #################################################

/**
 *  @brief Return 0 if the object is not indexable; otherwise, return one more
 *  than the maximum index allowed by \c toIndexedObjectPtr(); in other words,
 *  return the equivalent of the \c size() accessor on a vector.
 *
 *  @note  Minor hack: if the object is indexable and its size() is zero,
 *  we cannot return zero since this would mean the object is not indexable.
 *  In this case, we return -1.  I know, this is not the most elegant thing
 *  in the world...
 *
 *  Note that, for performance, the function \c isConvertibleToObjectPtr() is
 *  not called again; it is assumed that the user _knows_ that the object is
 *  accessible.
 */
template <class T>
inline int indexableObjectSize(const T& x)
{
    return 0;
}

template <class T>
inline int indexableObjectSize(const Array<T>& x)
{
    return (x.size() > 0? x.size() : -1);
}

template <class T>
inline int indexableObjectSize(const TVec<T>& x)
{
    return (x.size() > 0? x.size() : -1);
}


//#####  toObjectPtr  #########################################################

// Hack to work with earlier versions of Boost
typedef boost::is_convertible<int,int>::type  boost_true_type;
typedef boost::is_convertible<void,int>::type boost_false_type;

template <class T>
Object* toObjectPtrImpl(const T&, const boost_false_type&)
{
    PLERROR("Attempting to perform impossible conversion from type '%s' to Object*",
            TypeTraits<T>::name().c_str());
    return 0;
}

template <class T>
Object* toObjectPtrImpl(const T& x, const boost_true_type&)
{
    return const_cast<Object*>(static_cast<const Object*>(x));
}


/**
 * @brief Attempt to return a pointer to \c Object (or an error if the passed
 * argument cannot be considered an \c Object subclass)
 *
 * Remark: this version differs substantially from the previous
 * implementation, and now relies on Boost's type_traits library.
 */

template<class T>
inline Object* toObjectPtr(const T& x)
{
    typedef typename boost::remove_cv<T>::type T_nocv;
    typedef typename boost::is_convertible<T_nocv,  const Object*> T_isconv;
    typedef typename boost::is_convertible<T_nocv*, const Object*> pT_isconv;

    if (pT_isconv::value)
        return toObjectPtrImpl(&x, pT_isconv());
    else
        // May end up in the PLerror branch -- this is wanted
        return toObjectPtrImpl(x, T_isconv());
}

template<class T>
inline Object* toObjectPtr(const PP<T>& x)
{
    if (x.isNotNull())
        return toObjectPtr(*static_cast<T *>(x));
    else
        return 0;
    
}


//#####  toIndexedObjectPtr  ##################################################

/**
 * @brief Return the \c Object* at index \c i of an \c Array or \c TVec
 *
 * Produces a PLError if the conversion cannot be done.
 */
template<class T>
Object* toIndexedObjectPtr(const Array<T> &x, int i)
{
    return toObjectPtr(static_cast<T &>(x[i]));
}

template<class T>
Object* toIndexedObjectPtr(const TVec<T> &x, int i)
{
    return toObjectPtr(static_cast<T &>(x[i]));
}

template<class T>
Object *toIndexedObjectPtr(const T&, int) // Never to be called stub
{
    PLERROR("toIndexedObjectPtr() - Object is not indexable"); return 0;
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
