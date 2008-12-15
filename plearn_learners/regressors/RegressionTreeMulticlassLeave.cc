// -*- C++ -*-

// RegressionTreeMulticlassLeave.cc
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
 * $Id: RegressionTreeMulticlassLeave.cc, v 1.0 2004/07/19 10:00:00 Bengio/Kegl/Godbout    *
 * This file is part of the PLearn library.                                     *
 ******************************************************************************** */

#include "RegressionTreeMulticlassLeave.h"
#include "RegressionTreeRegisters.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(RegressionTreeMulticlassLeave,
                        "Object to represent the leaves of a regression tree.",
                        "It maintains the necessary statistics to compute the output and the train error\n"
                        "of the samples in the leave.\n"
    );

RegressionTreeMulticlassLeave::RegressionTreeMulticlassLeave()
    : objective_function("l1")
{
    build();
}

RegressionTreeMulticlassLeave::~RegressionTreeMulticlassLeave()
{
}

void RegressionTreeMulticlassLeave::declareOptions(OptionList& ol)
{ 
    declareOption(ol, "multiclass_outputs", 
                  &RegressionTreeMulticlassLeave::multiclass_outputs,
                  OptionBase::buildoption,
                  "A vector of possible output values when solving a multiclass problem.\n"
                  "The leave will output the value with the largest weight sum.");
    declareOption(ol, "objective_function",
                  &RegressionTreeMulticlassLeave::objective_function,
                  OptionBase::buildoption,
                  "The function to be used to compute the leave error.\n"
                  "Current supported values are l1 and l2 (default is l1).");
      
    declareOption(ol, "multiclass_weights_sum",
                  &RegressionTreeMulticlassLeave::multiclass_weights_sum,
                  OptionBase::learntoption,
                  "A vector to count the weight sum of each possible output "
                  "for the sample in this leave.\n");
    declareOption(ol, "l1_loss_function_factor",
                  &RegressionTreeMulticlassLeave::l1_loss_function_factor,
                  OptionBase::learntoption,
                  "2 / loss_function_weight.\n");
    declareOption(ol, "l2_loss_function_factor",
                  &RegressionTreeMulticlassLeave::l2_loss_function_factor,
                  OptionBase::learntoption,
                  "2 / pow(loss_function_weight, 2.0).\n");
    inherited::declareOptions(ol);
}

void RegressionTreeMulticlassLeave::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(multiclass_outputs, copies);
    deepCopyField(objective_function, copies);
    deepCopyField(l1_loss_function_factor, copies);
    deepCopyField(l2_loss_function_factor, copies);
    deepCopyField(multiclass_weights_sum, copies);
}

void RegressionTreeMulticlassLeave::build()
{
    inherited::build();
    build_();
}

void RegressionTreeMulticlassLeave::build_()
{
}

void RegressionTreeMulticlassLeave::initStats()
{
    length = 0;
    weights_sum = 0.0;
    if (loss_function_weight != 0.0)
    {
        l1_loss_function_factor = 2.0 / loss_function_weight;
        l2_loss_function_factor = 2.0 / pow(loss_function_weight, 2);
    }
    else
    {
        l1_loss_function_factor = 1.0;
        l2_loss_function_factor = 1.0;
    }
    multiclass_weights_sum.resize(multiclass_outputs.length());
    multiclass_weights_sum.fill(0);
}

void RegressionTreeMulticlassLeave::addRow(int row)
{
    real weight = train_set->getWeight(row);
    real target = train_set->getTarget(row);
    addRow(row, target, weight);
}

void RegressionTreeMulticlassLeave::addRow(int row, real target, real weight,
                                 Vec outputv, Vec errorv)
{
    addRow(row, target, weight);
    getOutputAndError(outputv,errorv);
}

void RegressionTreeMulticlassLeave::addRow(int row, real target, real weight)
{
    length += 1;
    weights_sum += weight;
    int multiclass_found = 0;
    //if target are 0,1,2,... it can be optimized by multiclass_weights_sum[target]
    //for the general case: by using a table with index being the target and the value the needed index
    for (int mc_ind = 0; mc_ind < multiclass_outputs.length(); mc_ind++)
    {
        if (target == multiclass_outputs[mc_ind])
        {
            multiclass_weights_sum[mc_ind] += weight;
            multiclass_found = 1;      
            break;      
        }
    }
    if (multiclass_found < 1) 
        PLERROR("RegressionTreeMultilassLeave: Unknown target: %g row: %d\n",
                target, row);
}

