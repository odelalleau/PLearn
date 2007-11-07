// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1999-2002 Yoshua Bengio
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

/*! \file PLearn/plearn/math/RowMapSparseMatrix.h */

#ifndef ROWMAPSPARSEMATRIX
#define ROWMAPSPARSEMATRIX

#include <map>
#include "Mat.h"
#include "TMat.h"
#include "SparseMatrix.h"

namespace PLearn {
using namespace std;


/*!   
  Sparse matrices implemented with STL maps.
  
  We assume that there are elements in each ROW.
  
  We associate an STL map to each row: column index --> value
  
  Space used is about O( size_of_elements * number_of_non_zero_elements )
  
  Random access time is O(log(number_of_elements_per_row))
  
  Row-wise iterations can be done in constant time per access.
  
  Binary or ascii load/save streaming are available.
  Recommended filename extensions are .armsm and .brmsm 
  respectively for Ascii Row Map Sparse Matrix or Binary Row Map Sparse Matrix.
  
*/

template <class T>
class RowMapSparseMatrix : public PPointable {
public:
    mutable vector< map<int,T> > rows;
protected:
    int _width;

public:
    bool save_binary;
    T null_elem;

    RowMapSparseMatrix(int n_rows=0, int n_columns=0, T nullelem=0) 
        : rows(n_rows), _width(n_columns), save_binary(true), null_elem(nullelem) {}

//    RowMapSparseMatrix(string filename) { load(filename); }

    RowMapSparseMatrix(const Mat& m, bool fill_all=true, T nullelem=0) : rows(m.length()), _width(m.width()), 
                                                                         save_binary(true), null_elem(nullelem)
    {
        if (fill_all)
            for (int i=0;i<length();i++)
            {
                real* r=m[i];
                map<int,T>& row_i=rows[i];
                for (int j=0;j<width();j++)
                    row_i[j]=T(r[j]);
            }
        else
            for (int i=0;i<length();i++)
            {
                real* r=m[i];
                map<int,T>& row_i=rows[i];
                for (int j=0;j<width();j++){
                    if(T(r[j])!=0)
                        row_i[j]=T(r[j]);
                }
            }
    }

    //!  Accepts a FORTRAN formatted sparse matrix as an initializer.
    RowMapSparseMatrix(const SparseMatrix& sm, int n_rows, int n_cols, T nullelem=0) : 
        rows(n_rows), _width(n_cols), save_binary(false), null_elem(nullelem) {

        for (int j = 0; j < n_cols; j++) {
            int bcol_j = (int)sm.beginRow[j];
            int ecol_j = (int)sm.endRow[j];
            for (int row_i = bcol_j; row_i <= ecol_j; row_i++) {
                int i = (int)sm.row[row_i];
                (*this)(i, j) = (T)sm.values[row_i];
            }
        }

    }

    //!  THIS ALSO CLEARS THE MATRIX
    void resize(int n_rows, int n_columns) {
        rows.resize(n_rows);
        _width=n_columns;
        clear();
    }
      
    //!  this is equivalent to setting all values to "0" (the default value of T)
    void clear() {
        int s=rows.size();
        for (int i=0;i<s;i++)
            rows[i].clear();
    }

    void clearRow(int i)
    {
        rows[i].clear();
    }

    T& operator()(int i, int j) { 
#ifdef BOUNDCHECK      
        if (i<0 || i>=length() || j<0 || j>=width())
            PLERROR("RowMapSparseMatrix: out-of-bound access to (%d,%d), dims=(%d,%d)",
                    i,j,length(),width());
#endif
        map<int,T>& row_i = rows[i];
        typename map<int,T>::iterator it = row_i.find(j);
        if (it==row_i.end())
            return null_elem;
        return it->second;
    }


