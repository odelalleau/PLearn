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

#include "IfThenElseVariable.h"
#include "Var_utils.h"
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;


/** IfThenElseVariable **/

PLEARN_IMPLEMENT_OBJECT(
    IfThenElseVariable,
    "Variable that represents the element-wise IF-THEN-ELSE",
    "Variable that represents the element-wise IF-THEN-ELSE:\n"
    "- The first parent is the test (0 or different from 0)\n"
    "- The second parent is the value returned when the test is !=0,\n"
    "- The third parent is the value returned when the test is ==0,\n");

IfThenElseVariable::IfThenElseVariable(Var IfVar, Var ThenVar, Var ElseVar)
    : inherited(IfVar & ThenVar & (VarArray)ElseVar,ThenVar->length(), ThenVar->width())
{
    build_();
}

void
IfThenElseVariable::build()
{
    inherited::build();
    build_();
}

void
IfThenElseVariable::build_()
{
    if (varray.size()) {
        if (varray[1]->length() != varray[2]->length() || varray[1]->width() != varray[2]->width())
            PLERROR("In IfThenElseVariable: ElseVar and ThenVar must have the same size");
        if (!varray[0]->isScalar() && (varray[0]->length() != varray[1]->length() || varray[0]->width() != varray[1]->width()))
            PLERROR("In IfThenElseVariable: IfVar must either be a scalar or have the same size as ThenVar and ElseVar");
    }
}

void IfThenElseVariable::recomputeSize(int& l, int& w) const
{
    if (varray.size()) {
        l = varray[1]->length();
        w = varray[1]->width();
    } else
        l = w = 0;
}

void IfThenElseVariable::fprop()
{
    if(If()->isScalar())
    {
        bool test = !fast_exact_is_equal(If()->valuedata[0], 0);
        if (test)
            value << Then()->value;
        else
            value << Else()->value;
    }
    else
    {
        real* ifv = If()->valuedata;
        real* thenv = Then()->valuedata;
        real* elsev = Else()->valuedata;
        for (int k=0;k<nelems();k++)
        {
            if ( !fast_exact_is_equal(ifv[k], 0) )
                valuedata[k]=thenv[k];
            else
                valuedata[k]=elsev[k];
        }
    }
}


void IfThenElseVariable::bprop()
{
    if(If()->isScalar())
    {
        if (!fast_exact_is_equal(If()->valuedata[0], 0))
            Then()->gradient += gradient;
        else
            Else()->gradient += gradient;
    }
    else
    {
        real* ifv = If()->valuedata;
        real* theng = Then()->gradientdata;
        real* elseg = Else()->gradientdata;
        for (int k=0;k<nelems();k++)
        {
            if ( !fast_exact_is_equal(ifv[k], 0)  )
                theng[k] += gradientdata[k];
            else
                elseg[k] += gradientdata[k];
        }
    }
}


void IfThenElseVariable::symbolicBprop()
{
    Var zero(length(), width());
    Then()->accg(ifThenElse(If(), g, zero));
    Else()->accg(ifThenElse(If(), zero, g));
}


void IfThenElseVariable::rfprop()
{
    if (rValue.length()==0) resizeRValue();
    if(If()->isScalar())
    {
        if (!fast_exact_is_equal(If()->valuedata[0], 0))
            rValue << Then()->rValue;
        else
            rValue << Else()->rValue;
    }
    else
    {
        real* ifv = If()->valuedata;
        real* rthenv = Then()->rvaluedata;
        real* relsev = Else()->rvaluedata;
        for (int k=0;k<nelems();k++)
        {
            if ( !fast_exact_is_equal(ifv[k], 0) )
                rvaluedata[k]=rthenv[k];
            else
                rvaluedata[k]=relsev[k];
        }
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
