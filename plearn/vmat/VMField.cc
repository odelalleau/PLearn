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



/*
 * $Id$
 * This file is part of the PLearn library.
 ******************************************************* */
#include "VMField.h"

namespace PLearn {
using namespace std;

/** VMField **/

VMField::VMField(const string& the_name, FieldType the_fieldtype)
    : name(the_name), fieldtype(the_fieldtype) {}

bool VMField::operator==(const VMField& other) const
{
    return (name==other.name && fieldtype==other.fieldtype);
}

bool VMField::operator!=(const VMField& other) const
{
    return !((*this)==other);
}


/** VMFieldStat **/

VMFieldStat::VMFieldStat(int the_maxndiscrete)
    : nmissing_(0), nnonmissing_(0), npositive_(0), nnegative_(0), sum_(0.),
      sumsquare_(0.), min_(FLT_MAX), max_(-FLT_MAX),
      maxndiscrete(the_maxndiscrete) {}

void VMFieldStat::update(real val)
{
    if(is_missing(val))
        nmissing_++;
    else
    {
        nnonmissing_++;
        sum_ += val;
        sumsquare_ += val*val;
        if(val>0.)
            npositive_++;
        else if(val<0.)
            nnegative_++;
        if(val<min_)
            min_ = val;
        if(val>max_)
            max_ = val;
        if(maxndiscrete>0)
        {
            if(int(counts.size())<maxndiscrete)
                counts[val]++;
            else // reached maxndiscrete. Stop counting and reset counts.
            {
                maxndiscrete = -1;
                counts.clear();
            }
        }
    }
}

void VMFieldStat::write(PStream& out) const
{
    out << nmissing_ << ' '
        << nnonmissing_ << ' '
        << npositive_ << ' '
        << nnegative_ << ' '
        << sum_ << ' '
        << sumsquare_ << ' '
        << min_ << ' '
        << max_ << "    ";

    out << counts.size() << "  ";

    map<real,int>::const_iterator it = counts.begin();
    map<real,int>::const_iterator countsend = counts.end();
    while(it!=countsend)
    {
        out << it->first << ' ' << it->second << "  ";
        ++it;
    }
}

void VMFieldStat::read(PStream& in)
{
    in >> nmissing_ >> nnonmissing_ >> npositive_ >> nnegative_
       >> sum_ >> sumsquare_ >> min_ >> max_ ;

    int ndiscrete;
    real value;
    int count;
    counts.clear();
    in >> ndiscrete;
    for(int k=0; k<ndiscrete; k++)
    {
        in >> value >> count;
        counts[value] = count;
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