    const T& operator()(int i, int j) const { 
#ifdef BOUNDCHECK      
        if (i<0 || i>=length() || j<0 || j>=width())
            PLERROR("RowMapSparseMatrix: out-of-bound access to (%d,%d), dims=(%d,%d)",
                    i,j,length(),width());
#endif
        map<int,T>& row_i = rows[i];
        typename map<int,T>::const_iterator it = row_i.find(j);
        if (it==row_i.end())
            return null_elem;
        return it->second;
    }

    T get(int i, int j) const { return (*this)(i,j); }

    void set(int i, int j, T v) const { 
#ifdef BOUNDCHECK      
        if (i<0 || i>=length() || j<0 || j>=width())
            PLERROR("RowMapSparseMatrix: out-of-bound access to (%d,%d), dims=(%d,%d)",
                    i,j,length(),width());
#endif
        map<int,T>& row_i = rows[i];
        row_i[j] = v;
    }

    bool exists(int i, int j) const { 
#ifdef BOUNDCHECK      
        if (i<0 || i>=length() || j<0 || j>=width())
            PLERROR("RowMapSparseMatrix: out-of-bound access to (%d,%d), dims=(%d,%d)",
                    i,j,length(),width());
#endif
        map<int,T>& row_i = rows[i];
        typename map<int,T>::const_iterator it = row_i.find(j);
        return (it!=row_i.end());
    }

/*!       Get i-th row. Exemple to iterate on i-th row:
      
map<int,T>& row_i = A(i); <  note very important: row_i is a reference (&)
map<int,T>::const_iterator it = row_i.begin();
map<int,T>::const_iterator end = row_i.end();
for (;it!=end;++it)
{
int j = it->first;
T Aij = it->second;
...
}
*/
    map<int,T>& operator()(int i) { return rows[i]; }
    map<int,T>& getRow(int i) { return rows[i]; }

    //!  NOTE THIS IS A BIT EXPENSIVE!
    int size() const { 
        int _size=0;
        for (unsigned int i=0;i<rows.size();i++)
            _size += rows[i].size();
        return _size;
    }
    int length() const { return rows.size(); }
    int width() const { return _width; }

    void copyRowFrom(int i, const map<int,T>& from_row, bool clear_rest=true)
    {
        map<int,T>& row = rows[i];
        if (clear_rest)
            clearRow(i);
        typename map<int,T>::const_iterator it = from_row.begin();
        typename map<int,T>::const_iterator end = from_row.end();
        for (;it!=end;++it)
            row[it->first]=it->second;
    }

#if 0
    void save(string filename) const { 
        ofstream out(filename.c_str()); write(out); 
    }
    void saveAscii(string filename) { 
        bool old_mode=save_binary;
        save_binary = false; save(filename); 
        save_binary=old_mode;
    }
    //!  determines automatically whether file is binary or ascii
    void load(string filename) { ifstream in(filename.c_str()); read(in); }
    void read(istream& in) {
        readHeader(in,"RowMapSparseMatrix");
        int n_bytes=0;
        readField(in,"sizeof(T)",n_bytes,4);
        readField(in,"save_binary",save_binary);
        if (n_bytes!=sizeof(T) && save_binary)
            PLERROR("RowMapSparseMatrix<T>::read, input matrix has sizeof(T)=%d, expected %d",n_bytes,sizeof(T));
        int n_rows;
        readField(in,"n_rows",n_rows);
        rows.resize(n_rows);
        readField(in,"width",_width);
        readField(in,"null_elem",null_elem);
        for (int i=0;i<n_rows;i++)
        {
            map<int,T>& row_i = rows[i];
            int n_elements;
            if (save_binary)
            {
                binread(in,n_elements);
                for (int j=0;j<n_elements;j++)
                {
                    int col;
                    T value;
                    binread(in,col);
                    binread(in,value);
                    row_i[col]=value;
                }
            } else
            {
                PLearn::read(in,n_elements);
                for (int j=0;j<n_elements;j++)
                {
                    int col;
                    T value;
                    PLearn::read(in,col);
                    PLearn::read(in,value);
                    row_i[col]=value;
                }
            }
        }
        readFooter(in,"RowMapSparseMatrix");
    }

