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
   * $Id: PLearnerOutputVMatrix.cc,v 1.18 2004/11/12 20:10:31 larocheh Exp $
   ******************************************************* */

// Authors: Yoshua Bengio

/*! \file PLearnerOutputVMatrix.cc */


#include "PLearnerOutputVMatrix.h"

namespace PLearn {
using namespace std;


PLearnerOutputVMatrix::PLearnerOutputVMatrix()
 :inherited(),
  put_raw_input(false),
  put_non_input(true),
  train_learners(false),
  compute_output_once(false)
  /* ### Initialize all fields to their default value */
{}

PLearnerOutputVMatrix::PLearnerOutputVMatrix
(VMat data_,TVec<PP<PLearner> > learners_, bool put_raw_input_, bool train_learners_, bool compute_output_once_, bool put_non_input_) 
: data(data_),learners(learners_),
  put_raw_input(put_raw_input_),
  put_non_input(put_non_input_),
  train_learners(train_learners_),
  compute_output_once(compute_output_once_)
{
  build();
}


PLEARN_IMPLEMENT_OBJECT(PLearnerOutputVMatrix, 
                        "Use a PLearner (or a set of them) to transform the input part of a data set into the learners outputs",
                        "The input part of this VMatrix is obtained from the input part an original data set on which\n"
                        "one or more PLearner's computeOutput method is applied. The other columns of the original data set\n"
                        "are copied as is. Optionally, the raw input can be copied as well\n"
                        "always in the input part of the new VMatrix. The order of the elements of a new row is as follows:\n"
                        "  - the outputs of the learners (concatenated) when applied on the input part of the original data,\n"
                        "  - optionally, the raw input part of the original data,\n"
                        "  - optionally, all the non-input columns of the original data");

void PLearnerOutputVMatrix::getNewRow(int i, const Vec& v) const
{
  int c=0;
  if (learners_need_train) {
    // We need to train the learners first.
    for (int i = 0; i < learners.length(); i++)
    {
      PP<VecStatsCollector> stats = new VecStatsCollector();
      learners[i]->setTrainStatsCollector(stats);
      learners[i]->train();
      stats->finalize();
    }
    learners_need_train = false;
  }
  data->getRow(i,row);
  if(compute_output_once)
  {
    for (int j=0;j<learners.length();j++)
    {
      v.subVec(c,learners[j]->outputsize()) << complete_learners_output[j](i);
      c += learners[j]->outputsize();
    }
  }
  else
  {
    for (int j=0;j<learners.length();j++)
    {
      Vec out_j = learners_output(j);
      learners[j]->computeOutput(learner_input,out_j);
    }
    v.subVec(0,c=learners_output.size()) << learners_output.toVec();
  }
 
  if (put_raw_input)
  {
    v.subVec(c,learner_input->length()) << learner_input;
    c+=learner_input->length();
  }
  if (put_non_input)
    v.subVec(c,non_input_part_of_data_row.length()) << non_input_part_of_data_row;
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

   declareOption(ol, "data", &PLearnerOutputVMatrix::data, OptionBase::buildoption,
                 "The original data set (a VMat)");

   declareOption(ol, "learners", &PLearnerOutputVMatrix::learners, OptionBase::buildoption,
                 "The vector of PLearners which will be applied to the data set");

   declareOption(ol, "put_raw_input", &PLearnerOutputVMatrix::put_raw_input, OptionBase::buildoption,
                 "Whether to include in the input part of this VMatrix the raw data input part");

   declareOption(ol, "put_non_input", &PLearnerOutputVMatrix::put_non_input, OptionBase::buildoption,
                 "Whether to include in this VMatrix the original target and weights.");

   declareOption(ol, "train_learners", &PLearnerOutputVMatrix::train_learners, OptionBase::buildoption,
                "If set to 1, the learners will be train on 'data' before computing the output");

   declareOption(ol, "compute_output_once", &PLearnerOutputVMatrix::compute_output_once, OptionBase::buildoption,
                "If set to 1, the output of the learners will be computed once and stored");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

////////////
// build_ //
////////////
void PLearnerOutputVMatrix::build_()
{
  if (data && learners.length()>0 && learners[0])
  {
    learners_need_train = train_learners;
    row.resize(data->width());

    if (train_learners) {
      // Set the learners' training set.
      for (int i = 0; i < learners.length(); i++) {
        learners[i]->setTrainingSet(data);
      }
      
      // Note that the learners will be train only if we actually call getRow().
      // Hugo: except if compute_output_once is true
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
          complete_learners_output[i].resize(data->length(),learners[i]->outputsize());
        }
        learners_need_train = false;

        Vec input_row = row.subVec(0,data->inputsize());

        for(int i=0; i<data->length();i++)
        {
          data->getRow(i,row);
          for (int j=0;j<learners.length();j++)
          {
            Vec out_j = complete_learners_output[j](i);
            learners[j]->computeOutput(input_row,out_j);
          }
        }
      }

    if (data->inputsize() < 0)
      PLERROR("In PLearnerOutputVMatrix::build_ - The 'data' matrix has a negative inputsize");
    if (data->targetsize() < 0)
      PLERROR("In PLearnerOutputVMatrix::build_ - The 'data' matrix has a negative targetsize");
    learner_input = row.subVec(0,data->inputsize());
    learner_target = row.subVec(data->inputsize(),data->targetsize());
    non_input_part_of_data_row = row.subVec(data->inputsize(),data->width()-data->inputsize());
    learners_output.resize(learners->length(),learners[0]->outputsize());
    inputsize_ = 0;
    for (int i=0;i<learners->length();i++)
      inputsize_ += learners[i]->outputsize();
    if (put_raw_input) 
      inputsize_ += data->inputsize();
    if (put_non_input) {
      targetsize_ = data->targetsize();
      weightsize_ = data->weightsize();
      width_ = inputsize_ + targetsize_ + weightsize_;
    } else {
      targetsize_ = 0;
      weightsize_ = 0;
      width_ = inputsize_;
    }
    length_ = data->length();

    // Set field info.
    fieldinfos.resize(width_);
    if (put_non_input && data->getFieldInfos().size() >= data->inputsize() + data->targetsize()) {
      // We can retrieve the information for the target columns.
      for (int i = 0; i < data->targetsize(); i++) {
        fieldinfos[i + this->inputsize()] = data->getFieldInfos()[i + data->inputsize()];
      }
    }
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
  deepCopyField(non_input_part_of_data_row, copies);
  deepCopyField(complete_learners_output, copies);
  deepCopyField(data, copies);
  deepCopyField(learners, copies);
}

} // end of namespace PLearn

