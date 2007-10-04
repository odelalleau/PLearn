// -*- C++ -*-

// BaseRegressorConfidence.cc
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
 * $Id: BaseRegressorConfidence.cc, v 1.0 2004/07/19 10:00:00 Bengio/Kegl/Godbout        *
 * This file is part of the PLearn library.                                     *
 ******************************************************************************** */

#include "BaseRegressorConfidence.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(BaseRegressorConfidence,
                        "A PLearner to provide a confidence function to a generic base regressor.", 
                        "The algorithm computes an outut between 0 and 1 based on a local density function.\n"
    );

BaseRegressorConfidence::BaseRegressorConfidence()     
    : number_of_neighbors(1),
      sigma(0.1),
      raise_confidence(1.0),
      lower_confidence(0.0)
{
}

BaseRegressorConfidence::~BaseRegressorConfidence()
{
}

void BaseRegressorConfidence::declareOptions(OptionList& ol)
{ 
    declareOption(ol, "number_of_neighbors", &BaseRegressorConfidence::number_of_neighbors, OptionBase::buildoption,
                  "The number of nearest neighbors to consider.\n");
    declareOption(ol, "sigma", &BaseRegressorConfidence::sigma, OptionBase::buildoption,
                  "The variance of the distribution on the target.\n");
    declareOption(ol, "raise_confidence", &BaseRegressorConfidence::raise_confidence, OptionBase::buildoption,
                  "If the computed confidence is greater or equal to this level, it will be raised to 1.0.\n");
    declareOption(ol, "lower_confidence", &BaseRegressorConfidence::lower_confidence, OptionBase::buildoption,
                  "If the computed confidence is lower than this level, it will be lowered to 0.0.\n");
      
    declareOption(ol, "neighbors", &BaseRegressorConfidence::neighbors, OptionBase::learntoption,
                  "The matrice of indices of nearest neighbors rows from the training set.\n");
    declareOption(ol, "nearest_neighbbors_target_mean", &BaseRegressorConfidence::nearest_neighbbors_target_mean, OptionBase::learntoption,
                  "The vector of neairest neighborstarget means from the trainingset.\n");;
    inherited::declareOptions(ol);
}

void BaseRegressorConfidence::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(number_of_neighbors, copies);
    deepCopyField(sigma, copies);
    deepCopyField(raise_confidence, copies);
    deepCopyField(lower_confidence, copies);
    deepCopyField(neighbors, copies);
    deepCopyField(nearest_neighbbors_target_mean, copies);
}

void BaseRegressorConfidence::build()
{
    inherited::build();
    build_();
}

void BaseRegressorConfidence::build_()
{
    if (train_set)
    {
        input_to_search.resize(train_set->inputsize());
        target_to_search.resize(1);
        input_to_compare.resize(train_set->inputsize());
        target_to_compare.resize(1);
        neighbors.resize(train_set->length(), number_of_neighbors);
        nearest_neighbbors_target_mean.resize(train_set->length());
        two_sigma_square = 2.0 * pow(sigma, 2);
        root_two_pi_sigma_square = sigma * 2.506628274631;
    }
}

void BaseRegressorConfidence::train()
{
    for(int row_to_search = 0; row_to_search < train_set.length(); row_to_search++)
    {
        BottomNI<real> neighbors_search(number_of_neighbors);
        nearest_neighbbors_target_mean[row_to_search] = 0.0;
        train_set->getExample(row_to_search, input_to_search, target_to_search, weight_to_search);
        for(int row_to_compare = 0; row_to_compare < train_set.length(); row_to_compare++)
        {
            train_set.getExample(row_to_compare, input_to_compare, target_to_compare, weight_to_compare);
            neighbors_search.update(powdistance(input_to_search, input_to_compare), row_to_compare);
        }
        neighbors_search.sort();
        for(int row_to_compare = 0; row_to_compare < number_of_neighbors; row_to_compare++)
        {
            TVec< pair<real,int> > indices = neighbors_search.getBottomN();
            neighbors(row_to_search, row_to_compare) = indices[row_to_compare].second;
            train_set->getExample(neighbors(row_to_search, row_to_compare), input_to_compare, target_to_compare, weight_to_compare);
            nearest_neighbbors_target_mean[row_to_search] += target_to_compare[0];
        }
        nearest_neighbbors_target_mean[row_to_search] = nearest_neighbbors_target_mean[row_to_search] / number_of_neighbors;
    }
}

void BaseRegressorConfidence::verbose(string the_msg, int the_level)
{
    if (verbosity >= the_level)
        cout << the_msg << endl;
}

void BaseRegressorConfidence::forget()
{
}

int BaseRegressorConfidence::outputsize() const
{
    return 2;
}

TVec<string> BaseRegressorConfidence::getTrainCostNames() const
{
    TVec<string> return_msg(1);
    return_msg[0] = "mse";
    return return_msg;
}

TVec<string> BaseRegressorConfidence::getTestCostNames() const
{ 
    return getTrainCostNames();
}

void BaseRegressorConfidence::computeOutput(const Vec& inputv, Vec& outputv) const
{
    Vec train_set_inputv;
    Vec train_set_targetv;
    real train_set_weight;
    train_set_inputv.resize(train_set->inputsize());
    train_set_targetv.resize(1);
    real distance = REAL_MAX;
    int nearest_neighbor = -1;
    for (int row = 0; row < train_set->length(); row++)
    {
        train_set->getExample(row, train_set_inputv, train_set_targetv, train_set_weight);
        if (powdistance(inputv, train_set_inputv) < distance)
        {
            distance = powdistance(inputv, train_set_inputv);
            nearest_neighbor = row;
        }
    }
    outputv[1] = exp(-1.0 * pow((outputv[0] - nearest_neighbbors_target_mean[nearest_neighbor]), 2) / two_sigma_square); //  / root_two_pi_sigma_square?
    outputv[0] = nearest_neighbbors_target_mean[nearest_neighbor];
    if (outputv[1] >= raise_confidence) outputv[1] = 1.0;
    if (outputv[1] < lower_confidence) outputv[1] = 0.0;
}

void BaseRegressorConfidence::computeOutputAndCosts(const Vec& inputv, const Vec& targetv, Vec& outputv, Vec& costsv) const
{
    computeOutput(inputv, outputv);
    computeCostsFromOutputs(inputv, outputv, targetv, costsv);
}

void BaseRegressorConfidence::computeCostsFromOutputs(const Vec& inputv, const Vec& outputv, const Vec& targetv, Vec& costsv) const
{
    costsv[0] = pow((outputv[0] - targetv[0]), 2);
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
