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
   * $Id: Array.h,v 1.12 2003/08/13 08:13:16 plearner Exp $
   * This file is part of the PLearn library.
   ******************************************************* */


/*! \file PLearnLibrary/PLearnCore/Array.h */

#ifndef ARRAY_INC
#define ARRAY_INC

#include <map>
#include <set>
#include <string>
#include <vector>
#include "TypeTraits.h"
#include "general.h"
#include "fileutils.h"
#include "TMat.h"



namespace PLearn <%
using namespace std;


template <class T> 
class Array: public TVec<T>
{
  public:

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

    Array<T>(const Array& other)
      : TVec<T>(other.storage->size())
      {
        length_ = other.size();
        offset_ = other.offset();
        iterator array = data();
        for(int i=0; i<length_; i++)
          array[i] = other[i];
      }

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

    void operator=(const Array& other)
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

    void print(ostream& out) const
    {
      iterator array = data();
      for(int i=0; i<length_; i++)
        out << array[i] << endl;
    }

  int findFirstOccurence(const T& elem)
    {
      for(int i=0;i<array_size;i++)
        if(elem==array[i])
          return i;
      return -1;
    }


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


template <class T>
inline PStream & operator>>(PStream &in, Array<T> &a)
{ readSequence(in, a); return in; }

template <class T>
inline PStream & operator<<(PStream &out, const Array<T> &a)
{ writeSequence(out, a); return out; };

template<class T>
ostream& operator<<(ostream& out, const Array<T>& a)
{ a.print(out); return out; }

template <class T>
inline void deepCopyField(Array<T>& field, CopiesMap& copies)
{ field.makeDeepCopyFromShallowCopy(copies); }

template<class T>
Array<T> operator&(const T& elem, const Array<T>& a)
{ return Array<T>(elem) & a; }

template<class T>
Array<T>& operator&=(Array<T>& a, const T& elem)
{ a.append(elem); return a; }

template<class T>
Array<T>& operator&=(Array<T>& a, const Array<T>& ar)
{ a.append(ar); return a; }

template<class T>
Array<T>& operator&=(Array<T>& a, const vector<T> &ar)
{ a.append(ar); return a; }

template<class T>
Array<T> operator&(const Array<T>& a, const T& elem)
{
  Array<T> newarray(a.size(), a.size()+1);
  newarray = a;
  newarray.append(elem);
  return newarray;
}

template<class T>
Array<T> operator&(const Array<T>& a, const Array<T>& ar)
{
  Array<T> newarray(a.size(), a.size()+ar.size());
  newarray = a;
  newarray.append(ar);
  return newarray;
}

template<class T>
Array<T> operator&(const Array<T>& a, const vector<T> &ar)
{
  Array<T> newarray(a.size(), a.size() + ar.size());
  newarray = a;
  newarray.append(ar);
  return newarray;
}

inline string join(const Array<string>& s, const string& separator)
{
  string result;
  for(int i=0; i<s.size(); i++)
  {
    result += s[i];
    if(i<s.size()-1)
      result += separator;
  }
  return result;
}

//!  This will allow a convenient way of building arrays of Matrices by writing ex: m1&m2&m3
template<class T>
inline Array< TVec<T> > operator&(const TVec<T>& m1, const TVec<T>& m2) { return Array< TVec<T> >(m1,m2); } 

template<class T>
TVec<T> concat(const Array< TVec<T> >& varray)
{
  int l = 0;
  for(int k=0; k<varray.size(); k++)
    l += varray[k].length();
 
  TVec<T> result(l);
  real* resdata = result.data();
  for(int k=0; k<varray.size(); k++)
  {
    const TVec<T>& v = varray[k];
    real* vdata = varray[k].data();
    for(int i=0; i<v.length(); i++)
      resdata[i] = vdata[i];
    resdata += v.length();
  }
  return result;
}

template<class T>
TMat<T> vconcat(const Array< TMat<T> >& ar)
{
  int l = 0;
  int w = ar[0].width();
  for(int n=0; n<ar.size(); n++)
  {
    if(ar[n].width() != w)
      PLERROR("In Mat vconcat(Array<Mat> ar) all Mats do not have the same width()!");
    l += ar[n].length();
  }
  TMat<T> result(l, w);
  int pos = 0;
  for(int n=0; n<ar.size(); n++)
  {
    result.subMatRows(pos, ar[n].length()) << ar[n];
    pos+=ar[n].length();  // do not put this line after the n++ in the for loop, or it will cause a bug!
  }
  return result;
}

template<class T>
TMat<T> hconcat(const Array< TMat<T> >& ar)
{
  int w = 0;
  int l = ar[0].length();
  for(int n=0; n<ar.size(); n++)
  {
    if(ar[n].length() != l)
      PLERROR("In Mat hconcat(Array<Mat> ar) all Mats do not have the same length()!");
    w += ar[n].width();
  }
  TMat<T> result(l, w);
  int pos = 0;
  for(int n=0; n<ar.size(); n++)
  {
    result.subMatColumns(pos, ar[n].width()) << ar[n];
    pos+=ar[n].width(); // do not put this line after the n++ in the for loop, or it will cause a bug!
  }
  return result;
}

template<class T>
inline TMat<T> vconcat(const TMat<T>& m1, const TMat<T>& m2) { return vconcat(Array< TMat<T> >(m1,m2)); }

template<class T>
inline TMat<T> hconcat(const TMat<T>& m1, const TMat<T>& m2) { return hconcat(Array< TMat<T> >(m1,m2)); }

//!  This will allow a convenient way of building arrays of Matrices by writing ex: m1&m2&m3
template<class T>
inline Array< TMat<T> > operator&(const TMat<T>& m1, const TMat<T>& m2) { return Array< TMat<T> >(m1,m2); } 
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

%> // end of namespace PLearn

#endif
