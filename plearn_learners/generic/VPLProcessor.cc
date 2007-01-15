// -*- C++ -*-

// VPLProcessor.cc
//
// Copyright (C) 2005, 2006 Pascal Vincent 
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

/* *******************************************************      
 * $Id: VPLProcessor.cc 5480 2006-05-03 18:57:39Z plearner $ 
 ******************************************************* */

// Authors: Pascal Vincent

/*! \file VPLProcessor.cc */

#include <sstream>

#include "VPLProcessor.h"
#include <plearn/vmat/ProcessingVMatrix.h>
#include <plearn/vmat/FilteredVMatrix.h>
#include <plearn/base/tostring.h>

namespace PLearn {
using namespace std;

VPLProcessor::VPLProcessor() 
    :orig_inputsize(-1),
     orig_targetsize(-1),
     use_filtering_prg_for_repeat(false),
     repeat_id_field_name(""),
     repeat_count_field_name("")
{
}

PLEARN_IMPLEMENT_OBJECT(
    VPLProcessor,
    "Learner whose training-set, inputs and outputs can be pre/post-processed by VPL code",
    "See VMatLanguage for the definition of the allowed VPL syntax."
    );

void VPLProcessor::declareOptions(OptionList& ol)
{
    declareOption(ol, "filtering_prg", &VPLProcessor::filtering_prg, OptionBase::buildoption,
                  "Optional program string in VPL language to apply as filtering on the training VMat.\n"
                  "This program is to produce a single value interpreted as a boolean: only the rows for which\n"
                  "it evaluates to non-zero will be kept.\n"
                  "An empty string means NO FILTERING.");

    declareOption(ol, "input_prg", &VPLProcessor::input_prg, OptionBase::buildoption,
                  "Program string in VPL language to be applied to each raw input \n"
                  "to generate the new preprocessed input.\n"
                  "Note that names must be given to the generated values with :fieldname VPL syntax.\n"
                  "An empty string means NO PREPROCESSING. (initial raw input is used as is)");

    declareOption(ol, "target_prg", &VPLProcessor::target_prg, OptionBase::buildoption,
                  "Program string in VPL language to be applied to a dataset row\n"
                  "to generate a proper target.\n"
                  "Note that names must be given to the generated values with :fieldname VPL syntax.\n"
                  "If it's an empty string, then we'll use the original target from the data set");
  
    declareOption(ol, "weight_prg", &VPLProcessor::weight_prg, OptionBase::buildoption,
                  "Program string in VPL language to be applied to a dataset row\n"
                  "to generate a proper weight.\n"
                  "Note that names must be given to the generated values with :fieldname VPL syntax.\n"
                  "If it's an empty string, then we'll use the original weight from the data set");

    declareOption(ol, "extra_prg", &VPLProcessor::extra_prg, OptionBase::buildoption,
                  "Program string in VPL language to be applied to a dataset row\n"
                  "to generate proper extra fields.\n"
                  "Note that names must be given to the generated values with :fieldname VPL syntax.\n"
                  "If it's an empty string, then we'll use the original extra fields from the data set");


    declareOption(ol, "use_filtering_prg_for_repeat", &VPLProcessor::use_filtering_prg_for_repeat, OptionBase::buildoption,
                  "When true, the result of the filtering program indicates the number of times a row should be repeated (0..n).\n"
                  "(sets FilteredVMatrix::allow_repeat_rows.)");

    declareOption(ol, "repeat_id_field_name", &VPLProcessor::repeat_id_field_name, OptionBase::buildoption,
                  "Field name for the repetition id (0, 1, ..., n-1).  No field is added if empty.");

    declareOption(ol, "repeat_count_field_name", &VPLProcessor::repeat_count_field_name, OptionBase::buildoption,
                  "Field name for the number of repetitions (n).  No field is added if empty.");

    // learnt

    declareOption(ol, "orig_fieldnames", &VPLProcessor::orig_fieldnames, OptionBase::learntoption,
                  "original fieldnames of the training set");
    declareOption(ol, "orig_inputsize", &VPLProcessor::orig_inputsize, OptionBase::learntoption,
                  "original inputsize of the training set");
    declareOption(ol, "orig_targetsize", &VPLProcessor::orig_targetsize, OptionBase::learntoption,
                  "original targetsize of the training set");


    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void VPLProcessor::build_()
{
    if(train_set.isNull() && (orig_inputsize>0 || orig_targetsize>0) ) // we're probably reloading a saved VPLProcessor
        initializeInputPrograms();

}

void VPLProcessor::build()
{
    inherited::build();
    build_();
}


void VPLProcessor::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields 
    // ### that you wish to be deepCopied rather than 
    // ### shallow-copied.

    input_prg_.makeDeepCopyFromShallowCopy(copies);
 

    deepCopyField(input_prg_fieldnames, copies);
    deepCopyField(processed_input, copies);
    deepCopyField(orig_fieldnames, copies);
}

int VPLProcessor::outputsize() const
{
    if(!input_prg.empty())
        return input_prg_fieldnames.length();

    return inputsize();
}

void VPLProcessor::forget()
{
    inherited::forget();
    stage = 0;
}
    
void VPLProcessor::initializeInputPrograms()
{
    if(!input_prg.empty())
    {
        //input_prg_.setSourceFieldNames(orig_fieldnames.subVec(0,orig_inputsize));
        input_prg_.setSourceFieldNames(orig_fieldnames);
        input_prg_.compileString(input_prg, input_prg_fieldnames);
    }
    else
    {
        input_prg_.clear();
        input_prg_fieldnames.resize(0);
    }

}

/// @todo Check if this method works according to the documentation. For
/// example, the next to last line (VMatr processed_trainset = new ...) has 
/// no impact.
void VPLProcessor::setTrainingSet(VMat training_set, bool call_forget)
{
    orig_fieldnames = training_set->fieldNames();
    orig_inputsize  = training_set->inputsize();
    orig_targetsize  = training_set->targetsize();
    initializeInputPrograms();

    VMat filtered_trainset = training_set;
    PPath filtered_trainset_metadatadir = getExperimentDirectory() / "filtered_train_set.metadata";
    if(!filtering_prg.empty())
        filtered_trainset = new FilteredVMatrix(training_set, filtering_prg, filtered_trainset_metadatadir, verbosity>1,
                                                use_filtering_prg_for_repeat, repeat_id_field_name, repeat_count_field_name);

    VMat processed_trainset = new ProcessingVMatrix(filtered_trainset, input_prg, target_prg, weight_prg, extra_prg);
    inherited::setTrainingSet(training_set, call_forget); // will call forget if needed
}

VMat VPLProcessor::processDataSet(VMat dataset) const
{
    VMat filtered_dataset = dataset;
    PPath filtered_dataset_metadatadir = getExperimentDirectory() / "filtered_dataset.metadata";
    if(!filtering_prg.empty())
        filtered_dataset = new FilteredVMatrix(dataset, filtering_prg, filtered_dataset_metadatadir, verbosity>1,
                                               use_filtering_prg_for_repeat, repeat_id_field_name, repeat_count_field_name);

    // Since ProcessingVMatrix produces 0 length vectors when given an empty
    // program (which is not the behavior that VPLProcessors is documented as
    // implementing), we need to replace each program that is an empty string
    // by a small VPL snippet that copies all the fields for the input or
    // target, etc.

    // First compute the start of each section (input, target, etc.) in the
    // columns of the dataset.
    const int start_of_targets = dataset->inputsize();
    const int start_of_weights = start_of_targets + dataset->targetsize();
    const int start_of_extras = start_of_weights + dataset->weightsize();

    // Now compute each processing_*_prg program.
    string processing_input_prg = input_prg;
    if (processing_input_prg.empty() && dataset->inputsize() > 0) {
        processing_input_prg = "[%0:%" + tostring(start_of_targets-1) + "]";
    }
    
    string processing_target_prg = target_prg;
    if (processing_target_prg.empty() && dataset->targetsize() > 0) {
        processing_target_prg = "[%" + tostring(start_of_targets) + ":%" + tostring(start_of_weights-1) + "]";
    }

    string processing_weight_prg = weight_prg;
    if (processing_weight_prg.empty() && dataset->weightsize() > 0) {
        processing_weight_prg = "[%" + tostring(start_of_weights) + ":%" + tostring(start_of_extras-1) + "]";
    }

    string processing_extras_prg = extra_prg;
    if (processing_extras_prg.empty() && dataset->extrasize() > 0) {
        processing_extras_prg = "[%" + tostring(start_of_extras) + ":END]";
    }
    
    return new ProcessingVMatrix(filtered_dataset, processing_input_prg,
                                 processing_target_prg, processing_weight_prg,
                                 processing_extras_prg);
}

void VPLProcessor::computeOutput(const Vec& input, Vec& output) const
{
    output.resize(outputsize());
    Vec newinput= input;
    if(!input_prg.empty())
    {
        processed_input.resize(input_prg_fieldnames.length());
        input_prg_.run(input, processed_input);
        newinput= processed_input;
    }

    output << newinput;
}

void VPLProcessor::computeOutputAndCosts(const Vec& input, const Vec& target, 
                                                   Vec& output, Vec& costs) const
{ 
    output.resize(outputsize());
    costs.resize(nTestCosts());

    costs.fill(-1);
    return computeOutput(input, output);
}

void VPLProcessor::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                                     const Vec& target, Vec& costs) const
{ 
    Vec nonconst_output = output; // to make the constipated compiler happy
    computeOutputAndCosts(input, target, nonconst_output, costs); 
}

TVec<string> VPLProcessor::getOutputNames() const
{
    if(!input_prg.empty())//output_prg_)
        return input_prg_fieldnames;

    VMat trainset= getTrainingSet();
    if(trainset==0)
        PLERROR("in VPLProcessor::getOutputNames: no train set specified yet.");

    return trainset->inputFieldNames();
}


void VPLProcessor::train()
{}

TVec<std::string> VPLProcessor::getTestCostNames() const
{return TVec<string>(0);}

TVec<string> VPLProcessor::getTrainCostNames() const
{return TVec<string>(0);}



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
