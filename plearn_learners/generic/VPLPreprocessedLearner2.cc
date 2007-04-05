// -*- C++ -*-

// VPLPreprocessedLearner2.cc
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
 * $Id: VPLPreprocessedLearner2.cc 5480 2006-05-03 18:57:39Z plearner $ 
 ******************************************************* */

// Authors: Pascal Vincent

/*! \file VPLPreprocessedLearner2.cc */


#include "VPLPreprocessedLearner2.h"
#include <plearn/vmat/ProcessingVMatrix.h>
#include <plearn/vmat/FilteredVMatrix.h>
#include <plearn/base/tostring.h>

namespace PLearn {
using namespace std;

VPLPreprocessedLearner2::VPLPreprocessedLearner2() 
    :orig_inputsize(-1),
     orig_targetsize(-1),
     use_filtering_prg_for_repeat(false),
     repeat_id_field_name(""),
     repeat_count_field_name(""),
     ignore_test_costs(false)

{
}

PLEARN_IMPLEMENT_OBJECT(
    VPLPreprocessedLearner2,
    "Learner whose training-set, inputs and outputs can be pre/post-processed by VPL code",
    "See VMatLanguage for the definition of the allowed VPL syntax."
    );

void VPLPreprocessedLearner2::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify  
    // ### one of OptionBase::buildoption, OptionBase::learntoption or 
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    // ### ex:
    // declareOption(ol, "myoption", &VPLPreprocessedLearner2::myoption, OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    declareOption(ol, "learner", &VPLPreprocessedLearner2::learner_,
                  OptionBase::buildoption,
                  "The embedded learner");

    declareOption(ol, "filtering_prg", &VPLPreprocessedLearner2::filtering_prg, OptionBase::buildoption,
                  "Optional program string in VPL language to apply as filtering on the training VMat.\n"
                  "It's the resulting filtered training set that is passed to the underlying learner.\n"
                  "This program is to produce a single value interpreted as a boolean: only the rows for which\n"
                  "it evaluates to non-zero will be kept.\n"
                  "An empty string means NO FILTERING.");

    declareOption(ol, "input_prg", &VPLPreprocessedLearner2::input_prg, OptionBase::buildoption,
                  "Program string in VPL language to be applied to each raw input \n"
                  "to generate the new preprocessed input.\n"
                  "Note that names must be given to the generated values with :fieldname VPL syntax.\n"
                  "An empty string means NO PREPROCESSING. (initial raw input is used as is)");

    declareOption(ol, "target_prg", &VPLPreprocessedLearner2::target_prg, OptionBase::buildoption,
                  "Program string in VPL language to be applied to a dataset row\n"
                  "to generate a proper target for the underlying learner.\n"
                  "Note that names must be given to the generated values with :fieldname VPL syntax.\n"
                  "If it's an empty string, then we'll use the original target from the data set");
  
    declareOption(ol, "weight_prg", &VPLPreprocessedLearner2::weight_prg, OptionBase::buildoption,
                  "Program string in VPL language to be applied to a dataset row\n"
                  "to generate a proper weight for the underlying learner.\n"
                  "Note that names must be given to the generated values with :fieldname VPL syntax.\n"
                  "If it's an empty string, then we'll use the original weight from the data set");

    declareOption(ol, "extra_prg", &VPLPreprocessedLearner2::extra_prg, OptionBase::buildoption,
                  "Program string in VPL language to be applied to a dataset row\n"
                  "to generate proper extra fields for the underlying learner.\n"
                  "Note that names must be given to the generated values with :fieldname VPL syntax.\n"
                  "If it's an empty string, then we'll use the original extra fields from the data set");

    declareOption(ol, "output_prg", &VPLPreprocessedLearner2::output_prg, OptionBase::buildoption,
                  "Program string in VPL language to obtain postprocessed output\n"
                  "from a concatenation of the raw input fields and the underlying learner's outputs\n"
                  "The underlying learner's outputs are typically named out0, out1, out2, ...\n"
                  "Note that outputnames must be given to the generated values with :fieldname VPL syntax.\n"
                  "If it's an empty string, then we'll output the underlying learner's outputs.\n");

    declareOption(ol, "costs_prg", &VPLPreprocessedLearner2::costs_prg, OptionBase::buildoption,
                  "Program string in VPL language to obtain postprocessed test costs\n"
                  "from a concatenation of the raw input fields and target fields, \n"
                  "and the underlying learner's outputs and test costs.\n"
                  "Note that names must be given to the generated values with :fieldname VPL syntax.\n"
                  "If it's an empty string, then we'll output the underlying learner's test costs.\n"
                  "Note that this processing is only applied to test costs, not to train costs which are returned as is.");

    declareOption(ol, "orig_fieldnames", &VPLPreprocessedLearner2::orig_fieldnames, OptionBase::learntoption,
                  "original fieldnames of the training set");
    declareOption(ol, "orig_inputsize", &VPLPreprocessedLearner2::orig_inputsize, OptionBase::learntoption,
                  "original inputsize of the training set");
    declareOption(ol, "orig_targetsize", &VPLPreprocessedLearner2::orig_targetsize, OptionBase::learntoption,
                  "original targetsize of the training set");


    declareOption(ol, "use_filtering_prg_for_repeat", &VPLPreprocessedLearner2::use_filtering_prg_for_repeat, OptionBase::buildoption,
                  "When true, the result of the filtering program indicates the number of times a row should be repeated (0..n).\n"
                  "(sets FilteredVMatrix::allow_repeat_rows.)");

    declareOption(ol, "repeat_id_field_name", &VPLPreprocessedLearner2::repeat_id_field_name, OptionBase::buildoption,
                  "Field name for the repetition id (0, 1, ..., n-1).  No field is added if empty.");

    declareOption(ol, "repeat_count_field_name", &VPLPreprocessedLearner2::repeat_count_field_name, OptionBase::buildoption,
                  "Field name for the number of repetitions (n).  No field is added if empty.");

    declareOption(ol, "ignore_test_costs", &VPLPreprocessedLearner2::ignore_test_costs, OptionBase::buildoption,
                  "WARNING: THIS IS AN UGLY HACK!!\n"
                  "When set to true, computeOutputAndCosts will simply call computeOutput and return bogus costs.");




    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void VPLPreprocessedLearner2::build_()
{
    if(train_set.isNull() && (orig_inputsize>0 || orig_targetsize>0) ) // we're probably reloading a saved VPLPreprocessedLearner2
    {
        initializeInputPrograms();
        initializeOutputPrograms();
    }
    else if(!costs_prg.empty())
        VMatLanguage::getOutputFieldNamesFromString(costs_prg, costs_prg_fieldnames);
}

