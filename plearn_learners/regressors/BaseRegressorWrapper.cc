// -*- C++ -*-

// BaseRegressorWrapper.cc
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
 * $Id: BaseRegressorWrapper.cc, v 1.0 2004/07/19 10:00:00 Bengio/Kegl/Godbout        *
 * This file is part of the PLearn library.                                     *
 ******************************************************************************** */

#include "BaseRegressorWrapper.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(BaseRegressorWrapper,
                        "A PLearner to wrap around a generic regressor to serve as a base learner to LocalMedBoost.", 
                        "Algorithm built to serve as a base regressor for the LocalMedBoost algorithm.\n"
                        "It separates a confidence fonction from the output when making a prediction.\n"
                        "It also computes mse, base confidence and base reward costs as expected by the boosting algorithm.\n"
                        "To do that, it reformats output and cost in the format expected by boosting.\n"
                        "It is intended also to implement in the fututre, various confidence functions.\n"
                        "We could also use it to implement various missing value imputation strategies.\n"
    );

BaseRegressorWrapper::BaseRegressorWrapper()     
    : loss_function_weight(1.0),
      mean_imputation(0),
      regression_tree(0),
      use_confidence_function(0),
      use_base_regressor_confidence(0)
{
}

BaseRegressorWrapper::~BaseRegressorWrapper()
{
}

void BaseRegressorWrapper::declareOptions(OptionList& ol)
{ 
    declareOption(ol, "loss_function_weight", &BaseRegressorWrapper::loss_function_weight, OptionBase::buildoption,
                  "The hyper parameter to balance the error and the confidence factor.\n");
    declareOption(ol, "mean_imputation", &BaseRegressorWrapper::mean_imputation, OptionBase::buildoption,
                  "If set to 1, the algorithm will compute the mean vector of the input variables from the training set,\n"
                  "and replace missing values with the computed means.\n");
    declareOption(ol, "regression_tree", &BaseRegressorWrapper::regression_tree, OptionBase::buildoption,
                  "If set to 1, the tree_regressor_template is used instead of the base_regressor_template.\n"
                  "It permits to sort the train set only once for all boosting iterations.\n");       
    declareOption(ol, "use_confidence_function", &BaseRegressorWrapper::use_confidence_function, OptionBase::buildoption,
                  "If set to 1, the confidence_function_template is used to build a confidence estimates on the base regressor prediction.\n"
                  "Otherwise, if the confidence from the base regressor is not used, the confidence is always set to 1.0.\n");
    declareOption(ol, "use_base_regressor_confidence", &BaseRegressorWrapper::use_base_regressor_confidence, OptionBase::buildoption,
                  "If set to 1, the confidence is taken from the second output of the base regressor.\n");
    declareOption(ol, "base_regressor_template", &BaseRegressorWrapper::base_regressor_template, OptionBase::buildoption,
                  "The template for the base regressor to be boosted.\n"); 
    declareOption(ol, "tree_regressor_template", &BaseRegressorWrapper::tree_regressor_template, OptionBase::buildoption,
                  "The template for a RegressionTree base regressor when the regression_tree option is set to 1.\n");  
    declareOption(ol, "confidence_function_template", &BaseRegressorWrapper::confidence_function_template, OptionBase::buildoption,
                  "The template for the confidence function to be learnt from the train set.\n"); 
    declareOption(ol, "sorted_train_set", &BaseRegressorWrapper::sorted_train_set, OptionBase::buildoption,
                  "A sorted train set when using a tree as a base regressor\n");
      
    declareOption(ol, "base_regressor", &BaseRegressorWrapper::base_regressor, OptionBase::learntoption,
                  "The base regressor built from the template.\n");
    declareOption(ol, "confidence_function", &BaseRegressorWrapper::confidence_function, OptionBase::learntoption,
                  "The confidence function learnt from the train set.\n");
    declareOption(ol, "base_regressor_train_set", &BaseRegressorWrapper::base_regressor_train_set, OptionBase::learntoption,
                  "The VMat train set prepared for the base regressor.\n"
                  "It apllies the chosen missing value management strategies.\n");
    declareOption(ol, "variable_means", &BaseRegressorWrapper::variable_means, OptionBase::learntoption,
                  "The vector with the computed means on all input dimension to perform mean imputation if applicable.\n");
    inherited::declareOptions(ol);
}

void BaseRegressorWrapper::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(loss_function_weight, copies);
    deepCopyField(mean_imputation, copies);
    deepCopyField(regression_tree, copies);
    deepCopyField(use_confidence_function, copies);
    deepCopyField(use_base_regressor_confidence, copies);
    deepCopyField(base_regressor_template, copies);
    deepCopyField(tree_regressor_template, copies);
    deepCopyField(confidence_function_template, copies);
    deepCopyField(sorted_train_set, copies);
    deepCopyField(base_regressor, copies);
    deepCopyField(confidence_function, copies);
    deepCopyField(base_regressor_train_set, copies);
}

