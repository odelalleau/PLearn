// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999,2000 Pascal Vincent, Yoshua Bengio and University of Montreal
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
   * $Id: TMat_impl.h,v 1.2 2003/03/18 18:29:56 ducharme Exp $
   * AUTHORS: Pascal Vincent & Yoshua Bengio & Rejean Ducharme
   * This file is part of the PLearn library.
   ******************************************************* */

/*! \file PLearnLibrary/PLearnCore/TMat_impl.h */

#ifndef TMAT_IMPL_H
#define TMAT_IMPL_H

namespace PLearn <%
using namespace std;


// **************
// **** TVec ****
// **************


  //!  Builds a Vec containing values ranging from start to stop with step
  //!  e.g., Vec(0,n-1,1) returns a vector of length() n, with 0,1,...n-1.
  //!  creates range (start, start+step, ..., stop)
  template <class T>
  TVec<T>::TVec(const T& start, const T& stop, const T& step)
    :length_(0), offset_(0)
  {
    // first count the size n
    T val;
    int n=0;
    for(val=start; val<=stop; val+=step)
      ++n;

    if(n)
    {
      resize(n);
      TVec<T>::iterator it = begin();
      TVec<T>::iterator itend = end();      
      for(val=start; it!=itend; ++it, val+=step)
        *it = val;
    }
  }


//!  The returned TMat will view the same data
template <class T>
TMat<T> TVec<T>::toMat(int newlength, int newwidth) const
{
  TMat<T> tm;
  tm.offset_ = offset_;
  tm.mod_ = newwidth;
  tm.width_ = newwidth;
  tm.length_ = newlength;
  tm.storage = storage;
  return tm;
}


// Deep copying
template<class T>
void TVec<T>::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  deepCopyField(storage, copies);
}

template<class T>
TVec<T> TVec<T>::deepCopy(map<const void*, void*>& copies) const
{
  // First do a shallow copy
  TVec<T> deep_copy = *this;
  // Transform the shallow copy into a deep copy
  deep_copy.makeDeepCopyFromShallowCopy(copies);
  // return the completed deep_copy
  return deep_copy;
}

template <class T>
void TVec<T>::input(istream& in) const
{
  T* v = data();
  for(int i=0; i<length(); i++)
  {
    if(!(in>>v[i]))
      PLERROR("In TVec::input error encountered while reading vector");
    
  }
}

template <class T>
void TVec<T>::print(ostream& out) const
{
  if(storage && 0 < length())
    {
      out.setf(ios::fmtflags(0),ios::floatfield);
      T* v = data();
      for(int i=0; i<length(); i++)
	out << setiosflags(ios::left) << setprecision(7) << setw(11) << v[i] << ' ';
      out.flush();
    }
}

template <class T>
void TVec<T>::print(ostream& out, const string& separator) const
{
  out.setf(ios::fmtflags(0),ios::floatfield);
  T* v = data();
  for(int i=0; i<length()-1; i++)
    out << v[i] << separator;
  out << v[length()-1];
  out.flush();
}

template <class T>
void TVec<T>::printcol(ostream& out) const
{
  T* v = data();
  for(int i=0; i<length(); i++)
    out << v[i] << "\n";
  out.flush();
}




// ***********************
// * Fonctions pout TVec *
// ***********************

/*!   select the elements of the source as specified by the
  vector of indices (between 0 and source.length()-1) into
  the destination vector (which must have the same length()
  as the indices vector).
*/
template<class T, class I>
void selectElements(const TVec<T>& source, const TVec<I>& indices, TVec<T>& destination)
{
  int ni = indices.length();
  if (ni!=destination.length())
    PLERROR("select(Vec,Vec,Vec): last 2 arguments have lengths %d != %d",
          indices.length(),destination.length());
  I* indx = indices.data();
  T* dest = destination.data();
  T* src = source.data();
#ifdef BOUNDCHECK
  int n=source.length();
#endif
  for (int i=0;i<ni;i++)
    {
      int pos = int(indx[i]);
#ifdef BOUNDCHECK
      if (pos<0 || pos>=n)
        PLERROR("select(Vec,Vec,Vec) indices[%d]=%d out of bounds (0,%d)",
              i,pos,n-1);
#endif
      dest[i] = src[pos];
    }
}

