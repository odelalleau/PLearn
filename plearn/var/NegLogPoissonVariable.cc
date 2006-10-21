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
 * $Id: NegLogPoissonVariable.cc 4412 2005-11-02 19:00:20Z tihocan $
 * This file is part of the PLearn library.
 ******************************************************* */

#include "NegLogPoissonVariable.h"

namespace PLearn {
using namespace std;

/** NegLogPoissonVariable **/

PLEARN_IMPLEMENT_OBJECT(NegLogPoissonVariable,
                        "Negative loglikelihood of the poisson distribution",
                        "Negative loglikelihood of the poisson distribution\n"
                        "cost =  sum_i {exp(output_i) - output_i * target_i + log(target_i!)}\n"
                        "Thus, wrt usual notation, output_i = log(lambda_i)");

// We can link the notation used above to the usual
// statistical notation in the following way:
// lambda = exp(output_i) 
// where exponentiation ensures a positive value for lambda
// and k = target_i, the number of observed events
// Thus, the cost corresponds to the negative log likelihood
// of the Poisson density

NegLogPoissonVariable::NegLogPoissonVariable(VarArray& the_varray)
    : inherited(the_varray, 1, 1)
{
    build_();
}

void
NegLogPoissonVariable::build()
{
    inherited::build();
    build_();
}

void
NegLogPoissonVariable::build_()
{
    if (varray[0] && varray[1] && (varray[0]->size() != varray[1]->size()))
        PLERROR("In NegLogPoissonVariable: netout and target must have the same size");
    if (varray.size()>2 && varray[0] && varray[2] && (varray[0]->size() != varray[2]->size()))
        PLERROR("In NegLogPoissonVariable: netout and weight must have the same size");
}

void NegLogPoissonVariable::recomputeSize(int& l, int& w) const
{ l=1, w=1; }

void NegLogPoissonVariable::fprop()
{
    real cost = 0.0;
    for (int i=0; i<varray[0]->size(); i++)
    {
        real output = varray[0]->valuedata[i];
        real target = varray[1]->valuedata[i];
        real weight = 1;
        if (varray.size()>2)
            weight = varray[2]->valuedata[i];
        cost += exp(output) * weight - (output + pl_log(weight) ) * target + pl_gammln(target+1);
    }
    valuedata[0] = cost;
}

void NegLogPoissonVariable::bprop()
{
    real gr = *gradientdata;
    for (int i=0; i<varray[0]->size(); i++)
    {
        real output = varray[0]->valuedata[i];
        real target = varray[1]->valuedata[i];
        real weight = 1;
        if (varray.size()>2)
            weight = varray[2]->valuedata[i];
        varray[0]->gradientdata[i] += gr* ( exp(output) * weight - target );
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
