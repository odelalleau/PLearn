// -*- C++ -*-

// PlusManyVariable.cc
//
// Copyright (C) 2008 Olivier Delalleau
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

// Authors: Olivier Delalleau

/*! \file PlusManyVariable.cc */


#include "PlusManyVariable.h"

namespace PLearn {
using namespace std;

/** PlusManyVariable **/

PLEARN_IMPLEMENT_OBJECT(
    PlusManyVariable,
    "Sum of an arbitrary number of variables (that must have same sizes).",
    ""
);

//////////////////////
// PlusManyVariable //
//////////////////////
PlusManyVariable::PlusManyVariable()
{}

PlusManyVariable::PlusManyVariable(const VarArray& the_array,
                                   bool call_build_):
    inherited(the_array, the_array[0]->length(), the_array[0]->width(),
              call_build_)
{
    if (call_build_)
        build_();
}

///////////////////
// recomputeSize //
///////////////////
void PlusManyVariable::recomputeSize(int& l, int& w) const
{
    if (varray.length() > 0) {
        l = varray[0]->length();
        w = varray[0]->width();
    } else
        l = w = 0;
}

///////////
// fprop //
///////////
void PlusManyVariable::fprop()
{
    fillValue(0);
    int n = nelems();
    for (int i = 0; i < varray.length(); i++) {
        real* vi = varray[i]->valuedata;
        for (int k=0; k<n; k++)
            valuedata[k] += vi[k];
    }
}

///////////
// bprop //
///////////
void PlusManyVariable::bprop()
{
    int n = nelems();
    for (int i = 0; i < varray.length(); i++) {
        real* vi = varray[i]->gradientdata;
        for (int k=0; k<n; k++)
            vi[k] += gradientdata[k];
    }
}

///////////
// build //
///////////
void PlusManyVariable::build()
{
    inherited::build();
    build_();
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void PlusManyVariable::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
}

////////////////////
// declareOptions //
////////////////////
void PlusManyVariable::declareOptions(OptionList& ol)
{
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void PlusManyVariable::build_()
{
    // Check sizes are coherent.
    if (varray.length() >= 2) {
        int l = varray[0]->length();
        int w = varray[1]->width();
        for (int i = 1; i < varray.length(); i++)
            if (varray[i]->length() != l || varray[i]->width() != w)
                PLERROR("In PlusManyVariable::build_ - All source variables "
                        "must have the same size");
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
