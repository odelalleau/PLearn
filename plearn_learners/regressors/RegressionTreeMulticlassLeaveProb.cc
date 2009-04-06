// -*- C++ -*-

// RegressionTreeMulticlassLeaveProb.cc
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
 * $Id: RegressionTreeMulticlassLeaveProb.cc, v 1.0 2004/07/19 10:00:00 Bengio/Kegl/Godbout    *
 * This file is part of the PLearn library.                                     *
 ******************************************************************************** */

#include "RegressionTreeMulticlassLeaveProb.h"
#include "RegressionTreeRegisters.h"
#include <plearn/math/TMat_maths_impl.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(RegressionTreeMulticlassLeaveProb,
                        "Object to represent the leaves of a regression tree.",
                        "It maintains the necessary statistics to compute the output and the train error\n"
                        "of the samples in the leave.\n"
    );

RegressionTreeMulticlassLeaveProb::RegressionTreeMulticlassLeaveProb()
    : nb_class(-1),
      objective_function("l1")
{
    build();
}

RegressionTreeMulticlassLeaveProb::~RegressionTreeMulticlassLeaveProb()
{
}

void RegressionTreeMulticlassLeaveProb::declareOptions(OptionList& ol)
{ 
    inherited::declareOptions(ol);

    declareOption(ol, "nb_class", 
                  &RegressionTreeMulticlassLeaveProb::nb_class,
                  OptionBase::buildoption,
                  "The number of class. Should be numbered from 0 to nb_class -1.\n"
        );

    declareOption(ol, "objective_function",
                  &RegressionTreeMulticlassLeaveProb::objective_function,
                  OptionBase::buildoption,
                  "The function to be used to compute the leave error.\n"
                  "Current supported values are l1 and l2 (default is l1).");
      
    declareOption(ol, "multiclass_weights_sum",
                  &RegressionTreeMulticlassLeaveProb::multiclass_weights_sum,
                  OptionBase::learntoption,
                  "A vector to count the weight sum of each possible output "
                  "for the sample in this leave.\n");
    redeclareOption(ol, "loss_function_factor",
                  &RegressionTreeMulticlassLeaveProb::loss_function_factor,
                  OptionBase::learntoption,
                  "The loss fct factor. Depend of the objective_function.\n");
}

void RegressionTreeMulticlassLeaveProb::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(objective_function, copies);
    deepCopyField(multiclass_weights_sum, copies);
}

void RegressionTreeMulticlassLeaveProb::build()
{
    inherited::build();
    build_();
}

void RegressionTreeMulticlassLeaveProb::build_()
{
}

void RegressionTreeMulticlassLeaveProb::initStats()
{
    length_ = 0;
    weights_sum = 0.0;
    if (loss_function_weight != 0.0)
    {
        if(objective_function == "l1")
            loss_function_factor = 2.0 / loss_function_weight;
        else
            loss_function_factor = 2.0 / pow(loss_function_weight, 2);
    }
    else
    {
        loss_function_factor = 1.0;
    }
    multiclass_weights_sum.resize(nb_class);
    multiclass_weights_sum.fill(0);
}

void RegressionTreeMulticlassLeaveProb::addRow(int row)
{
    real weight = train_set->getWeight(row);
    real target = train_set->getTarget(row);
    RegressionTreeMulticlassLeaveProb::addRow(row, target, weight);
}

void RegressionTreeMulticlassLeaveProb::addRow(int row, real target, real weight,
                                 Vec outputv, Vec errorv)
{
    RegressionTreeMulticlassLeaveProb::addRow(row, target, weight);
    RegressionTreeMulticlassLeaveProb::getOutputAndError(outputv,errorv);
}

void RegressionTreeMulticlassLeaveProb::addRow(int row, real target, real weight)
{
    length_ += 1;
    weights_sum += weight;
    multiclass_weights_sum[int(target)] += weight;
}

void RegressionTreeMulticlassLeaveProb::addRow(int row, Vec outputv, Vec errorv)
{
    RegressionTreeMulticlassLeaveProb::addRow(row);
    RegressionTreeMulticlassLeaveProb::getOutputAndError(outputv,errorv);    
}

void RegressionTreeMulticlassLeaveProb::removeRow(int row, Vec outputv, Vec errorv)
{
    real weight = train_set->getWeight(row);
    real target = train_set->getTarget(row);
    RegressionTreeMulticlassLeaveProb::removeRow(row,target,weight,outputv,errorv);
}

