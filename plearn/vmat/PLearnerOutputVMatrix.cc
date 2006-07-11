// -*- C++ -*-

// PLearnerOutputVMatrix.cc
//
// Copyright (C) 2003 Yoshua Bengio
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
 * $Id$
 ******************************************************* */

// Authors: Yoshua Bengio

/*! \file PLearnerOutputVMatrix.cc */


#include "PLearnerOutputVMatrix.h"

namespace PLearn {
using namespace std;


PLearnerOutputVMatrix::PLearnerOutputVMatrix(bool call_build_)
    :inherited(call_build_),
     put_raw_input(false),
     put_non_input(true),
     train_learners(false),
     compute_output_once(false)
    /* ### Initialize all fields to their default value */
{
    if( call_build_ )
        build_();
}

PLearnerOutputVMatrix::PLearnerOutputVMatrix(VMat source_,
                                             TVec< PP<PLearner> > learners_,
                                             bool put_raw_input_,
                                             bool train_learners_,
                                             bool compute_output_once_,
                                             bool put_non_input_,
                                             bool call_build_)
    : inherited(source_, call_build_),
      learners(learners_),
      put_raw_input(put_raw_input_),
      put_non_input(put_non_input_),
      train_learners(train_learners_),
      compute_output_once(compute_output_once_)
{
    if( call_build_ )
        build_();
}

PLearnerOutputVMatrix::PLearnerOutputVMatrix(VMat source_,
                                             PP<PLearner> learner,
                                             bool put_raw_input_,
                                             bool train_learners_,
                                             bool compute_output_once_,
                                             bool put_non_input_,
                                             bool call_build_)
    : inherited(source_, call_build_),
      put_raw_input(put_raw_input_),
      put_non_input(put_non_input_),
      train_learners(train_learners_),
      compute_output_once(compute_output_once_)
{
    learners.resize(1);
    learners[0] = learner;
    if( call_build_ )
        build_();
}

PLEARN_IMPLEMENT_OBJECT(PLearnerOutputVMatrix,
                        "Use a PLearner to transform the input part of a"
                        " source data set",
                        "The input part of this VMatrix is obtained from the"
                        " input part of a source\n"
                        "data set on which one or more PLearner's"
                        " computeOutput method is applied.\n"
                        "The other columns of the source data set are copied"
                        " as is.\n"
                        "Optionally, the raw input can be copied as well"
                        " always in the input part of\n"
                        "the new VMatrix. The order of the elements of a new"
                        " row is as follows:\n"
                        "  - the outputs of the learners (concatenated) when"
                        " applied on the input part\n"
                        "    of the source data,\n"
                        "  - optionally, the raw input part of the source"
                        " data,\n"
                        "  - optionally, all the non-input columns of the"
                        " source data\n"
                        "\n"
                        "When the learner has to be trained, a different"
                        " dataset can be used for the\n"
                        "training and the output, by using the 'data_train'"
                        " option.\n");

void PLearnerOutputVMatrix::getNewRow(int i, const Vec& v) const
{
    int c=0;
    if (learners_need_train) {
        // We need to train the learners first.
        for (int k = 0; k < learners.length(); k++)
        {
            PP<VecStatsCollector> stats = new VecStatsCollector();
            learners[k]->setTrainStatsCollector(stats);
            learners[k]->train();
            stats->finalize();
        }
        learners_need_train = false;
    }
    source->getRow(i,row);

    if(compute_output_once)  {
        // Use precomputed outputs
        for (int j=0;j<learners.length();j++)
        {
            v.subVec(c,learners[j]->outputsize())
                << complete_learners_output[j](i);
            c += learners[j]->outputsize();
        }
    }

    else {
        // Compute output for each learner; now allow each learner to have a
        // different outputsize.  The variable 'learners_output' is kept for
        // backwards compatibility, but is no longer strictly necessary
        for (int j=0;j<learners.length();j++)
        {
            int cur_outputsize = learners[j]->outputsize();
            learners_output[j].resize(cur_outputsize);
            learners[j]->computeOutput(learner_input, learners_output[j]);
            v.subVec(c, cur_outputsize) << learners_output[j];
            c += cur_outputsize;
        }
    }

    if (put_raw_input)
    {
        v.subVec(c,learner_input->length()) << learner_input;
        c+=learner_input->length();
    }
    if (put_non_input)
        v.subVec(c,non_input_part_of_source_row.length())
            << non_input_part_of_source_row;
}

////////////////////
// declareOptions //
////////////////////
void PLearnerOutputVMatrix::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify
    // ### one of OptionBase::buildoption, OptionBase::learntoption or
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    declareOption(ol, "data", &PLearnerOutputVMatrix::source,
                  (OptionBase::learntoption | OptionBase::nosave),
                  "DEPRECATED - Use 'source' instead.");

    declareOption(ol, "learners", &PLearnerOutputVMatrix::learners,
                  OptionBase::buildoption,
                  "The vector of PLearners which will be applied to 'source'"
                  " data set.");

    declareOption(ol, "put_raw_input", &PLearnerOutputVMatrix::put_raw_input,
                  OptionBase::buildoption,
                  "Whether to include in the input part of this VMatrix the"
                  " raw input part\n"
                  "of 'source'.\n");

