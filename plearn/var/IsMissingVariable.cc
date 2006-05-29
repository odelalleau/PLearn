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

#include "IsMissingVariable.h"

namespace PLearn {
using namespace std;


/** IsMissingVariable **/

PLEARN_IMPLEMENT_OBJECT(IsMissingVariable,
                        "ONE LINE DESCR",
                        "NO HELP");

IsMissingVariable::IsMissingVariable(Variable* input1, bool parall, bool the_set_parallel_missing_output, Vec the_parallel_missing_outputs)
    : inherited(input1, parall?input1->length():1, parall?input1->width():1), parallel(parall), set_parallel_missing_output(the_set_parallel_missing_output), parallel_missing_outputs(the_parallel_missing_outputs)
{}

void
IsMissingVariable::declareOptions(OptionList &ol)
{
    declareOption(ol, "parallel", &IsMissingVariable::parallel, OptionBase::buildoption, "");
    inherited::declareOptions(ol);
    declareOption(ol, "parallel_missing_outputs", &IsMissingVariable::parallel_missing_outputs, OptionBase::buildoption, "");
    declareOption(ol, "set_parallel_missing_output", &IsMissingVariable::set_parallel_missing_output, OptionBase::buildoption, "");
    inherited::declareOptions(ol);
}

void IsMissingVariable::build()
{
    inherited::build();
    build_();
}

void IsMissingVariable::build_()
{
    if(parallel_missing_outputs.length() != input->size())
        PLERROR("In IsMissingVariable::build_(): parallel_missing_outputs' size is different from input size.");
}

void IsMissingVariable::recomputeSize(int& l, int& w) const
{
    if (input) {
        l = parallel ? input->length() : 1;
        w = parallel ? input->width() : 1;
    } else
        l = w = 0;
}

void IsMissingVariable::fprop()
{
    if (parallel)
    {
        if(!set_parallel_missing_output)
            for(int i=0; i<nelems(); i++)
                valuedata[i] = finite(input->valuedata[i]);
        else
            for(int i=0; i<nelems(); i++)
            {
                if(!finite(input->valuedata[i]))
                    valuedata[i] = parallel_missing_outputs[i];
                else
                    valuedata[i] = input->valuedata[i];
            }
    }
    else
    {
        bool nomissing=true;
        for(int i=0; i<nelems(); i++)
            nomissing = nomissing && finite(input->valuedata[i]);
        valuedata[0] = !nomissing;
    }
}


// not really differentiable (zero gradient almost everywhere)
void IsMissingVariable::bprop() {}

void IsMissingVariable::symbolicBprop() {}

void IsMissingVariable::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(parallel_missing_outputs, copies);
    //PLERROR("IsMissingVariable::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
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
