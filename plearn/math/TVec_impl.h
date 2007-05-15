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


/*! \file PLearnLibrary/PLearnCore/TMat.h */

#ifndef TVec_impl_INC
#define TVec_impl_INC

#include "TVec_decl.h"
#include <plearn/io/pl_io.h>

namespace PLearn {
using namespace std;


// *****************************
// **** Fonctions pour TVec ****
// *****************************

// Deep copying
template<class T>
void TVec<T>::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    deepCopyField(storage, copies);
}

template<class T>
TVec<T> TVec<T>::deepCopy(CopiesMap& copies) const
{
    // First do a shallow copy
    TVec<T> deep_copy = *this;
    // Transform the shallow copy into a deep copy
    deep_copy.makeDeepCopyFromShallowCopy(copies);
    // return the completed deep_copy
    return deep_copy;
}

template <class T>
inline TVec<T> deepCopy(const TVec<T>& source)
{ 
    CopiesMap copies; //!<  create empty map
    return deepCopy(source, copies);
}

template <class T>
inline TVec<T> deepCopy(const TVec<T>& source, CopiesMap& copies)
{ 
    return source.deepCopy(copies);
}

template <class T>
inline void deepCopyField(TVec<T>& field, CopiesMap& copies)
{
    field.makeDeepCopyFromShallowCopy(copies);
}


template<class T>
void swap( TVec<T>& a, TVec<T>& b)
{ swap_ranges(a.begin(), a.end(), b.begin()); }

//! copy TVec << TVec 
template<class T>
inline void operator<<(const TVec<T>& m1, const TVec<T>& m2)
{
#ifdef BOUNDCHECK
    if(m1.size()!=m2.size())
        PLERROR("In operator<<(v1,v2) the 2 TVecs must have the same number of elements (%d != %d)", m1.size(), m2.size());
#endif
    if (m1.isNotEmpty())
        copy(m2.begin(), m2.end(), m1.begin());
}

//! copy TVec << TVec  (different types)
template<class T, class U>
void operator<<(const TVec<T>& m1, const TVec<U>& m2)
{
#ifdef BOUNDCHECK
    if(m1.size()!=m2.size())
        PLERROR("In operator<<(m1,m2) the 2 matrices must have the same number of elements (%d != %d)", m1.size(), m2.size());
#endif
    if (m1.isNotEmpty())
        copy_cast(m2.begin(), m2.end(), m1.begin());
}

//! copy TVec >> TVec
template<class T, class U>
inline void operator>>(const TVec<T>& m1, const TVec<U>& m2)
{ m2 << m1; }

// old .pvec format
template<class T>
void savePVec(const string& filename, const TVec<T>& vec)
{ PLERROR("savePVec only implemented for float and double"); }

template<class T>
void loadPVec(const string& filename, TVec<float>& vec)
{ PLERROR("loadPVec only implemented for float and double"); }


//!  Read and Write from C++ stream:
//!  write saves length and read resizes accordingly
//! (the raw modes don't write any size information)

template <class T> inline PStream &
operator<<(PStream &out, const TVec<T> &v)
{ 
    v.write(out); 
    return out;
}

template <class T> 
PStream & operator>>(PStream &in, TVec<T> &v)
{
    v.read(in);
    return in;
}


template<class T>      
void binwrite(ostream& out, const TVec<T>& v)
{
    int l = v.length();
    PLearn::binwrite(out,l);
    if (l<200000)
        PLearn::binwrite(out,v.data(),l);
    else for (int i=0;i<l;i+=200000)
        PLearn::binwrite(out,&v[i],std::min(200000,l-i));
}

template<class T>
void binread(istream& in, TVec<T>& v)
{
    int l;
    PLearn::binread(in,l);
    v.resize(l);
    if (l<200000)
        PLearn::binread(in,v.data(),l);
    else for (int i=0;i<l;i+=200000)
        PLearn::binread(in,&v[i],std::min(200000,l-i));
}

template<class T>      
void binwrite_double(ostream& out, const TVec<T>& v)
{
    int l = v.length();
    PLearn::binwrite(out,l);
    if (l<200000)
        PLearn::binwrite_double(out,v.data(),l);
    else for (int i=0;i<l;i+=200000)
        PLearn::binwrite_double(out,&v[i],std::min(200000,l-i));
}