//! put in destination 1's when (*this)[i]==value, 0 otherwise
template<class T>
void elementsEqualTo(const TVec<T>& source, const T& value, const TVec<T>& destination)
{
  #if BOUNDCHECK
  if (source.length()!=destination.length())
    PLERROR("elementsEqualTo(Vec(%d),%f,Vec(%d)): incompatible dimensions",
          source.length(),value,destination.length());
  #endif
  T* src=source.data();
  T* dst=destination.data();
  for (int i=0;i<destination.length();i++)
    if (src[i]==value) dst[i]=1.0;
    else dst[i]=0.0;
}

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

/*  NOW IN Array.h
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
*/

//! if the element to remove is the first or the last one, 
//! then a submatrix (a view) of m will be returned (for efficiency)
//! otherwise, it is a fresh copy with the element removed.
template<class T>
TVec<T> removeElement(const TVec<T>& v, int elemnum)
{
  if(elemnum==0)
    return v.subVec(1,v.length()-1);
  else if(elemnum==v.length()-1)
    return v.subVec(0,v.length()-1);
  else
    return concat(v.subVec(0,elemnum),
                  v.subVec(elemnum+1,v.length()-(elemnum+1)));
}


// **************
// **** TMat ****
// **************

template <class T>
TMat<T>::TMat<T>(int the_length, int the_width, const TVec<T>& v)
  : offset_(v.offset()), mod_(the_width), length_(the_length), width_(the_width), storage(v.storage)
{
  if(length()*width()!=v.length())
    PLERROR("In Mat constructor from Vec: length()*width() of matrix must be equal to length() of Vec");
}

template <class T>
TVec<T> TMat<T>::operator()(int rownum) const
{
#ifdef BOUNDCHECK
  if(rownum<0 || rownum>=length())
    PLERROR("OUT OF BOUND ACCESS IN TMat_impl::operator()(int rownum)");
#endif
  TVec<T> tv;
  tv.length_ = width();
  tv.offset_ = offset_ + mod()*rownum;
  tv.storage = storage;
  return tv;
}

template <class T>
TVec<T> TMat<T>::toVecCopy() const
{
  TVec<T> v(length()*width());
  v << *this;
  return v;
}

//!  Views same data (not always possible)
template <class T>
TVec<T> TMat<T>::toVec() const
{
  if(length()>1 && width()<mod())
    PLERROR("In Mat::toVec internal structure of this Mat makes it impossible to build a Vec that would view exactly the same data. Consider using toVecCopy() instead!");
 
  TVec<T> v;
  v.offset_ = offset_;
  v.length_ = length()*width();
  v.storage = storage;
  return v;
}

template <class T>
int TMat<T>::findRow(const TVec<T>& row) const
{
  for(int i=0; i<length(); i++)
    if( (*this)(i)==row )
      return i;
  return -1;
}

template <class T>
void TMat<T>::appendRow(const TVec<T>& newrow)
{
#ifdef BOUNDCHECK
  if(newrow.length()!=width())
    PLERROR("In TMat::appendRow newrow vector should be as long as the matrix is wide");
#endif
  resize(length()+1,width());
  (*this)(length()-1) << newrow;
}


// C++ stream output
template <class T>
void TMat<T>::print(ostream& out) const
{
  out.flags(ios::left);
  for(int i=0; i<length(); i++)
    {
      const T* m_i = rowdata(i);
      for(int j=0; j<width(); j++)
        out << setw(11) << m_i[j] << ' ';
      out << "\n";
    }
  out.flush();
}

template <class T>
void TMat<T>::input(istream& in) const
{
  for(int i=0; i<length(); i++)
    {
      T* v = rowdata(i);
      for (int j=0;j<width();j++)
        {
          if(!(in>>v[j]))
            PLERROR("In TMat<T>::input error encountered while reading matrix");
        }
    }
}


// Deep copying

template<class T>
void TMat<T>::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  deepCopyField(storage, copies);
}

template<class T>
TMat<T> TMat<T>::deepCopy(map<const void*, void*>& copies) const
{
  // First do a shallow copy
  TMat<T> deep_copy = *this;
  // Transform the shallow copy into a deep copy
  deep_copy.makeDeepCopyFromShallowCopy(copies);
  // return the completed deep_copy
  return deep_copy;
}


// *****************************
// **** Fonctions pour TMat ****
// *****************************


