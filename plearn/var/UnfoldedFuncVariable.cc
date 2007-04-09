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

#include "UnfoldedFuncVariable.h"
//#include "PLMPI.h"
//#include "DisplayUtils.h"

namespace PLearn {
using namespace std;



/** UnfoldedFuncVariable **/

PLEARN_IMPLEMENT_OBJECT(UnfoldedFuncVariable, "Variable that puts in the rows of its output matrix the value\n"
                        "of a Func evaluated on each row of an input matrix.\n",
                        "The input_matrix and output matrix have n_unfold rows. A separate propagation path\n"
                        "is created that maps (using the Func as a template) each input row to each output row.\n"
                        "The parents of this variable include the non-input parents of the Func.\n");

UnfoldedFuncVariable::UnfoldedFuncVariable()
    : transpose(0)
{}

UnfoldedFuncVariable::UnfoldedFuncVariable(Var inputmatrix, Func the_f, bool the_transpose)
    : inherited(nonInputParentsOfPath(the_f->inputs,the_f->outputs) & inputmatrix, 
                the_transpose ? the_f->outputs[0]->length()*the_f->outputs[0]->width() : inputmatrix->length(),
                the_transpose ? inputmatrix->width() : the_f->outputs[0]->length()*the_f->outputs[0]->width()),
      input_matrix(inputmatrix), 
      f(the_f),
      transpose(the_transpose)
{
    build();
}

void UnfoldedFuncVariable::build()
{
    inherited::build();
    build_();
}

void UnfoldedFuncVariable::build_()
{
    if (f) {
        if(f->outputs.size()!=1)
            PLERROR("In UnfoldedFuncVariable: function must have a single variable output (maybe you can vconcat the vars into a single one prior to calling sumOf, if this is really what you want)");
        f->inputs.setDontBpropHere(true);
        int n_unfold = transpose ? input_matrix->width() : input_matrix->length();
        inputs.resize(n_unfold);
        outputs.resize(n_unfold);
        f_paths.resize(n_unfold);
        for (int i=0;i<n_unfold;i++)
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

void UnfoldedFuncVariable::declareOptions(OptionList& ol)
{
    declareOption(ol, "f", &UnfoldedFuncVariable::f, OptionBase::buildoption, 
                  "    Func that is replicated for each element of the 'bag' taken from the VMat.");

    declareOption(ol, "input_matrix", &UnfoldedFuncVariable::input_matrix, OptionBase::buildoption, 
                  "    Var that contains the data, with multiple consecutive rows forming one bag.\n");

    declareOption(ol, "transpose", &UnfoldedFuncVariable::transpose, OptionBase::buildoption, 
                  "    If set to 1, then instead puts in the columns of the output matrix the values\n"
                  "    of f at the columns of the input matrix.");

    inherited::declareOptions(ol);
}


void UnfoldedFuncVariable::recomputeSize(int& l, int& w) const
{
    if (f && f->outputs.size()) {
        w = f->outputs[0]->length()*f->outputs[0]->width();
        if (transpose) {
            l = w;
            w = input_matrix->width();
        } else {
            l = input_matrix->length();
        }
    } else
        l = w = 0;
}


void UnfoldedFuncVariable::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(input_matrix, copies);
    deepCopyField(f, copies);
    deepCopyField(inputs, copies);
    deepCopyField(outputs, copies);
    deepCopyField(f_paths, copies);
}


void UnfoldedFuncVariable::fprop()
{
    int n_unfold = transpose ? input_matrix->width() : input_matrix->length();
    for (int i=0;i<n_unfold;i++) {
        if (transpose) {
            Vec tmp = input_matrix->matValue.column(i).toVecCopy(); // TODO something more efficient
            inputs[i] << tmp;
        } else {
            inputs[i] << input_matrix->matValue(i);
        }
        f_paths[i].fprop();
        if (transpose) {
            matValue.column(i) << outputs[i]->value;
        } else {
            matValue(i) << outputs[i]->value;
        }
    }
}


void UnfoldedFuncVariable::bprop()
{ 
    int n_unfold = transpose ? input_matrix->width() : input_matrix->length();
    for (int i=0;i<n_unfold;i++)
    {
        f_paths[i].clearGradient();
        if (transpose) {
            Vec tmp = matGradient.column(i).toVecCopy(); // TODO more efficient + check while it compiled without tmp = toVecCopy
            outputs[i]->gradient << tmp;
        } else {
            outputs[i]->gradient << matGradient(i);
        }
        f_paths[i].bprop();
    }
}


void UnfoldedFuncVariable::printInfo(bool print_gradient)
{
    int n_unfold = transpose ? input_matrix->width() : input_matrix->length();
    for (int i=0;i<n_unfold;i++)
        f_paths[i].printInfo(print_gradient);
    cout << info() << " : " << getName() << "[" << (void*)this << "]" 
         << "(input_matrix=" << (void*)input_matrix << " ";
    for(int i=0; i<n_unfold; i++) cout << (void*)outputs[i] << " ";
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
