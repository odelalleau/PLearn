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
   * $Id: PLearnerOutputVMatrix.cc,v 1.1 2003/09/23 01:04:02 yoshua Exp $
   ******************************************************* */

// Authors: Yoshua Bengio

/*! \file PLearnerOutputVMatrix.cc */


#include "PLearnerOutputVMatrix.h"

namespace PLearn <%
using namespace std;


PLearnerOutputVMatrix::PLearnerOutputVMatrix()
  :inherited(), put_raw_input(false), put_costs(false)
  /* ### Initialise all fields to their default value */
{
}

PLEARN_IMPLEMENT_OBJECT(PLearnerOutputVMatrix, 
                        "Use a PLearner to transform the input part of a data set into the learner's outputs",
                        "The input part of this VMatrix is obtained from the input part an original data set on which\n"
                        "a PLearner's computeOutput method is applied. The other columns of the original data set\n"
                        "are copied as is. Optionally, the raw input can be copied as well, and the costs of the\n"
                        "learner (which depend on the target part of the original data) can be copied as well,\n"
                        "always in the input part of the new VMatrix. The order of the elements of a new row is as follows:\n"
                        "  - the output of the learner when applied on the input part of the original data,\n"
                        "  - optionally, the raw input part of the original data,\n"
                        "  - optionally, the costs of the learner for that original data example,\n"
                        "  - all the non-input columns of the original data.");

void PLearnerOutputVMatrix::getRow(int i, Vec v) const
{
  int c=0;
  data->getRow(i,row);
  learner->computeOutput(learner_input,learner_output);
  v.subVec(0,c=learner_output->length()) << learner_output;
  if (put_raw_input)
  {
    v.subVec(c,learner_input->length()) << learner_input;
    c+=learner_input->length();
  }
  if (put_costs)
  {
    learner->computeCostsFromOutputs(learner_input,learner_output,learner_target,learner_costs);
    v.subVec(c,learner_costs->length()) << learner_costs;
    c+=learner_costs->length();
  }
  v.subVec(c,non_input_part_of_data_row.length()) << non_input_part_of_data_row;
}

void PLearnerOutputVMatrix::declareOptions(OptionList& ol)
{
  // ### Declare all of this object's options here
  // ### For the "flags" of each option, you should typically specify  
  // ### one of OptionBase::buildoption, OptionBase::learntoption or 
  // ### OptionBase::tuningoption. Another possible flag to be combined with
  // ### is OptionBase::nosave

   declareOption(ol, "data", &PLearnerOutputVMatrix::data, OptionBase::buildoption,
                 "The original data set (a VMat)");
   declareOption(ol, "learner", &PLearnerOutputVMatrix::learner, OptionBase::buildoption,
                 "The PLearner which will be applied to the data set");
   declareOption(ol, "put_raw_input", &PLearnerOutputVMatrix::put_raw_input, OptionBase::buildoption,
                 "whether to include in the input part of this VMatrix the raw data input part");
   declareOption(ol, "put_costs", &PLearnerOutputVMatrix::put_raw_input, OptionBase::buildoption,
                 "whether to include in the input part of this VMatrix the learner costs");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void PLearnerOutputVMatrix::build_()
{
  if (data && learner)
  {
    row.resize(data->width());
    learner_input = row.subVec(0,data->inputsize());
    learner_target = row.subVec(data->inputsize(),data->targetsize());
    int it=data->inputsize()+data->targetsize();
    non_input_part_of_data_row = row.subVec(it,data->width()-it);
    learner_output.resize(learner->outputsize());
    learner_costs.resize(learner->nTestCosts());
  }
}

// ### Nothing to add here, simply calls build_
void PLearnerOutputVMatrix::build()
{
  inherited::build();
  build_();
}

void PLearnerOutputVMatrix::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  deepCopyField(row, copies);
  deepCopyField(learner_input, copies);
  deepCopyField(learner_output, copies);
  deepCopyField(learner_target, copies);
  deepCopyField(learner_costs, copies);
  deepCopyField(non_input_part_of_data_row, copies);
}

%> // end of namespace PLearn