// select the rows of the source as specified by the
// vector of indices (between 0 and source.length()-1), copied into
// the destination matrix (which must have the same length()
// as the indices vector).
template <class T, class I>
void selectRows(const TMat<T>& source, const TVec<I>& row_indices, TMat<T>& destination)
{
  int ni = row_indices.length();
  if (ni!=destination.length())
    PLERROR("selectRows(Mat,Vec,Mat): last 2 arguments have lengths %d != %d",
        ni,destination.length());
  I* indx = row_indices.data();
#ifdef BOUNDCHECK
  int n=source.length();
#endif
  for (int i=0;i<ni;i++)
  {
    int pos = int(indx[i]);
#ifdef BOUNDCHECK
    if (pos<0 || pos>=n)
      PLERROR("selectRows(Mat,Vec,Mat) indices[%d]=%d out of bounds (0,%d)",
          i,pos,n-1);
#endif
    destination(i) << source(pos);
  }
}

// select the colums of the source as specified by the
// vector of indices (between 0 and source.length()-1), copied into
// the destination matrix (which must have the same width()
// as the indices vector).
template <class T, class I>
void selectColumns(const TMat<T>& source, const TVec<I>& column_indices, TMat<T>& destination)
{
  int ni = column_indices.length();
  if (ni!=destination.width())
    PLERROR("selectColums(Mat,Vec,Mat): last 2 arguments have dimensions %d != %d",
        ni,destination.width());
  I* indx = column_indices.data();
#ifdef BOUNDCHECK
  int n=source.width();
#endif
  for (int i=0;i<ni;i++)
  {
    int pos = int(indx[i]);
#ifdef BOUNDCHECK
    if (pos<0 || pos>=n)
      PLERROR("selectColumns(Mat,Vec,Mat) indices[%d]=%d out of bounds (0,%d)",
          i,pos,n-1);
#endif
    destination.column(i) << source.column(pos);
  }
}

// select a submatrix of specified rows and colums of the source with
// two vectors of indices. The elements that are both in the specified rows
// and columns are copied into the destination matrix (which must have the 
// same length() as the row_indices vector, and the same width() as the length()
// of the col_indices vector).
template <class T, class I>
void select(const TMat<T>& source, const TVec<I>& row_indices, const TVec<I>& column_indices, TMat<T>& destination)
{
  int rni = row_indices.length();
  int cni = column_indices.length();
  if (rni!=destination.length() || cni!=destination.width())
    PLERROR("select(Mat(%d,%d),Vec(%d),Vec(%d),Mat(%d,%d)): arguments have incompatible dimensions",
        source.length(),source.width(),rni,cni,destination.length(),destination.width());
  I* rindx = row_indices.data();
  I* cindx = column_indices.data();
#ifdef BOUNDCHECK
  int nr=source.length();
  int nc=source.width();
#endif
  for (int i=0;i<rni;i++)
  {
    int ri=(int)rindx[i];
#ifdef BOUNDCHECK
    if (ri<0 || ri>=nr)
      PLERROR("select(Mat,Vec,Vec,Mat) row_indices[%d]=%d out of bounds (0,%d)",
          i,ri,nr-1);
#endif
    T* dest_row = destination[i];
    T* src_row = source[ri];
    for (int j=0;j<cni;j++)
    {
      int cj = (int)cindx[j];
#ifdef BOUNDCHECK
      if (cj<0 || cj>=nc)
        PLERROR("select(Mat,Vec,Vec,Mat) col_indices[%d]=%d out of bounds (0,%d)",
            i,cj,nc-1);
#endif
      dest_row[j] = src_row[cj];
    }
  }
}

/* NOW IN Array.h
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
*/

template<class T>
TMat<T> removeRow(const TMat<T>& m, int rownum)
{
  if(rownum==0)
    return m.subMatRows(1,m.length()-1);
  else if(rownum==m.length()-1)
    return m.subMatRows(0,m.length()-1);
  else
    return vconcat(m.subMatRows(0,rownum),
                   m.subMatRows(rownum+1,m.length()-(rownum+1)));
}

template<class T>
TMat<T> removeColumn(const TMat<T>& m, int colnum)
{
  if(colnum==0)
    return m.subMatColumns(1,m.width()-1);
  else if(colnum==m.width()-1)
    return m.subMatColumns(0,m.width()-1);
  else
    return hconcat(m.subMatColumns(0,colnum),
                   m.subMatColumns(colnum+1,m.width()-(colnum+1)));
}

template<class T>
TMat<T> diagonalmatrix(const TVec<T>& v)
{
  TMat<T> m(v.length(), v.length());
  for(int i=0; i<v.length(); i++)
    m(i,i) = v[i];
  return m;
}


%> // end of namespace PLearn

#endif // TMAT_IMPL_H