    //!  mode used to write depends on save_binary flag
    void write(ostream& out) const {
        writeHeader(out,"RowMapSparseMatrix");
        int sizeofT = sizeof(T);
        writeField(out,"sizeof(T)",sizeofT);
        writeField(out,"save_binary",save_binary);
        writeField(out,"n_rows",rows.size());
        writeField(out,"width",_width);
        writeField(out,"null_elem",null_elem);
        int s=rows.size();
        for (int i=0;i<s;i++)
        {
            const map<int,T>& row_i = rows[i];
            typename map<int,T>::const_iterator it = row_i.begin();
            typename map<int,T>::const_iterator end = row_i.end();
            int n_elements = row_i.size();
            if (save_binary)
            {
                binwrite(out,n_elements);
                for (;it!=end;++it)
                {
                    binwrite(out,it->first);
                    binwrite(out,it->second);
                }
            }
            else
            {
                PLearn::write(out,n_elements);
                for (;it!=end;++it)
                {
                    PLearn::write(out,it->first);
                    PLearn::write(out,it->second);
                }
            }
        }
        writeFooter(out,"RowMapSparseMatrix");
    }
/*!       saves the non-zero elements in an ascii file with the following simple format:
  first line: <length> <width> <numberNonZero>
  subsequent lines: <row> <column> <value>
*/
    void saveNonZeroElements(string filename) const {
        ofstream out(filename.c_str());
        out << length() << " " << width() << " " << size() << endl;
        int s=rows.size();
        for (int i=0;i<s;i++)
        {
            const map<int,T>& row_i = rows[i];
            typename map<int,T>::const_iterator it = row_i.begin();
            typename map<int,T>::const_iterator end = row_i.end();
            for (;it!=end;++it)
                out << i << " " << it->first << " " << it->second << endl;
        }
    }

#endif
    //!  multiply a sparse matrix by a full vector and set resulting vector
    //!    y = matrix * x
    void product(const Vec& x, Vec& y) {
        real* _y=y.data();
        real* _x=x.data();
#ifdef BOUNDCHECK
        if (y.length()!=length() || x.length()!=width())
            PLERROR("RowMapSparseMatrix::product: inconsistent arguments (%d,%d), dims=(%d,%d)",
                    x.length(),y.length(),length(),width());
        if (null_elem!=0)
            PLERROR("RowMapSparseMatrix::product works only when null_elem=0, but was %g\n",null_elem);
#endif
        int s=rows.size();
        for (int i=0;i<s;i++)
        {
            real res=0;
            map<int,T>& row_i = rows[i];
            typename map<int,T>::const_iterator it = row_i.begin();
            typename map<int,T>::const_iterator end = row_i.end();
            for (;it!=end;++it)
                res += it->second * _x[it->first];
            _y[i] = res;
        }
    }

    Mat toMat()
    {
        Mat res(length(),width());
        if (null_elem!=0) res.fill(null_elem);
        for (int i=0;i<length();i++)
        {
            map<int,T>& row_i = rows[i];
            typename map<int,T>::const_iterator it = row_i.begin();
            typename map<int,T>::const_iterator end = row_i.end();
            real* res_i=res[i];
            for (;it!=end;++it)
                res_i[it->first] = it->second;
        }
        return res;
    }