// ### Nothing to add here, simply calls build_
void VPLPreprocessedLearner2::build()
{
    inherited::build();
    build_();
}


void VPLPreprocessedLearner2::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields 
    // ### that you wish to be deepCopied rather than 
    // ### shallow-copied.

    deepCopyField(learner_, copies);    

    input_prg_.makeDeepCopyFromShallowCopy(copies);
    target_prg_.makeDeepCopyFromShallowCopy(copies);
    weight_prg_.makeDeepCopyFromShallowCopy(copies);
    extra_prg_.makeDeepCopyFromShallowCopy(copies);
    output_prg_.makeDeepCopyFromShallowCopy(copies);
    costs_prg_.makeDeepCopyFromShallowCopy(copies);
 
    deepCopyField(input_prg_fieldnames, copies);
    deepCopyField(target_prg_fieldnames, copies);
    deepCopyField(weight_prg_fieldnames, copies);
    deepCopyField(extra_prg_fieldnames, copies);
    deepCopyField(output_prg_fieldnames, copies);
    deepCopyField(costs_prg_fieldnames, copies);
    deepCopyField(row, copies);
    deepCopyField(processed_input, copies);
    deepCopyField(processed_target, copies);
    deepCopyField(processed_weight, copies);
    deepCopyField(processed_extra, copies);
    deepCopyField(pre_output, copies);
    deepCopyField(pre_costs, copies);
}