void RegressionTreeMulticlassLeave::addRow(int row, Vec outputv, Vec errorv)
{
    addRow(row);
    getOutputAndError(outputv,errorv);    
}

void RegressionTreeMulticlassLeave::removeRow(int row, Vec outputv, Vec errorv)
{
    real weight = train_set->getWeight(row);
    real target = train_set->getTarget(row);
    removeRow(row,target,weight,outputv,errorv);
}

void RegressionTreeMulticlassLeave::removeRow(int row, real target, real weight,
                                 Vec outputv, Vec errorv){
    removeRow(row,target,weight);
    getOutputAndError(outputv,errorv);
}

void RegressionTreeMulticlassLeave::removeRow(int row, real target, real weight)
{
    length -= 1;
    weights_sum -= weight;
    PLASSERT(length>=0);
    PLASSERT(weights_sum>=0);
    PLASSERT(length>0 || weights_sum==0);
    bool found=false;
    //can be optimized: see addRow
    for (int mc_ind = 0; mc_ind < multiclass_outputs.length(); mc_ind++)
    {
        if (target == multiclass_outputs[mc_ind])
        {
            multiclass_weights_sum[mc_ind] -= weight;
            found=true;
            break;      
        }
    }
    PLASSERT(found);
}

void RegressionTreeMulticlassLeave::getOutputAndError(Vec& output, Vec& error)const
{
#ifdef BOUNDCHECK
    if(multiclass_outputs.length()<=0)
        PLERROR("In RegressionTreeMulticlassLeave::getOutputAndError() -"
                " multiclass_outputs must not be empty");
#endif
    if(length==0){        
        output.clear();
        output[0]=MISSING_VALUE;
        error.clear();
        return;
    }
    int mc_winer = 0;
    //index of the max. Is their an optimized version?
    for (int mc_ind = 1; mc_ind < multiclass_outputs.length(); mc_ind++)
    {
        if (multiclass_weights_sum[mc_ind] > multiclass_weights_sum[mc_winer])
            mc_winer = mc_ind;
    }
    output[0] = multiclass_outputs[mc_winer];
    if (missing_leave)
    {
        output[1] = 0.0;
        error[0] = 0.0;
        error[1] = weights_sum;
        error[2] = 0.0;
    }
    else
    {
        output[1] = multiclass_weights_sum[mc_winer] / weights_sum;;
        error[0] = 0.0;
        if (objective_function == "l1")
        {
            for (int mc_ind = 0; mc_ind < multiclass_outputs.length();mc_ind++)
            {
                error[0] += abs(output[0] - multiclass_outputs[mc_ind]) 
                    * multiclass_weights_sum[mc_ind];
            }
            error[0] *= l1_loss_function_factor * length / weights_sum;
            if (error[0] < 1E-10) error[0] = 0.0;
            if (error[0] > weights_sum * l1_loss_function_factor)
                error[2] = weights_sum * l1_loss_function_factor;
            else error[2] = error[0];
        }
        else
        {
            for (int mc_ind = 0; mc_ind < multiclass_outputs.length();mc_ind++)
            {
                error[0] += pow(output[0] - multiclass_outputs[mc_ind], 2) 
                    * multiclass_weights_sum[mc_ind];
            }
            error[0] *= l2_loss_function_factor * length / weights_sum;
            if (error[0] < 1E-10) error[0] = 0.0;
            if (error[0] > weights_sum * l2_loss_function_factor) 
                error[2] = weights_sum * l2_loss_function_factor; 
            else error[2] = error[0];
        }
        error[1] = (1.0 - output[1]) * length;
    }
}

void RegressionTreeMulticlassLeave::printStats()
{
    cout << " l " << length;
    Vec output(2);
    Vec error(3);
    getOutputAndError(output,error);
    cout << " o0 " << output[0];
    cout << " o1 " << output[1];
    cout << " e0 " << error[0];
    cout << " e1 " << error[1];
    cout << " ws " << weights_sum;
    cout << endl;
    cout << " mws " << multiclass_weights_sum << endl;
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
