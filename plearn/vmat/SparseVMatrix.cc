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

#include "SparseVMatrix.h"

namespace PLearn {
using namespace std;


/** SparseVMatrix **/

PLEARN_IMPLEMENT_OBJECT(SparseVMatrix, "ONE LINE DESC", "NO HELP");

SparseVMatrix::SparseVMatrix(const string& filename)
    : nelements(0), positions(0), values(0), rows(0)
{
    load(filename);
    updateMtime(filename);
}

SparseVMatrix::SparseVMatrix(VMat m)
    : inherited(m.length(),m.width()), nelements(0), positions(0), values(0), rows(0)
{
    fieldinfos = m->getFieldInfos();                // Copy the field infos

    updateMtime(m);

    if(m.width()>USHRT_MAX)
        PLERROR("In SparseVMatrix constructor: m.width()=%d can't be greater than USHRT_MAX=%d",m.width(),USHRT_MAX);
    Vec v(m.width());
    real* vptr = v.data();

    // First count nelements
    nelements = 0;
    if(m->hasStats()) // use the stats!
    {
        for(int j=0; j<m.width(); j++)
        {
            const VMFieldStat& st = m->fieldStat(j);
            nelements += st.nmissing() + st.npositive() + st.nnegative();
        }
    }
    else // let's count them ourself
    {
        for(int i=0; i<m.length(); i++)
        {
            m->getRow(i,v);
            for(int j=0; j<v.length(); j++)
                if(!fast_exact_is_equal(vptr[j], 0.))
                    nelements++;
        }
    }

    // Now allocate space for those elements
    if(nelements>0)
    {
        positions = new unsigned short[nelements];
        values = new float[nelements];
        int l=length();
        rows = new SparseVMatrixRow[l];

        int pos = 0;
        // Fill the representation
        for(int i=0; i<m.length(); i++)
        {
            m->getRow(i,v);
            SparseVMatrixRow& r = rows[i];
            r.row_startpos = pos;
            int nelem = 0;
            for(int j=0; j<v.length(); j++)
                if(!fast_exact_is_equal(vptr[j], 0.))
                {
                    positions[pos] = j;
                    values[pos] = (float)vptr[j];
                    pos++;
                    nelem++;
                }
            r.nelements = nelem;
        }
    }
}

void
SparseVMatrix::build()
{
    inherited::build();
    build_();
}

void
SparseVMatrix::build_()
{
    // TODO
}

void
SparseVMatrix::declareOptions(OptionList &ol)
{
    inherited::declareOptions(ol);
}

void SparseVMatrix::getNewRow(int i, const Vec& v) const
{
#ifdef BOUNDCHECK
    if(i<0 || i>=length())
        PLERROR("In SparseVMatrix::getNewRow, row number i=%d OUT OF BOUNDS (matrix is %dx%d)",i,length(),width());
    if(v.length()!=width())
        PLERROR("In SparseVMatrix::getNewRow, length of v (%d) is different from width of VMatris (%d)",v.length(),width());
#endif

    if(nelements==0)
        v.clear();
    else
    {
        SparseVMatrixRow row_i = rows[i];
        float* valueptr =  values + row_i.row_startpos;
        unsigned short* positionptr = positions + row_i.row_startpos;
        int n = row_i.nelements;

        real* vdata = v.data();

        int j = 0;
        while(n--)
        {
            int nextpos = (int) *positionptr++;
            real nextval = (real) *valueptr++;
            while(j<nextpos)
                vdata[j++] = 0.;
            vdata[j++] = nextval;
        }
        while(j<v.length())
            vdata[j++] = 0.;
    }
}

real SparseVMatrix::dot(int i1, int i2, int inputsize) const
{
#ifdef BOUNDCHECK
    if(i1<0 || i1>=length() || i2<0 || i2>=length() || inputsize>width())
        PLERROR("IN SparseVMatrix::dot OUT OF BOUNDS");
#endif

    if(nelements==0)
        return 0.;

    SparseVMatrixRow row_1 = rows[i1];
    float* valueptr_1 =  values + row_1.row_startpos;
    unsigned short* positionptr_1 = positions + row_1.row_startpos;
    int n_1 = row_1.nelements;

    SparseVMatrixRow row_2 = rows[i2];
    float* valueptr_2 =  values + row_2.row_startpos;
    unsigned short* positionptr_2 = positions + row_2.row_startpos;
    int n_2 = row_2.nelements;

    real res = 0.;

    while(n_1 && n_2)
    {
        if(*positionptr_1>=inputsize)
            break;
        if(*positionptr_1==*positionptr_2)
        {
            res += (*valueptr_1)*(*valueptr_2);
            positionptr_1++;
            valueptr_1++;
            n_1--;
            positionptr_2++;
            valueptr_2++;
            n_2--;
        }
        else if(*positionptr_1<*positionptr_2)
        {
            positionptr_1++;
            valueptr_1++;
            n_1--;
        }
        else
        {
            positionptr_2++;
            valueptr_2++;
            n_2--;
        }
    }

    return res;
}

real SparseVMatrix::dot(int i, const Vec& v) const
{
#ifdef BOUNDCHECK
    if(i<0 || i>=length() || v.length()>width())
        PLERROR("IN SparseVMatrix::dot OUT OF BOUNDS");
#endif

    if(nelements==0)
        return 0.;

    SparseVMatrixRow row_i = rows[i];
    float* valueptr =  values + row_i.row_startpos;
    unsigned short* positionptr = positions + row_i.row_startpos;
    int n = row_i.nelements;

    real* vdata = v.data();
    real res = 0.;

    while(n--)
    {
        int nextpos = (int) *positionptr++;
        real nextval = (real) *valueptr++;
        if(nextpos>=v.length())
            break;
        res += nextval*vdata[nextpos];
    }
    return res;
}
/*
  void SparseVMatrix::write(ostream& out) const
  {
  writeHeader(out,"SparseVMatrix");
  writeField(out,"length",length_);
  writeField(out,"width",width_);
  writeField(out,"fieldinfos",fieldinfos);
  writeField(out,"fieldstats",fieldstats);
  writeField(out,"nelements",nelements);
  write_ushort(out,positions,nelements,false);
  write_float(out,values,nelements,false);
  for(int i=0; i<length(); i++)
  {
  write_int(out,rows[i].nelements);
  write_int(out,rows[i].row_startpos);
  }
  writeFooter(out,"SparseVMatrix");
  }

  void SparseVMatrix::oldread(istream& in)
  {
  readHeader(in,"SparseVMatrix");
  readField(in,"length",length_);
  readField(in,"width",width_);
  readField(in,"fieldinfos",fieldinfos);
  fieldinfos.resize(0); // to fix current bug in setting fieldinfos
  readField(in,"fieldstats",fieldstats);

  if(nelements>0)
  {
  delete[] positions;
  delete[] values;
  delete[] rows;
  }
  readField(in,"nelements",nelements);
  positions = new unsigned short[nelements];
  values = new float[nelements];
  rows = new SparseVMatrixRow[length()];

  read_ushort(in,positions,nelements,false);
  read_float(in,values,nelements,false);
  for(int i=0; i<length(); i++)
  {
  rows[i].nelements = read_int(in);
  rows[i].row_startpos = read_int(in);
  }
  readFooter(in,"SparseVMatrix");
  }
*/
SparseVMatrix::~SparseVMatrix()
{
    if(nelements>0)
    {
        delete[] positions;
        delete[] values;
        delete[] rows;
    }
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
