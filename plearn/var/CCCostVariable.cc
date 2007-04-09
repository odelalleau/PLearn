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

#include "CCCostVariable.h"
#include <plearn/sys/PLMPI.h>
#include <plearn/display/DisplayUtils.h>

namespace PLearn {
using namespace std;



/** CCCostVariable **/

PLEARN_IMPLEMENT_OBJECT(CCCostVariable,
                        "Variable that computes the (mean) correlation between the errors and the value of a candidat node for Cascade Correlation",
                        "NO HELP");

CCCostVariable::CCCostVariable(VMat the_distr, Func the_f_error, Func the_f_candidate)
    : inherited(nonInputParentsOfPath(the_f_candidate->inputs,the_f_candidate->outputs), 
                1,
                1),
      distr(the_distr), f_error(the_f_error), f_candidate(the_f_candidate),
      input_value(the_distr->width()),
      input_gradient(distr->inputsize()),
      error_output_value(the_f_error->outputs[0]->size()),
      candidate_output_value(the_f_candidate->outputs[0]->size()),
      error_correlations(the_f_error->outputs[0]->size()),
      adjusted_gradient(1)
{
    build_();
}

void
CCCostVariable::build()
{
    inherited::build();
    build_();
}

void
CCCostVariable::build_()
{
    if (f_error && f_candidate && distr) {
        mean_error.resize(f_error->outputs[0]->size());
        input_value.resize(distr->inputsize() + distr->targetsize() + distr->weightsize());
        input_gradient.resize(distr->inputsize());
        if(f_error->outputs.size() != 1)
            PLERROR("In CCCostVariable: error function must have a single variable output (maybe you can vconcat the vars into a single one prior to calling sumOf, if this is really what you want)");
      
        if(f_error->outputs[0].width() != 1)
            PLERROR("In CCCostVariable: the error function's output must be a column vector ");
        f_error->inputs.setDontBpropHere(true);
      
        if(f_candidate->outputs.size() != 1)
            PLERROR("In CCCostVariable: candidate node function must have a single variable output (maybe you can vconcat the vars into a single one prior to calling sumOf, if this is really what you want)");
      
        if(f_candidate->outputs[0].width() != 1 || f_candidate->outputs[0].length() != 1)
            PLERROR("In CCCostVariable: the candidate node function's output must be a column vector ");
    }
}

void
CCCostVariable::declareOptions(OptionList &ol)
{
    declareOption(ol, "distr", &CCCostVariable::distr, OptionBase::buildoption, "");
    declareOption(ol, "f_error", &CCCostVariable::f_error, OptionBase::buildoption, "");
    declareOption(ol, "f_candidate", &CCCostVariable::f_candidate, OptionBase::buildoption, "");
    
    inherited::declareOptions(ol);
}


void CCCostVariable::recomputeSize(int& l, int& w) const
{
    w = 1;
    l = 1;
}


void CCCostVariable::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(distr, copies);
    deepCopyField(f_error, copies);
    deepCopyField(f_candidate, copies);
}


void CCCostVariable::fprop()
{
    f_error->recomputeParents();
    f_candidate->recomputeParents();
    mean_error.clear();
    mean_candidate=0;

    // Compute the means of the candidate node and of the error for every output
    for(int i=0; i<distr->length(); i++)
    {
        input_value.resize(distr->width());
        distr->getRow(i, input_value);
        input_value.resize(distr->inputsize()+distr->targetsize()+distr->weightsize());
        f_error->fprop(input_value, error_output_value);
        mean_error += error_output_value;
        f_candidate->fprop(input_value.subVec(0,distr->inputsize()),candidate_output_value);
        mean_candidate += candidate_output_value[0];
    }

    mean_error /= distr->length();
    mean_candidate /= distr->length();

    value.clear();
    error_correlations.clear();
    for(int i=0; i<distr->length(); i++)
    {
        input_value.resize(distr->width());
        distr->getRow(i, input_value);
        input_value.resize(distr->inputsize()+distr->targetsize()+distr->weightsize());
        f_error->fprop(input_value, error_output_value);
        f_candidate->fprop(input_value.subVec(0,distr->inputsize()),candidate_output_value);
        for(int j=0; j<error_correlations.length(); j++)
            error_correlations[j] += (candidate_output_value[0]-mean_candidate)*(error_output_value[j]-mean_error[j]);
    }
    for(int j=0; j<error_correlations.length(); j++)
        value[0] -= abs(error_correlations[j]);
    value[0] /= distr->length();
}


void CCCostVariable::bprop()
{ fbprop(); }


void CCCostVariable::fbprop()
{
    fprop();

    for(int i=0; i<distr->length(); i++)
    {
        input_value.resize(distr->width());
        distr->getRow(i, input_value);
        input_value.resize(distr->inputsize()+distr->targetsize()+distr->weightsize());
        f_error->fprop(input_value, error_output_value);
        for(int j=0; j<error_correlations.length(); j++)
        {
      
            adjusted_gradient[0] = -1*gradient[0]*(error_output_value[j]-mean_error[j])
                * (error_correlations[j] > 0 ? 1 : -1)/distr->length();
            f_candidate->fbprop(input_value.subVec(0,distr->inputsize()),candidate_output_value,input_gradient, adjusted_gradient); // could be more efficient (do just a bprop: not implemented) 
        }
    }

}


void CCCostVariable::symbolicBprop()
{
    PLERROR("In CCCostVariable::symbolicBprop() : Not implemented");
}

void CCCostVariable::rfprop()
{
    PLERROR("In CCCostVariable::rfprop() : Not implemented");
}


void CCCostVariable::printInfo(bool print_gradient)
{
  
    fprop();

    for(int i=0; i<distr->length(); i++)
    {
        input_value.resize(distr->width());
        distr->getRow(i, input_value);
        input_value.resize(distr->inputsize()+distr->targetsize()+distr->weightsize());
        f_error->fprop(input_value, error_output_value);
        for(int j=0; j<error_correlations.length(); j++)
        {
      
            adjusted_gradient[0] = -1*gradient[0]*(error_output_value[j]-mean_error[j])
                * (error_correlations[j] > 0 ? 1 : -1);
            f_candidate->fbprop(input_value.subVec(0,distr->inputsize()),candidate_output_value,input_gradient, adjusted_gradient); // could be more efficient 
        }
        f_candidate->fproppath.printInfo(print_gradient);
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
