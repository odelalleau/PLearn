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

#include "MemoryVMatrix.h"

namespace PLearn {
using namespace std;



/** MemoryVMatrix **/

PLEARN_IMPLEMENT_OBJECT(
    MemoryVMatrix,
    "A VMatrix whose data is stored in memory.",
    "The data can either be given directly by a Mat, or by another VMat that\n"
    "will be precomputed in memory at build time.\n"
);

///////////////////
// MemoryVMatrix //
///////////////////
MemoryVMatrix::MemoryVMatrix()
    : synch_data(true),
      data(Mat()),
      deep_copy_memory_data(true)
{
    memory_data = data;
}

MemoryVMatrix::MemoryVMatrix(int l, int w)
    : inherited(l, w),
      synch_data(false),
      deep_copy_memory_data(true)
{
    data.resize(l,w);
    memory_data = data;
    defineSizes(data.width(), 0, 0);
}

MemoryVMatrix::MemoryVMatrix(const Mat& the_data)
    : inherited(the_data.length(), the_data.width()),
      synch_data(true),
      data(the_data),
      deep_copy_memory_data(true)

{
    memory_data = the_data;
    defineSizes(the_data.width(), 0, 0);
}

MemoryVMatrix::MemoryVMatrix(VMat the_source, bool call_build_):
    inherited(the_source->length(), the_source->width(), call_build_),
    synch_data(false),
    source(the_source),
    deep_copy_memory_data(true)
{
    if (call_build_)
        build_();
}

////////////////////
// declareOptions //
////////////////////
void MemoryVMatrix::declareOptions(OptionList& ol)
{
    declareOption(ol, "data", &MemoryVMatrix::data, OptionBase::buildoption,
                  "The external Mat source");

    declareOption(ol, "data_vm", &MemoryVMatrix::source,
                  (OptionBase::learntoption | OptionBase::nosave),
                  "DEPRECATED - Use 'source' instead.");

    declareOption(ol, "source", &MemoryVMatrix::source,
                  OptionBase::buildoption,
                  "The (optional) source VMatrix. Will overwrite 'data'"
                  " if provided.");

    declareOption( ol, "fieldnames", &MemoryVMatrix::fieldnames,
                   OptionBase::buildoption,
                   "If provided, will be used to set this VMatrix's"
                   " fieldnames." );

    declareOption( ol, "deep_copy_memory_data", &MemoryVMatrix::deep_copy_memory_data,
                   OptionBase::buildoption,
                   "If true, when this object is deep copied, we will deep copy the memory_data");

    /* This field was declared as an option, but the author does not remember
     * why. The problem is that we do not want it to be a learnt option, since
     * it may save the whole dataset pointed by 'source', which could waste a
     * lot of disk space.
     * As a result, the two lines below are now commented out.
    declareOption(ol, "memory_data", &MemoryVMatrix::memory_data, OptionBase::learntoption,
                  "The underlying Mat with the data.");
    */

    inherited::declareOptions(ol);
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void MemoryVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    if(deep_copy_memory_data){
        deepCopyField(memory_data,  copies);
        deepCopyField(data,         copies);
    }
    deepCopyField(fieldnames,   copies);
    deepCopyField(source,       copies);
}

////////////
// build_ //
////////////
void MemoryVMatrix::build_()
{
    if (source) {
        // Precompute data from source
        memory_data = source->toMat();
        copySizesFrom(source);
        setMetaInfoFrom(source);
        synch_data = false;
    } else {
        synch_data = true;
    }
    if (synch_data) {
        memory_data = data;
        // We temporarily set data to a new empty Mat, so that memory_data
        // can be safely resized.
        data = Mat();
    }
    if (this->length() >= 0 && this->length() != memory_data.length()) {
        // New length specified.
        memory_data.resize(this->length(), memory_data.width());
    }
    if (this->width() >= 0 && this->width() != memory_data.width()) {
        // New width specified.
        memory_data.resize(memory_data.length(), this->width());
    }
    if (this->length() < 0 && memory_data.length() >= 0) {
        // Take the length from the data matrix.
        this->length_ = memory_data.length();
    }
    if (this->width() < 0 && memory_data.width() >= 0) {
        // Take the width from the data matrix.
        this->width_ = memory_data.width();
    }
    if (synch_data)
        // Restore data so that it is equal to memory_data.
        data = memory_data;

    if ( fieldnames.length() == memory_data.width() )
        declareFieldNames(fieldnames);
}

///////////
// build //
///////////
void MemoryVMatrix::build()
{
    inherited::build();
    build_();
}

/////////
// get //
/////////
real MemoryVMatrix::get(int i, int j) const
{ return memory_data(i,j); }

void MemoryVMatrix::put(int i, int j, real value)
{ memory_data(i,j) = value; }

void MemoryVMatrix::getColumn(int i, Vec v) const
{ v << memory_data.column(i); }

void MemoryVMatrix::getSubRow(int i, int j, Vec v) const
{
#ifdef BOUNDCHECK
    if (j+v.length()>width())
        PLERROR("MemoryVMatrix::getSubRow(int i, int j, Vec v) OUT OF BOUNDS. "
                "j=%d, v.length()=%d, width()=%d", j, v.length(), width());
#endif
    if (v.length() > 0)
        v.copyFrom(memory_data[i]+j, v.length());
}

////////////
// getRow //
////////////
void MemoryVMatrix::getRow(int i, Vec v) const
{
    PLASSERT( v.length() == width_ );
    if (width_ > 0)
        v.copyFrom(memory_data[i], width_);
}

////////////
// getMat //
////////////
void MemoryVMatrix::getMat(int i, int j, Mat m) const
{ m << memory_data.subMat(i,j,m.length(),m.width()); }

///////////////
// putSubRow //
///////////////
void MemoryVMatrix::putSubRow(int i, int j, Vec v)
{
#ifdef BOUNDCHECK
    if (j+v.length()>width())
        PLERROR("MemoryVMatrix::putSubRow(int i, int j, Vec v) OUT OF BOUNDS. "
                "j=%d, v.length()=%d, width()=%d", j, v.length(), width());
#endif
    if (v.length() > 0)
        v.copyTo(memory_data[i]+j);
}

//////////
// fill //
//////////
void MemoryVMatrix::fill(real value)
{ memory_data.fill(value); }


////////////
// putRow //
////////////
void MemoryVMatrix::putRow(int i, Vec v)
{
    if (v.length() > 0)
        v.copyTo(memory_data[i]);
}

////////////
// putMat //
////////////
void MemoryVMatrix::putMat(int i, int j, Mat m)
{ memory_data.subMat(i,j,m.length(),m.width()) << m; }

///////////////
// appendRow //
///////////////
void MemoryVMatrix::appendRow(Vec v)
{
    memory_data.appendRow(v);
    length_++;
}

///////////
// toMat //
///////////
Mat MemoryVMatrix::toMat() const
{ return memory_data; }

////////////
// subMat //
////////////
VMat MemoryVMatrix::subMat(int i, int j, int l, int w)
{
    MemoryVMatrix* result = new MemoryVMatrix(memory_data.subMat(i,j,l,w));
    result->deep_copy_memory_data=deep_copy_memory_data;
    result->setMetaInfoFrom(this);
    return (VMat)result;
}

/////////
// dot //
/////////
real MemoryVMatrix::dot(int i1, int i2, int inputsize) const
{
#ifdef BOUNDCHECK
    if(inputsize>width())
        PLERROR("In MemoryVMatrix::dot inputsize>width()");
#endif
    real* v1 = memory_data.rowdata(i1);
    real* v2 = memory_data.rowdata(i2);
    real res = 0.;
    for(int k=0; k<inputsize; k++)
        res += (*v1++) * (*v2++);
    return res;
}

real MemoryVMatrix::dot(int i, const Vec& v) const
{
#ifdef BOUNDCHECK
    if(v.length()>width())
        PLERROR("In MemoryVMatrix::dot length of vector v is greater than VMat's width");
#endif
    if (v.length() > 0) {
        real* v1 = memory_data.rowdata(i);
        real* v2 = v.data();
        real res = 0.;
        for(int k=0; k<v.length(); k++)
            res += v1[k]*v2[k];
        return res;
    }
    return 0.0;                                // in the case of a null vector
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
