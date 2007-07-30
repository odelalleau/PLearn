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
 * $Id$
 * AUTHORS: Pascal Vincent & Yoshua Bengio & Rejean Ducharme
 * This file is part of the PLearn library.
 ******************************************************* */

/*! \file PLearnLibrary/PLearnCore/TMat_impl.h */

#ifndef TMAT_IMPL_H
#define TMAT_IMPL_H

#include "TMat_decl.h"
#include "TMatElementIterator_impl.h"
#include "TMatRowsIterator_impl.h"
#include "TMatRowsAsArraysIterator_impl.h"
#include "TMatColRowsIterator_impl.h"

//#include "algo.h"

namespace PLearn {
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
        iterator it = begin();
        iterator itend = end();      
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
void TVec<T>::input(PStream& in) const
{
    T* v = data();
    int l = length();
    for(int i=0; i<l; i++)
    {
        in.skipBlanksAndCommentsAndSeparators();
        if(in.peek()==EOF || in.eof())
            PLERROR("In TVec::input encountered EOF before reading the last element of %d",l);
        in>>v[i];
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
#ifdef BOUNDCHECK
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

// Returns an index vector I so that (*this)(I) returns a sorted version
// of this vec in ascending order.
namespace {
  template <class T>
  struct index_cmp : public binary_function<int, int, bool>
  {
      const TVec<T>& m_values;
      index_cmp(const Vec& values): m_values(values) { }        
      bool operator()(int x, int y) { return m_values[x] < m_values[y]; }
  };
}
// Actual body of the method
template <class T>
TVec<int> TVec<T>::sortingPermutation() const
{    
    TVec<int> indices(length_);
    for (int i=0; i < length_; i++) indices[i] = i;
    sort(indices.begin(), indices.end(), index_cmp<T>(*this));
    return indices;
}


// **************
// **** TMat ****
// **************

template <class T>
TMat<T>::TMat(int the_length, int the_width, const TVec<T>& v)
    : offset_(v.offset()), mod_(the_width), length_(the_length), width_(the_width), storage(v.storage)
{
    if(length()*width()!=v.length())
        PLERROR("In Mat constructor from Vec: length()*width() of matrix must be equal to length() of Vec");
}


template <class T>
TVec<T> TMat<T>::toVecCopy() const
{
    TVec<T> v(length()*width());
    v << *this;
    return v;
}

//!  Views same data (not always possible)
//!  Actually it's the matrix view rows by rows

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
    if(newrow.length()!=width() && width() > 0)
        PLERROR("In TMat::appendRow newrow vector should be as long as the matrix is wide (%d != %d)", newrow.length(), width());
#endif
    if (storage) {
        resize(length()+1, newrow.length(), storage->length());
    } else {
        // This Mat is empty: it has no storage, so using storage would crash.
        resize(length()+1, newrow.length());
    }
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

template <class T>
void TMat<T>::input(PStream& in) const
{
    for(int i=0; i<length(); i++)
    {
        T* v = rowdata(i);
        for (int j=0;j<width();j++)
        {
            if (!in)
                PLERROR("In TMat<T>::input error encountered while reading matrix");
            else
                in>>v[j];
        }
    }
}

template <class T>
void TMat<T>::resizePreserve(int new_length, int new_width, int extra)
{
    int usage      = storage->usage();
    int new_size   = new_length*MAX(mod(),new_width);
    int new_offset = usage>1?offset_:0;
    if (new_size>storage->length() || new_width>mod())
    {
        int extracols=0, extrarows=0;
        if (extra>min(new_width,new_length))
        {
            // if width has increased, bet that it will increase again in the future,
            // similarly for length,  so allocate the extra as extra mod
            float l=float(length_), l1=float(new_length),
                w=float(width_),  w1=float(new_width),
                x=float(extra);
            // Solve the following equations to apportion the extra 
            // while keeping the same percentage increase in width and length:
            //   Solve[{x+w1*l1==w2*l2,(w2/w1 - 1)/(l2/l1 - 1) == (w1/w - 1)/(l1/l - 1)},{w2,l2}]
            // This is a quadratic system which has two solutions: {w2a,l2a} and {w2b,l2b}:
            float w2a = 
                w1*(-1 - l1/(l - l1) + w1/w + (l1*w1)/(l*w - l1*w) + 
                    (2*l*(-w + w1)*x)/
                    (2*l*l1*w*w1 - l1*l1*w*w1 - l*l1*w1*w1 + 
                     sqrt(square(l1*l1*w*w1 - l*l1*w1*w1) + 
                          4*l*(l - l1)*l1*w*(w - w1)*w1*(l1*w1 + x))));
            float l2a = -(-l1*l1*w*w1 + l*l1*w1*w1 + 
                          sqrt(square(l1*l1*w*w1 - l*l1*w1*w1) + 
                               4*l*(l - l1)*l1*w*(w - w1)*w1*(l1*w1 + x)))/(2*l*(w - w1)*w1);
            float w2b =w1*(-1 - l1/(l - l1) + w1/w + (l1*w1)/(l*w - l1*w) - 
                           (2*l*(-w + w1)*x)/
                           (-2*l*l1*w*w1 + l1*l1*w*w1 + l*l1*w1*w1 + 
                            sqrt(square(l1*l1*w*w1 - l*l1*w1*w1) + 
                                 4*l*(l - l1)*l1*w*(w - w1)*w1*(l1*w1 + x))));
            float l2b = (l1*l1*w*w1 - l*l1*w1*w1 + 
                         sqrt(square(l1*l1*w*w1 - l*l1*w1*w1) + 
                              4*l*(l - l1)*l1*w*(w - w1)*w1*(l1*w1 + x)))/(2*l*(w - w1)*w1);

            // pick one that is feasible and maximizes the mod
            if (w2b>w2a && w2b>w1 && l2b>l1) {
                extracols=int(ceil(w2b-w1));
                extrarows=int(ceil(l2b-l1));
            }
            else if (w2a>w1 && l2a>l1) {
                extrarows=int(ceil(l2a-l1));
                extracols=int(ceil(w2a-w1));
            }
            else { // no valid solution to the system of equation, use a heuristic
                extracols = max(0,int(ceil(sqrt(real(extra))/new_length)));
                extrarows = max(0,int((extra+l1*w1)/(w1+extracols) - l1));
            }

        }
        storage->resizeMat(new_length,new_width,extrarows,extracols,
                           new_offset,mod_,length_,width_,offset_);
        mod_ = new_width + extracols;
    }
    offset_ = new_offset;
}

template <class T>
inline void TMat<T>::resizeBoundCheck(int new_length, int new_width)
{
    if(new_length<0 || new_width<0)
        PLERROR("IN TMat::resize(int new_length, int new_width)\nInvalid arguments (%d, %d)", new_length, new_width);
}

template <class T>
void TMat<T>::resizeModError()
{
    PLERROR("IN TMat::resize(int new_length, int new_width) - For safety "
            "reasons, increasing the width() beyond mod()-offset_ modulo "
            "mod() is not allowed when the storage is shared with others");
}


// Deep copying

template<class T>
void TMat<T>::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    deepCopyField(storage, copies);
}

template<class T>
TMat<T> TMat<T>::deepCopy(CopiesMap& copies) const
{
    // First do a shallow copy
    TMat<T> deep_copy = *this;
    // Transform the shallow copy into a deep copy
    deep_copy.makeDeepCopyFromShallowCopy(copies);
    // return the completed deep_copy
    return deep_copy;
}

// Iterateurs

template<class T>
TMatElementIterator<T> TMat<T>::begin() const
{ return TMatElementIterator<T>(data(), width_, mod_); }

template<class T>
TMatElementIterator<T> TMat<T>::end() const
{ return TMatElementIterator<T>(data()+length_*mod_, width_, mod_); }


template<class T>
TMatRowsIterator<T> TMat<T>::rows_begin() {
    return TMatRowsIterator<T>(data(), width_, mod_);
}

template<class T>
TMatRowsIterator<T> TMat<T>::rows_end() {
    return TMatRowsIterator<T>(data()+length_*mod_, width_, mod_);
}


template<class T>
TMatRowsAsArraysIterator<T> TMat<T>::rows_as_arrays_begin() {
    return TMatRowsAsArraysIterator<T>(data(), width_, mod_);
}

template<class T>
TMatRowsAsArraysIterator<T> TMat<T>::rows_as_arrays_end() {
    return TMatRowsAsArraysIterator<T>(data()+length_*mod_, width_, mod_);
}

template<class T>
TMatColRowsIterator<T> TMat<T>::col_begin(int column) {
    return TMatColRowsIterator<T>(data() + column, mod_);
}

template<class T>
TMatColRowsIterator<T> TMat<T>::col_end(int column) {
    return TMatColRowsIterator<T>(data()+length_*mod_+column, mod_);
}

template<class T>
bool TMat<T>::operator==(const TMat<T>& other) const
{
    if ( length() != other.length() || width() != other.width() )
        return false;
  
    iterator it       = begin();
    iterator end_     = end();
    iterator other_it = other.begin();

    for(; it != end_; ++it, ++other_it)
        if(*it != *other_it)
            return false;
  
    return true;
}

template<class T>
bool TMat<T>::isEqual(const TMat<T>& other, real precision) const
{
    if ( length() != other.length() || width() != other.width() )
        return false;
  
    iterator it       = begin();
    iterator end_     = end();
    iterator other_it = other.begin();

    for(; it != end_; ++it, ++other_it)
        if( !is_equal(*it,*other_it, 10.0, precision, precision) )
            return false;
  
    return true;
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

    if (row_indices.isEmpty())
        // Nothing to select. In addition, 'destination' is empty too since it
        // has zero length, according to the test above. Thus there is nothing
        // to do.
        return;

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
// vector of indices (between 0 and source.width()-1), copied into
// the destination matrix (which must have the same width()
// as the indices vector).
template <class T, class I>
void selectColumns(const TMat<T>& source, const TVec<I>& column_indices, TMat<T>& destination)
{
    int ni = column_indices.length();
    if (ni!=destination.width())
        PLERROR("selectColums(Mat,Vec,Mat): last 2 arguments have dimensions %d != %d",
                ni,destination.width());

    if (column_indices.isEmpty())
        // Nothing to select. In addition, 'destination' is empty too since it
        // has zero width, according to the test above. Thus there is nothing
        // to do.
        return;

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



// *****************************
// **** Fonctions pour TMat ****
// *****************************

template <class T> inline TMat<T> deepCopy(const TMat<T> source)
{
    CopiesMap copies; //!< create empty map
    return deepCopy(source, copies);
}

template <class T> inline TMat<T>
deepCopy(const TMat<T> source, CopiesMap copies)
{ return source.deepCopy(copies); }

template <class T>
inline void deepCopyField(TMat<T>& field, CopiesMap& copies)
{
    field.makeDeepCopyFromShallowCopy(copies);
}

template<class T>
void clear(const TMat<T>& x)
{ 
    if(x.isCompact())
    {
        typename TMat<T>::compact_iterator it = x.compact_begin();
        typename TMat<T>::compact_iterator itend = x.compact_end();
        for(; it!=itend; ++it)
            clear(*it);
    }
    else
    {
        typename TMat<T>::iterator it = x.begin();
        typename TMat<T>::iterator itend = x.end();
        for(; it!=itend; ++it)
            clear(*it);
    }
}

template<class T>
void swap( TMat<T>& a, TMat<T>& b)
{ swap_ranges(a.begin(), a.end(), b.begin()); }

//! copy TMat << TMat 
template<class T>
inline void operator<<(const TMat<T>& m1, const TMat<T>& m2)
{
#ifdef BOUNDCHECK
    if(m1.size()!=m2.size())
        PLERROR("In operator<<(m1,m2) the 2 matrices must have the same number of elements\n"
                "m1: (%d, %d) && m2: (%d, %d)", m1.length(), m1.width(), m2.length(), m2.width());
#endif
    if (m1.isNotEmpty())
        copy(m2.begin(), m2.end(), m1.begin());
}
  
//! copy TMat << TMat  (different types)
template<class T, class U>
void operator<<(const TMat<T>& m1, const TMat<U>& m2)
{
#ifdef BOUNDCHECK
    if(m1.size()!=m2.size())
        PLERROR("In operator<<(m1,m2) the 2 matrices must have the same number of elements");
#endif
    if (m1.isNotEmpty())
        copy_cast(m2.begin(), m2.end(), m1.begin());
}

//! copy TMat << Tvec 
template<class T>
inline void operator<<(const TMat<T>& m1, const TVec<T>& m2)
{
#ifdef BOUNDCHECK
    if(m1.size()!=m2.size())
        PLERROR("In operator<<(m1,m2) the 2 matrices must have the same number of elements;\t m1.size()= %d;\t m2.size= %d", m1.size(), m2.size());
#endif
    if (m1.isNotEmpty())
        copy(m2.begin(), m2.end(), m1.begin());
}

//! copy TMat << Tvec  (different types)
template<class T, class U>
inline void operator<<(const TMat<T>& m1, const TVec<U>& m2)
{
#ifdef BOUNDCHECK
    if(m1.size()!=m2.size())
        PLERROR("In operator<<(m1,m2) the 2 matrices must have the same number of elements");
#endif
    if (m1.isNotEmpty())
        copy_cast(m2.begin(), m2.end(), m1.begin());
}

//! copy TVec << TMat
template<class T>
inline void operator<<(const TVec<T>& m1, const TMat<T>& m2)
{
#ifdef BOUNDCHECK
    if(m1.size()!=m2.size())
        PLERROR("In operator<<(m1,m2) the 2 matrices must have the same number of elements");
#endif
    if (m1.isNotEmpty())
        copy(m2.begin(), m2.end(), m1.begin());
}

//! copy TVec << TMat  (different types)
template<class T, class U>
inline void operator<<(const TVec<T>& m1, const TMat<U>& m2)
{
#ifdef BOUNDCHECK
    if(m1.size()!=m2.size())
        PLERROR("In operator<<(m1,m2) the 2 matrices must have the same number of elements");
#endif
    if (m1.isNotEmpty())
        copy_cast(m2.begin(), m2.end(), m1.begin());
}

//! copy TMat >> TMat
template<class T, class U>
inline void operator>>(const TMat<T>& m1, const TMat<U>& m2)
{ m2 << m1; }

//! copy TVec >> TMat
template<class T, class U>
inline void operator>>(const TVec<T>& m1, const TMat<U>& m2)
{ m2 << m1; }

//! copy TMat >> Tvec
template<class T, class U>
inline void operator>>(const TMat<T>& m1, const TVec<U>& m2)
{ m2 << m1; }



//! printing a TMat
template <class T>
inline ostream& operator<<(ostream& out, const TMat<T>& m)
{ 
    m.print(out);
    return out;
}

//! inputing a TMat

template <class T>
inline istream& operator>>(istream& in, const TMat<T>& m)
{ 
    m.input(in);
    return in;
}

//!  returns a view of this vector as a single row matrix
template <class T>
inline TMat<T> rowmatrix(const TVec<T>& v)
{ return v.toMat(1,v.length()); }

//!  returns a view of this vector as a single column matrix
template <class T>
inline TMat<T> columnmatrix(const TVec<T>& v)
{ return v.toMat(v.length(),1); }

// select the rows of the source as specified by the
// vector of indices (between 0 and source.length()-1), copied into
// the destination matrix (which must have the same length()
// as the indices vector).
template <class T, class I>
void selectRows(const TMat<T>& source, const TVec<I>& row_indices, TMat<T>& destination);

// select the colums of the source as specified by the
// vector of indices (between 0 and source.length()-1), copied into
// the destination matrix (which must have the same width()
// as the indices vector).
template <class T, class I>
void selectColumns(const TMat<T>& source, const TVec<I>& column_indices, TMat<T>& destination);

// select a submatrix of specified rows and colums of the source with
// two vectors of indices. The elements that are both in the specified rows
// and columns are copied into the destination matrix (which must have the 
// same length() as the row_indices vector, and the same width() as the length()
// of the col_indices vector).
template <class T>
void select(const TMat<T>& source, const TVec<T>& row_indices, const TVec<T>& column_indices, TMat<T>& destination);

//! returns a new mat which is m with the given row removed
//! if the row to remove is the first or the last one, 
//! then a submatrix (a view) of m will be returned (for efficiency)
//! otherwise, it is a fresh copy with the row removed. 
template<class T>
TMat<T> removeRow(const TMat<T>& m, int rownum);

//! returns a new mat which is m with the given column removed
//! if the column to remove is the first or the last one, 
//! then a submatrix (a view) of m will be returned (for efficiency)
//! otherwise, it is a fresh copy with the column removed. 
template<class T>
TMat<T> removeColumn(const TMat<T>& m, int colnum);


template<class T>
TMat<T> diagonalmatrix(const TVec<T>& v);

// old .pmat format
template<class T>
void savePMat(const string& filename, const TMat<T>& mat)
{ PLERROR("savePMat only implemented for float and double"); }

template<class T>
void loadPMat(const string& filename, TMat<float>& mat)
{ PLERROR("loadPMat only implemented for float and double"); }

inline void deepCopyField(Mat*& field, CopiesMap& copies)
{
    if (field)
    {
        CopiesMap::iterator it = copies.find(field);
        if (it != copies.end())                
            field = static_cast<Mat*>(it->second);
        else
        {
            // Throw an error. The reason is that:
            // - if the Mat* pointer points to a matrix that has already been
            // deep-copied, I am unsure whether 'copies' contains the correct
            // pointer, thus we may end up here even though we should reuse the
            // previous deep copy,
            // - if the Mat* pointer points to a matrix that has not been deep
            // copied yet, then when that matrix is deep copied it will not
            // actually be the same as the matrix we may create here.
            PLERROR("In deepCopyField(Mat*& field, CopiesMap& copies) - You "
                    "cannot deep copy a Mat* directly.");
            /* Old code.
            Mat* newM = new Mat; 
            (*newM) = field->deepCopy(copies);
            copies[field] = newM;
            field = newM;
            */
        }
    }
}


//!  Read and Write from C++ stream:
//!  write saves length() and width(), and read resizes accordingly

//!  Read and Write from C++ stream:
//!  write saves length and read resizes accordingly
//! (the raw modes don't write any size information)

template <class T> inline PStream &
operator<<(PStream &out, const TMat<T> &m)
{ 
    m.write(out); 
    return out;
}

template <class T> 
PStream & operator>>(PStream &in, TMat<T> &m)
{
    m.read(in);
    return in;
}

inline string join(const TVec<string>& s, const string& separator)
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

} // end of namespace PLearn

#endif // TMAT_IMPL_H


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
