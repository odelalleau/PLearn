
// -*- C++ -*-

// RegularGridVMatrix.cc
//
// Copyright (C) 2003  Pascal Vincent
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

/*! \file RegularGridVMatrix.cc */
#include "RegularGridVMatrix.h"
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;


RegularGridVMatrix::RegularGridVMatrix()
{
    // ...

    // ### You may or may not want to call build_() to finish building the object
    // build_();
}

RegularGridVMatrix::RegularGridVMatrix(TVec<int> the_dimensions, TVec< pair<real,real> > the_range)
    :dimensions(the_dimensions.copy()), range(the_range.copy())
{
    build_();
}


PLEARN_IMPLEMENT_OBJECT(RegularGridVMatrix, "ONE LINE DESCR",
                        "RegularGridVMatrix represents the list of coordinates along a regularly spaced grid.");

void RegularGridVMatrix::getNewRow(int i, const Vec& v) const
{
    int d = width();
    if(v.length()!=d)
        PLERROR("In RegularGridVMatrix::getNewRow, size of v (%d) differs from vmat width (%d)",v.length(), d);
    if(i<0 || i>=length())
        PLERROR("In RegularGridVMatrix::getNewRow, row %d out of range [0,%d]",i, length()-1);
    int idx = i;
    for(int k=d-1; k>=0; k--)
    {
        int idx_k = idx%dimensions[k];
        idx = idx/dimensions[k];
        v[k] = range[k].first + (range[k].second-range[k].first)/(dimensions[k]-1)*idx_k;
    }
}

void RegularGridVMatrix::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    // ### ex:
    // declareOption(ol, "myoption", &RegularGridVMatrix::myoption, OptionBase::buildoption,
    //               "Help text describing this option");
    // ...
    declareOption(ol, "dimensions", &RegularGridVMatrix::dimensions, OptionBase::buildoption,
                  "A vector of integers giving the number of sample coordinates\n"
                  "for each dimension of the grid. Ex for a 100x100 2D grid: [ 100 100 ]\n");
    declareOption(ol, "range", &RegularGridVMatrix::range, OptionBase::buildoption,
                  "A vector of low:high pairs with as many dimensions as the grid\n"
                  "ex for 2D: [ -10:10 -3:4 ] \n");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void RegularGridVMatrix::build_()
{
    width_ = dimensions.length();
    length_ = (width_ ? product(dimensions) : 0);
    if(inputsize_<0)
    {
        inputsize_ = width_;
        targetsize_ = 0;
        weightsize_ = 0;
    }
}

// ### Nothing to add here, simply calls build_
void RegularGridVMatrix::build()
{
    inherited::build();
    build_();
}

void RegularGridVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(dimensions, copies);
    deepCopyField(range, copies);
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