    declareOption(ol, "put_non_input", &PLearnerOutputVMatrix::put_non_input,
                  OptionBase::buildoption,
                  "Whether to include in this VMatrix the original target and"
                  " weights.");

    declareOption(ol, "train_learners", &PLearnerOutputVMatrix::train_learners,
                  OptionBase::buildoption,
                  "If set to 1, the learners will be train on 'source' (or"
                  " 'data_train' if present)\n"
                  "before computing the output.\n");

    declareOption(ol, "data_train", &PLearnerOutputVMatrix::data_train,
                  OptionBase::buildoption,
                  "If provided and 'train_learners' is set to 1, the learner"
                  " will be trained\n"
                  "on this dataset.\n");

    declareOption(ol, "compute_output_once",
                  &PLearnerOutputVMatrix::compute_output_once,
                  OptionBase::buildoption,
                  "If set to 1, the output of the learners will be computed"
                  " once and stored");

    declareOption(ol, "fieldinfos_source",
                  &PLearnerOutputVMatrix::fieldinfos_source,
                  OptionBase::buildoption,
                  "If provided, the fieldnames will be copied from this VMat.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void PLearnerOutputVMatrix::build_()
{
    if (source && learners.length()>0 && learners[0])
    {
        learners_need_train = train_learners;
        row.resize(source->width());

        if (train_learners) {
            // Set the learners' training set.
            for (int i = 0; i < learners.length(); i++) {
                if (data_train)
                    learners[i]->setTrainingSet(data_train);
                else
                    learners[i]->setTrainingSet(source);
            }

            // Note that the learners will be train only if we actually
            // call getRow() or if compute_output_once is true
        }

        if(compute_output_once)
        {
            complete_learners_output.resize(learners.length());
            for (int i = 0; i < learners.length(); i++) {
                if(train_learners)
                {
                    PP<VecStatsCollector> stats = new VecStatsCollector();
                    learners[i]->setTrainStatsCollector(stats);
                    learners[i]->train();
                    stats->finalize();
                }
                complete_learners_output[i].resize(source->length(),
                                                   learners[i]->outputsize());
            }
            learners_need_train = false;

            Vec input_row = row.subVec(0,source->inputsize());

            for(int i=0; i<source->length();i++)
            {
                source->getRow(i,row);
                for (int j=0;j<learners.length();j++)
                {
                    Vec out_j = complete_learners_output[j](i);
                    learners[j]->computeOutput(input_row,out_j);
                }
            }
        }

        if (source->inputsize() < 0)
            PLERROR("In PLearnerOutputVMatrix::build_ - The 'source' matrix"
                    " has a negative inputsize");
        if (source->targetsize() < 0)
            PLERROR("In PLearnerOutputVMatrix::build_ - The 'source' matrix"
                    " has a negative targetsize");
        if (source->weightsize() < 0)
            PLERROR("In PLearnerOutputVMatrix::build_ - The 'source' matrix"
                    " has a negative weightsize");

        // Some further state variable initializations
        learner_input = row.subVec(0,source->inputsize());
        learner_target = row.subVec(source->inputsize(),source->targetsize());
        non_input_part_of_source_row =
            row.subVec(source->inputsize(),
                       source->width() - source->inputsize());
        learners_output.resize(learners->length());

        // Compute the total width of the VMatrix and the width of the various
        // components
        inputsize_ = 0;
        for (int i=0;i<learners->length();i++)
            inputsize_ += learners[i]->outputsize();
        if (put_raw_input)
            inputsize_ += source->inputsize();
        if (put_non_input) {
            targetsize_ = source->targetsize();
            weightsize_ = source->weightsize();
            extrasize_  = source->extrasize();
            width_ = inputsize_ + targetsize_ + weightsize_ + extrasize_;
        }
        else {
            targetsize_ = 0;
            weightsize_ = 0;
            width_ = inputsize_;
        }
        length_ = source->length();

        // Set field info.
        if (fieldinfos_source) 
            setFieldInfos(fieldinfos_source->getFieldInfos());
        else
        {
            TVec<string> fieldnames;
            for(int k=0; k<learners.length(); k++)
                fieldnames.append(learners[k]->getOutputNames());
            if(put_raw_input)
                fieldnames.append(source->inputFieldNames());
            if(put_non_input)
            {
                fieldnames.append(source->targetFieldNames());
                fieldnames.append(source->weightFieldNames());
                fieldnames.append(source->extraFieldNames());
            }
            declareFieldNames(fieldnames);
        }
        /* OLD CODE
        else {
            fieldinfos.resize(width_);
            if (put_non_input &&
                source->getFieldInfos().size() >= source->inputsize()
                                                    + source->targetsize())
            {
                // We can retrieve the information for the target columns.
                for (int i = 0; i < source->targetsize(); i++) 
                {
                    fieldinfos[i + this->inputsize()] =
                        source->getFieldInfos()[i + source->inputsize()];
                }
            }
        }
        */
    }
}

///////////
// build //
///////////
void PLearnerOutputVMatrix::build()
{
    inherited::build();
    build_();
}

void PLearnerOutputVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(row, copies);
    deepCopyField(learner_input, copies);
    deepCopyField(learners_output, copies);
    deepCopyField(learner_target, copies);
    deepCopyField(non_input_part_of_source_row, copies);
    deepCopyField(complete_learners_output, copies);
    deepCopyField(data_train, copies);
    deepCopyField(learners, copies);
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
