// -*- C++ -*-

// ConstrainedSourceVariable.cc
//
// Copyright (C) 2008 Pascal Vincent
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

// Authors: Pascal Vincent

/*! \file ConstrainedSourceVariable.cc */


#include "ConstrainedSourceVariable.h"
#include<plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;

/** ConstrainedSourceVariable **/

PLEARN_IMPLEMENT_OBJECT(
    ConstrainedSourceVariable,
    "SourceVariable that after each update, modifies values as needed to satisfy simple constraints",
    "The currently supported constraint is rows having norm 1.\n"
    "i.e. after each update rows are divided by their norm.\n");


void ConstrainedSourceVariable::satisfyConstraints()
{
    switch(constraint_mode)
    {
    case 0:
        for(int i=0; i<matValue.length(); i++)
            normalize(matValue(i), 2);
        break;
    default:
        PLERROR("Invalid constraint_mode %d",constraint_mode);
    }
}


void ConstrainedSourceVariable::declareOptions(OptionList& ol)
{
    declareOption(
        ol, "constraint_mode", &ConstrainedSourceVariable::constraint_mode, OptionBase::buildoption,
        "The constraint_mode: \n"
        "0: divide each row by its L2 norm after each update");
    inherited::declareOptions(ol);
}


void ConstrainedSourceVariable::build_()
{ }

void ConstrainedSourceVariable::build()
{
    inherited::build();
    build_();
}

void ConstrainedSourceVariable::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
}


//#####  update*  #############################################################

bool ConstrainedSourceVariable::update(real step_size, Vec direction_vec, real coeff, real b)
{
    bool ret = inherited::update(step_size, direction_vec, coeff, b);
    satisfyConstraints();
    return ret;
}

bool ConstrainedSourceVariable::update(Vec step_sizes, Vec direction_vec, real coeff, real b)
{
    bool ret = inherited::update(step_sizes, direction_vec, coeff, b);
    satisfyConstraints();
    return ret;
}

bool ConstrainedSourceVariable::update(real step_size, bool clear)
{
    bool ret = inherited::update(step_size, clear);
    satisfyConstraints();
    return ret;
}

bool ConstrainedSourceVariable::update(Vec new_value)
{
    bool ret = inherited::update(new_value);
    satisfyConstraints();
    return ret;
}

void ConstrainedSourceVariable::updateAndClear()
{
    inherited::updateAndClear();
    satisfyConstraints();
}

void ConstrainedSourceVariable::updateWithWeightDecay(real step_size, real weight_decay,
                                                 bool L1, bool clear)
{
    inherited::updateWithWeightDecay(step_size, weight_decay, L1, clear);
    satisfyConstraints();
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
