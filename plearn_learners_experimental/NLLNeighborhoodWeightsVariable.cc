// -*- C++ -*-

// NLLNeighborhoodWeightsVariable.cc
//
// Copyright (C) 2006 Hugo Larochelle
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

// Authors: Hugo Larochelle

/*! \file NLLNeighborhoodWeightsVariable.cc */


#include "NLLNeighborhoodWeightsVariable.h"

namespace PLearn {
using namespace std;

/** NLLNeighborhoodWeightsVariable **/

PLEARN_IMPLEMENT_OBJECT(
    NLLNeighborhoodWeightsVariable,
    "Weights updated online, based on negative log-likelihood of the neighbors",
    "See DeepFeatureExtractorNNet for more details. This variable is very\n"
    "much oriented towards its usage in this PLearner.\n"
    "Note that this variable does not do bprop!!!" 
    );

NLLNeighborhoodWeightsVariable::NLLNeighborhoodWeightsVariable()
    : n(-1), alpha(-1)
{}

// constructor from input variables.
NLLNeighborhoodWeightsVariable::NLLNeighborhoodWeightsVariable(Variable* neighbor_nlls, Variable* neighbor_indexes, int the_n, real the_alpha)
    : inherited(neighbor_nlls, neighbor_indexes, neighbor_nlls->length(), 1),
      n(the_n), alpha(the_alpha)
{
    build();
}

void NLLNeighborhoodWeightsVariable::recomputeSize(int& l, int& w) const
{
    if (input1 && input2) {
        l = input1->length();
        w = 1;
    } else
        l = w = 0;
}

void NLLNeighborhoodWeightsVariable::fprop()
{
    int index;
    for(int i=0; i<length(); i++)
    {
        index = (int)input2->valuedata[i];
        #ifdef BOUNDCHECK
        if(index < 0 || index >= online_weights_log_sum.length())
            PLERROR("In NLLNeighborhoodWeightsVariable::fprop(): input2->valuedata[%d] should be between 0 and n=%d",index,n);
        #endif
        if(is_missing(online_weights_log_sum[index]))
        {
            online_weights_log_sum[index] = -1*input1->valuedata[i];
        }
        else
        {
            online_weights_log_sum[index] = logadd(log_1_minus_alpha + online_weights_log_sum[index], log_alpha - input1->valuedata[i]);
        }
    }
    for(int i=0; i<length(); i++)
    {
        valuedata[i] = exp(-input1->valuedata[i] - online_weights_log_sum[(int)input2->valuedata[i]]);
    }
    //cout << "weights="<< value << endl;
}

void NLLNeighborhoodWeightsVariable::bprop()
{
    
}

// ### You can implement these methods:
// void NLLNeighborhoodWeightsVariable::bbprop() {}
// void NLLNeighborhoodWeightsVariable::symbolicBprop() {}
// void NLLNeighborhoodWeightsVariable::rfprop() {}


// ### Nothing to add here, simply calls build_
void NLLNeighborhoodWeightsVariable::build()
{
    inherited::build();
    build_();
}

void NLLNeighborhoodWeightsVariable::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(online_weights_log_sum, copies);

    //PLERROR("NLLNeighborhoodWeightsVariable::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

void NLLNeighborhoodWeightsVariable::declareOptions(OptionList& ol)
{
    declareOption(ol, "n", &NLLNeighborhoodWeightsVariable::n,
                  OptionBase::buildoption,
                  "Total number of points to be weighted");

    declareOption(ol, "alpha", &NLLNeighborhoodWeightsVariable::alpha,
                  OptionBase::buildoption,
                  "Exponential decay");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void NLLNeighborhoodWeightsVariable::build_()
{
    if(input1 && input2)
    {
        if(n<=0)
            PLERROR("In NLLNeighborhoodWeightsVariable::build_(): n must be > 0");
        if(alpha <= 0 || alpha >= 1)
            PLERROR("In NLLNeighborhoodWeightsVariable::build_(): alpha must be in ]0,1[");
        log_alpha = pl_log(alpha);
        log_1_minus_alpha = pl_log(1-alpha);
        online_weights_log_sum.resize(n);
        online_weights_log_sum.fill(MISSING_VALUE);
        if(input1->width() != 1)
            PLERROR("In NLLNeighborhoodWeightsVariable::build_(): input1 must be column vector");
        if(input2->width() != 1)
            PLERROR("In NLLNeighborhoodWeightsVariable::build_(): input2 must be column vector");
        if(input1->size() != input2->size())
            PLERROR("In NLLNeighborhoodWeightsVariable::build_(): input1 and input2 must be of same size");
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
