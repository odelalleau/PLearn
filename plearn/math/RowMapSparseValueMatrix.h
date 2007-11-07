// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2003 Marie Ouimet
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

/*! \file PLearn/plearn/math/RowMapSparseValueMatrix.h */

#ifndef ROWMAPSPARSEVALUEMATRIX
#define ROWMAPSPARSEVALUEMATRIX

#include "RowMapSparseMatrix.h"

namespace PLearn {
using namespace std;

/*!   

** Warning **: this class inherits methods from RowMapSparseMatrix<T> 
that are not necessarily correct when "value" is different from 0.
The following methods are correct (plus those redefined below):
- isSymmetric
- resize
- clear

The value of elements that is not specified is given by the field "value".

Sparse matrices implemented with STL maps.
  
We assume that there are elements in each ROW.
  
We associate an STL map to each row: column index --> value
  
Space used is about O( size_of_elements * number_of_non_value_elements )
  
Random access time is O(log(number_of_elements_per_row))
  
Row-wise iterations can be done in constant time per access.
  
Binary or ascii load/save streaming are available.
Recommended filename extensions are .armsm and .brmsm 
respectively for Ascii Row Map Sparse Matrix or Binary Row Map Sparse Matrix.
  
*/

template <class T>
class RowMapSparseValueMatrix: public RowMapSparseMatrix<T> {
public:

    T value;

    RowMapSparseValueMatrix(T value_=0, int n_rows=0, int n_columns=0) 
        : RowMapSparseMatrix<T>(n_rows, n_columns), value(value_)
    {}

    RowMapSparseValueMatrix(T value_, string filename)
        : RowMapSparseMatrix<T>(filename), value(value_)
    {}

    RowMapSparseValueMatrix(T value_, const Mat& m, int fill_mode=0) 
        :  RowMapSparseMatrix<T>(m.length(), m.width()), value(value_)
    {
        switch(fill_mode){
        case 0:
            //fill all
            for (int i=0;i<length();i++)
            {
                real* r=m[i];
                map<int,T>& row_i=rows[i];
                for (int j=0;j<width();j++)
                    row_i[j]=T(r[j]);
            }
            break;
        case 1:
            //fill only if entry != value 
            for (int i=0;i<length();i++)
            {
                real* r=m[i];
                map<int,T>& row_i=rows[i];
                for (int j=0;j<width();j++){
                    if(T(r[j])!=value_)
                        row_i[j]=T(r[j]);
                }
            }
            break;
        case 2:
            //fill only if entry < value 
            for (int i=0;i<length();i++)
            {
                real* r=m[i];
                map<int,T>& row_i=rows[i];
                for (int j=0;j<width();j++){
                    if(T(r[j])<value_)
                        row_i[j]=T(r[j]);
                }
            }
            break;
        default:
            PLERROR("RowMapSparseValueMatrix: fill_mode must be 0, 1 or 2.");
        }
    }

    //!  Accepts a FORTRAN formatted sparse matrix as an initializer.
    RowMapSparseValueMatrix(T value_, const SparseMatrix& sm, int n_rows, int n_cols) 
        : RowMapSparseMatrix<T>(sm, n_rows, n_cols), value(value_)
    {}

    Mat toMat() 
    {
        Mat res(length(),width(),value);
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


    T& operator()(int i, int j) { 
#ifdef BOUNDCHECK      
        if (i<0 || i>=length() && j<0 || j>=width())
            PLERROR("RowMapSparseValueMatrix: out-of-bound access to (%d,%d), dims=(%d,%d)",
                    i,j,length(),width());
#endif
        return rows[i][j];
    }

    const T& operator()(int i, int j) const { 
#ifdef BOUNDCHECK      
        if (i<0 || i>=length() && j<0 || j>=width())
            PLERROR("RowMapSparseValueMatrix: out-of-bound access to (%d,%d), dims=(%d,%d)",
                    i,j,length(),width());
#endif
        const map<int,T>& row_i = rows[i];
        typename map<int,T>::const_iterator it = row_i.find(j);
        if (it==row_i.end())
            return value;
        return it->second;
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



/*!       average across rows on one hand, and in parallel average across columns
  (thus getting two averages). The boolean argument specifies whether
  the average is across the explicit elements (the ones not equal to "value") or
  across everything.
*/
    void averageAcrossRowsAndColumns(Vec avg_across_rows, Vec avg_across_columns,
                                     bool only_on_non_value=false){
        avg_across_rows.resize(width());
        avg_across_columns.resize(length());
        avg_across_rows.clear();
        avg_across_columns.clear();
        TVec<int> column_counts(width());

        if (only_on_non_value){
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
                avg_cols_i /= n;
            }
            for (int j=0;j<width();j++)
                avg_across_rows[j] /= column_counts[j];
        }
        else {
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
                avg_cols_i += value*(width()-n);
                avg_cols_i /= width(); //store average of ith row
            }
            //compute average of each column
            for (int j=0;j<width();j++){
                avg_across_rows[j] += value*(length() - column_counts[j]);
                avg_across_rows[j] /= length();
            }
        }
    }

    real euclidianDistance(map<int, real>& map1, map<int, real>& map2) {
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
                diff = val1 - value;
                sum += (diff * diff);
                beg1++;
                if (beg1 == end1) fend1 = true;
            } else if (col1 > col2) 
            {
                diff = value - val2;
                sum += (diff * diff);
                beg2++;
                if (beg2 == end2) fend2 = true;
            }
        }
        //return sqrt(sum);
        //!  This is not a "true" euclidian distance
        return sum;
    }

};


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
