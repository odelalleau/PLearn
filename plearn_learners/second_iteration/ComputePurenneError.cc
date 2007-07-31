// -*- C++ -*-

// ComputePurenneError.cc
// Copyright (c) 1998-2002 Pascal Vincent
// Copyright (C) 1999-2002 Yoshua Bengio and University of Montreal
// Copyright (c) 2002 Jean-Sebastien Senecal, Xavier Saint-Mleux, Rejean Ducharme
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


/* ********************************************************************************    
 * $Id: ComputePurenneError.cc, v 1.0 2004/07/19 10:00:00 Bengio/Kegl/Godbout        *
 * This file is part of the PLearn library.                                     *
 ******************************************************************************** */

#include "ComputePurenneError.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(ComputePurenneError,
                        "A PLearner to compute the prediction error of Vincent.", 
                        "\n"
    );

ComputePurenneError::ComputePurenneError()  
{
}

ComputePurenneError::~ComputePurenneError()
{
}

void ComputePurenneError::declareOptions(OptionList& ol)
{
    inherited::declareOptions(ol);
}

void ComputePurenneError::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
}

void ComputePurenneError::build()
{
    inherited::build();
    build_();
}

void ComputePurenneError::build_()
{
}

void ComputePurenneError::train()
{
    int row;
    Vec sample_input(train_set->inputsize());
    Vec sample_target(train_set->targetsize());
    real sample_weight;
    Vec sample_output(2);
    Vec sample_costs(4);
    ProgressBar* pb = NULL;
    if (report_progress)
    {
        pb = new ProgressBar("Purenne error: computing the train statistics: ", train_set->length());
    } 
    train_stats->forget();
    for (row = 0; row < train_set->length(); row++)
    {  
        train_set->getExample(row, sample_input, sample_target, sample_weight);
        computeOutput(sample_input, sample_output);
        computeCostsFromOutputs(sample_input, sample_output, sample_target, sample_costs); 
        train_stats->update(sample_costs);
        if (report_progress) pb->update(row);
    }
    train_stats->finalize();
    if (report_progress) delete pb; 
}

void ComputePurenneError::forget()
{
}

int ComputePurenneError::outputsize() const
{
    return 2;
}

TVec<string> ComputePurenneError::getTrainCostNames() const
{
    TVec<string> return_msg(4);
    return_msg[0] = "mse";
    return_msg[1] = "class_error";
    return_msg[2] = "linear_class_error";
    return_msg[3] = "square_class_error";
    return return_msg;
}

TVec<string> ComputePurenneError::getTestCostNames() const
{ 
    return getTrainCostNames();
}

void ComputePurenneError::computeOutput(const Vec& inputv, Vec& outputv) const
{
    outputv[0] = inputv[0];
    outputv[1] = inputv[1];
}

void ComputePurenneError::computeOutputAndCosts(const Vec& inputv, const Vec& targetv, Vec& outputv, Vec& costsv) const
{
    computeOutput(inputv, outputv);
    computeCostsFromOutputs(inputv, outputv, targetv, costsv);
}

void ComputePurenneError::computeCostsFromOutputs(const Vec& inputv, const Vec& outputv, const Vec& targetv, Vec& costsv) const
{
    costsv[0] = pow((outputv[0] - targetv[0]), 2);
    costsv[1] = outputv[1] == targetv[1] ? 0 : 1;
    costsv[2] = int(round(fabs(outputv[1] - targetv[1])));
    costsv[3] = pow((outputv[1] - targetv[1]), 2);
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
