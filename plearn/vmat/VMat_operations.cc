// -*- C++ -*-

// VMat_operations.cc
//
// Copyright (C) 2004 Pascal Vincent
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
 ******************************************************* */

// Authors: Pascal Vincent

/*! \file VMat_operations.cc */


#include "VMat.h"
#include "VMat_operations.h"
#include <plearn/math/random.h>
#include <plearn/io/TmpFilenames.h>
#include <plearn/io/IntVecFile.h>

namespace PLearn {
using namespace std;


VMat grep(VMat d, int col, Vec values, bool exclude)
{
    Vec indices(d.length());
    int nrows = 0;
    for(int i=0; i<d.length(); i++)
    {
        bool contains = values.contains(d(i,col));
        if( (!exclude && contains) || (exclude && !contains) )
            indices[nrows++] = i;
    }
    indices = indices.subVec(0,nrows);
    return d.rows(indices.copy());
}

//! returns a map mapping all different values appearing in column col to their number of occurences
map<real, int> countOccurencesInColumn(VMat m, int col)
{
    map<real, int> counts; // result we will return
    map<real, int>::iterator found;
    int l = m.length();
    for(int i=0; i<l; i++)
    {
        real val = m(i,col);
        if (is_missing(val))
            // The 'nan' real value has to be dealt with separately. Here, we just
            // raise an error to keep it simple.
            PLERROR("In countOccurencesInColumn - Found a missing value, this case is currently not handled");
        found = counts.find(val);
        if(found==counts.end())
            counts[val] = 1;
        else
            found->second++;
    }
    return counts;
}

//! returns a map mapping all different values appearing in column col to a vector of the corresponding row indices in the VMat
map<real, TVec<int> > indicesOfOccurencesInColumn(VMat m, int col)
{
    map< real, TVec<int> > indices; // result we will return
    map<real, int> counts = countOccurencesInColumn(m,col);
    map<real, int>::iterator it = counts.begin();
    map<real, int>::iterator itend = counts.end();
    for(; it!=itend; ++it)
    {
        indices[it->first].resize(it->second); // allocate the exact amount of memory
        indices[it->first].resize(0); // reset the size to 0 so we can do appends...
    }
    int l = m.length();
    for(int i=0; i<l; i++)
        indices[m(i,col)].push_back(i);
    return indices;
}

VMat grep(VMat d, int col, Vec values, const string& indexfile, bool exclude)
{
    if(!isfile(indexfile))
    {
        IntVecFile indices(indexfile,true);
        for(int i=0; i<d.length(); i++)
        {
            bool contains = values.contains(d(i,col));
            if( (!exclude && contains) || (exclude && !contains) )
                indices.append(i);
        }
    }
    return d.rows(indexfile);
}

VMat filter(VMat d, const string& indexfile)
{
    if(!isfile(indexfile) || filesize(indexfile)==0)
    {
        IntVecFile indices(indexfile,true);
        Vec v(d.width());
        for(int i=0; i<d.length(); i++)
        {
            d->getRow(i,v);
            if(!v.hasMissing())
                indices.append(i);
        }
    }
    return d.rows(indexfile);
}


VMat shuffle(VMat d)
{
    Vec indices(0, d.length()-1, 1); // Range-vector
    shuffleElements(indices);
    return d.rows(indices);
}

VMat bootstrap(VMat d, bool reorder, bool norepeat)
{
    Vec indices;
    if (norepeat)
    {
        indices = Vec(0, d.length()-1, 1); // Range-vector
        shuffleElements(indices);
        indices = indices.subVec(0,int(0.667 * d.length()));
        if (reorder)
            sortElements(indices);
        return d.rows(indices);
    }
    else
    {
        indices.resize(d.length());
        for (int i=0;i<d.length();i++)
            indices[i] = uniform_multinomial_sample(d.length());
    }
    if (reorder)
        sortElements(indices);
    return d.rows(indices);
}


VMat rebalanceNClasses(VMat inputs, int nclasses, const string& filename)
{
    if (!isfile(filename))
    {
        IntVecFile indices(filename, true);
        Vec last = inputs.lastColumn()->toMat().toVecCopy();
        const int len = last.length();
        Vec capacity(nclasses);
        Array<Vec> index(nclasses);
        Array<Vec> index_used(nclasses);
        for (int i=0; i<nclasses; i++) index[i].resize(len);
        for (int i=0; i<nclasses; i++) index_used[i].resize(len);
        real** p_index;
        p_index = new real*[nclasses];
        for (int i=0; i<nclasses; i++) p_index[i] = index[i].data();
        for (int i=0; i<nclasses; i++) index_used[i].clear();
        for (int i=0; i<len; i++)
        {
            int class_i = int(last[i]);
            *p_index[class_i]++ = i;
            capacity[class_i]++;
        }
        for (int i=0; i<nclasses; i++) index[i].resize(int(capacity[i]));
        for (int i=0; i<nclasses; i++) index_used[i].resize(int(capacity[i]));

        Mat class_length(nclasses,2);
        for (int i=0; i<nclasses; i++)
        {
            class_length(i,0) = capacity[i];
            class_length(i,1) = i;
        }
        sortRows(class_length);
        Vec remap = class_length.column(1).toVecCopy();

        vector<int> n(nclasses,0);
        int new_index = -1;
        for (int i=0; i<len; i++)
        {
            int c = i%nclasses;
            int c_map = int(remap[c]);
            if (c == 0)
            {
                if (fast_exact_is_equal(n[0], capacity[c_map])) n[0] = 0;
                new_index = int(index[c_map][n[0]++]);
            }
            else
            {
                if (fast_exact_is_equal(n[c], capacity[c_map]))
                {
                    n[c] = 0;
                    index_used[c_map].clear();
                }
                bool index_found = false;
                int start_pos = binary_search(index[c_map], real(new_index));
                for (int j=start_pos+1; j<capacity[c_map]; j++)
                {
                    if (fast_exact_is_equal(index_used[c_map][j], 0))
                    {
                        index_used[c_map][j] = 1;
                        new_index = int(index[c_map][j]);
                        index_found = true;
                        n[c]++;
                        break;
                    }
                }
                if (!index_found)
                {
                    for (int j=0; j<start_pos; j++)
                    {
                        if (fast_exact_is_equal(index_used[c_map][j], 0))
                        {
                            index_used[c_map][j] = 1;
                            new_index = int(index[c_map][j]);
                            index_found = true;
                            n[c]++;
                            break;
                        }
                    }
                }
                if (!index_found)
                    PLERROR("In rebalanceNClasses:  something got wrong!");
            }
            indices.put(i, new_index);
        }

        delete[] p_index;
    }
    return inputs.rows(filename);
}

void fullyRebalance2Classes(VMat inputs, const string& filename, bool save_indices)
{
    if (!isfile(filename))
    {
        int len = inputs.length();

        int n_zeros = 0;
        int n_ones = 0;
        Vec zeros(len);
        Vec ones(len);

        Vec last = inputs.lastColumn()->toMat().toVecCopy();
        for (int i=0; i<len;i++)
        {
            if (fast_exact_is_equal(last[i], 0))
                zeros[n_zeros++] = i;
            else
                ones[n_ones++] = i;
        }
        zeros.resize(n_zeros);
        ones.resize(n_ones);

        TmpFilenames tmpfile(1);
        string fname = save_indices ? filename : tmpfile.addFilename();
        IntVecFile indices(fname, true);
        int max_symbols = MAX(n_zeros, n_ones);
        for (int i=0; i<max_symbols; i++)
        {
            indices.put(2*i, int(zeros[i%n_zeros]));
            indices.put(2*i+1, int(ones[i%n_ones]));
        }
        if (!save_indices)
        {
            VMat vm = inputs.rows(fname);
            vm.save(filename);
        }
    }
}

VMat temporalThreshold(VMat distr, int threshold_date, bool is_before,
                       int yyyymmdd_col)
{
    Vec indices(distr->length());
    int n_data = 0;
    for (int i=0; i<distr->length(); i++)
    {
        int reference_date = (int)distr(i, yyyymmdd_col);
        if (is_before ? reference_date<=threshold_date : reference_date>=threshold_date)
            indices[n_data++] = i;
    }
    indices.resize(n_data);

    return distr.rows(indices);
}

VMat temporalThreshold(VMat distr, int threshold_date, bool is_before,
                       int yyyy_col, int mm_col, int dd_col)
{
    Vec indices(distr->length());
    int n_data = 0;
    for (int i=0; i<distr->length(); i++)
    {
        int reference_date = 10000*(int)distr(i, yyyy_col) + 100*(int)distr(i, mm_col) + (int)distr(i, dd_col);
        if (is_before ? reference_date<=threshold_date : reference_date>=threshold_date)
            indices[n_data++] = i;
    }
    indices.resize(n_data);

    return distr.rows(indices);
}

} // end of namespace PLearn


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
