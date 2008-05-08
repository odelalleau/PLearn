// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2001-2002 Nicolas Chapados, Ichiro Takeuchi, Jean-Sebastien Senecal
// Copyright (C) 2002 Xiangdong Wang, Christian Dorion

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

#include "ConcatColumnsVariable.h"
#include "SubMatVariable.h"

namespace PLearn {
using namespace std;


/** ConcatColumnsVariable **/

PLEARN_IMPLEMENT_OBJECT(
        ConcatColumnsVariable,
        "Concatenation of the columns of several variables",
        ""
);

///////////////////////////
// ConcatColumnsVariable //
///////////////////////////
ConcatColumnsVariable::ConcatColumnsVariable(const VarArray& vararray,
                                             bool call_build_):
    inherited(vararray.nonNull(), vararray.maxLength(), vararray.sumOfWidths(),
              call_build_)
{
    if (call_build_)
        build_();
}

///////////
// build //
///////////
void ConcatColumnsVariable::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void ConcatColumnsVariable::build_()
{
    if (varray->length()) {
        int l = varray[0]->length();
        for (int i = 1; i < varray.size(); i++)
            if (l != varray[i]->length())
                PLERROR("ConcatColumnsVariable: all non-null variables must have the same length (%d != %d)", l, varray[i]->length());
    }
}

void ConcatColumnsVariable::recomputeSize(int& l, int& w) const
{
    if (varray) {
        l = varray.maxLength();
        w = varray.sumOfWidths();
    } else
        l = w = 0;
}

void ConcatColumnsVariable::fprop()
{
    int n_rows = matValue.length();
    int m_start = 0;
    int mod = matValue.mod();
    for (int m=0;m<varray.size();m++)
    {
        real* mp = varray[m]->valuedata;
        int n_cols = varray[m]->matValue.width();
        real* p = &valuedata[m_start];
        for (int i=0;i<n_rows;i++,p+=mod)
            for (int j=0;j<n_cols;j++,mp++)
                p[j] = *mp;
        m_start+=n_cols;
    }
}


void ConcatColumnsVariable::bprop()
{
    int n_rows = matValue.length();
    int m_start = 0;
    int mod = matValue.mod();
    for (int m=0;m<varray.size();m++)
    {
        real* mp = varray[m]->gradientdata;
        int n_cols = varray[m]->matGradient.width();
        real* p = &gradientdata[m_start];
        for (int i=0;i<n_rows;i++,p+=mod)
            for (int j=0;j<n_cols;j++,mp++)
                *mp += p[j];
        m_start+=n_cols;
    }
}


void ConcatColumnsVariable::symbolicBprop()
{
    int k=0;
    for (int n=0; n<varray.size(); n++) {
        Var vn = varray[n];
        vn->accg(new SubMatVariable(g, 0, k, length(), vn->width()));
        k += vn->width();
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
