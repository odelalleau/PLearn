// -*- C++ -*-

// SecondIterationWrapper.cc
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
 * $Id: SecondIterationWrapper.cc, v 1.0 2004/07/19 10:00:00 Bengio/Kegl/Godbout        *
 * This file is part of the PLearn library.                                     *
 ******************************************************************************** */

#include "SecondIterationWrapper.h"
#include <plearn/vmat/FileVMatrix.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(SecondIterationWrapper,
                        "A PLearner to wrap around a generic regressor to calculate the predicted class.", 
                        "Algorithm built to wrap around a generic regressor in the context of the second iteration\n"
                        "of the annual sales estimation project.\n"
                        "It calculates the predicted class and computes the cse error. It do not work with classifier\n"
    );

SecondIterationWrapper::SecondIterationWrapper()  
  : class_prediction(1)
{
}

SecondIterationWrapper::~SecondIterationWrapper()
{
}

void SecondIterationWrapper::declareOptions(OptionList& ol)
{ 
    declareOption(ol, "class_prediction", &SecondIterationWrapper::class_prediction, OptionBase::buildoption,
                  "When set to 1 (default), indicates the base regression is on the class target.\n"
                  "Otherwise, we assume the regression is on the sales target.\n"); 
 
    declareOption(ol, "base_regressor_template", &SecondIterationWrapper::base_regressor_template, OptionBase::buildoption,
                  "The template for the base regressor to be used.\n");  
 
    declareOption(ol, "ref_train", &SecondIterationWrapper::ref_train, OptionBase::buildoption,
                  "The reference set to compute train statistics.\n");  
 
    declareOption(ol, "ref_test", &SecondIterationWrapper::ref_test, OptionBase::buildoption,
                  "The reference set to compute test statistice.\n");  
 
    declareOption(ol, "ref_sales", &SecondIterationWrapper::ref_sales, OptionBase::buildoption,
                  "The reference set to de-gaussianize the prediction.\n"); 
 
    declareOption(ol, "train_dataset", &SecondIterationWrapper::train_dataset, OptionBase::buildoption,
                  "The train data set.\n"); 
 
    declareOption(ol, "test_dataset", &SecondIterationWrapper::test_dataset, OptionBase::buildoption,
                  "The test data set.\n"); 
      
    declareOption(ol, "base_regressor", &SecondIterationWrapper::base_regressor, OptionBase::learntoption,
                  "The base regressor built from the template.\n");
    inherited::declareOptions(ol);
}

void SecondIterationWrapper::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(class_prediction, copies);
    deepCopyField(ref_train, copies);
    deepCopyField(ref_test, copies);
    deepCopyField(ref_sales, copies);
    deepCopyField(test_dataset, copies);
    deepCopyField(base_regressor_template, copies);
    deepCopyField(base_regressor, copies);
}

void SecondIterationWrapper::build()
{
    inherited::build();
    build_();
}

void SecondIterationWrapper::build_()
{
    if (train_set)
    {
        if (class_prediction == 0)
            if(!ref_train || !ref_test || !ref_test || !train_dataset || !test_dataset)
                PLERROR("In SecondIterationWrapper: missing reference data sets to compute statistics");
        margin = 0;
        loan = 1;
        sales = 2;
        tclass = 3;
        base_regressor = ::PLearn::deepCopy(base_regressor_template);
        base_regressor->setTrainingSet(train_set, true);
        base_regressor->setTrainStatsCollector(new VecStatsCollector);
        if (class_prediction == 0) search_table = ref_sales->toMat();
        sample_input.resize(train_set->inputsize());
        sample_target.resize(train_set->targetsize());
        sample_output.resize(base_regressor->outputsize());
        sample_costs.resize(4);
    }
}

void SecondIterationWrapper::train()
{
    base_regressor->nstages = nstages;
    base_regressor->train();
    if (class_prediction == 1) computeClassStatistics();
    else computeSalesStatistics();
}