    bool isSymmetric(real tolerance=0)
    {
        for (int i=0;i<length();i++)
        {
            map<int,T>& row_i = rows[i];
            typename map<int,T>::const_iterator it = row_i.begin();
            typename map<int,T>::const_iterator end = row_i.end();
            for (;it!=end;++it)
                if (it->first!=i)
                {
                    T& other_guy = (*this)(it->first,i);
                    if (fabs(other_guy - it->second)>tolerance)
                        return false;
                }
        }
        return true;
    }

/*!       if A(i,j) is specified and not A(j,i)
  then set A(j,i)=A(i,j). 
  if both were specified but different,
  up to the tolerance (in abs. value difference),
  then abort the operation and return false (there is a problem!).
  Otherwise return true.
*/
    bool fillSymmetricPart(real tolerance=0)
    {
        for (int i=0;i<length();i++)
        {
            map<int,T>& row_i = rows[i];
            typename map<int,T>::const_iterator it = row_i.begin();
            typename map<int,T>::const_iterator end = row_i.end();
            for (;it!=end;++it)
            {
                int j=it->first;
                if (j!=i)
                {
                    //!  see if entry i exists in row j
                    map<int,T>& row_j= rows[j];
                    typename map<int,T>::iterator symm_it =row_j.find(i);
                    if (symm_it==row_j.end())
                        //!  not found, then set A(j,i) to A(i,j)
                        row_j[i]=it->second;
                    else
                        //!  check if they are equal
                        if (fabs(symm_it->second - it->second)>tolerance)
                            return false;
                }
            }
        }
        return true;
    }

    //!  set d[i] = A[i,i]
    void diag(Vec& d)
    {
        real* d_=d.data();
        for (int i=0;i<length();i++)
            d_[i] = (*this)(i,i);
    }

    //!  d = diagonal(A*A'), i.e.
    //!  d[i] = |A[i]|^2 where A[i] is i-th row
    void diagonalOfSquare(Vec& d)
    {
        real* d_=d.data();
        for (int i=0;i<length();i++)
        {
            real sum2=0;
            map<int,T>& row_i = rows[i];
            typename map<int,T>::const_iterator it = row_i.begin();
            typename map<int,T>::const_iterator end = row_i.end();
            for (;it!=end;++it)
                sum2 += it->second * it->second;
            d_[i] = sum2;
        }
    }

    //!  return dot product of i-th row with vector v
    real dotRow(int i, Vec v)
    {
#ifdef BOUNDCHECK
        if (v.length()!=width())
            PLERROR("RowMapSparseMatrix::dotRow(%d,v), v.length_=%d != matrix width=%d",
                    i,v.length(),width());
        if (null_elem!=0)
            PLERROR("RowMapSparseMatrix::dotRow works only when null_elem=0, but was %g\n",null_elem);
#endif
        real s = 0;
        real* v_=v.data();
        map<int,T>& row_i = rows[i];
        typename map<int,T>::const_iterator it = row_i.begin();
        typename map<int,T>::const_iterator end = row_i.end();
        for (;it!=end;++it)
            s += it->second * v_[it->first];
        return s;
    }

    //!  return dot product of j-th column with vector v
    real dotColumn(int j, Vec v)
    {
#ifdef BOUNDCHECK
        if (v.length()!=length())
            PLERROR("RowMapSparseMatrix::dotColumn(%d,v), v.length_=%d != matrix length=%d",
                    j,v.length(),length());
        if (null_elem!=0)
            PLERROR("RowMapSparseMatrix::dotColumn works only when null_elem=0, but was %g\n",null_elem);
#endif
        PLWARNING("RowMapSparseMatrix is not appropriate to perform dotColumn operations");
        real s=0;
        real* v_=v.data();
        for (int i=0;i<v.length();i++)
            s += (*this)(i,j) * v_[i];
        return s;
    }

    //!  M = A' * A
    void transposeProduct(RowMapSparseMatrix& m, bool verbose = false)
    {
#ifdef BOUNDCHECK
        if (null_elem!=0)
            PLERROR("RowMapSparseMatrix::dotColumn works only when null_elem=0, but was %g\n",null_elem);
#endif
        RowMapSparseMatrix<T>& self = (*this);
        int n = self.length();
        RowMapSparseMatrix<T> mt(n, n);
        transpose(m, mt);
        int nnz = 0;
        for (int i = 0; i < n; i++)
        {
            if (verbose)
            {
                if (i % 10 == 0 && i != 0) { 
                    cout << "[" << i << "]" << " ";
                    if (i % 100 == 0) cout << endl;
                    else cout.flush();
                }
            }
            for (int j = 0; j < n; j++)
            {
                T val =  multiplyVecs(mt(i), mt(j));
                if (val != 0)
                {
                    nnz++;
                    self(i, j) = val;
                }
            }
        }
        if (verbose) 
            cout << endl;
    }

