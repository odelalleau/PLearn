// -*- C++ -*-

// ComputeDond2Target.cc
//
// Copyright (C) 2006 Dan Popovici, Pascal Lamblin
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

// Authors: Dan Popovici

/*! \file ComputeDond2Target.cc */

#define PL_LOG_MODULE_NAME "ComputeDond2Target"

#include "ComputeDond2Target.h"
#include <plearn/io/pl_log.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    ComputeDond2Target,
    "Computes the class target for the training and the test datasets.",
    "the program reorders the input variables to group them between the binary variables,\n"
    "the discrete variables and the continuous variables.\n"
    "It reorders the variables in the order specified by the input vector option.\n"
    "It also computes the predicted class target from the predicted annual sales figure\n"
    "of the financial institution and computes the real class target.\n"
);

/////////////////////////
// ComputeDond2Target //
/////////////////////////
ComputeDond2Target::ComputeDond2Target()
  : unknown_sales(0)
{
}
    
////////////////////
// declareOptions //
////////////////////
void ComputeDond2Target::declareOptions(OptionList& ol)
{
    declareOption(ol, "input_vector", &ComputeDond2Target::input_vector,
                  OptionBase::buildoption,
                  "The variables to assemble in the input vector by names.\n"
                  "To ease the following steps they are grouped with the binary variables first,\n"
                  "the discrete variables, the continuous variables and finally some variables unused in the training.\n");

    declareOption(ol, "unknown_sales", &ComputeDond2Target::unknown_sales,
                  OptionBase::buildoption,
                  "If set to 1 and annual sales attribute is missing, the class will be set to missing.");

    declareOption(ol, "target_sales", &ComputeDond2Target::target_sales,
                  OptionBase::buildoption,
                  "The column of the real annual sales used to compute the real class target.");

    declareOption(ol, "predicted_sales", &ComputeDond2Target::predicted_sales,
                  OptionBase::buildoption,
                  "The column of the predicted annual sales used to compute the predicted class target.");

    declareOption(ol, "margin", &ComputeDond2Target::margin,
                  OptionBase::buildoption,
                  "The column of the total authorized margins including SLA.");

    declareOption(ol, "loan", &ComputeDond2Target::loan,
                  OptionBase::buildoption,
                  "The column of the total loan balances excluding mortgages.");

    declareOption(ol, "output_path", &ComputeDond2Target::output_path,
                  OptionBase::buildoption,
                  "The file path for the targeted output file.");

    inherited::declareOptions(ol);
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void ComputeDond2Target::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    deepCopyField(input_vector, copies);
    deepCopyField(unknown_sales, copies);
    deepCopyField(target_sales, copies);
    deepCopyField(predicted_sales, copies);
    deepCopyField(margin, copies);
    deepCopyField(loan, copies);
    deepCopyField(output_path, copies);
    inherited::makeDeepCopyFromShallowCopy(copies);

}

///////////
// build //
///////////
void ComputeDond2Target::build()
{
    // ### Nothing to add here, simply calls build_().
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void ComputeDond2Target::build_()
{
    MODULE_LOG << "build_() called" << endl;
    if (train_set)
    {
        computeTarget();
    }
}

void ComputeDond2Target::computeTarget()
{    
    // initialize primary dataset
    main_row = 0;
    main_col = 0;
    main_length = train_set->length();
    main_width = train_set->width();
    main_input.resize(main_width);
    main_names.resize(main_width);
    ins_width = input_vector.size();
    predicted_class = ins_width;
    target_class = ins_width + 1;
    output_width = ins_width + 2;
    output_variable_src.resize(ins_width);
    output_names.resize(output_width);
    output_vec.resize(output_width);
    main_names << train_set->fieldNames();
    for (ins_col = 0; ins_col < ins_width; ins_col++)
    {
        for (main_col = 0; main_col < main_width; main_col++)
        {
            if (input_vector[ins_col] == main_names[main_col]) break;
        }
        if (main_col >= main_width) PLERROR("In ComputeDond2Target: no field with this name in input dataset: %", (input_vector[ins_col]).c_str());
        output_variable_src[ins_col] = main_col;
        output_names[ins_col] = input_vector[ins_col];
    }
    output_names[predicted_class] = "CLASSE_PRED";
    output_names[target_class] = "CLASSE_REEL";
    
    // initialize output datasets
    output_file = new FileVMatrix(output_path + ".pmat", main_length, output_names);
    output_file->defineSizes(output_width, 0, 0);
    
    //Now, we can group the input and compute the class target
    ProgressBar* pb = 0;
    pb = new ProgressBar( "Computing target classes", main_length);
    for (main_row = 0; main_row < main_length; main_row++)
    {
        train_set->getRow(main_row, main_input);
        for (ins_col = 0; ins_col < ins_width; ins_col++)
        {
            output_vec[ins_col] = main_input[output_variable_src[ins_col]];
        }
        if (is_missing(main_input[predicted_sales])) main_input[predicted_sales] = 0.0;
        commitment = 0.0;
        if (!is_missing(main_input[margin])) commitment += main_input[margin];
        if (!is_missing(main_input[loan])) commitment += main_input[loan];
        if (main_input[predicted_sales] < 1000000.0 && commitment < 200000.0) output_vec[predicted_class] = 1.0;
        else if (main_input[predicted_sales] < 10000000.0 && commitment < 1000000.0) output_vec[predicted_class] = 2.0;
        else if (main_input[predicted_sales] < 100000000.0 && commitment < 20000000.0) output_vec[predicted_class] = 3.0;
        else output_vec[predicted_class] = 4.0;
        if (is_missing(main_input[target_sales]) && unknown_sales == 0)
            PLERROR("In ComputeDond2Target: no target information for record: %i", main_row);
        commitment = 0.0;
        if (!is_missing(main_input[margin])) commitment += main_input[margin];
        if (!is_missing(main_input[loan])) commitment += main_input[loan];
        if (is_missing(main_input[target_sales]))  output_vec[target_class] = main_input[target_sales];
        else if (main_input[target_sales] < 1000000.0 && commitment < 200000.0) output_vec[target_class] = 1.0;
        else if (main_input[target_sales] < 10000000.0 && commitment < 1000000.0) output_vec[target_class] = 2.0;
        else if (main_input[target_sales] < 100000000.0 && commitment < 20000000.0) output_vec[target_class] = 3.0;
        else output_vec[target_class] = 4.0;
        output_file->putRow(main_row, output_vec);
        pb->update( main_row );
    }
    delete pb;
}

VMat ComputeDond2Target::getOutputFile()
{
    return output_file;
}

int ComputeDond2Target::outputsize() const {return 0;}
void ComputeDond2Target::train()
{
    PLERROR("ComputeDond2Target: we are done here");
}
void ComputeDond2Target::computeOutput(const Vec&, Vec&) const {}
void ComputeDond2Target::computeCostsFromOutputs(const Vec&, const Vec&, const Vec&, Vec&) const {}
TVec<string> ComputeDond2Target::getTestCostNames() const
{
    TVec<string> result;
    result.append( "MSE" );
    return result;
}
TVec<string> ComputeDond2Target::getTrainCostNames() const
{
    TVec<string> result;
    result.append( "MSE" );
    return result;
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
