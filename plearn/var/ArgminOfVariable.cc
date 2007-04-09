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

#include "ArgminOfVariable.h"

namespace PLearn {
using namespace std;


/** ArgminOfVariable **/
/* returns the value of v within the_values_of_v that gives the lowest
   value of a scalar expression (which may depend on inputs). */
ArgminOfVariable::ArgminOfVariable(Variable* the_v,
                                   Variable* the_expression,
                                   Variable* the_values_of_v,
                                   const VarArray& the_inputs)
    :NaryVariable(the_inputs,1,1), inputs(the_inputs), expression(the_expression),
     values_of_v(the_values_of_v), v(the_v)
{
    if (!v->isScalar() || !values_of_v->isVec())
        PLERROR("ArgminOfVariable currently implemented only for a scalar v and a vector values_of_v");
    vv_path = propagationPath(inputs,values_of_v);
    e_path = propagationPath(inputs& (VarArray)v, expression);
    v_path = propagationPath(v, expression);
}


PLEARN_IMPLEMENT_OBJECT(ArgminOfVariable, "ONE LINE DESCR", "NO HELP");

void ArgminOfVariable::recomputeSize(int& l, int& w) const
{ l=1; w=1; }


void ArgminOfVariable::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(inputs, copies);
    deepCopyField(expression, copies);
    deepCopyField(values_of_v, copies);
    deepCopyField(v, copies);
    deepCopyField(vv_path, copies);
    deepCopyField(e_path, copies);
    deepCopyField(v_path, copies);
}



void ArgminOfVariable::fprop()
{
    vv_path.fprop(); // compute influence of inputs on values_of_v
    real min_value_of_expression = FLT_MAX;
    real argmin_value_of_v = values_of_v->value[0];
    for (int i=0;i<values_of_v->nelems();i++)
    {
        v->value[0] = values_of_v->value[i];
        if (i==0)
            e_path.fprop(); // compute influence of v and inputs on expression
        else
            v_path.fprop(); // otherwise, keep influence of inputs fixed
        real e = expression->value[0];
        if (e<min_value_of_expression)
        {
            min_value_of_expression = e;
            argmin_value_of_v = v->value[0];
            index_of_argmin = i;
        }
    }
    value[0] = argmin_value_of_v;
}


void ArgminOfVariable::bprop()
{
    vv_path.clearGradient();
    values_of_v->gradientdata[index_of_argmin] = gradientdata[0];
    vv_path.bprop();
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
