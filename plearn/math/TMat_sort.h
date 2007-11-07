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
 * AUTHORS: Pascal Vincent & Yoshua Bengio & Rejean Ducharme
 * This file is part of the PLearn library.
 ******************************************************* */

/*! \file PLearn/plearn/math/TMat_sort.h */

#ifndef TMat_sort_INC
#define TMat_sort_INC

#include <algorithm>
#include "TVec_impl.h"
#include "TMat_impl.h"

namespace PLearn {
using namespace std;


template<class T> class SelectedIndicesCmp
{
private:
    TVec<int> indices;
    bool use_less_than; 

public:
    typedef T first_argument_type;
    typedef T second_argument_type;
    typedef bool result_type;

    SelectedIndicesCmp(TVec<int> the_indices, bool use_lexical_less_than=true)
        :indices(the_indices), use_less_than(use_lexical_less_than)
    {}

    bool operator()(const T& x, const T& y)
    {
        int n = indices.length();
        if(use_less_than) // lexical <
        {
            for(int k=0; k<n; k++)
            {
                int i = indices[k];
                if(x[i]<y[i])
                    return true;
                else if(x[i]>y[i])
                    return false;
            }
            return false;
        }
        else // lexical >
        {
            for(int k=0; k<n; k++)
            {
                int i = indices[k];
                if(x[i]>y[i])
                    return true;
                else if(x[i]<y[i])
                    return false;
            }
            return false;
        }
    }
};

template<class T>
void sortRows(TMat<T>& mat, const TVec<int>& key_columns, bool increasing_order=true)
{
    SelectedIndicesCmp< Array<T> > cmp(key_columns,increasing_order);
    sort(mat.rows_as_arrays_begin(), mat.rows_as_arrays_end(), cmp);
}


//! Sorts the elements of vec in place
template<class T>
inline void sortElements(const TVec<T>& vec) 
{ sort(vec.begin(),vec.end()); }


//! Uses partial_sort.
//! Sorts only the first k smallests rows and put it in the k first rows.
//! The other rows are in an arbitrary order.
//! If sortk is 0, the k smallest rows are put in the k first rows but in an 
//! arbitrary order. 
//! This implementation should be very efficient, but it does two memory 
//! allocation: a first one of mat.length()*(sizeof(real)+sizeof(int))
//! and a second one of mat.length()*sizeof(int).
template<class T>
void partialSortRows(const TMat<T>& mat, int k, int sortk=1, int col=0)
{
    if (k > mat.length())
        PLERROR("In partialSortRows - The number of rows to sort (%d) must "
                "be less than the length of the matrix (%d)",
                k, mat.length());
    vector< pair<T,int> > order(mat.length());
    typename vector< pair<T,int> >::iterator it = order.begin();
    T* ptr = mat.data()+col;
    for(int i=0; i<mat.length(); ++i, ptr+=mat.mod(), ++it)
    {
        it->first = *ptr;
        it->second = i;
    }

    typename vector< pair<T,int> >::iterator middle = order.begin();
    for(int i=0; i<k; ++i, ++middle);

    partial_sort(order.begin(),middle,order.end());
    
    // Build the new destination position array
    // (destpos is the inverse map of order.second)
    vector<int> destpos(mat.length());  
    for(int i=0; i<mat.length(); ++i)
        destpos[order[i].second] = i;

    // Put elements wich are in the rows k to mat.length()-1 at their place if
    // their destpos is in the range 0 to k-1. If not, we leave them there.
    for(int startpos = mat.length()-1; startpos>=k; startpos--)
    {
        int dest = destpos[startpos];
        if(dest!=-1)
        {
            while(dest<k)
            {
                mat.swapRows(startpos,dest);
                int newdest = destpos[dest];
                destpos[dest] = -1;
                dest = newdest;
            }
            destpos[startpos] = -1;
        }
    }

    if(sortk) {
        // Put the k firsts rows in the right order
        for(int startpos = 0; startpos<k; startpos++)
        {
            int dest = destpos[startpos];      
            if(dest!=-1)
            {
                while(dest!=startpos)
                {
                    mat.swapRows(startpos,dest);
                    int newdest = destpos[dest];
                    destpos[dest] = -1;
                    dest = newdest;
                }
                destpos[startpos] = -1;
            }
        }
    }
}

//! This implementation should be very efficient,
//! but it does two memory allocation: a first one of mat.length()*(sizeof(real)+sizeof(int))
//! and a second one of mat.length()*sizeof(int).
//! (Note: due to the implementation of the column sorting, this function
//! always performs a STABLE SORT (in the sense of the STL stable_sort
//! function).  There is no need to explicitly call stable_sort to achieve
//! the effect, iff increasing_order=true)
template<class T>
void sortRows(const TMat<T>& mat, int col=0, bool increasing_order=true)
{
    vector< pair<T,int> > order(mat.length());
    typename vector< pair<T,int> >::iterator it = order.begin();
    T* ptr = mat.data()+col;
    for(int i=0; i<mat.length(); ++i, ptr+=mat.mod(), ++it)
    {
        it->first = *ptr;
        it->second = i;
    }

    if(increasing_order)
        sort(order.begin(),order.end());
    else 
        sort(order.begin(), order.end(), greater< pair<T,int> >() );

    // Build the new destination position array
    // (destpos is the inverse map of order.second)
    vector<int> destpos(mat.length());  
    for(int i=0; i<mat.length(); ++i)
        destpos[order[i].second] = i;

    // Now put the full rows in order...
    for(int startpos = 0; startpos<mat.length(); startpos++)
    {
        int dest = destpos[startpos];      
        if(dest!=-1)
        {
            while(dest!=startpos)
            {
                mat.swapRows(startpos,dest);
                int newdest = destpos[dest];
                destpos[dest] = -1;
                dest = newdest;
            }
            destpos[startpos] = -1;
        }
    }
}



/*   Not taken from Numerical Recipies: This is a quickly written and hihghly unefficient algorithm!   */
/*   Sorts the columns of the given matrix, in ascending order of its rownum row's elements   */
template<class T>
void sortColumns(const TMat<T>& mat, int rownum)
{
    int j,jj,i;
    T min;
    T tmp;
    int mincol;
  
    if(rownum>=mat.length())
        PLERROR("In sortColumns: no rownumumn %d in matrix (%dx%d)",rownum,mat.width(),mat.length());

    for(j=0; j<mat.width(); j++)
    {
        // look, starting from col j, for the col with the minimum rownum element
        mincol = j;
        min = mat(rownum,j);
        for(jj=j; jj<mat.width(); jj++)
        {
            if(mat(rownum,jj)<min)
            {
                min = mat(rownum,jj);
                mincol = jj;
            }
        }
        if(mincol>j) /*   Found a col with inferior value   */
        {
            /*   So let's exchange cols j and mincol   */
            for(i=0; i<mat.length(); i++)
            {
                tmp = mat(i,j);
                mat(i,j) = mat(i,mincol);
                mat(i,mincol) = tmp;
            }
        }
    }
}



// returns the position k of x in src such that src[k] <= x < src[k+1]
// (returns -1 if x < src[0]  or  N-1 if x >= src[N-1])
// BE CAREFULL: src must be sort BEFORE the fonction binary_search is called
template<class T>
int binary_search(const TVec<T>& src, T x)
{
    const int len = src.length();

    if (x < src[0]) return -1;
    else if (x >= src[len-1]) return len-1;

    int start = 0;
    int end = len-1;
    for (;;) {
        int k = (start+end)/2;
        if (x >= src[k]  &&  x < src[k+1]) {
            if (src[k] == src[k+1]) {
                start = k;
                end = k+1;
                while (start>0  &&  src[start-1]==src[start]) start--;
                while (end<len-1  &&  src[end]==src[end+1]) end++;
                k = (start+end)/2;
            }
            return k;
        }
        else if (x > src[k])
            start = k+1;
        else
            end = k;
    }
}

// Binary search according to the given column c of a matrix.  The column
// MUST BE SORTED.  Return the row k such that src(k,c) <= x < src(k+1).
// Returns -1 if x < src(0,c) or N-1 if x >= src(N-1,c).
template <class T>
int binary_search(const TMat<T>& src, int c, T x)
{
    const int len = src.length();

    if (x < src(0,c))
        return -1;
    else if (x >= src(len-1,c))
        return len-1;

    int start = 0, end = len-1;
    for (;;) {
        int k = (start+end)/2;
        if (x >= src(k,c) && x < src(k+1,c)) {
            if (src(k,c) == src(k+1,c)) {
                start = k;
                end = k+1;
                while (start > 0 && src(start-1,c) == src(start,c))
                    --start;
                while (end < len-1 && src(end,c) == src(end+1,c))
                    ++end;
                k = (start+end)/2;
            }
            return k;
        }
        else if (x > src(k,c))
            start = k+1;
        else
            end = k;
    }
}


} // end of namespace PLearn
 
#endif // TMat_sort_INC


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
