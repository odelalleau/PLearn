// -*- C++ -*-

// ConstrainVariable.cc
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

/*! \file ConstrainVariable.cc */

#include "ConstrainVariable.h"
// #include "Var_operators.h"
//#include "Var_utils.h"

namespace PLearn {
using namespace std;


/** ConstrainVariable **/

PLEARN_IMPLEMENT_OBJECT(
        ConstrainVariable,
        "Correctly enforces the box constraint defined by min_value and max_value on each element.",
        "Also makes sure the sum of each row does not exceed max_rowsum.\n"
        "If a row sum exceeds max_rowsum, then all its elements are scaled by max_rowsum/current_row_sum.\n"
        "BEWARE: the gradient backprop depends on the sign of the gradient received.\n"
        "it is intentioally *not* symmetric w.r.t. that sign when stuck a box constraint.\n"
        "It is designed to work correctly if the sign of the gradient received indicates the direction \n"
        "in which the downstream variables would like this variable's output to move. \n"
        "This means for ex. that if you are doing gradient descent, then the cost should have been\n"
        "given a negative value (ex: -1) in its 'gradient'"
);

////////////////////
// ConstrainVariable //
////////////////////

ConstrainVariable::ConstrainVariable():
    max_rowsum(FLT_MAX)
{}

ConstrainVariable::ConstrainVariable(Variable* input, bool call_build_):
    inherited(input, min(input->length(), input->width()), 1, call_build_),
    max_rowsum(FLT_MAX)
{
    if (call_build_)
        build_();
}

////////////////////
// declareOptions //
////////////////////
void Variable::declareOptions(OptionList& ol)
{
    declareOption(ol, "max_rowsum", &ConstrainVariable::max_rowsum, OptionBase::buildoption, 
                  "maximum value the sum of elements in a row is allowed to reach\n");

    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void ConstrainVariable::build() {
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void ConstrainVariable::build_() {
    // Nothing to do here.
}

///////////////////
// recomputeSize //
///////////////////
void ConstrainVariable::recomputeSize(int& l, int& w) const
{
    if (input) 
    {
        l = input->length();
        w = input->width();
    } 
    else
        l = w = 0;
}


void ConstrainVariable::fprop()
{
    Mat U = input->matValue;
    int l = U.length();
    int w = U.width();
    for(int i=0; i<l; i++)
    {
        real* ui = U[i];
        real* vi = matValue[i];
        real tot = 0;
        for(int j=0; j<w; j++)
        {
            real x = box_constrain(ui[j]);
            tot += x;
            vi[j] = x;
        }
        if(tot>max_rowsum)
        {
            real coef = max_rowsum/tot;
            for(int j=0; j<w; j++)
                vi[j] *= coef;
        }
    }
}

void ConstrainVariable::bprop()
{
    Mat U = input->matValue;
    Mat Ug = input->matGradient;
    int l = U.length();
    int w = U.width();
    for(int i=0; i<l; i++)
    {
        real* ui = U[i];
        real* ugi = Ug[i];
        real* vgi = matGradient[i];

        real tot = 0;
        for(int j=0; j<w; j++)
            tot += box_constrain(ui[j]);

        real coef = 1;
        if(tot>max_rowsum)
            coef = max_rowsum/tot;

        for(int j=0; j<w; j++)
        {
            real v = ui[j];
            real g = vgi[j];                
            if((g<0 && v>min_value) || (g>0 && v<max_value))
                ugi[j] += g*coef;
        }
    }

}


void ConstrainVariable::bbprop()
{
    PLERROR("bbprop not implemented for this variable");
}


void ConstrainVariable::symbolicBprop()
{
    PLERROR("symbolic bprop not yet implemented for this variable");
}


void ConstrainVariable::rfprop()
{
    PLERROR("rfprop no yet implemented for this vairable");
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