void VPLPreprocessedLearner2::setValidationSet(VMat validset)
{
    PLASSERT( learner_ );
    inherited::setValidationSet(validset);
    learner_->setValidationSet(validset);
}

void VPLPreprocessedLearner2::setTrainStatsCollector(PP<VecStatsCollector> statscol)
{
    PLASSERT( learner_ );
    inherited::setTrainStatsCollector(statscol);
    learner_->setTrainStatsCollector(statscol);
}

int VPLPreprocessedLearner2::outputsize() const
{
    if(!output_prg.empty())
        return output_prg_fieldnames.length();
    else
    {
        PLASSERT( learner_ );
        return learner_->outputsize();
    }
}

void VPLPreprocessedLearner2::setExperimentDirectory(const PPath& the_expdir)
{
    PLASSERT( learner_ );
    inherited::setExperimentDirectory(the_expdir);
    learner_->setExperimentDirectory(the_expdir);
}

void VPLPreprocessedLearner2::forget()
{
    PLASSERT( learner_);
    learner_->forget();
    stage = 0;
}
    
void VPLPreprocessedLearner2::train()
{
    PLASSERT( learner_ );
    learner_->train();
    stage = learner_->stage;
}

void VPLPreprocessedLearner2::initializeInputPrograms()
{
    if(!input_prg.empty())
    {
        input_prg_.setSourceFieldNames(orig_fieldnames.subVec(0,orig_inputsize));
        input_prg_.compileString(input_prg, input_prg_fieldnames);
    }
    else
    {
        input_prg_.clear();
        input_prg_fieldnames.resize(0);
    }

    if(!target_prg.empty() && !ignore_test_costs)
    {
        target_prg_.setSourceFieldNames(orig_fieldnames);
        target_prg_.compileString(target_prg, target_prg_fieldnames);
    }
    else
    {
        target_prg_.clear();
        target_prg_fieldnames.resize(0);
    }

    if(!weight_prg.empty() && !ignore_test_costs)
    {
        weight_prg_.setSourceFieldNames(orig_fieldnames);
        weight_prg_.compileString(weight_prg, weight_prg_fieldnames);
    }
    else
    {
        weight_prg_.clear();
        weight_prg_fieldnames.resize(0);
    }

    if(!extra_prg.empty())
    {
        extra_prg_.setSourceFieldNames(orig_fieldnames);
        extra_prg_.compileString(extra_prg, extra_prg_fieldnames);
    }
    else
    {
        extra_prg_.clear();
        extra_prg_fieldnames.resize(0);
    }

}

void VPLPreprocessedLearner2::initializeOutputPrograms()
{
    TVec<string> orig_input_fieldnames = orig_fieldnames.subVec(0,orig_inputsize);
    TVec<string> orig_target_fieldnames = orig_fieldnames.subVec(orig_inputsize, orig_targetsize);

    if(!output_prg.empty())
    {
        output_prg_.setSourceFieldNames(concat(orig_input_fieldnames,learner_->getOutputNames()) );
        output_prg_.compileString(output_prg, output_prg_fieldnames);
    }
    else
    {
        output_prg_.clear();
        output_prg_fieldnames.resize(0);
    }

    if(!costs_prg.empty())
    {
        costs_prg_.setSourceFieldNames(concat(orig_input_fieldnames,orig_target_fieldnames,learner_->getOutputNames(),learner_->getTestCostNames()) );
        costs_prg_.compileString(costs_prg, costs_prg_fieldnames);
    }
    else
    {
        costs_prg_.clear();
        costs_prg_fieldnames.resize(0);
    }
}

void VPLPreprocessedLearner2::setTrainingSet(VMat training_set, bool call_forget)
{
    PLASSERT( learner_ );

    bool training_set_has_changed = !train_set || !(train_set->looksTheSameAs(training_set));
    if (call_forget && !training_set_has_changed)
        // In this case, learner_->build() will not have been called, which may
        // cause trouble if it updates data from the training set.
        learner_->build();

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
    learner_->setTrainingSet(processed_trainset, false);
    inherited::setTrainingSet(training_set, call_forget); // will call forget if needed

    initializeOutputPrograms();
}

