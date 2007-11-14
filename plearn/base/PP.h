// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
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
 * $Id$
 * AUTHORS: Pascal Vincent & Yoshua Bengio
 * This file is part of the PLearn library.
 ******************************************************* */


/*! \file PP.h */

#ifndef SMART_POINTER_INC
#define SMART_POINTER_INC

#include <typeinfo>
#include "TypeTraits.h"
#include "CopiesMap.h"
#include "plerror.h"

namespace PLearn {
using std::string;

class PPointable
{
private:
    int refcount;

public:
    inline PPointable()
        :refcount(0)
    {}

    inline PPointable(const PPointable& other)
        :refcount(0)
    {}    

    inline void ref() const
    { const_cast<PPointable*>(this)->refcount++; }

    inline void unref() const
    {
        const_cast<PPointable*>(this)->refcount--;
        if(refcount==0)
            delete this;
    }

    inline int usage() const
    { return refcount; }

    virtual ~PPointable() {}
};

template<class T>
class PP
{
protected:
    T* ptr;

public:

    //! The type of the element this smart pointer points to.
    typedef T element_type;

    //! empty constructor
    inline PP<T>()
        :ptr(0)
    {}

    //! copie constructor with ordinary ptr
    inline PP<T>(const T* the_ptr)
    {
        ptr = const_cast<T*>(the_ptr);
        if(ptr)
            ptr->ref();
    }

    //! copie constructor with same type PP
    inline PP<T>(const PP<T>& other)
    { 
        ptr = const_cast<T*>((T*)other);
        if(ptr)
            ptr->ref();
    }

    //! copie constructor with other type PP
    template <class U>
    explicit PP<T>(const PP<U>& other)
    {
        if(other.isNull())
            ptr = 0;
        else
        {
            //!  this line is to make sure at compile time
            //!  that U and T are compatible (one is a subclass of the other)
            ptr = static_cast<T*>((U*)other);

/*!             this line is to make sure at execution time
  that the true class of other.ptr is compatible
  with T, i.e. other.ptr is a T or a subclass of 
  (otherwise return ptr = 0).
*/
            ptr = dynamic_cast<T*>((U*)other);

/*!             Note that dynamic_cast<T*>(const_cast<T*>(static_cast<const T*>((U*)other)))
  does not work properly (the dynamic_cast returns non-null when it
  should return 0, when the dynamic type is not correct).
*/
            if (!ptr)
                PLERROR("In PP constructor from smart pointer "
                        "of other class (constructing %s from %s)",
                        typeid(T).name(),typeid(U).name());
            ptr->ref();
        }
    }
  
    inline bool isNull() const 
    { return ptr==0; }

    inline bool isNotNull() const 
    { return ptr!=0; }

    //! conversion to ordinary ptr
    inline operator T*() const
    { return ptr; }

    //! access to PPointable methods
    inline T* operator->() const
    { return ptr; }
  
    //! access to the pointed object
    inline T& operator*() const
    { return *ptr; }
  
    //! affectation operator to ordinary ptr
    inline PP<T>& operator=(const T* otherptr)
    {
        if(otherptr!=ptr)
        {
            if(ptr)
                ptr->unref();
            ptr = const_cast<T*>(otherptr);
            if(ptr)
                ptr->ref();
        }
        return *this;
    }

    //! affectation operator to same type PP
    inline PP<T>& operator=(const PP<T>& other)
    { return operator=((T*)other); }

    inline bool operator==(const PP<T>& other) const
    { return ptr==other.ptr; }

    inline bool operator==(const T* other) const
    { return ptr==other; }

    inline ~PP()
    { 
        if(ptr)
            ptr->unref();
    }

};

template <class T>
inline bool operator==(const T* ptr, const PP<T>& b)
{ return ptr==b.ptr; }

///////////////////
// deepCopyField //
///////////////////
//!  Any pointer or smart pointer: call deepCopy()
template <class T>
inline void deepCopyField(PP<T>& field, CopiesMap& copies)
{
    if (field) {
        // Check the usage of the object pointed by 'field': it should be > 1,
        // because 'field' is a shallow copy. However, it *could* happen that it
        // is only 1, if the object that pointed to the same object has been deleted.
        // Since this is usually not wanted, we display a warning if this happens.
        // Indeed, there is a risk of this causing trouble, because the 'copies' map
        // may contain invalid mappings refering to now deleted objects.
        if (field->usage() == 1)
            PLWARNING("In deepCopyField(PP<T>& field, ...) - The usage() of the underlying object is only 1, this is unusual\n"
                      "( Did you call inherited::makeDeepCopyFromShallowCopy twice?  You can't. )");
        field = field->deepCopy(copies);
    }
}

//!  A simple template function
template<class T>
T* deepCopy(PP<T> source, CopiesMap& copies)
{ return deepCopy((T*)source, copies); }

//!  This function simply calls the previous one with an initially empty map
/*!
  makes a copie of the PP with all it's fields, respecting the dependance shceme 
  between elements and without allowing double copies of equal elements.
*/
template<class T>
inline T* deepCopy(PP<T> source)
{ 
    if (source.isNull()) return NULL;

    CopiesMap copies; //!<  create empty map
    return deepCopy(source, copies);
}


template<class T>
class TypeTraits< PP<T> >
{
public:
    static string name()
    { return string("PP< ") + TypeTraits<T>::name()+" >"; }

    static inline unsigned char little_endian_typecode()
    { return 0xFF; }

    static inline unsigned char big_endian_typecode()
    { return 0xFF; }
};

template <class A, class B>
class MultiMap : public PPointable
{
public:
    std::multimap<A,B> map;
};


template <class T>
T* get_pointer(PP<T> const& p)
{
    return p;
}

template <class T>
inline int sizeInBytes(PP<T> x) { 
    int n = sizeof(T*); 
    if (x) 
        n+= sizeInBytes(*x);  
    return n;
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
