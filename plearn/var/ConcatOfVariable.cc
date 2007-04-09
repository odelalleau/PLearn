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

#include "ConcatOfVariable.h"

namespace PLearn {
using namespace std;



/** ConcatOfVariable **/
/* concatenates the results of each operation in the loop into the resulting variable */

PLEARN_IMPLEMENT_OBJECT(ConcatOfVariable, "Concatenates the results of each operation in the loop into the resulting variable", "NO HELP");

ConcatOfVariable::ConcatOfVariable(VMat the_distr, Func the_f)
    : inherited(nonInputParentsOfPath(the_f->inputs, the_f->outputs), 
                the_f->outputs[0]->length() * the_distr->length(), 
                the_f->outputs[0]->width()),
      distr(the_distr), f(the_f)
{
    build_();
}

/* Old constructor
   ConcatOfVariable::ConcatOfVariable(Variable* the_output, const VarArray& the_inputs, VMat the_distr, const VarArray& the_parameters)
   :NaryVariable(nonInputParentsOfPath(the_inputs,the_output), the_output->length()*the_distr->length(), the_output->width()), inputs(the_inputs), distr(the_distr), output(the_output), parameters(the_parameters)
   {
   full_fproppath = propagationPath(inputs&parameters, output);
   fproppath = propagationPath(inputs, output);
   bproppath = propagationPath(parameters, output);
   }
*/


void
ConcatOfVariable::declareOptions(OptionList &ol)
{
    declareOption(ol, "distr", &ConcatOfVariable::distr, OptionBase::buildoption, "");
    declareOption(ol, "f", &ConcatOfVariable::f, OptionBase::buildoption, "");
    inherited::declareOptions(ol);
}

void
ConcatOfVariable::build()
{
    inherited::build();
    build_();
}

void
ConcatOfVariable::build_()
{
    if (distr && f) {
        if(f->outputs.size()!=1)
            PLERROR("In ConcatOfVariable constructor: function must have a single output variable");

        input_value.resize(distr->width());
        input_gradient.resize(distr->width());
    }
}


void ConcatOfVariable::recomputeSize(int& l, int& w) const
{
    if (f && distr) {
        l = f->outputs[0]->length() * distr->length();
        w = f->outputs[0]->width();
    } else
        l = w = 0;
}


void ConcatOfVariable::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(distr, copies);
    deepCopyField(f, copies);
    deepCopyField(input_value, copies);
    deepCopyField(input_gradient, copies);
}



void ConcatOfVariable::fprop()
{
    f->recomputeParents();

    int pos = 0;
    int singleoutputsize = f->outputs[0]->nelems();
    for(int i=0; i<distr->length(); i++, pos+=singleoutputsize)
    {
        distr->getRow(i,input_value);
        f->fprop(input_value, value.subVec(pos,singleoutputsize));
    }
}


void ConcatOfVariable::bprop()
{
    fbprop();
}


void ConcatOfVariable::fbprop()
{
    f->recomputeParents();

    int pos = 0;
    int singleoutputsize = f->outputs[0]->nelems();
    for(int i=0; i<distr->length(); i++, pos+=singleoutputsize)
    {
        distr->getRow(i, input_value);
        f->fbprop(input_value, value.subVec(pos,singleoutputsize), 
                  input_gradient, gradient.subVec(pos,singleoutputsize));
        // We don't use the computed input_gradients, as the input is a dummy variable.
        // The gradients on other (non-input) variables which we are interested in, 
        // have been accumulated by the call as a side effect.
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
