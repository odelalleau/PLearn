// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2001 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2002 Pascal Vincent, Julien Keable, Xavier Saint-Mleux
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

#include "CompactVMatrix.h"
#include <plearn/math/TMat_maths.h>
#include <plearn/math/random.h>

namespace PLearn {
using namespace std;

union short_and_twobytes
{
    unsigned short us;
    unsigned char twobytes[2];
};

/** CompactVMatrix **/

// norman: added static "inizialization"
unsigned char CompactVMatrix::n_bits_in_byte[256];

void CompactVMatrix::set_n_bits_in_byte()
{
    if (n_bits_in_byte[255]!=8)
    {
        for (int i=0;i<256;i++)
        {
            int n=0;
            unsigned char byte=i;
            for (int j=0;j<8;j++)
            {
                n += byte & 1;
                byte >>= 1;
            }
            n_bits_in_byte[i]=n;
        }
    }
}

PLEARN_IMPLEMENT_OBJECT(CompactVMatrix, "ONE LINE DESCR", "NO HELP");

CompactVMatrix::CompactVMatrix()
    : n_symbols(0), n_fixedpoint(0), n_variables(0), one_hot_encoding(true), n_symbol_values(0),
      fixedpoint_min(0), fixedpoint_max(0), delta(0), variables_permutation(0)
{
}

CompactVMatrix::CompactVMatrix(int the_length, int nvariables, int n_binary,
                               int n_nonbinary_discrete,
                               int n_fixed_point, TVec<int>& n_symbolvalues,
                               Vec& fixed_point_min, Vec& fixed_point_max,
                               bool onehot_encoding)
    : inherited(the_length,nvariables), n_bits(n_binary),
      n_symbols(n_nonbinary_discrete), n_fixedpoint(n_fixed_point),
      n_variables(nvariables), one_hot_encoding(onehot_encoding),
      n_symbol_values(n_symbolvalues),
      fixedpoint_min(fixed_point_min), fixedpoint_max(fixed_point_max),
      delta(n_fixed_point), variables_permutation(n_variables)
{
    normal_width=n_bits+n_fixed_point;
    for (int i=0;i<n_symbols;i++)
        normal_width += n_symbol_values[i];
    setOneHotMode(one_hot_encoding);
    for (int i=0;i<n_variables;i++) variables_permutation[i]=i;
    for (int i=0;i<n_symbols;i++)
        delta[i]=(fixedpoint_max[i]-fixedpoint_min[i])/USHRT_MAX;
    symbols_offset = (int)ceil(n_bits/8.0);
    fixedpoint_offset = symbols_offset + n_symbols;
    row_n_bytes =  fixedpoint_offset + int(sizeof(unsigned short))*n_fixed_point;
    data.resize(length_ * row_n_bytes);
    set_n_bits_in_byte();
}

CompactVMatrix::CompactVMatrix(VMat m, int keep_last_variables_last, bool onehot_encoding)
    : inherited(m->length(),m->width()), one_hot_encoding(onehot_encoding),
      n_symbol_values(m->width()), variables_permutation(m->width())
{
    if (!m->hasStats())
    {
        cout << "CompactVMatrix(VMat, int): VMat did not have stats. Computing them." << endl;
        m->computeStats();
    }
    // determine which variables are binary discrete, multi-class discrete, or continuous
    n_bits = n_symbols = n_fixedpoint = 0;
    TVec<int> bits_position(m->width());
    TVec<int> symbols_position(m->width());
    TVec<int> fp_position(m->width());
    fixedpoint_min.resize(m->width());
    fixedpoint_max.resize(m->width());
    delta.resize(m->width());
    for (int i=0;i<m->width();i++)
    {
        VMFieldStat& stat = m->fieldstats[i];
        int n_values = (int)stat.counts.size(); // 0 means "continuous"
        bool counts_look_continuous = !isMapKeysAreInt(stat.counts);
        if (n_values == 0 || counts_look_continuous || i>=m->width()-keep_last_variables_last)
        {
            fixedpoint_min[n_fixedpoint]=stat.min();
            fixedpoint_max[n_fixedpoint]=stat.max();
            delta[n_fixedpoint]=(stat.max()-stat.min())/USHRT_MAX;
            fp_position[n_fixedpoint++]=i;
        }
        else
        {
            if (!fast_exact_is_equal(stat.min(), 0) ||
                !fast_exact_is_equal((stat.max()-stat.min()+1),
                                     stat.counts.size()))
                PLERROR("CompactVMatrix:: variable %d looks discrete but has zero-frequency intermediate values or min!=0",i);
            if (n_values==2)
                bits_position[n_bits++]=i;
            else if (n_values<=256)
            {
                symbols_position[n_symbols]=i;
                n_symbol_values[n_symbols++] = n_values;
            }
            else
            {
                fixedpoint_min[n_fixedpoint]=stat.min();
                fixedpoint_max[n_fixedpoint]=stat.max();
                delta[n_fixedpoint]=(stat.max()-stat.min())/USHRT_MAX;
                fp_position[n_fixedpoint++]=i;
            }
        }
    }
    fixedpoint_min.resize(n_fixedpoint);
    fixedpoint_max.resize(n_fixedpoint);
    delta.resize(n_fixedpoint);
    n_symbol_values.resize(n_symbols);
    n_variables = n_bits + n_symbols + n_fixedpoint;
    int j=0;
    for (int i=0;i<n_bits;i++,j++)
        variables_permutation[j]=bits_position[i];
    for (int i=0;i<n_symbols;i++,j++)
        variables_permutation[j]=symbols_position[i];
    for (int i=0;i<n_fixedpoint;i++,j++)
        variables_permutation[j]=fp_position[i];

    normal_width=n_bits+n_fixedpoint;
    for (int i=0;i<n_symbols;i++)
        normal_width += n_symbol_values[i];
    setOneHotMode(one_hot_encoding);
    symbols_offset = (int)ceil(n_bits/8.0);
    fixedpoint_offset = symbols_offset + n_symbols;
    row_n_bytes =  fixedpoint_offset + int(sizeof(unsigned short))*n_fixedpoint;
    data.resize(length_ * row_n_bytes);

    // copy the field infos and stats? not really useful with one-hot encoding
    // because of non-binary symbols being spread across many columns.
    if (!one_hot_encoding)
    {
        fieldinfos.resize(width_);
        fieldstats.resize(width_);
        for (int i=0;i<width_;i++)
        {
            fieldinfos[i]=m->getFieldInfos()[variables_permutation[i]];
            fieldstats[i]=m->fieldstats[variables_permutation[i]];
        }
    }
    else
    {
        fieldinfos.resize(0);
        fieldstats.resize(0);
    }

    // copy the data
    Vec mrow(m->width());
    for (int t=0;t<length_;t++)
    {
        m->getRow(t,mrow);
        encodeAndPutRow(t,mrow);
    }
    set_n_bits_in_byte();
}


CompactVMatrix::CompactVMatrix(const string& filename, int nlast) : RowBufferedVMatrix(0,0)
{
    load(filename);
    n_last=nlast;
    set_n_bits_in_byte();
}

CompactVMatrix::CompactVMatrix(CompactVMatrix* cvm, VMat m, bool rescale, bool check)
    : inherited(m->length(),m->width())
{
    if (cvm->width() != m->width())
        PLERROR("CompactVMatrix::CompactVMatrix(CompactVMatrix* cvm, VMat m,...), args have widths %d!=%d",
                cvm->width(),m->width());
    // copy all the ordinary fields
    row_n_bytes = cvm->row_n_bytes;
    data.resize(length_*row_n_bytes);
    n_bits = cvm->n_bits;
    n_symbols = cvm->n_symbols;
    n_fixedpoint = cvm->n_fixedpoint;
    n_variables = cvm->n_variables;
    n_symbol_values = cvm->n_symbol_values;
    fixedpoint_min = cvm->fixedpoint_min.copy();
    fixedpoint_max = cvm->fixedpoint_max.copy();
    delta = cvm->delta.copy();
    variables_permutation = cvm->variables_permutation;
    n_last = cvm->n_last;
    normal_width = cvm->normal_width;
    symbols_offset = cvm->symbols_offset;
    fixedpoint_offset = cvm->fixedpoint_offset;

    setOneHotMode(cvm->one_hot_encoding);
    Vec row(width_);
    int offs=width_-n_fixedpoint;
    if (rescale)
    {
        for (int i=0;i<length_;i++)
        {
            m->getRow(i,row);
            for (int j=0;j<n_fixedpoint;j++)
            {
                real element=row[offs+j];
                if (element<fixedpoint_min[j])
                    fixedpoint_min[j]=element;
                if (element>fixedpoint_max[j])
                    fixedpoint_max[j]=element;
            }
        }
        for (int j=0;j<n_fixedpoint;j++)
            delta[j]=(fixedpoint_max[j]-fixedpoint_min[j])/USHRT_MAX;
    }
    for (int i=0;i<length_;i++)
    {
        m->getRow(i,row);
        if (!rescale && check) // check that range is OK
        {
            for (int j=0;j<n_fixedpoint;j++)
            {
                real element=row[offs+j];
                if (element<fixedpoint_min[j] ||
                    element>fixedpoint_max[j])
                    PLERROR("CompactVMatrix::CompactVMatrix(CompactVMatrix* cvm, VMat m,...) out-of-range element(%d,%d)=%g not in [%g,%g]",
                            i,j,element,fixedpoint_min[j],fixedpoint_max[j]);
            }
        }
        putRow(i,row);
    }
}

void CompactVMatrix::setOneHotMode(bool on)
{
    one_hot_encoding=on;
    if (one_hot_encoding)
        width_ = normal_width;
    else
        width_ = n_variables;
}

void CompactVMatrix::getNewRow(int i, const Vec& v) const
{
#ifdef BOUNDCHECK
    if (i<0 || i>=length_)
        PLERROR("CompactVMatrix::getNewRow, row %d out of bounds [0,%d]",i,length_-1);
    if (v.length()!=width_)
        PLERROR("CompactVMatrix::getNewRow, length of v (%d) should be equal to width of VMat (%d)",v.length(),width());
#endif
    unsigned char* encoded_row = &data.data[i*row_n_bytes];
    real* vp=v.data();
    int c=0;
    for (int b=0;b<symbols_offset;b++)
    {
        unsigned char byte=encoded_row[b];
        for (int j=0;j<8 && c<n_bits;j++,c++)
        {
            int bit = byte & 1;
            byte >>= 1; // shift right once
            vp[c]=bit;
        }
    }
    for (int b=0;b<n_symbols;b++)
    {
        int byte = encoded_row[symbols_offset+b];
        if (one_hot_encoding)
        {
            int n=n_symbol_values[b];
            for (int j=0;j<n;j++) vp[c+j]=0;
            vp[c+byte]=1;
            c+=n;
        }
        else vp[c++]=byte;
    }
    unsigned char* fixed_point_numbers = &encoded_row[fixedpoint_offset];
    for (int j=0;j<n_fixedpoint;j++,c++)
    {
        unsigned char *uc = &fixed_point_numbers[2*j];
        short_and_twobytes u;
        u.twobytes[0]=uc[0];
        u.twobytes[1]=uc[1];
        real decoded = u.us*delta[j]+fixedpoint_min[j];
        // correct rounding errors for integers, due to fixed-point low precision
        real rounded_decoded = rint(decoded);
        if (fabs(rounded_decoded-decoded)<1e-4)
            decoded = rounded_decoded;
        vp[c]=decoded;
    }
}

//#define SANITYCHECK_CompactVMatrix
#define SANITYCHECK_CompactVMatrix_PRECISION 1e-5

real CompactVMatrix::dot(int i, int j, int inputsize) const
{
    if(inputsize!=width()-n_last)
        PLERROR("In CompactVMatrix::dot, in current implementation inputsize must be equal to width()-n_last");

    unsigned char* encoded_row_i = &data.data[i*row_n_bytes];
    unsigned char* encoded_row_j = &data.data[j*row_n_bytes];
    real dot_product=0.;
    int c=0;
    for (int b=0;b<symbols_offset;b++)
    {
        unsigned char byte_i=encoded_row_i[b];
        unsigned char byte_j=encoded_row_j[b];
        unsigned char byte_and = byte_i & byte_j;
#ifdef SANITYCHECK_CompactVMatrix
        real check=dot_product;
#endif
        // Here we want to count the number of ON bits in the byte_and
        // instead of looping through the bits, we look-up in a
        // pre-computed table (n_bits_in_byte), which has been set-up by set_n_bits_in_byte().
        dot_product += n_bits_in_byte[byte_and];
#ifdef SANITYCHECK_CompactVMatrix
        for (int j=0;j<8 && c<n_bits;j++,c++)
        {
            check += byte_and & 1;
            byte_and >>= 1; // shift right once
        }
        if (check!=dot_product)
            PLERROR("logic error in n_bits_in_byte");
#else
        c+=8;
        if (c>n_bits) c=n_bits;
#endif
    }
    if (c>width_-n_last)
        PLERROR("CompactVMatrix: n_last should be among discrete non-binary or continuous variables");
    for (int b=0;b<n_symbols && c<width_-n_last;b++)
    {
        int byte_i = encoded_row_i[symbols_offset+b];
        int byte_j = encoded_row_j[symbols_offset+b];
        if (byte_i==byte_j) dot_product++;
        if (one_hot_encoding)
            c+=n_symbol_values[b];
        else
            c++;
    }
    unsigned char* fixed_point_numbers_i = &encoded_row_i[fixedpoint_offset];
    unsigned char* fixed_point_numbers_j = &encoded_row_j[fixedpoint_offset];
    for (int k=0;k<n_fixedpoint-n_last && c<width_-n_last;k++,c++)
    {
        unsigned char *uc = &fixed_point_numbers_i[2*k];
        short_and_twobytes u;
        u.twobytes[0]=uc[0];
        u.twobytes[1]=uc[1];
        real decoded_i = u.us*delta[k]+fixedpoint_min[k];
        uc = &fixed_point_numbers_j[2*k];
        u.twobytes[0]=uc[0];
        u.twobytes[1]=uc[1];
        real decoded_j = u.us*delta[k]+fixedpoint_min[k];
#ifdef SANITYCHECK_CompactVMatrix
        real rounded_decoded_i = rint(decoded_i);
        if (fabs(rounded_decoded_i-decoded_i)<1e-4)
            decoded_i = rounded_decoded_i;
        real rounded_decoded_j = rint(decoded_j);
        if (fabs(rounded_decoded_j-decoded_j)<1e-4)
            decoded_j = rounded_decoded_j;
#endif
        dot_product += decoded_i * decoded_j;
    }

    return dot_product;
}

// I used the code for getRow as a basis to implement this call (Pascal)
real CompactVMatrix::dot(int i, const Vec& v) const
{
#ifdef BOUNDCHECK
    if (i<0 || i>=length_)
        PLERROR("CompactVMatrix::dot, row %d out of bounds [0,%d]",i,length_-1);
#endif

    if(v.length()!=width()-n_last)
        PLERROR("In CompactVMatrix::dot, in current implementation v.length() must be equal to width()-n_last");

    real dot_product = 0.;

    unsigned char* encoded_row = &data.data[i*row_n_bytes];
    real* vp=v.data();
    int c=0;
    for (int b=0;b<symbols_offset;b++)
    {
        unsigned char byte=encoded_row[b];
        for (int j=0;j<8 && c<n_bits;j++,c++)
        {
            int bit = byte & 1;
            byte >>= 1; // shift right once
            if(bit)
                dot_product += vp[c];
        }
    }
    for (int b=0;b<n_symbols;b++)
    {
        int byte = encoded_row[symbols_offset+b];
        if (one_hot_encoding)
        {
            int n=n_symbol_values[b];
            dot_product += vp[c+byte];
            c += n;
        }
        else
            dot_product += vp[c++]*byte;
    }
    // WARNING: COULD THIS CAUSE PROBLEMS IF fixedpoint_offset IS NOT A MULTIPLE OF 4
    // ON SOME MACHINES?
    unsigned char* fixed_point_numbers = &encoded_row[fixedpoint_offset];
    for (int j=0;j<n_fixedpoint-n_last && c<v.length();j++,c++)
    {
        unsigned char *uc = &fixed_point_numbers[2*j];
        short_and_twobytes u;
        u.twobytes[0]=uc[0];
        u.twobytes[1]=uc[1];
        real decoded = u.us*delta[j]+fixedpoint_min[j];
        // correct rounding errors for integers, due to fixed-point low precision
        real rounded_decoded = rint(decoded);
        if (fabs(rounded_decoded-decoded)<1e-4)
            decoded = rounded_decoded;
        dot_product += vp[c] * decoded;
    }

    // Very Slow SANITY CHECK
#ifdef SANITYCHECK_CompactVMatrix
    Vec v_i(v.length());
    getRow(i,v_i);
    real dot_product2 = PLearn::dot(v_i.subVec(0,v.length()),v);
    real diff = fabs(dot_product-dot_product2)/fabs(dot_product2);
    if(diff>SANITYCHECK_CompactVMatrix_PRECISION)
        PLERROR("IN CompactVMatrix::dot(int i=%d, v) SANITY CHECK FAILED: difference=%g",i,diff);
#endif

    return dot_product;
}


real CompactVMatrix::dotProduct(int i, int j) const
{ return dot(i,j,width()-n_last); }

real CompactVMatrix::squareDifference(int i, int j)
{
    if (row_norms.length()==0)
        row_norms = Vec(length_,-1.0);
    real normi = row_norms[i];
    if (normi<0) normi=row_norms[i]=dotProduct(i,i);
    real normj = row_norms[j];
    if (normj<0) normj=row_norms[j]=dotProduct(j,j);
    return normi + normj - 2 * dotProduct(i,j);
}

void CompactVMatrix::encodeAndPutRow(int i, Vec v)
{
    unsigned char* encoded_row = &data.data[i*row_n_bytes];
    real* vp=v.data();
    int* perm=variables_permutation.data();
    int c=0;
    // 1 vector element ==> 1 bit
    for (int b=0;b<symbols_offset;b++)
    {
        unsigned char byte=0;
        for (int j=0;j<8 && c<n_bits;j++,c++)
            byte |= int(vp[perm[c]]) << j; // shift to right bit position
        encoded_row[b]=byte;
    }
    //    1 vector element (integer between 0 and n-1) ==> 1 byte
    for (int b=0;b<n_symbols;b++,c++)
    {
        real val = vp[perm[c]];
        int s = int(val);
        if (!fast_exact_is_equal(s, val))
            PLERROR("CompactVMatrix::encodeAndPutRow(%d,v): v[%d]=%g not an integer",
                    i,int(perm[c]),val);
        encoded_row[symbols_offset+b] = s; // ASSUMES THAT v IS NOT ONE-HOT ENCODED
        if (s<0 || s>=n_symbol_values[b])
            PLERROR("CompactVMatrix::encodeAndPutRow(%d,v): v[%d]=%d not in expected range (0,%d)",
                    i,int(perm[c]),s,n_symbol_values[b]-1);
    }
    // WARNING: COULD THIS CAUSE PROBLEMS IF fixedpoint_offset IS NOT A MULTIPLE OF 4
    // ON SOME MACHINES?
    unsigned short* fixed_point_numbers = (unsigned short*)&encoded_row[fixedpoint_offset];
    for (int j=0;j<n_fixedpoint;j++,c++)
        fixed_point_numbers[j]=(unsigned short)((vp[perm[c]]-fixedpoint_min[j])/delta[j]);

    invalidateBuffer();
}

void CompactVMatrix::putRow(int i, Vec v)
{
    putSubRow(i,0,v);
}

void CompactVMatrix::putSubRow(int i, int j, Vec v)
{
    unsigned char* encoded_row = &data.data[i*row_n_bytes];
    real* vp=v.data();
    int c=0;
    // 1 vector element ==> 1 bit
    for (int b=0;b<symbols_offset;b++)
    {
        unsigned char byte=0;
        for (int k=0;k<8 && c<n_bits;k++,c++)
            if (c>=j)
                byte |= int(vp[c-j]) << k; // shift to right bit position
        encoded_row[b]=byte;
    }
    // if (one_hot_encoding)
    //   n vector elements in one-hot-code ==> 1 byte
    // else
    //   1 vector element (integer between 0 and n-1) ==> 1 byte
    int n=0;
    if (one_hot_encoding)
        for (int b=0;b<n_symbols;b++,c+=n)
        {
            n=n_symbol_values[b];
            if (c>=j)
            {
                int pos=-1;
                for (int k=0;k<n;k++)
                {
                    real vk=vp[c+k-j];
                    if (!fast_exact_is_equal(vk, 0) &&
                        !fast_exact_is_equal(vk, 1))
                        PLERROR("CompactVMatrix::putRow(%d,v): v[%d]=%g!=0 or 1 (not one-hot-code)",
                                i,c,vk);
                    if (fast_exact_is_equal(vk, 1))
                    {
                        if (pos<0) pos=k;
                        else PLERROR("CompactVMatrix::putRow(%d,v): %d-th symbol not one-hot-encoded",
                                     i,b);
                    }
                }
                if (pos<0)
                    PLERROR("CompactVMatrix::putRow(%d,v): %d-th symbol not one-hot-encoded",
                            i,b);
                encoded_row[symbols_offset+b] = pos;
            }
        }
    else
        for (int b=0;b<n_symbols;b++,c++)
            if (c>=j)
            {
                real val = vp[c-j];
                int s = int(val);
                if (!fast_exact_is_equal(s, val))
                    PLERROR("CompactVMatrix::encodeAndPutRow(%d,v): v[%d]=%g not an integer",
                            i,c,val);
                encoded_row[symbols_offset+b] = s; // ASSUMES THAT v IS NOT ONE-HOT ENCODED
                if (s<0 || s>=n_symbol_values[b])
                    PLERROR("CompactVMatrix::encodeAndPutRow(%d,v): v[%d]=%d not in expected range (0,%d)",
                            i,c,s,n_symbol_values[b]-1);
            }

    // 1 vector element (real betweeen fixedpoint_min and fixedpoint_max) ==> 2 bytes
    //
    // WARNING: COULD THIS CAUSE PROBLEMS IF fixedpoint_offset IS NOT A MULTIPLE OF 4
    // ON SOME MACHINES?
    unsigned short* fixed_point_numbers = (unsigned short*)&encoded_row[fixedpoint_offset];
    for (int k=0;k<n_fixedpoint;k++,c++)
        if (c>=j)
            fixed_point_numbers[k]=(unsigned short)((vp[c-j]-fixedpoint_min[k])/delta[k]);

    invalidateBuffer();
}

void CompactVMatrix::perturb(int i, Vec v, real noise_level, int n_last)
{
#ifdef BOUNDCHECK
    if (i<0 || i>=length_)
        PLERROR("CompactVMatrix::perturb, row %d out of bounds [0,%d]",i,length_-1);
    if (v.length()!=width_)
        PLERROR("CompactVMatrix::perturb, length of v (%d) should be equal to width of VMat (%d)",v.length(),width());
#endif
    if (fieldstats.size()!=n_variables)
        PLERROR("CompactVMatrix::perturb: stats not computed or wrong size");
    if (noise_level<0 || noise_level>1)
        PLERROR("CompactVMatrix::perturb: noise_level=%g, should be in [0,1]",noise_level);

    unsigned char* encoded_row = &data.data[i*row_n_bytes];
    real* vp=v.data();
    int c=0;
    int var=0;
    Vec probs(width_);
    for (int b=0;b<symbols_offset;b++)
    {
        unsigned char byte=encoded_row[b];
        for (int j=0;j<8 && c<n_bits;j++,c++,var++)
        {
            int bit = byte & 1;
            byte >>= 1; // shift right once
            vp[c]=binomial_sample((1-noise_level)*bit+noise_level*fieldstats[var].prob(1));
        }
    }
    for (int b=0;b<n_symbols;b++,var++)
    {
        int byte = encoded_row[symbols_offset+b];
        int nv=n_symbol_values[b];
        probs.resize(nv);
        VMFieldStat& stat=fieldstats[var];
        for (int val=0;val<nv;val++)
            if (val==byte)
                probs[val]=(1-noise_level)+noise_level*stat.prob(val);
            else
                probs[val]=noise_level*stat.prob(val);
        byte = multinomial_sample(probs);
        if (one_hot_encoding)
        {
            int n=n_symbol_values[b];
            for (int j=0;j<n;j++) vp[c+j]=0;
            vp[c+byte]=1;
            c+=n;
        }
        else vp[c++]=byte;
    }
    unsigned char* fixed_point_numbers = &encoded_row[fixedpoint_offset];
    for (int j=0;j<n_fixedpoint;j++,c++,var++)
    {
        unsigned char *uc = &fixed_point_numbers[2*j];
        short_and_twobytes u;
        u.twobytes[0]=uc[0];
        u.twobytes[1]=uc[1];
        real decoded = u.us*delta[j]+fixedpoint_min[j];
        // correct rounding errors for integers, due to fixed-point low precision
        real rounded_decoded = rint(decoded);
        if (fabs(rounded_decoded-decoded)<1e-4)
            decoded = rounded_decoded;
        if (var<n_variables-n_last)
        {
            int ntry=0;
            do
            {
                vp[c]=decoded+noise_level*fieldstats[var].stddev()*normal_sample();
                ntry++;
                if (ntry>=100)
                    PLERROR("CompactVMatrix::perturb:Something wrong in resampling, tried 100 times");
            }
            while (vp[c]<fixedpoint_min[j] || vp[c]>fixedpoint_max[j]);
        }
        else
            vp[c]=decoded;
    }
}
/*
  void CompactVMatrix::write(ostream& out) const
  {
  writeHeader(out,"CompactVMatrix");
  writeField(out,"length",length_);
  writeField(out,"width",normal_width);
  writeField(out,"fieldinfos",fieldinfos);
  writeField(out,"fieldstats",fieldstats);
  writeField(out,"row_n_bytes",row_n_bytes);
  writeField(out,"n_bits",n_bits);
  writeField(out,"n_symbols",n_symbols);
  writeField(out,"n_fixedpoint",n_fixedpoint);
  writeField(out,"one_hot_encoding",one_hot_encoding);
  writeField(out,"n_symbol_values",n_symbol_values);
  writeField(out,"fixedpoint_min",fixedpoint_min);
  writeField(out,"fixedpoint_max",fixedpoint_max);
  writeField(out,"delta",delta);
  writeField(out,"variables_permutation",variables_permutation);
  writeField(out,"symbols_offset",symbols_offset);
  writeField(out,"fixedpoint_offset",fixedpoint_offset);
  out.write((char*)data.data,data.length()*sizeof(unsigned char));
  writeFooter(out,"CompactVMatrix");
  }

  void CompactVMatrix::oldread(istream& in)
  {
  readHeader(in,"CompactVMatrix");
  readField(in,"length",length_);
  readField(in,"width",normal_width);
  readField(in,"fieldinfos",fieldinfos);
  fieldinfos.resize(0); // to fix current bug in setting fieldinfos
  readField(in,"fieldstats",fieldstats);
  readField(in,"row_n_bytes",row_n_bytes);
  readField(in,"n_bits",n_bits);
  readField(in,"n_symbols",n_symbols);
  readField(in,"n_fixedpoint",n_fixedpoint);
  n_variables = n_bits + n_symbols + n_fixedpoint;
  readField(in,"one_hot_encoding",one_hot_encoding);
  setOneHotMode(one_hot_encoding);
  readField(in,"n_symbol_values",n_symbol_values);
  readField(in,"fixedpoint_min",fixedpoint_min);
  readField(in,"fixedpoint_max",fixedpoint_max);
  readField(in,"delta",delta);
  readField(in,"variables_permutation",variables_permutation);
  readField(in,"symbols_offset",symbols_offset);
  readField(in,"fixedpoint_offset",fixedpoint_offset);
  data.resize(row_n_bytes*length_);
  in.read((char*)data.data,data.length()*sizeof(unsigned char));
  readFooter(in,"CompactVMatrix");
  }
*/
void CompactVMatrix::append(CompactVMatrix* vm)
{
    if (width_!=vm->width())
        PLERROR("CompactVMatrix::append, incompatible width %d vs %d",
                width_,vm->width());
    if (row_n_bytes!=vm->row_n_bytes)
        PLERROR("CompactVMatrix::append, incompatible row_n_bytes %d vs %d",
                row_n_bytes,vm->row_n_bytes);
    if (n_bits!=vm->n_bits)
        PLERROR("CompactVMatrix::append, incompatible n_bits %d vs %d",
                n_bits,vm->n_bits);
    if (n_symbols!=vm->n_symbols)
        PLERROR("CompactVMatrix::append, incompatible n_symbols %d vs %d",
                n_symbols,vm->n_symbols);
    if (n_fixedpoint!=vm->n_fixedpoint)
        PLERROR("CompactVMatrix::append, incompatible n_fixedpoint %d vs %d",
                n_fixedpoint,vm->n_fixedpoint);
    if (n_symbol_values!=vm->n_symbol_values)
    {
        //n_symbol_values.write(cerr); cerr << endl;
        //vm->n_symbol_values.write(cerr); cerr << endl;
        PLearn::write(cerr, n_symbol_values);
        cerr << endl;
        PLearn::write(cerr, vm->n_symbol_values);
        cerr << endl;
        PLERROR("CompactVMatrix::append, incompatible n_symbol_values");
    }
    bool rescale = false;
    for (int j=0;j<n_fixedpoint && !rescale;j++)
        if (fixedpoint_min[j]>vm->fixedpoint_min[j] ||
            fixedpoint_max[j]<vm->fixedpoint_max[j]) rescale=true;
    if (rescale)
    {
        cout << "The appended VMat has intervals that are wider than the current one." << endl;
        cout << "Start rescaling numeric variables fixed point representation." << endl;
        Vec new_min = fixedpoint_min.copy();
        Vec new_max = fixedpoint_max.copy();
        Vec new_delta = delta.copy();
        TVec<bool> change(n_fixedpoint);
        for (int j=0;j<n_fixedpoint;j++)
        {
            change[j]=false;
            if (fixedpoint_min[j]>vm->fixedpoint_min[j])
            {
                change[j]=true;
                new_min[j]=vm->fixedpoint_min[j];
            }
            if (fixedpoint_max[j]<vm->fixedpoint_max[j])
            {
                change[j]=true;
                new_max[j]=vm->fixedpoint_max[j];
            }
            if (change[j])
                new_delta[j]=(new_max[j]-new_min[j])/USHRT_MAX;
        }
        for (int r=0;r<length_;r++)
        {
            unsigned char* encoded_row = &data.data[r*row_n_bytes];
            unsigned char* fixed_point_numbers = &encoded_row[fixedpoint_offset];
            for (int j=0;j<n_fixedpoint;j++)
                if (change[j])
                {
                    // DECODE using previous min/max
                    unsigned char *uc = &fixed_point_numbers[2*j];
                    short_and_twobytes u;
                    u.twobytes[0]=uc[0];
                    u.twobytes[1]=uc[1];
                    real decoded = u.us*delta[j]+fixedpoint_min[j];
                    // correct rounding errors for integers, due to fixed-point low precision
                    real rounded_decoded = rint(decoded);
                    if (fabs(rounded_decoded-decoded)<1e-4)
                        decoded = rounded_decoded;
                    // ENCODE using new min/max
                    fixed_point_numbers[j]=(unsigned char)((decoded-new_min[j])/new_delta[j]);
                }
        }
        cout << "DONE rescaling numeric variables fixed point representation." << endl;
        fixedpoint_min << new_min;
        fixedpoint_max << new_max;
        delta << new_delta;
    }
    int new_length=length_+vm->length();
    data.resize(row_n_bytes*new_length);
    // copy the new data
    Vec row(width_);
    bool old_vm_encoding = vm->one_hot_encoding;
    bool old_encoding = one_hot_encoding;
    vm->one_hot_encoding=false;
    setOneHotMode(false);
    int old_length=length_;
    length_=new_length;
    for (int r=0;r<vm->length();r++)
    {
        vm->getRow(r,row);
        putRow(old_length+r,row);
    }
    vm->one_hot_encoding=old_vm_encoding;
    setOneHotMode(old_encoding);
}

void CompactVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    deepCopyField(data, copies);
    deepCopyField(n_symbol_values, copies);
    deepCopyField(fixedpoint_min, copies);
    deepCopyField(fixedpoint_max, copies);
    deepCopyField(variables_permutation, copies);
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