void RegressionTreeMulticlassLeaveProb::removeRow(int row, real target, real weight,
                                 Vec outputv, Vec errorv){
    RegressionTreeMulticlassLeaveProb::removeRow(row,target,weight);
    RegressionTreeMulticlassLeaveProb::getOutputAndError(outputv,errorv);
}

void RegressionTreeMulticlassLeaveProb::removeRow(int row, real target, real weight)
{
    length_ -= 1;
    weights_sum -= weight;
    PLASSERT(length_>=0);
    PLASSERT(weights_sum>=0);
    PLASSERT(length_>0 || weights_sum==0);
    multiclass_weights_sum[int(target)] -= weight;
}

void RegressionTreeMulticlassLeaveProb::getOutputAndError(Vec& output, Vec& error)const
{
#ifdef BOUNDCHECK
    if(nb_class<=0)
        PLERROR("In RegressionTreeMulticlassLeaveProb::getOutputAndError() -"
                " nb_class must be set.");
#endif
    if(length_==0){        
        output.fill(MISSING_VALUE);
        error.clear();
        return;
    }
    int mc_winer = 0;
    real conf = 0;
    //index of the max. Is their an optimized version?
    output[1] = multiclass_weights_sum[0] / weights_sum;
    for (int mc_ind = 1; mc_ind < nb_class; mc_ind++)
    {
        output[mc_ind+1]=multiclass_weights_sum[mc_ind] / weights_sum;
        if (multiclass_weights_sum[mc_ind] > multiclass_weights_sum[mc_winer])
            mc_winer = mc_ind;
    }
    output[0] = mc_winer;
    if (missing_leave)
    {
        error[0] = 0.0;
        error[1] = weights_sum;
        error[2] = 0.0;
    }
    else
    {
        conf = multiclass_weights_sum[mc_winer] / weights_sum;
        error[0] = 0.0;
        if (objective_function == "l1")
        {
            for (int mc_ind = 0; mc_ind < nb_class;mc_ind++)
            {
                error[0] += abs(mc_winer - mc_ind) 
                    * multiclass_weights_sum[mc_ind];
            }
        }
        else
        {
            for (int mc_ind = 0; mc_ind < nb_class;mc_ind++)
            {
                error[0] += pow(mc_winer - mc_ind, 2.) 
                    * multiclass_weights_sum[mc_ind];
            }
        }
        error[0] *= loss_function_factor * length_ / weights_sum;
        if (error[0] < 1E-10) error[0] = 0.0;
        if (error[0] > weights_sum * loss_function_factor)
            error[2] = weights_sum * loss_function_factor;
        else error[2] = error[0];
        error[1] = (1.0 - conf) * length_;
    }
}

TVec<string> RegressionTreeMulticlassLeaveProb::getOutputNames() const
{
    TVec<string> ret(nb_class+1);
    ret[0]="class_pred";
    for (int mc_ind = 0; mc_ind < nb_class;mc_ind++)
    {
        ret[mc_ind+1]="prob_class_"+tostring(mc_ind);
    }
    return ret;
}

void RegressionTreeMulticlassLeaveProb::addLeave(PP<RegressionTreeLeave> leave_){
    PP<RegressionTreeMulticlassLeaveProb> leave = (PP<RegressionTreeMulticlassLeaveProb>) leave_;

    if(leave->classname() == classname()){
        length_ += leave->length_;
        weights_sum += leave->weights_sum;
        multiclass_weights_sum += leave->multiclass_weights_sum;
    }else
        PLERROR("In %s::addLeave the leave to add should have the same class. It have %s.",
                classname().c_str(), leave->classname().c_str());
}

void RegressionTreeMulticlassLeaveProb::removeLeave(PP<RegressionTreeLeave> leave_){
    PP<RegressionTreeMulticlassLeaveProb> leave = (PP<RegressionTreeMulticlassLeaveProb>) leave_;

    if(leave->classname() == classname()){
        length_ -= leave->length_;
        weights_sum -= leave->weights_sum;
        multiclass_weights_sum -= leave->multiclass_weights_sum;
    }else
        PLERROR("In %s::addLeave the leave to add should have the same class. It have %s.",
                classname().c_str(), leave->classname().c_str());
}

void RegressionTreeMulticlassLeaveProb::printStats()
{
    cout << " l " << length_;
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
