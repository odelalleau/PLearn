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

#include "ExpVariable.h"
#include "LogAddVariable.h"
#include <plearn/math/pl_math.h>   //!< For logadd()
#include <plearn/math/TMat_maths.h>  //!< For apply()
#include "Var_operators.h"
//#include "Var_utils.h"

namespace PLearn {
using namespace std;



/** LogAddVariable **/


PLEARN_IMPLEMENT_OBJECT(LogAddVariable,
                        "output = log(exp(input1)+exp(input2)) but it is "
                        "computed in such a way as to preserve precision",
                        "NO HELP");

LogAddVariable::LogAddVariable(Variable* input1, Variable* input2)
    : inherited(input1, input2, input1->length(), input1->width())
{
    build_();
}

void
LogAddVariable::build()
{
    inherited::build();
    build_();
}

void
LogAddVariable::build_()
{
    if (input1 && input2) {
        if (input1->length() != input2->length()  ||  input1->width() != input2->width())
            PLERROR("PLogPVariable LogAddVariable input1 and input2 must have the same size");
    }
}

void LogAddVariable::recomputeSize(int& l, int& w) const
{
    if (input1) {
        l = input1->length();
        w = input1->width();
    } else
        l = w = 0;
}

void LogAddVariable::fprop()
{
    // Ugly hack to make it compile with ICC.
#ifdef __INTEL_COMPILER
    PLearn::apply(input1->value,input2->value,value, logadd_for_icc);
#else
    PLearn::apply(input1->value,input2->value,value, logadd);
#endif
}

void LogAddVariable::bprop()
{
    Vec grad1(nelems());
    grad1 = input1->value - value;
    apply(grad1, grad1, safeexp);
    input1->gradient += grad1%gradient;

    Vec grad2(nelems());
    grad2 = input2->value - value;
    apply(grad2, grad2, safeexp);
    input2->gradient += grad2%gradient;
}

void LogAddVariable::symbolicBprop()
{
    input1->accg(g * (exp(input1)/(exp(input1)+exp(input2))));
    input2->accg(g * (exp(input2)/(exp(input1)+exp(input2))));
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
