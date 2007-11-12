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
 * This file is part of the PLearn library.
 ******************************************************* */


/*! \file Array.h */

#ifndef Array_decl_INC
#define Array_decl_INC

#include <string>
#include <vector>
#include "TypeTraits.h"
#include <plearn/math/TMat_decl.h>

#include "ms_hash_wrapper.h"
namespace PLearn {
using std::string;
using std::vector;


template <class T> 
class Array: public TVec<T>
{
public:

    // norman: added scope for dependent name to resolve lookup
    //         See chap. 9.4.2 of C++ Templates, The Complete Guide 
    //         by Vandevoorde and Josuttis
    using TVec<T>::length_;
    using TVec<T>::offset_;
    using TVec<T>::storage;

    using TVec<T>::data;
    using TVec<T>::resize;

    typedef T* iterator;

    explicit Array<T>(int the_size=0, int extra_space = 10)
        : TVec<T>(the_size+extra_space)
    { length_ = the_size; }

    Array<T>(const T& elem1)
        : TVec<T>(1)
    { (*this)[0] = elem1; }

    Array<T>(const T& elem1, const T& elem2)
        : TVec<T>(2)
    {
        (*this)[0] = elem1;
        (*this)[1] = elem2;
    }

    Array<T>(const Array<T>& other)
        : TVec<T>(other.length())
    {
        length_ = other.size();
        offset_ = other.offset();
        iterator array = data();
        for(int i=0; i<length_; i++)
            array[i] = other[i];
    }

    Array<T>(const TVec<T>& other)
        : TVec<T>(other.copy())
    {}

    Array<T>(const vector<T> &other)
        : TVec<T>(other.size())
    {
        iterator array = data();
        for (int i = 0; i < length_; ++i)
            array[i] = other[i];
    }

    //!  To allow if(v) statements
    operator bool() const
    { return length_>0; }

    //!  To allow if(!v) statements
    bool operator!() const
    { return length_==0; }

    Array<T> subArray(int start, int len)
    {
        if (start+len>length_)
            PLERROR("Array::subArray start(%d)+len(%d)>size(%d)", start,len,length_);
        Array<T> newarray(len);
        iterator new_ar = newarray.data();
        iterator array = data();
        for (int i=0;i<len;i++)
            new_ar[i] = array[start+i];
        return newarray;
    }

    void clear()
    { length_ = 0; }

    void operator=(const Array<T>& other)
    {
        resize(other.size());
        iterator array = data();
        for(int i=0; i<length_; i++)
            array[i] = other[i];
    }

    void operator=(const TVec<T>& other)
    {
        resize(other.size());
        iterator array = data();
        for(int i=0; i<length_; i++)
            array[i] = other[i];
    }

    void operator=(const vector<T> &other)
    {
        resize(other.size());
        iterator array = data();
        for(int i = 0; i < length_; ++i)
            array[i] = other[i];
    }

    //! Makes this array a shared view of the given TVec
    void view(const TVec<T>& other)
    {
        TVec<T>::operator=(other);
    }

// EXPERIMENTALLY: PUT THOSE IN COMMENTS AND USE VERSION INHERITED FROM TVEC
// 
//     bool operator==(const Array<T>& other) const
//     {
// #ifdef BOUNDCHECK
//       if (this->size()!=other.size())
//         PLERROR("Array::operator== works on same-size arguments");
// #endif
//       iterator array = data();
//       for(int i=0; i<length_; i++)
//         if (array[i] != other[i]) return false;
//       return true;
//     }
// 
//     bool operator<(const Array<T>& other) const
//     {
// #ifdef BOUNDCHECK
//       if (this->size()!=other.size())
//         PLERROR("Array::operator< works on same-size arguments");
// #endif
//       iterator array = data();
//       for(int i=0; i<length_; i++)
//       {
//         if (array[i] < other[i]) return true;
//         else if (array[i] > other[i]) return false;
//       }
//       return false; // if == then not <
//     }
// 
//     bool operator<=(const Array<T>& other) const
//     {
// #ifdef BOUNDCHECK
//       if (this->size()!=other.size())
//         PLERROR("Array::operator< works on same-size arguments");
// #endif
//       iterator array = data();
//       for(int i=0; i<length_; i++)
//       {
//         if (array[i] < other[i]) return true;
//         else if (array[i] > other[i]) return false;
//       }
//       return true; // if == then <=
//     }
// 
// 
//     bool operator>(const Array<T>& other) const
//     {
// #ifdef BOUNDCHECK
//       if (this->size()!=other.size())
//         PLERROR("Array::operator< works on same-size arguments");
// #endif
//       iterator array = data();
//       for(int i=0; i<length_; i++)
//       {
//         if (array[i] > other[i]) return true;
//         else if (array[i] < other[i]) return false;
//       }
//       return false; // if == then not >
//     }
// 
//     bool operator>=(const Array<T>& other) const
//     {
// #ifdef BOUNDCHECK
//       if (this->size()!=other.size())
//         PLERROR("Array::operator< works on same-size arguments");
// #endif
//       iterator array = data();
//       for(int i=0; i<length_; i++)
//       {
//         if (array[i] > other[i]) return true;
//         else if (array[i] < other[i]) return false;
//       }
//       return true; // if == then >=
//     }

