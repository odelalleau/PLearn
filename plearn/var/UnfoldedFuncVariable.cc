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

PLEARN_IMPLEMENT_OBJECT(
        UnfoldedFuncVariable,
        "Computes the output of a Func over all elements on an input matrix.",
        "By default the function 'f' is applied over all rows of the input\n"
        "provided in 'input_matrix', but it may be applied on columns\n"
        "instead using the 'transpose' option.\n"
        "A separate propagation path is created (using the Func as a\n"
        "template) that maps each input to the corresponding output.\n"
        "The parents of this variable include the non-input parents of 'f'.");

//////////////////////////
// UnfoldedFuncVariable //
//////////////////////////
UnfoldedFuncVariable::UnfoldedFuncVariable():
    transpose(false)
{}

UnfoldedFuncVariable::UnfoldedFuncVariable(
        Var inputmatrix, Func the_f, bool the_transpose,
        Var bagsize, bool call_build_):
    inherited(VarArray(),
            the_transpose ? the_f->outputs[0]->length()
                                                * the_f->outputs[0]->width()
                          : inputmatrix->length(),
            the_transpose ? inputmatrix->width()
                          : the_f->outputs[0]->length()
                                                * the_f->outputs[0]->width(),
            call_build_),
      input_matrix(inputmatrix), 
      bag_size(bagsize),
      f(the_f),
      transpose(the_transpose)
{
    if (call_build_)
        build_();
}

///////////
// build //
///////////
void UnfoldedFuncVariable::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void UnfoldedFuncVariable::build_()
{
    if (f) {
        VarArray f_parents = nonInputParentsOfPath(f->inputs, f->outputs);
        varray.resize(f_parents.length() + 2);
        varray << (f_parents & input_matrix & bag_size);

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
        inherited::build(); // Re-build since varray has changed.
    }

    if (bag_size)
        PLASSERT( bag_size->isScalar() );
}

////////////////////
// declareOptions //
////////////////////
void UnfoldedFuncVariable::declareOptions(OptionList& ol)
{
    declareOption(ol, "f", &UnfoldedFuncVariable::f, OptionBase::buildoption, 
                  "    Func that is replicated for each element of the 'bag' taken from the VMat.");

    declareOption(ol, "input_matrix", &UnfoldedFuncVariable::input_matrix, OptionBase::buildoption, 
        "Var containing the data: multiple consecutive rows form one bag.");

    declareOption(ol, "bag_size", &UnfoldedFuncVariable::bag_size,
                  OptionBase::buildoption, 
        "Optional Var that contains the size of the bag being presented.\n"
        "If provided, then only the corresponding number of function values\n"
        "will be computed, while the rest of the output data matrix will be\n"
        "left untouched.");

    declareOption(ol, "transpose", &UnfoldedFuncVariable::transpose, OptionBase::buildoption, 
                  "    If set to 1, then instead puts in the columns of the output matrix the values\n"
                  "    of f at the columns of the input matrix.");

    inherited::declareOptions(ol);

    redeclareOption(ol, "varray", &UnfoldedFuncVariable::varray,
                    OptionBase::nosave,
            "This option is set at build time from other options.");
}

///////////////////
// recomputeSize //
///////////////////
void UnfoldedFuncVariable::recomputeSize(int& l, int& w) const
{
    if (f && f->outputs.size() > 0) {
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

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void UnfoldedFuncVariable::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(input_matrix, copies);
    deepCopyField(bag_size,     copies);
    deepCopyField(f,            copies);
    deepCopyField(inputs,       copies);
    deepCopyField(outputs,      copies);
    deepCopyField(f_paths,      copies);
}

///////////
// fprop //
///////////
void UnfoldedFuncVariable::fprop()
{
    int n_unfold = bag_size ? int(round(bag_size->value[0]))
                            : transpose ? input_matrix->width()
                                        : input_matrix->length();
    PLASSERT( !bag_size || is_equal(bag_size->value[0],
                                    round(bag_size->value[0])) );
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

///////////
// bprop //
///////////
void UnfoldedFuncVariable::bprop()
{ 
    int n_unfold = bag_size ? int(round(bag_size->value[0]))
                            : transpose ? input_matrix->width()
                                        : input_matrix->length();
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

///////////////
// printInfo //
///////////////
void UnfoldedFuncVariable::printInfo(bool print_gradient)
{
    int n_unfold = bag_size ? int(round(bag_size->value[0]))
                            : transpose ? input_matrix->width()
                                        : input_matrix->length();
    for (int i=0;i<n_unfold;i++)
        f_paths[i].printInfo(print_gradient);
    pout << info() << " : " << getName() << "[" << (void*)this << "]" 
         << "(input_matrix=" << (void*)input_matrix << " ";
    for(int i=0; i<n_unfold; i++)
        pout << (void*)outputs[i] << " ";
    pout << ") = " << value;
    if (print_gradient)
        pout << " gradient=" << gradient;
    pout << endl; 
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