    //!  add vector to each row; by default do it only on
    //!  the non-zero elements
    void addToRows(Vec row, bool only_on_non_zeros=true)
    {
#ifdef BOUNDCHECK
        if (null_elem!=0)
            PLERROR("RowMapSparseMatrix::add2Rows works only when null_elem=0, but was %g\n",null_elem);
#endif
        if (!only_on_non_zeros)
            PLERROR("RowMapSparseMatrix::add2Rows(Vec,bool) works only with bool=true");
        real* r=row.data();
        for (int i=0;i<length();i++)
        {
            map<int,T>& row_i = rows[i];
            typename map<int,T>::iterator it = row_i.begin();
            typename map<int,T>::const_iterator end = row_i.end();
            for (;it!=end;++it)
                it->second += r[it->first];
        }
    }
    //!  add vector to each column; by default do it only on
    //!  the non-zero elements
    void addToColumns(Vec col, bool only_on_non_zeros=true)
    {
#ifdef BOUNDCHECK
        if (null_elem!=0)
            PLERROR("RowMapSparseMatrix::add2Columns works only when null_elem=0, but was %g\n",null_elem);
#endif
        if (!only_on_non_zeros)
            PLERROR("RowMapSparseMatrix::add2Columns(Vec,bool) works only with bool=true");
        for (int i=0;i<length();i++)
        {
            real col_i=col[i];
            map<int,T>& row_i = rows[i];
            typename map<int,T>::iterator it = row_i.begin();
            typename map<int,T>::const_iterator end = row_i.end();
            for (;it!=end;++it)
                it->second += col_i;
        }
    }
    //!  add a scalar everywhere; by default do it only on
    //!  the non-zero elements
    void add(real scalar, bool only_on_non_zeros=true)
    {
        if (!only_on_non_zeros)
            PLERROR("RowMapSparseMatrix::add(real,bool) works only with bool=true");
        for (int i=0;i<length();i++)
        {
            map<int,T>& row_i = rows[i];
            typename map<int,T>::iterator it = row_i.begin();
            typename map<int,T>::const_iterator end = row_i.end();
            for (;it!=end;++it)
                it->second += scalar;
        }
    }


/*!       average across rows on one hand, and in parallel average across columns
  (thus getting two averages). The boolean argument specifies whether
  the average is across the explicit elements (the non-zeros) or
  across everything (currently unsupported).
*/
    void averageAcrossRowsAndColumns(Vec avg_across_rows, Vec avg_across_columns, 
                                     bool only_on_non_zeros=true)
    {
        if (!only_on_non_zeros)
            PLERROR("RowMapSparseMatrix::averageAcrossRowsAndColumns works only with only_on_non_zeros=true");
        avg_across_rows.resize(width());
        avg_across_columns.resize(length());
        avg_across_rows.clear();
        avg_across_columns.clear();
        TVec<int> column_counts(width());
        for (int i=0;i<length();i++)
        {
            real& avg_cols_i=avg_across_columns[i];
            real* avg_rows = avg_across_rows.data();
            map<int,T>& row_i = rows[i];
            typename map<int,T>::const_iterator it = row_i.begin();
            typename map<int,T>::const_iterator end = row_i.end();
            int n=0;
            for (;it!=end;++it)
            {
                avg_cols_i += it->second;
                int j=it->first;
                avg_rows[j] += it->second;
                n++;
                column_counts[j]++;
            }
            if (n>0)
                avg_cols_i /= n;
        }
        for (int j=0;j<width();j++)
            if (column_counts[j]>0)
                avg_across_rows[j] /= column_counts[j];
    }

/*!       average across rows on one hand, and in parallel average across columns
  (thus getting two averages). The boolean argument specifies whether
  the average is across the explicit elements (the non-zeros) or
  across everything (currently unsupported).
*/
    real sumRow(int i)
    {
        real s=0;
        map<int,T>& row_i = rows[i];
        typename map<int,T>::const_iterator it = row_i.begin();
        typename map<int,T>::const_iterator end = row_i.end();
        for (;it!=end;++it)
            s += it->second;
        return s;
    }