void BaseRegressorWrapper::build()
{
    inherited::build();
    build_();
}

void BaseRegressorWrapper::build_()
{
    if (train_set)
    {
        if (mean_imputation > 0)
        {
            PP<MeanImputationVMatrix> new_train_set = new MeanImputationVMatrix(train_set, 0.0);
            variable_means = new_train_set->getMeanVector();
            base_regressor_train_set = VMat(new_train_set);
        }
        else
        {
            base_regressor_train_set = train_set;
        }
        if (regression_tree > 0)
        {
            tree_regressor = ::PLearn::deepCopy(tree_regressor_template);
            tree_regressor->setSortedTrainSet(sorted_train_set);
            tree_regressor->setOption("loss_function_weight", tostring(loss_function_weight));
            base_regressor = tree_regressor;
        }
        else
        {
            base_regressor = ::PLearn::deepCopy(base_regressor_template);
        }
        base_regressor->setTrainingSet(base_regressor_train_set, true);
        base_regressor->setTrainStatsCollector(new VecStatsCollector);
        if (use_confidence_function > 0)
        {
            confidence_function = ::PLearn::deepCopy(confidence_function_template);
            confidence_function->setTrainingSet(base_regressor_train_set, true);
            confidence_function->setTrainStatsCollector(new VecStatsCollector);
            confidence_function->train();
        }
    }
}

void BaseRegressorWrapper::train()
{
    base_regressor->train();
    /*
      cout << "testing the confidence function" << endl;
      Vec train_input;
      Vec train_target;
      Vec train_output;
      real train_weight;
      train_input->resize(train_set->inputsize());
      train_target->resize(train_set->targetsize());
      train_output->resize(2);
      for (int row = 0; row < 25; row++)  
      {
      train_set->getExample(row, train_input, train_target, train_weight);
      cout << "row: " << row << " target: " << train_target[0];
      computeOutput(train_input, train_output);
      }
      PLERROR("We are done for now");
    */
}

void BaseRegressorWrapper::verbose(string the_msg, int the_level)
{
    if (verbosity >= the_level)
        cout << the_msg << endl;
}

void BaseRegressorWrapper::forget()
{
}

int BaseRegressorWrapper::outputsize() const
{
    return 2;
}

TVec<string> BaseRegressorWrapper::getTrainCostNames() const
{
    TVec<string> return_msg(3);
    return_msg[0] = "mse";
    return_msg[1] = "base confidence";
    return_msg[2] = "base reward";
    return return_msg;
}

TVec<string> BaseRegressorWrapper::getTestCostNames() const
{ 
    return getTrainCostNames();
}

void BaseRegressorWrapper::computeOutput(const Vec& inputv, Vec& outputv) const
{
    Vec base_regressor_inputv;
    Vec base_regressor_outputv;
    Vec confidence_outputv;
    base_regressor_inputv.resize(train_set->inputsize());
    base_regressor_outputv.resize(base_regressor->outputsize());
    for (int variable = 0; variable < train_set->inputsize(); variable++)
    {
        base_regressor_inputv[variable] = inputv[variable];
        if (mean_imputation > 0 && is_missing(inputv[variable]) ) base_regressor_inputv[variable] = variable_means[variable];
    }
    base_regressor->computeOutput(base_regressor_inputv, base_regressor_outputv);
    outputv[0] = base_regressor_outputv[0];
//  cout << "base regressor output: " << outputv[0];
    if (use_base_regressor_confidence > 0)
    {
        outputv[1] = base_regressor_outputv[1];
    }
    else
    {
        if (use_confidence_function > 0)
        {
            confidence_outputv.resize(confidence_function->outputsize());
            confidence_outputv[0] = base_regressor_outputv[0];
            confidence_function->computeOutput(base_regressor_inputv, confidence_outputv);
            outputv[1] = confidence_outputv[1];
        }
        else
        {
            outputv[1] = 1.0;
        }
    }
//  cout << "confidence output: " << confidence_outputv[0];
//  cout << " confidence: " << outputv[1] << endl;
}

void BaseRegressorWrapper::computeOutputAndCosts(const Vec& inputv, const Vec& targetv, Vec& outputv, Vec& costsv) const
{
    computeOutput(inputv, outputv);
    computeCostsFromOutputs(inputv, outputv, targetv, costsv);
}

void BaseRegressorWrapper::computeCostsFromOutputs(const Vec& inputv, const Vec& outputv, const Vec& targetv, Vec& costsv) const
{
    costsv[0] = pow((outputv[0] - targetv[0]), 2);
    costsv[1] = outputv[1];
    costsv[2] = 1.0 - (2.0 * loss_function_weight * costsv[0]);
}

void BaseRegressorWrapper::setSortedTrainSet(PP<RegressionTreeRegisters> the_sorted_train_set)
{
    sorted_train_set = the_sorted_train_set;
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