/*
void VPLPreprocessedLearner2::test(VMat testset, PP<VecStatsCollector> test_stats, VMat testoutputs, VMat testcosts) const
{

    inherited::test(testset, test_stats, testoutputs, testcosts);
*/
/*
    VMat filtered_testset = testset;
    PPath filtered_testset_metadatadir = getExperimentDirectory() / "filtered_test_set.metadata";

    // DO NOT FILTER THE TESTSET
    //if(!filtering_prg.empty())
        //filtered_testset = new FilteredVMatrix(testset, filtering_prg, filtered_testset_metadatadir, verbosity>1);

    VMat processed_testset = new ProcessingVMatrix(filtered_testset, input_prg, target_prg, weight_prg, extra_prg);

    int l = processed_testset.length();
    Vec input;
    Vec target;
    real weight;
    Vec proc_input;
    Vec proc_target;
    real proc_weight;

    Vec output(outputsize());

    Vec costs(nTestCosts());

    // testset->defineSizes(inputsize(),targetsize(),weightsize());

    PP<ProgressBar> pb;
    if(report_progress) 
        pb = new ProgressBar("Testing learner",l);

    if (l == 0) {
        // Empty test set: we give -1 cost arbitrarily.
        costs.fill(-1);
        test_stats->update(costs);
    }


    perr << "VPLPreprocessedLearner2::test class=" << this->classname()
         << "\tl=" << l 
         << "\tinputsize=" << processed_testset->inputsize() 
         << "\ttargetsize=" << processed_testset->targetsize() 
         << "\tweightsize=" << processed_testset->weightsize() 
         << endl;

    for(int i=0; i<l; i++)
    {
        processed_testset.getExample(i, proc_input, proc_target, proc_weight);
        filtered_testset.getExample(i, input, target, weight);
      
        // Always call computeOutputAndCosts, since this is better
        // behaved with stateful learners
        pre_costs.resize(learner_->nTestCosts());
        learner_->computeOutputAndCosts(proc_input,proc_target,pre_output,pre_costs);

        if(!output_prg.empty())
            output_prg_.run(concat(input,pre_output), output);
        else
            output << pre_output;

        if(!costs_prg.empty())
            costs_prg_.run(concat(input,target,pre_output,pre_costs), costs);
        else
            costs << pre_costs;
      
        if(testoutputs)
            testoutputs->putOrAppendRow(i,output);

        if(testcosts)
            testcosts->putOrAppendRow(i, costs);

        if(test_stats)
            test_stats->update(costs,proc_weight);

        if(report_progress)
            pb->update(i);
    }
*/
/*
}
*/



void VPLPreprocessedLearner2::computeOutput(const Vec& input, Vec& output) const
{
    PLASSERT( learner_ );
    output.resize(outputsize());
    Vec newinput = input;
    if(!input_prg.empty())
    {
        processed_input.resize(input_prg_fieldnames.length());
        input_prg_.run(input, processed_input);
        newinput = processed_input;
    }

    if(!output_prg.empty())
    {
        learner_->computeOutput(newinput, pre_output);
        // as context for output postproc
        output_prg_.run(concat(input,pre_output), output);
    }
    else
        learner_->computeOutput(newinput, output);
    
}