    //!  multiply each (non-zero) element
    void operator*=(real scalar)
    {
#ifdef BOUNDCHECK
        if (null_elem!=0)
            PLERROR("RowMapSparseMatrix::operator* works only when null_elem=0, but was %g\n",null_elem);
#endif
        for (int i=0;i<length();i++)
        {
            map<int,T>& row_i = rows[i];
            typename map<int,T>::iterator it = row_i.begin();
            typename map<int,T>::const_iterator end = row_i.end();
            for (;it!=end;++it)
                it->second *= scalar;
        }
    }
      
    //!  D = S'
    static void transpose(RowMapSparseMatrix<T>& src, RowMapSparseMatrix<T>& dest)
    {
        dest.clear();
        for (int i = 0; i < src.length(); i++)
        {
            map<int, T>& row = src(i);
            typename map<int, T>::iterator beg = row.begin();
            typename map<int, T>::iterator end = row.end();
            while (beg != end)
            {
                int col = beg->first;
                T val = beg->second;
                dest(col, i) = val;
                ++beg;
            }
        }
    }
      
    //!  Sparse vectors mutliplication
    static real multiplyVecs(map<int, T>& map1, map<int, T>& map2) 
    {
        if (map1.size() == 0 || map2.size() == 0)
            return 0;
        typename map<int, T>::iterator beg1 = map1.begin();
        typename map<int, T>::iterator beg2 = map2.begin();
        typename map<int, T>::iterator end1 = map1.end();
        typename map<int, T>::iterator end2 = map2.end();
        int col1, col2;
        T val1, val2, sum = 0;
        bool fend1 = (beg1 == end1), fend2 = (beg2 == end2);
        int OUT = getMaxColumnIndex(map1, map2) + 1;
      
        while (!fend1 || !fend2) {
            if (!fend1)
                col1 = beg1->first;
            else
                col1 = OUT;
            if (!fend2)
                col2 = beg2->first;
            else
                col2 = OUT;
            val1 = beg1->second;
            val2 = beg2->second;
            if (col1 == col2) {
                sum += (val1 * val2);
                ++beg1;
                if (beg1 == end1) fend1 = true;
                ++beg2;
                if (beg2 == end2) fend2 = true;
            } else if (col1 < col2) {
                //sum += (val1 * val2);
                ++beg1;
                if (beg1 == end1) fend1 = true;
            } else if (col1 > col2) {
                //sum += (val1 * val2);
                ++beg2;
                if (beg2 == end2) fend2 = true;
            }
        }
        return sum;
    }

