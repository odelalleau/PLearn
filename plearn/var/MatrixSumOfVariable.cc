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

#include "MatrixSumOfVariable.h"

namespace PLearn {
using namespace std;


/** MatrixSumOfVariable **/

PLEARN_IMPLEMENT_OBJECT(MatrixSumOfVariable,
                        "ONE LINE DESCR",
                        "NO HELP");

MatrixSumOfVariable::MatrixSumOfVariable(VMat the_distr, Func the_f, int the_nsamples, int the_input_size)
    : inherited(nonInputParentsOfPath(the_f->inputs,the_f->outputs), the_f->outputs[0]->length(), the_f->outputs[0]->width()),
      distr(the_distr), f(the_f), nsamples(the_nsamples), input_size(the_input_size)
    //, curpos(0), input_value(the_distr->width()*nsamples), input_gradient(the_distr->width()*nsamples), output_value(the_f->outputs[0]->size())
{
    build_();
}

void
MatrixSumOfVariable::build()
{
    inherited::build();
    build_();
}

void
MatrixSumOfVariable::build_()
{
    curpos = 0;
    if (f && distr) {
        if(f->outputs.size()!=1)
            PLERROR("In MatrixSumOfVariable: function must have a single variable output (maybe you can vconcat the vars into a single one prior to calling sumOf, if this is really what you want)");

        input_value.resize(distr->width() * nsamples);
        input_gradient.resize(distr->width() * nsamples);
        output_value.resize(f->outputs[0]->size());

        if(nsamples == -1)
            nsamples = distr->length();
        f->inputs.setDontBpropHere(true);
    }
}

void
MatrixSumOfVariable::declareOptions(OptionList &ol)
{
    declareOption(ol, "distr", &MatrixSumOfVariable::distr, OptionBase::buildoption, "");
    declareOption(ol, "f", &MatrixSumOfVariable::f, OptionBase::buildoption, "");
    declareOption(ol, "nsamples", &MatrixSumOfVariable::nsamples, OptionBase::buildoption, "");
    declareOption(ol, "input_size", &MatrixSumOfVariable::input_size, OptionBase::buildoption, "");
    inherited::declareOptions(ol);
}

void MatrixSumOfVariable::recomputeSize(int& l, int& w) const
{
    if (f) {
        l = f->outputs[0]->length();
        w = f->outputs[0]->width();
    } else
        l = w = 0;
}


void MatrixSumOfVariable::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(distr, copies);
    deepCopyField(f, copies);
}


void MatrixSumOfVariable::fprop()
{
    Vec oneInput_value(distr->width());
    f->recomputeParents();

    int inputpos=0;
    int targetpos=nsamples*input_size;
    for (int i=0; i<nsamples; i++)
    {
        distr->getRow(curpos, oneInput_value);
        for (int j=0; j<input_size; j++,inputpos++)
            input_value[inputpos]=oneInput_value[j];
        for (int j=input_size; j<distr.width(); j++,targetpos++)
            input_value[targetpos] = oneInput_value[j];
        if(++curpos==distr.length())
            curpos=0;
    }
    f->fprop(input_value, value);
}


void MatrixSumOfVariable::bprop()
{ fbprop(); }


void MatrixSumOfVariable::fbprop()
{
    Vec oneInput_value(distr->width());
    f->recomputeParents();
  
    int inputpos=0;
    int targetpos=nsamples*input_size;
    for (int i=0; i<nsamples; i++)
    {
        distr->getRow(curpos, oneInput_value);
        for (int j=0; j<input_size; j++,inputpos++)
            input_value[inputpos]=oneInput_value[j];
        for (int j=input_size; j<distr.width(); j++,targetpos++)
            input_value[targetpos] = oneInput_value[j];
        if(++curpos==distr.length())
            curpos=0;
    }
    f->fbprop(input_value, value, input_gradient, gradient);
}


void MatrixSumOfVariable::symbolicBprop()
{
    PLERROR("MatrixSumOfVariable::symbolicBprop not implemented.");
}


void MatrixSumOfVariable::rfprop()
{
    PLERROR("MatrixSumOfVariable::rfprop not implemented.");
}


void MatrixSumOfVariable::printInfo(bool print_gradient)
{
    PLERROR("MatrixSumOfVariable::printInfo not implemented.");
    Vec input_value(distr->width());
    Vec input_gradient(distr->width());
    Vec output_value(nelems());

    f->recomputeParents();
    value.clear();

    for(int i=0; i<nsamples; i++)
    {
        distr->getRow(curpos++,input_value);
        if (print_gradient)
            f->fbprop(input_value, output_value, input_gradient, gradient);
        else
            f->fprop(input_value, output_value);
        value += output_value;
        if(++curpos>=distr->length())
            curpos = 0;
        f->fproppath.printInfo(print_gradient);
    }
    cout << info() << " : " << getName() << " = " << value;
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
