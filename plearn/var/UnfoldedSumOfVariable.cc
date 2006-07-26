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

#include "UnfoldedSumOfVariable.h"
#include <plearn/display/DisplayUtils.h>

namespace PLearn {
using namespace std;

/** UnfoldedSumOfVariable **/

PLEARN_IMPLEMENT_OBJECT(UnfoldedSumOfVariable, "Variable that sums the value of a Func evaluated on each row of a matrix.\n", 
                        "However, unlike the SumOfVariable, it does so by unfolding the Func (up to given maximum number\n"
                        "of times 'max_bag_size'), and it allows that number to be variable. Each of the unfolded Func\n"
                        "is applied on a different row of the input matrix. The number of rows to sum is specified on the\n"
                        "fly by another input, the bag_size.\n");

UnfoldedSumOfVariable::UnfoldedSumOfVariable(Var inputmatrix, Var bagsize, Func the_f, int max_bagsize)
    : inherited(nonInputParentsOfPath(the_f->inputs,the_f->outputs) & inputmatrix & bagsize, 
                the_f->outputs[0]->length(), 
                the_f->outputs[0]->width()),
      input_matrix(inputmatrix), bag_size(bagsize),
      f(the_f), max_bag_size(max_bagsize)
{
    build();
}

void UnfoldedSumOfVariable::build()
{
    inherited::build();
    build_();
}

void UnfoldedSumOfVariable::build_()
{
    if (f) {
        if(f->outputs.size()!=1)
            PLERROR("In UnfoldedSumOfVariable: function must have a single variable output (maybe you can vconcat the vars into a single one prior to calling sumOf, if this is really what you want)");
        f->inputs.setDontBpropHere(true);
        inputs.resize(max_bag_size);
        outputs.resize(max_bag_size);
        f_paths.resize(max_bag_size);
        for (int i=0;i<max_bag_size;i++)
        {
            inputs[i].resize(f->inputs.size());
            for (int j = 0; j < f->inputs.size(); j++) {
                inputs[i][j] = Var(f->inputs[j]->length(), f->inputs[j]->width());
            }
            outputs[i] = f(inputs[i])[0];
            f_paths[i] = propagationPath(inputs[i],outputs[i]);
        }
    }
}

void UnfoldedSumOfVariable::declareOptions(OptionList& ol)
{
    declareOption(ol, "f", &UnfoldedSumOfVariable::f, OptionBase::buildoption, 
                  "    Func that is replicated for each element of the 'bag' taken from the VMat.");

    declareOption(ol, "input_matrix", &UnfoldedSumOfVariable::input_matrix, OptionBase::buildoption, 
                  "    Var that contains the data, with multiple consecutive rows forming one bag.\n"
                  "    The last row of a bag has a non-missing target value.\n");

    declareOption(ol, "bag_size", &UnfoldedSumOfVariable::bag_size, OptionBase::buildoption, 
                  "    Var that gives the size of the bag (number of rows of input_matrix to consider).\n");

    declareOption(ol, "max_bag_size", &UnfoldedSumOfVariable::max_bag_size, OptionBase::buildoption, 
                  "    maximum number of examples in a bag (more than that in input_matrix will trigger a run-time error).\n");

    inherited::declareOptions(ol);
}

void UnfoldedSumOfVariable::recomputeSize(int& l, int& w) const
{
    if (f && f->outputs.size()) {
        l = f->outputs[0]->length();
        w = f->outputs[0]->width();
    } else
        l = w = 0;
}

//! To use varDeepCopyField.
#ifdef __INTEL_COMPILER
#pragma warning(disable:1419)  // Get rid of compiler warning.
#endif
extern void varDeepCopyField(Var& field, CopiesMap& copies);
#ifdef __INTEL_COMPILER
#pragma warning(default:1419)
#endif


void UnfoldedSumOfVariable::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    NaryVariable::makeDeepCopyFromShallowCopy(copies);
    varDeepCopyField(input_matrix, copies);
    varDeepCopyField(bag_size, copies);
    deepCopyField(f, copies);
    deepCopyField(inputs, copies);
    deepCopyField(outputs, copies);
    deepCopyField(f_paths, copies);
}


void UnfoldedSumOfVariable::fprop()
{
    value.clear();
    int bagsize = (int)bag_size->valuedata[0];
    if (bagsize>max_bag_size)
        PLERROR("UnfoldedSumOfVariable: bag size=%d > expected max. bag size(%d)",
                bagsize,max_bag_size);
    for (int i=0;i<bagsize;i++)
    {
        inputs[i] << input_matrix->matValue(i);
        f_paths[i].fprop();
        value += outputs[i]->value;
    }
}


void UnfoldedSumOfVariable::bprop()
{ 
    int bagsize = (int)bag_size->valuedata[0];
    for (int i=0;i<bagsize;i++)
    {
        f_paths[i].clearGradient();
        outputs[i]->gradient << gradient;
        f_paths[i].bprop();
    }
}


void UnfoldedSumOfVariable::printInfo(bool print_gradient)
{
    for (int i=0;i<max_bag_size;i++)
        f_paths[i].printInfo(print_gradient);
    cout << info() << " : " << getName() << "[" << (void*)this << "]" 
         << "(input_matrix=" << (void*)input_matrix << ", bag_size=" 
         << (void*)bag_size << ", ";
    for(int i=0; i<max_bag_size; i++) cout << (void*)outputs[i] << " ";
    cout << ") = " << value;
    if (print_gradient) cout << " gradient=" << gradient;
    cout << endl; 
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