    static real euclidianDistance(map<int, real>& map1, map<int, real>& map2) 
    {
        if (map1.size() == 0 || map2.size() == 0)
            return 0;
        map<int, real>::iterator beg1 = map1.begin();
        map<int, real>::iterator beg2 = map2.begin();
        map<int, real>::iterator end1 = map1.end();
        map<int, real>::iterator end2 = map2.end();
        int col1, col2;
        real val1, val2, diff, sum = 0;
        bool fend1 = (beg1 == end1), fend2 = (beg2 == end2);
        int OUT = getMaxColumnIndex(map1, map2) + 1;
	
        while (!fend1 || !fend2) 
        {
            if (!fend1)
                col1 = beg1->first;
            else
                col1 = OUT;
            if (!fend2)
                col2 = beg2->first;
            else
                col2 = OUT;
            val1 = beg1->second;
            val2 = beg2->second;
            if (col1 == col2) 
            {
                diff = val1 - val2;
                sum += (diff * diff);
                beg1++;
                if (beg1 == end1) fend1 = true;
                beg2++;
                if (beg2 == end2) fend2 = true;
            } else if (col1 < col2) 
            {
                diff = val1;
                sum += (diff * diff);
                beg1++;
                if (beg1 == end1) fend1 = true;
            } else if (col1 > col2) 
            {
                diff = val2;
                sum += (diff * diff);
                beg2++;
                if (beg2 == end2) fend2 = true;
            }
        }
        //return sqrt(sum);
        //!  This is not a "true" euclidian distance
        return sum;
    }

//!  Vec dest[i] = map1[i] - map2[i]
    static void substractVecs(map<int, real>& map1, map<int, real>& map2, Vec& dest) 
    {
        map<int, real>::iterator beg1 = map1.begin();
        map<int, real>::iterator beg2 = map2.begin();
        map<int, real>::iterator end1 = map1.end();
        map<int, real>::iterator end2 = map2.end();
        int col1, col2;
        real val1, val2;
        bool fend1 = (beg1 == end1), fend2 = (beg2 == end2);
        int OUT = getMaxColumnIndex(map1, map2) + 1;
      
        while (!fend1 || !fend2) 
        {
            if (!fend1)
                col1 = beg1->first;
            else
                col1 = OUT;
            if (!fend2)
                col2 = beg2->first;
            else
                col2 = OUT;
            val1 = beg1->second;
            val2 = beg2->second;
            if (col1 == col2) 
            {
                dest[col1] = val1 - val2;
                beg1++;
                if (beg1 == end1) fend1 = true;
                beg2++;
                if (beg2 == end2) fend2 = true;
            } else if (col1 < col2) 
            {
                dest[col1] = val1;
                beg1++;
                if (beg1 == end1) fend1 = true;
            } else if (col1 > col2) 
            {
                dest[col2] = 0 - val2;
                beg2++;
                if (beg2 == end2) fend2 = true;
            }
        }
    } 
    
    //!  Return the last non-zero position of a sparse vector (used for
    //!  sparse operations that need to know the "end" of a vector).
    static int getMaxColumnIndex(map<int, T>& map1, map<int, T>& map2) 
    {
        map<int, real>::iterator end1 = map1.end();
        map<int, real>::iterator end2 = map2.end();
        --end1;
        --end2;
        return MAX(end1->first, end2->first);
    }

/*!       Export as matlab readable [i, j , v] format to file <out>
  Matlab : (1) load <out> (2) A = spconvert(out)
  (note: the file extension must be '.dat')
*/
    void exportToMatlabReadableFormat(string filename)
    {
        ofstream out(filename.c_str());
#ifdef USEDOUBLE
        out.precision(20);
#else
        out.precision(6);
#endif
        for (unsigned int i = 0; i < rows.size(); i++)
        {
            typename map<int, T>::iterator beg = rows[i].begin();
            typename map<int, T>::iterator end = rows[i].end();
            while (beg != end)
            {
                out << i + 1 << " " << beg->first + 1 << " " << beg->second << endl;
                beg++;
            }
        }
        //!  Add the bottom right corner, to make sure that the sparse matrix has
        //!  the right dimensions
        int l = length();
        int w = width();
        l--; w--;
        map<int, T>& row_l=rows[l];
        if (row_l.find(w) == row_l.end())
            out << l + 1 << " " << w + 1 << " " << 0 << endl;
    }

