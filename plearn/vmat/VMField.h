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
 * This file is part of the PLearn library.
 ******************************************************* */


/*! \file VMField.h */

#ifndef VMField_INC
#define VMField_INC

#include <plearn/base/general.h>
#include <plearn/io/PStream.h>

namespace PLearn {
using namespace std;

//!  a VMField contains a fieldname and a fieldtype
class VMField
{
public:

    enum FieldType
    {
        UnknownType = 0,
        Continuous,
        DiscrGeneral,
        DiscrMonotonic,
        DiscrFloat,
        Date
    };

    string name;
    FieldType fieldtype;

    VMField(const string& the_name="", FieldType the_fieldtype=UnknownType);

    bool operator==(const VMField& other) const;
    bool operator!=(const VMField& other) const;

};

PStream& operator>>(PStream& in, VMField::FieldType& x); // dummy placeholder; do not call

PStream& operator>>(PStream& in, VMField& x);
PStream& operator<<(PStream& out, const VMField& x);

//!  this class holds simple statistics about a field
class VMFieldStat
{
protected:
    int nmissing_;  //!<  number of missing values
    int nnonmissing_;  //!<  number of non-missing values
    int npositive_; //!<  number of values >0
    int nnegative_; //!<  number of values <0
    double sum_;    //!<  sum of all non missing values
    double sumsquare_; //!<  sum of square of all non missing values
    real min_;       //!<  minimum value
    real max_;       //!<  maximum value

    //!  maximum number of different discrete values to keep track of
    int maxndiscrete;

public:

    //!  counts of discrete values. If the size of counts exceeds maxndiscrete
    //!  maxndiscrete is set to -1, counts is erased, and we stop counting!
    map<real,int> counts;

    VMFieldStat(int the_maxndiscrete=255);

    int count() const { return nmissing_ + nnonmissing_; } //!<  should be equal to length of VMField
    int nmissing() const { return nmissing_; }
    int nnonmissing() const { return nnonmissing_; }
    int npositive() const { return npositive_; }
    int nnegative() const { return nnegative_; }
    int nzero() const { return nnonmissing_ - (npositive_+nnegative_); }
    real sum() const { return real(sum_); }
    real sumsquare() const { return real(sumsquare_); }
    real min() const { return min_; }
    real max() const { return max_; }
    real mean() const { return real(sum_/nnonmissing_); }
    real variance() const { return real((sumsquare_ - square(sum_)/nnonmissing_)/(nnonmissing_-1)); }
    real stddev() const { return sqrt(variance()); }

    real prob(real value) { return counts[value]/real(nnonmissing()); }

    void update(real val);

    void write(PStream& out) const;
    void read(PStream& in);
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
