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
 * $Id: PotentialsVariable.cc 3994 2005-08-25 13:35:03Z chapados $
 * This file is part of the PLearn library.
 ******************************************************* */

#include "PotentialsVariable.h"
#include <plearn/display/DisplayUtils.h>

namespace PLearn {
using namespace std;



/** PotentialsVariable **/

PLEARN_IMPLEMENT_OBJECT(PotentialsVariable,
                        "Variable that computes potentials for several targets using a single function",
                        "NO HELP");

PotentialsVariable::PotentialsVariable(Var the_input, Var the_comp_input, Var the_dp_target, Var the_target_dist_reps, Var the_output, VarArray the_params, VMat the_distr)
: inherited(the_input & the_comp_input & the_dp_target & the_params & the_target_dist_reps, 
                2, // Temporary value
                1 // idem
        ),
    distr(the_distr), 
    input(the_input),
    output(the_output),
    comp_input(the_comp_input),
    dp_target(the_dp_target),
    target_dist_reps(the_target_dist_reps),
    params(the_params)
{
    build_();
}

void
PotentialsVariable::build()
{
    inherited::build();
    build_();
}

void
PotentialsVariable::build_()
{
    if ( input && output && comp_input && dp_target && params.size() != 0) 
    {
        if(output->size() != 1) PLERROR("In PotentialsVariable::build_(): to_output function has output size different from 1");
        proppath = propagationPath(comp_input & dp_target & params, output);

        // Initialize temp_comps
        
        temp_comps.resize(1);
        temp_comps[0].resize(proppath.size());
        for(int i=0; i<proppath.size(); i++)
            temp_comps[0][i] = Var(proppath[i]->length(), proppath[i]->width());
        
    }
}

void
PotentialsVariable::declareOptions(OptionList &ol)
{
    declareOption(ol, "distr", &PotentialsVariable::distr, OptionBase::buildoption, "");
    declareOption(ol, "output", &PotentialsVariable::output, OptionBase::buildoption, "");
    declareOption(ol, "comp_input", &PotentialsVariable::comp_input, OptionBase::buildoption, "");
    declareOption(ol, "dp_target", &PotentialsVariable::dp_target, OptionBase::buildoption, "");
    declareOption(ol, "target_dist_reps", &PotentialsVariable::target_dist_reps, OptionBase::buildoption, "");
    declareOption(ol, "input", &PotentialsVariable::input, OptionBase::buildoption, "");
    declareOption(ol, "params", &PotentialsVariable::params, OptionBase::buildoption, "");
    inherited::declareOptions(ol);
}


void PotentialsVariable::recomputeSize(int& l, int& w) const
{
    if (distr) {
        l = distr->getValues(input->value,distr->inputsize()).size();
        w = 1;
    } else
        l = w = 0;
}


void PotentialsVariable::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    NaryVariable::makeDeepCopyFromShallowCopy(copies);
    
    varDeepCopyField(output, copies);
    varDeepCopyField(comp_input, copies);
    varDeepCopyField(dp_target, copies);
    varDeepCopyField(target_dist_reps, copies);
    varDeepCopyField(input, copies);

    deepCopyField(distr, copies);
    deepCopyField(temp_comps, copies);
    deepCopyField(values, copies);
    deepCopyField(proppath, copies);
    deepCopyField(params, copies);
}


void PotentialsVariable::fprop()
{
    //temp_comps.resize(length(),proppath.nelems());
    values = distr->getValues(input->value,distr->inputsize());
    
    for(int i=0; i<length(); i++)
    {        
        
        // Add some elements in temp_comps if necessary
        if(temp_comps.length() < length())
        {
            int last_size = temp_comps.length();
            temp_comps.resize(length());
            for(int j=last_size; j<temp_comps.length(); j++)
            {
                temp_comps[j].resize(proppath.size());
                for(int k=0; k<proppath.size(); k++)
                    temp_comps[j][k] = Var(proppath[k]->length(), proppath[k]->width());
            }
        }
        
        // Point to temp_comps for values and gradients of proppath
        for(int j=0; j<proppath.size(); j++)
        {
            proppath[j]->matValue = temp_comps[i][j]->matValue;
            proppath[j]->value = proppath[j]->matValue.toVec();
            proppath[j]->valuedata = proppath[j]->value.data();

            proppath[j]->matGradient = temp_comps[i][j]->matGradient;
            proppath[j]->gradient = proppath[j]->matGradient.toVec();
            proppath[j]->gradientdata = proppath[j]->gradient.data();
        }
        
        dp_target->value << target_dist_reps->matValue((int)values[i]);
        proppath.fprop();
        //proppath >> temp_comps(i);
        value[i] = output->value[0];
    }
}


void PotentialsVariable::bprop()
{  
    for(int i=0; i<length(); i++)
    {
        
        // Point to temp_comps for values and gradients of proppath
        for(int j=0; j<proppath.size(); j++)
        {
            proppath[j]->matValue = temp_comps[i][j]->matValue;
            proppath[j]->value = proppath[j]->matValue.toVec();
            proppath[j]->valuedata = proppath[j]->value.data();

            proppath[j]->matGradient = temp_comps[i][j]->matGradient;
            proppath[j]->gradient = proppath[j]->matGradient.toVec();
            proppath[j]->gradientdata = proppath[j]->gradient.data();
        }
        
        //proppath << temp_comps(i);
        dp_target->value << target_dist_reps->matValue((int)values[i]);
        proppath.clearGradient();
        dp_target->clearGradient();
        output->gradient[0] = gradient[i];
        proppath.bprop();
        target_dist_reps->matGradient((int)values[i]) += dp_target->gradient;
        target_dist_reps->updateRow((int)values[i]);
    }
}



void PotentialsVariable::symbolicBprop()
{
    PLERROR("In PotentialsVariable::symbolicBprop(): not implemented");
}


void PotentialsVariable::rfprop()
{
    PLERROR("In PotentialsVariable::rfprop(): not implemented");
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