void VPLPreprocessedLearner2::computeOutputAndCosts(const Vec& input, const Vec& target, 
                                                   Vec& output, Vec& costs) const
{ 
    output.resize(outputsize());
    costs.resize(nTestCosts());

    if(ignore_test_costs)
    {
        costs.fill(-1);
        return computeOutput(input, output);
    }

    PLASSERT( learner_ );
    PLASSERT(input.length()==inputsize());
    PLASSERT(target.length()==targetsize());

    Vec newinput = input;
    if(!input_prg.empty())//input_prg_)
    {
        processed_input.resize(input_prg_fieldnames.length());
        input_prg_.run(input, processed_input);
        newinput = processed_input;
    }

    Vec orig_row = concat(input,target);
    orig_row.resize(orig_fieldnames.length());

    Vec newtarget = target;
    if(!target_prg.empty())//target_prg_)
    {
        processed_target.resize(target_prg_fieldnames.length());
        target_prg_.run(orig_row, processed_target);
        newtarget = processed_target;
    }

    pre_costs.resize(learner_->nTestCosts());
    learner_->computeOutputAndCosts(newinput, newtarget, pre_output, pre_costs);

    if(!output_prg.empty())//output_prg_)
        output_prg_.run(concat(input,pre_output), output);
    else
        output << pre_output;

   
    if(!costs_prg.empty())//costs_prg_)
        costs_prg_.run(concat(input,target,pre_output,pre_costs), costs);
    else
        costs << pre_costs;
}

void VPLPreprocessedLearner2::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                                     const Vec& target, Vec& costs) const
{ 
    Vec nonconst_output = output; // to make the constipated compiler happy
    computeOutputAndCosts(input, target, nonconst_output, costs); 
}

bool VPLPreprocessedLearner2::computeConfidenceFromOutput(
    const Vec& input, const Vec& output,
    real probability, TVec< pair<real,real> >& intervals) const
{
    int d = outputsize();
    if(d!=output.length())
        PLERROR("In VPLPreprocessedLearner2::computeConfidenceFromOutput, length of passed output (%d)"
                "differes from outputsize (%d)!",output.length(),d);

    PLASSERT( learner_ );
    Vec newinput = input;
    if(!input_prg.empty())//input_prg_)
    {
        processed_input.resize(input_prg_fieldnames.length());
        input_prg_.run(input, processed_input);
        newinput = processed_input;
    }

    bool status = false;
    if(output_prg.empty())//!output_prg_) // output is already the output of the underlying learner
        status = learner_->computeConfidenceFromOutput(newinput, output, probability, intervals);
    else // must recompute the output of underlying learner, and post-process returned intervals
    {
        learner_->computeOutput(newinput, pre_output);
        TVec< pair<real,real> > pre_intervals;
        status = learner_->computeConfidenceFromOutput(newinput, pre_output, probability, pre_intervals);
        if(!status) // no confidence computation available
        {
            intervals.resize(d);
            for(int k=0; k<d; k++)
                intervals[k] = pair<real,real>(MISSING_VALUE,MISSING_VALUE);
        }
        else // postprocess low and high vectors
        {
            int ud = learner_->outputsize(); // dimension of underlying learner's output
            // first build low and high vectors
            Vec low(ud);
            Vec high(ud);
            for(int k=0; k<ud; k++)
            {
                pair<real,real> p = pre_intervals[k];
                low[k] = p.first;
                high[k] = p.second;
            }
            Vec post_low(d); // postprocesed low
            Vec post_high(d); // postprocessed high

            output_prg_.run(concat(input,low), post_low);
            output_prg_.run(concat(input,high), post_high);

            // Now copy post_low and post_high to intervals
            intervals.resize(d);
            for(int k=0; k<d; k++)
                intervals[k] = pair<real,real>(post_low[k],post_high[k]);
        }
    }
    return status;
}

TVec<string> VPLPreprocessedLearner2::getOutputNames() const
{
    if(!output_prg.empty())//output_prg_)
        return output_prg_fieldnames;
    else
        return learner_->getOutputNames();
}


TVec<string> VPLPreprocessedLearner2::getTestCostNames() const
{
    if(!costs_prg.empty())//costs_prg_)
        return costs_prg_fieldnames;
    else
        return learner_->getTestCostNames();
}

TVec<string> VPLPreprocessedLearner2::getTrainCostNames() const
{
    PLASSERT( learner_ );
    return learner_->getTrainCostNames();
}

void VPLPreprocessedLearner2::resetInternalState()
{
    PLASSERT( learner_ );
    learner_->resetInternalState();
}

bool VPLPreprocessedLearner2::isStatefulLearner() const
{
    PLASSERT( learner_ );
    return learner_->isStatefulLearner();
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