    //!  Compute the matrix density, that is :
    //!    number_of_nonzero_elements / (length*width)
    real density()
    {
        return size() / real(length() * width());
    }

};

template <class T>
void product(RowMapSparseMatrix<T>& M, const Vec& x, Vec& y) { M.product(x,y); }

// result[j] = sum_i mat[i,j]
template <class T>
void columnSum(RowMapSparseMatrix<T> mat, TVec<T>& result)
{
    int n = mat.length();
    result.resize(n);
    result.clear();
    for (int i=0;i<n;i++)
    {
        map<int,T>& row_i = mat.rows[i];
        typename map<int,T>::iterator Rit = row_i.begin();
        typename map<int,T>::const_iterator Rend = row_i.end();
        for (;Rit!=Rend;++Rit)
            result[Rit->first] += Rit->second;
    }
}

// res[i,j] = scale*(mat[i,j] - avg[i] - avg[j] + mean(avg))
template <class T>
void doubleCentering(RowMapSparseMatrix<T>& mat, TVec<T>& avg, RowMapSparseMatrix<T>& res, T scale=1)
{
    T moy = mean(avg);
    int n=avg.length();
    T* a = avg.data();
    if (scale==T(1))
    {
        if (&mat != &res)
            for (int i=0;i<n;i++)
            {
                map<int,T>& Mi = mat.rows[i];
                map<int,T>& Ri = res.rows[i];
                T term = moy-a[i];
                typename map<int,T>::iterator Mit = Mi.begin();
                typename map<int,T>::const_iterator Mend = Mi.end();
                typename map<int,T>::iterator Rit = Ri.begin();
                typename map<int,T>::const_iterator Rend = Ri.end();
                for (;Mit!=Mend && Rit!=Rend;)
                {
                    if (Mit->first==Rit->first)
                        Rit->second = Mit->second - a[Rit->first] + term;
                    else if (Mit->first<Rit->first)
                        ++Mit;
                    else
                        ++Rit;
                }
            }
        else
            for (int i=0;i<n;i++)
            {
                map<int,T>& Ri = res.rows[i];
                typename map<int,T>::iterator Rit = Ri.begin();
                typename map<int,T>::const_iterator Rend = Ri.end();
                T term = moy-a[i];
                for (;Rit!=Rend;++Rit)
                    Rit->second += term - a[Rit->first];

            }
    }
    else
    {
        if (&mat != &res)
            for (int i=0;i<n;i++)
            {
                map<int,T>& Mi = mat.rows[i];
                map<int,T>& Ri = res.rows[i];
                T term = moy-a[i];
                typename map<int,T>::iterator Mit = Mi.begin();
                typename map<int,T>::const_iterator Mend = Mi.end();
                typename map<int,T>::iterator Rit = Ri.begin();
                typename map<int,T>::const_iterator Rend = Ri.end();
                for (;Mit!=Mend && Rit!=Rend;)
                {
                    if (Mit->first==Rit->first)
                        Rit->second = scale*(Mit->second - a[Rit->first] + term);
                    else if (Mit->first<Rit->first)
                        ++Mit;
                    else
                        ++Rit;
                }
            }
        else
            for (int i=0;i<n;i++)
            {
                map<int,T>& Ri = res.rows[i];
                typename map<int,T>::iterator Rit = Ri.begin();
                typename map<int,T>::const_iterator Rend = Ri.end();
                T term = moy-a[i];
                for (;Rit!=Rend;++Rit)
                    Rit->second = scale*(Rit->second + term - a[Rit->first]);

            }
    }
}



template <class T>
void averageAcrossRowsAndColumns(RowMapSparseMatrix<T> mat, 
                                 Vec avg_across_rows, Vec avg_across_columns, 
                                 bool only_on_non_zeros=true)
{
    mat.averageAcrossRowsAndColumns(avg_across_rows, avg_across_columns, only_on_non_zeros);
}

template <class T>
void addToRows(RowMapSparseMatrix<T> mat, Vec row, bool only_on_non_zeros=true)
{
    mat.addToRows(row,only_on_non_zeros);
}

template <class T>
void addToColumns(RowMapSparseMatrix<T> mat, Vec row, bool only_on_non_zeros=true)
{
    mat.addToColumns(row,only_on_non_zeros);
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