    void print(ostream& out) const
    {
        iterator array = data();
        for(int i=0; i<length_; i++)
            out << array[i] << endl;
    }

    int findFirstOccurence(const T& elem)
    {
        for(int i=0;i<this->array_size;i++)
            if(elem==this->array[i])
                return i;
        return -1;
    }

    //! Deep copy of an array is not the same as for a TVec, because
    //! the shallow copy automatically creates a new storage.
    void makeDeepCopyFromShallowCopy(CopiesMap& copies);


    // DEPRECATED! Call PStream& << arr instead (This is for backward compatibility only)
    void write(ostream &out_) const
    {
        PStream out(&out_);
        out << *this;
    }

    /*
     * NOTE: FIX_ME
     * If newread changes the state of the stream (e.g. eof), 
     * the original stream will NOT reflect this state... 
     * 'in' will have it's state changed, but not 'in_'.
     * This can be a major problem w/ 'asignstreams'...
     *                            - xsm
     */

    // DEPRECATED! Call PStream& >> arr instead (This is for backward compatibility only)
    void read(istream &in_)
    {
        PStream in(&in_);
        in >> *this;
    }

    //!  used by Hash  (VERY DIRTY: TO BE REMOVED [Pascal])
    inline operator char*() const { if(this->isNull()) return 0; else return (char*)data(); }

    // norman: removed const. With inline is useless (and .NET doesn't like it)
    // Old code:
    //inline const size_t byteLength() const { return length()*sizeof(T); }
    inline size_t byteLength() const { return this->size()*sizeof(T); }

/*  PAS UTILISE
    void increaseCapacity(int increase = 10)
    {
    T* newarray = new T[array_capacity+increase];
    for(int i=0; i<length_; i++)
    newarray[i] = array[i];
    delete[] array;
    array = newarray;
    array_capacity += increase;
    }
*/

};

template<class T>
class TypeTraits< Array<T> >
{
public:
    static inline string name()
    { return string("Array< ") + TypeTraits<T>::name()+" >"; }

    static inline unsigned char little_endian_typecode()
    { return 0xFF; }

    static inline unsigned char big_endian_typecode()
    { return 0xFF; }

};


template <class T>
class Array2ArrayMap : public PPointable
{
public:
    multimap<Array<T>,Array<T> > map;
};

/*template <class T> 
  struct hash_to_multimapArray {
  size_t operator()(const PP<multimap<Array<T>,Array<T> > > m) const
  {
  if (a)
  return hashbytes((char*)a->data(),a->size()*sizeof(T));
  return 0;
  }
  };
*/


} // end of namespace PLearn


// define hash function (replace the below declaration)
SET_HASH_FUNCTION(PLearn::Array<T>, T, a, PLearn::hashbytes((char*)a.data(),a.size()*sizeof(T)) )

//template <class T> 
//struct hash_Array {
//  size_t operator()(const Array<T>& a) const
//  {
//    return hashbytes((char*)a.data(),a.size()*sizeof(T));
//  }
//};

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