template<class T>
void binread_double(istream& in, TVec<T>& v)
{
    int l;
    PLearn::binread(in,l);
    v.resize(l);
    if (l<200000)
        PLearn::binread_double(in,v.data(),l);
    else for (int i=0;i<l;i+=200000)
        PLearn::binread_double(in,&v[i],std::min(200000,l-i));
}


template<class T>
inline ostream& operator<<(ostream& out, const TVec<T>& v)
{ 
    v.print(out);
    return out;
}

template<class T>
inline istream& operator>>(istream& in, const TVec<T>& v)
{ 
    v.input(in);
    return in;
}


/*!   select the elements of the source as specified by the
  vector of indices (between 0 and source.length()-1) into
  the destination vector (which must have the same length()
  as the indices vector).
*/
template<class T, class I>
void selectElements(const TVec<T>& source, const TVec<I>& indices, TVec<T>& destination);

//! put in destination 1's when (*this)[i]==value, 0 otherwise
template<class T>
void elementsEqualTo(const TVec<T>& source, const T& value, const TVec<T>& destination);

template<class T>
TVec<T> concat(const TVec<T>& v1, const TVec<T>& v2)
{
    TVec<T> result(v1.length()+v2.length());
    for(int i=0; i<v1.length(); i++)
        result[i] = v1[i];
    for(int i=0; i<v2.length(); i++)
        result[i+v1.length()] = v2[i];
    return result;
}

template<class T>
TVec<T> concat(const TVec<T>& v1, const TVec<T>& v2, const TVec<T>& v3)
{
    TVec<T> result;
    result.concat(v1,v2,v3);
    return result;
}

template<class T>
TVec<T> concat(const TVec<T>& v1, const TVec<T>& v2, const TVec<T>& v3, const TVec<T>& v4)
{
    TVec<T> result;
    result.concat(v1,v2,v3,v4);
    return result;
}


//template<class T>
//TVec<T> concat(const Array< TVec<T> >& varray);

//! if the element to remove is the first or the last one, 
//! then a submatrix (a view) of m will be returned (for efficiency)
//! otherwise, it is a fresh copy with the element removed.
template<class T>
TVec<T> removeElement(const TVec<T>& v, int elemnum);


//! A simple family of relational operators for TVec
template <class T>
bool operator<=(const TVec<T>& left, const TVec<T>& right)
{
    if (left.size() != right.size())
        PLERROR("Left and right vectors must have the same size in operator<=");
    return std::inner_product(left.begin(), left.end(), right.begin(),
                              true, std::logical_and<bool>(),
                              std::less_equal<T>());
}

template <class T>
bool operator>=(const TVec<T>& left, const TVec<T>& right)
{
    if (left.size() != right.size())
        PLERROR("Left and right vectors must have the same size in operator>=");
    return std::inner_product(left.begin(), left.end(), right.begin(),
                              true, std::logical_and<bool>(),
                              std::greater_equal<T>());
}

// This is a lexicographical definition for operator<
template <class T>
bool operator<(const TVec<T>& left, const TVec<T>& right)
{
    if (left.size() != right.size())
        PLERROR("Left and right vectors must have the same size in operator<");
    int size = left.size();
    const T* ldata = left.data();
    const T* rdata = right.data();
    for ( ; size ; ++ldata, ++rdata, --size) {
        if (*ldata < *rdata)
            return true;
        if (*ldata > *rdata)
            return false;
        // Continue loop if both are equal
    }
    return false;                              // both vectors are equal at
    // this point; cannot be <
}

// This is a lexicographical definition for operator>
template <class T>
bool operator>(const TVec<T>& left, const TVec<T>& right)
{
    if (left.size() != right.size())
        PLERROR("Left and right vectors must have the same size in operator>");
    int size = left.size();
    const T* ldata = left.data();
    const T* rdata = right.data();
    for ( ; size ; ++ldata, ++rdata, --size) {
        if (*ldata < *rdata)
            return false;
        if (*ldata > *rdata)
            return true;
        // Continue loop if both are equal
    }
    return false;                              // both vectors are equal at
    // this point; cannot be >
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
