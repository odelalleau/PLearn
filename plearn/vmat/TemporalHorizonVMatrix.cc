// -*- C++ -*-

// TemporalHorizonVMatrix.cc
//
// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2003 Rejean Ducharme, Yoshua Bengio
// Copyright (C) 2003 Pascal Vincent
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

#include "TemporalHorizonVMatrix.h"

namespace PLearn {
using namespace std;

/** TemporalHorizonVMatrix **/

PLEARN_IMPLEMENT_OBJECT(TemporalHorizonVMatrix,
                        "Delay the last targetsize entries of a source VMat",
                        "VMat class that delay the last entries of a source"
                        " VMat by a certain horizon.\n");

TemporalHorizonVMatrix::TemporalHorizonVMatrix(bool call_build_)
    : inherited(call_build_)
{
    // don't call build_() because it would do nothing
    /* if( call_build_ )
        build_(); */
}

TemporalHorizonVMatrix::TemporalHorizonVMatrix(VMat the_source,
                                               int the_horizon,
                                               int target_size,
                                               bool call_build_)
    : inherited(the_source,
                the_source.length()-the_horizon, the_source->width(),
                call_build_),
      horizon(the_horizon),
      targetsize(target_size)
{
    fieldinfos = source->fieldinfos;
    row_delay.resize(width());
    for (int i=0; i<width(); i++)
        row_delay[i] = i<width()-targetsize ? 0 : horizon;

    defineSizes(source->inputsize(),
                source->targetsize(),
                source->weightsize());

    // don't call build_() because it would do nothing
    /* if( call_build_ )
        build_(); */
}

real TemporalHorizonVMatrix::get(int i, int j) const
{ return source->get(i+row_delay[j], j); }

void TemporalHorizonVMatrix::put(int i, int j, real value)
{ source->put(i+row_delay[j], j, value); }

real TemporalHorizonVMatrix::dot(int i1, int i2, int inputsize) const
{
    real res = 0.;
    for(int k=0; k<inputsize; k++)
        res += source->get(i1+row_delay[k],k)*source->get(i2+row_delay[k],k);
    return res;
}

real TemporalHorizonVMatrix::dot(int i, const Vec& v) const
{
    real res = 0.;
    for(int k=0; k<v.length(); k++)
        res += source->get(i+row_delay[k],k)*v[k];
    return res;
}

real TemporalHorizonVMatrix::getStringVal(int col, const string & str) const
{ return source->getStringVal(col, str); }

string TemporalHorizonVMatrix::getValString(int col, real val) const
{ return source->getValString(col,val); }

string TemporalHorizonVMatrix::getString(int row, int col) const
{ return source->getString(row+row_delay[col],col); }

const map<string,real>& TemporalHorizonVMatrix::getStringToRealMapping(int col) const
{ return source->getStringToRealMapping(col);}

const map<real,string>& TemporalHorizonVMatrix::getRealToStringMapping(int col) const
{ return source->getRealToStringMapping(col);}

void TemporalHorizonVMatrix::declareOptions(OptionList &ol)
{
    declareOption(ol, "distr", &TemporalHorizonVMatrix::source,
                  (OptionBase::learntoption | OptionBase::nosave),
                  "DEPRECATED - Use 'source' instead.");

    declareOption(ol, "horizon", &TemporalHorizonVMatrix::horizon,
                  OptionBase::buildoption,
                  "The temporal value by which to delay the source VMat");

    declareOption(ol, "targetsize", &TemporalHorizonVMatrix::targetsize,
                  OptionBase::buildoption,
                  "The number of last entries to delay");

    inherited::declareOptions(ol);
}

void TemporalHorizonVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
}

///////////
// build //
///////////
void TemporalHorizonVMatrix::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void TemporalHorizonVMatrix::build_()
{
    if (source) {
        length_ = source->length()-horizon;
        width_ = source->width();
        fieldinfos = source->fieldinfos;

        row_delay.resize(width());
        for (int i=0; i<width(); i++)
            row_delay[i] = i<width()-targetsize ? 0 : horizon;

        defineSizes(source->inputsize(),
                    source->targetsize(),
                    source->weightsize());
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