void SecondIterationWrapper::computeClassStatistics()
{
    real sample_weight;
    ProgressBar* pb = NULL;
    if (report_progress)
        pb = new ProgressBar("Second Iteration : computing the train statistics: ",
                             train_set->length());

    train_stats->forget();
    for (int row = 0; row < train_set->length(); row++)
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

void SecondIterationWrapper::computeSalesStatistics()
{
    int row;
    Vec sample_input(train_set->inputsize());
    Vec sample_target(train_set->targetsize());
    Vec reference_vector(ref_train->width());
    real sample_weight;
    Vec sample_output(base_regressor->outputsize());
    Vec sample_costs(4);
    Vec train_mean(3);
    Vec train_std_error(3);
    Vec valid_mean(3);
    Vec valid_std_error(3);
    Vec test_mean(3);
    Vec test_std_error(3);
    real sales_prediction;
    real commitment;
    real predicted_class;
    ProgressBar* pb = NULL;
    if (report_progress)
    {
        pb = new ProgressBar("Second Iteration : computing the train statistics: ", train_set->length());
    } 
    train_stats->forget();
    for (row = 0; row < train_set->length(); row++)
    {  
        train_set->getExample(row, sample_input, sample_target, sample_weight);
        ref_train->getRow(row, reference_vector);
        computeOutput(sample_input, sample_output);
        sales_prediction = deGaussianize(sample_output[0]);
        commitment = 0.0;
        if (!is_missing(reference_vector[margin])) commitment += reference_vector[margin];
        if (!is_missing(reference_vector[loan])) commitment += reference_vector[loan];
        if (sales_prediction < 1000000.0 && commitment < 200000.0) predicted_class = 1.0;
        else if (sales_prediction < 10000000.0 && commitment < 1000000.0) predicted_class = 2.0;
        else if (sales_prediction < 100000000.0 && commitment < 20000000.0) predicted_class = 3.0;
        else predicted_class = 4.0;
        sample_costs[0] = pow((sales_prediction - reference_vector[sales]), 2);
        sample_costs[1] = pow((predicted_class - reference_vector[tclass]), 2);
        if (predicted_class == reference_vector[tclass]) sample_costs[2] = 0.0;
        else sample_costs[2] = 1.0;
        train_stats->update(sample_costs);
        if (report_progress) pb->update(row);
    }
    train_stats->finalize();
    train_mean << train_stats->getMean();
    train_std_error << train_stats->getStdError();
    if (report_progress) delete pb; 
    if (report_progress)
    {
        pb = new ProgressBar("Second Iteration : computing the valid statistics: ", train_dataset->length() - train_set->length());
    } 
    train_stats->forget();
    for (row = train_set->length(); row < train_dataset->length(); row++)
    {  
        train_dataset->getExample(row, sample_input, sample_target, sample_weight);
        ref_train->getRow(row, reference_vector);
        computeOutput(sample_input, sample_output);
        sales_prediction = deGaussianize(sample_output[0]);
        commitment = 0.0;
        if (!is_missing(reference_vector[margin])) commitment += reference_vector[margin];
        if (!is_missing(reference_vector[loan])) commitment += reference_vector[loan];
        if (sales_prediction < 1000000.0 && commitment < 200000.0) predicted_class = 1.0;
        else if (sales_prediction < 10000000.0 && commitment < 1000000.0) predicted_class = 2.0;
        else if (sales_prediction < 100000000.0 && commitment < 20000000.0) predicted_class = 3.0;
        else predicted_class = 4.0;
        sample_costs[0] = pow((sales_prediction - reference_vector[sales]), 2);
        sample_costs[1] = pow((predicted_class - reference_vector[tclass]), 2);
        if (predicted_class == reference_vector[tclass]) sample_costs[2] = 0.0;
        else sample_costs[2] = 1.0;
        train_stats->update(sample_costs);
        if (report_progress) pb->update(row);
    }
    train_stats->finalize();
    valid_mean << train_stats->getMean();
    valid_std_error << train_stats->getStdError();
    if (report_progress) delete pb; 
    if (report_progress)
    {
        pb = new ProgressBar("Second Iteration : computing the test statistics: ", test_dataset->length());
    } 
    train_stats->forget();
    for (row = 0; row < test_dataset->length(); row++)
    {  
        test_dataset->getExample(row, sample_input, sample_target, sample_weight);
        ref_test->getRow(row, reference_vector);
        computeOutput(sample_input, sample_output);
        sales_prediction = deGaussianize(sample_output[0]);
        commitment = 0.0;
        if (!is_missing(reference_vector[margin])) commitment += reference_vector[margin];
        if (!is_missing(reference_vector[loan])) commitment += reference_vector[loan];
        if (sales_prediction < 1000000.0 && commitment < 200000.0) predicted_class = 1.0;
        else if (sales_prediction < 10000000.0 && commitment < 1000000.0) predicted_class = 2.0;
        else if (sales_prediction < 100000000.0 && commitment < 20000000.0) predicted_class = 3.0;
        else predicted_class = 4.0;
        sample_costs[0] = pow((sales_prediction - reference_vector[sales]), 2);
        sample_costs[1] = pow((predicted_class - reference_vector[tclass]), 2);
        if (predicted_class == reference_vector[tclass]) sample_costs[2] = 0.0;
        else sample_costs[2] = 1.0;
        train_stats->update(sample_costs);
        if (report_progress) pb->update(row);
    }
    train_stats->finalize();
    test_mean << train_stats->getMean();
    test_std_error << train_stats->getStdError();
    if (report_progress) delete pb;
    TVec<string> stat_names(6);
    stat_names[0] = "mse";
    stat_names[1] = "mse_stderr";
    stat_names[2] = "cse";
    stat_names[3] = "cse_stderr";
    stat_names[4] = "cle";
    stat_names[5] = "cle_stderr";
    VMat stat_file = new FileVMatrix(expdir + "class_stats.pmat", 3, stat_names);
    stat_file->put(0, 0, train_mean[0]);
    stat_file->put(0, 1, train_std_error[0]);
    stat_file->put(0, 2, train_mean[1]);
    stat_file->put(0, 3, train_std_error[1]);
    stat_file->put(0, 4, train_mean[2]);
    stat_file->put(0, 5, train_std_error[2]);
    stat_file->put(1, 0, valid_mean[0]);
    stat_file->put(1, 1, valid_std_error[0]);
    stat_file->put(1, 2, valid_mean[1]);
    stat_file->put(1, 3, valid_std_error[1]);
    stat_file->put(1, 4, valid_mean[2]);
    stat_file->put(1, 5, valid_std_error[2]);
    stat_file->put(2, 0, test_mean[0]);
    stat_file->put(2, 1, test_std_error[0]);
    stat_file->put(2, 2, test_mean[1]);
    stat_file->put(2, 3, test_std_error[1]);
    stat_file->put(2, 4, test_mean[2]);
    stat_file->put(2, 5, test_std_error[2]);
}

real SecondIterationWrapper::deGaussianize(real prediction)
{
    if (prediction < search_table(0, 0)) return search_table(0, 1);
    if (prediction > search_table(search_table.length() - 1, 0)) return search_table(search_table.length() - 1, 1);
    int mid;
    int min = 0;
    int max = search_table.length() - 1;
    while (max - min > 1)
    {
        mid = (max + min) / 2;
        real mid_val = search_table(mid, 0);
        if (prediction < mid_val) max = mid;
        else if (prediction > mid_val) min = mid;
        else min = max = mid;
    }
    if (min == max) return search_table(min, 1);
    return (search_table(min, 1) + search_table(max, 1)) / 2.0;
}

void SecondIterationWrapper::forget()
{
}

int SecondIterationWrapper::outputsize() const
{
    return base_regressor?base_regressor->outputsize():-1;
}

TVec<string> SecondIterationWrapper::getTrainCostNames() const
{
    TVec<string> return_msg(4);
    return_msg[0] = "mse";
    return_msg[1] = "square_class_error";
    return_msg[2] = "linear_class_error";
    return_msg[3] = "class_error";
    return return_msg;
}

TVec<string> SecondIterationWrapper::getTestCostNames() const
{ 
    return getTrainCostNames();
}

void SecondIterationWrapper::computeOutput(const Vec& inputv, Vec& outputv) const
{
    base_regressor->computeOutput(inputv, outputv);
}

void SecondIterationWrapper::computeOutputAndCosts(const Vec& inputv, const Vec& targetv, Vec& outputv, Vec& costsv) const
{
    computeOutput(inputv, outputv);
    computeCostsFromOutputs(inputv, outputv, targetv, costsv);
}

void SecondIterationWrapper::computeCostsFromOutputs(const Vec& inputv, const Vec& outputv, const Vec& targetv, Vec& costsv) const
{
    costsv[0] = pow((outputv[0] - targetv[0]), 2);
    if (class_prediction == 1)
    {
        real class_pred;
        if (outputv[0] <= 0.5) class_pred = 0.;
        else if (outputv[0] <= 1.5) class_pred = 1.0;
        else if (outputv[0] <= 2.5) class_pred = 2.0;
        else class_pred = 3.0;
        costsv[1] = pow((class_pred - targetv[0]), 2);
        costsv[2] = fabs(class_pred - targetv[0]);
        costsv[3] = class_pred == targetv[0]?0:1;
        return;
    }
    costsv[1] = 0.0;
    costsv[2] = 0.0;
    costsv[3] = 0.0;
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
