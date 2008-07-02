// -*- C++ -*-

// MultiTargetOneHotVMatrix.cc
//
// Copyright (C) 2004 Olivier Delalleau 
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
   * $Id: MultiTargetOneHotVMatrix.cc,v 1.7 2005/03/16 18:26:25 delallea Exp $ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file MultiTargetOneHotVMatrix.cc */


#include "MultiTargetOneHotVMatrix.h"
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;

////////////////////////////////
// MultiTargetOneHotVMatrix //
////////////////////////////////
MultiTargetOneHotVMatrix::MultiTargetOneHotVMatrix()
: missing_target(-FLT_MAX),
  reweight_targets(false),
  target_val("specific_target"),
  verbosity(1),
  hot_value(1),
  cold_value(0)
{
}

PLEARN_IMPLEMENT_OBJECT(MultiTargetOneHotVMatrix,
    "This VMatrix transforms a multi-target source into a single target one with an added one-hot vector.",
    "The source VMatrix is given by its input part (in 'source'), and by its target part\n"
    "(in 'source_target'). The target being d-dimensional, each data point is repeated\n"
    "in this VMatrix as many times as it has non-missing targets. For each non-missing\n"
    "target, a one-hot vector (with a '1' corresponding to this target index) is added\n"
    "to the input part, and the target value (now 1-dimensional) is either the value for\n"
    "this target, or the maximum of all target values (depending on 'target_val').\n"
    "If the 'target_descriptor' option is set, it must be a VMat of length 'd', that\n"
    "gives an additional input descriptor to append to each example depending on the\n"
    "target. This VMat's first column must be the name of the target, which must\n"
    "correspond to a field name in 'source_target'.\n"
    "If one wants to set a weight, it must be set in 'source'.\n"
);

////////////////////
// declareOptions //
////////////////////
void MultiTargetOneHotVMatrix::declareOptions(OptionList& ol)
{
  // ### Declare all of this object's options here
  // ### For the "flags" of each option, you should typically specify  
  // ### one of OptionBase::buildoption, OptionBase::learntoption or 
  // ### OptionBase::tuningoption. Another possible flag to be combined with
  // ### is OptionBase::nosave

  declareOption(ol, "source_target", &MultiTargetOneHotVMatrix::source_target, OptionBase::buildoption,
      "The VMat containing the target information for each sample of 'source'.");

  declareOption(ol, "target_descriptor", &MultiTargetOneHotVMatrix::target_descriptor, OptionBase::buildoption,
      "The VMat containing the additional target descriptors (optional).");

  declareOption(ol, "missing_target", &MultiTargetOneHotVMatrix::missing_target, OptionBase::buildoption,
      "A target in the source VMatrix equal to this value will be treated as missing.");

  declareOption(ol, "reweight_targets", &MultiTargetOneHotVMatrix::reweight_targets, OptionBase::buildoption,
      "If set to 1, a weight column will be appended, with a weight of 1 for the most\n"
      "frequent target, and > 1 for other targets (=> equal weight to each target).");

  declareOption(ol, "target_val", &MultiTargetOneHotVMatrix::target_val, OptionBase::buildoption,
      "How the target value is computed:\n"
      " - 'max'             : the maximum of all targets\n"
      " - 'specific_target' : the value of the target considered");

  declareOption(ol, "verbosity", &MultiTargetOneHotVMatrix::verbosity, OptionBase::buildoption,
      "Controls the amount of output.");

  declareOption(ol, "hot_value", &MultiTargetOneHotVMatrix::hot_value, OptionBase::buildoption,
      "Hot value in one-hot representation. If it is the same as the cold_value,\n"
      "then the actual target value, as defined by the target_val option, is used.");

  declareOption(ol, "cold_value", &MultiTargetOneHotVMatrix::cold_value, OptionBase::buildoption,
      "Cold value in one-hot representation.");

  declareOption(ol, "source_and_target", &MultiTargetOneHotVMatrix::source_and_target, OptionBase::buildoption,
      "The VMat containing both the source (input) and target information.");


  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

///////////
// build //
///////////
void MultiTargetOneHotVMatrix::build()
{
  inherited::build();
  build_();
}

////////////
// build_ //
////////////
void MultiTargetOneHotVMatrix::build_()
{
  // ### This method should do the real building of the object,
  // ### according to set 'options', in *any* situation. 
  // ### Typical situations include:
  // ###  - Initial building of an object from a few user-specified options
  // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
  // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
  // ### You should assume that the parent class' build_() has already been called.

  if (source && source_target) {
    updateMtime(source);
    updateMtime(source_target);
    updateMtime(target_descriptor);
    updateMtime(source_and_target);

    if (source->targetsize() > 0)
      PLERROR("In MultiTargetOneHotVMatrix::build_ - The source VMatrix must have a targetsize <= 0");
    if (source->length() != source_target->length())
      PLERROR("In MultiTargetOneHotVMatrix::build_ - 'source' and 'source_target' must have same length");
    // We need to browse through the data.
    PP<ProgressBar> pb;
    if (verbosity >= 1)
      pb = new ProgressBar("Building new data view", source->length());
//    int target_col = source->inputsize();
    Vec targets(source_target->width());
    int data_width = 3;
    if (target_descriptor)
      data_width++;
    if (reweight_targets)
      data_width++;
    data.resize(0, data_width);
    Vec data_row(data_width);
    int count = 0;
    TVec<int> count_targets;
    if (reweight_targets) {
      count_targets.resize(source_target->width());
      count_targets.clear();
    }
    for (int i = 0; i < source->length(); i++) {
      data_row[0] = i;
      source_target->getRow(i, targets);
      if (target_val == "max")
        data_row[2] = max(targets);
      for (int j = 0; j < targets.length(); j++) {
        data_row[1] = j;
        if (!is_missing(targets[j]) && targets[j] != missing_target) {
          count++;
          if (target_val == "specific_target")
            data_row[2] = targets[j];
          data.appendRow(data_row);
          if (reweight_targets)
            count_targets[j]++;
        }
      }
      if (pb)
        pb->update(i+1);
    }
    if (reweight_targets) {
      int max_target = argmax(count_targets);
      if (count_targets[max_target] == 0)
        PLERROR("In MultiTargetOneHotVMatrix::build_ - A target does not appear in the dataset, cannot reweight targets");
      Vec target_weights(count_targets.length());
      for (int i = 0; i < target_weights.length(); i++) {
        target_weights[i] = real(count_targets[max_target]) / real(count_targets[i]);
        if (verbosity >= 5)
          cout << "Weight for target " << i << ": " << target_weights[i]
               << " (count = " << count_targets[i] << ")" << endl;
      }
      for (int i = 0; i < data.length(); i++)
        data(i, data_width - 1) = target_weights[int(data(i, 1))];
    }
    source_inputsize = source->width();
    weightsize_ = 0;
    if (source->weightsize() > 0) {
      // There are weights.
      source_inputsize -= source->weightsize();
      weightsize_ = source->weightsize();
    }

    length_ = count;
    inputsize_ = source_inputsize + source_target->width();
    if (target_descriptor) {
      inputsize_ += target_descriptor->width() - 1;
      // Also build mapping from a target index to a row in 'target_descriptor'.
      int count_found = 0;
      TVec<int> map_target(source_target->width());
      for (int i = 0; i < target_descriptor->length(); i++) {
        string target_name = target_descriptor->getString(i, 0);
        int field_index = source_target->fieldIndex(target_name);
        if (field_index >= 0) {
          map_target[field_index] = i;
          count_found++;
        }
      }
      if (count_found != source_target->width())
        PLERROR("In MultiTargetOneHotVMatrix::build_ - Could not find all target descriptors in 'target_descriptor'");
      // Now fill 'data' accordingly.
      for (int i = 0; i < data.length(); i++)
        data(i, 3) = map_target[int(data(i, 1))];
    }
    targetsize_ = 1;
    width_ = inputsize_ + targetsize_ + weightsize_;
    // Add field names.
    TVec<VMField> source_infos = source->getFieldInfos();
    TVec<VMField> f_infos(source_infos.length());
    f_infos << source_infos;
    f_infos.resize(inputsize_);
    TVec<VMField> source_target_infos = source_target->getFieldInfos();
    f_infos.subVec(source_inputsize, source_target_infos.length()) << source_target_infos;
    if (target_descriptor) {
      TVec<VMField> target_des_infos = target_descriptor->getFieldInfos();
      f_infos.subVec(source_inputsize + source_target->width(), target_descriptor->width() - 1)
        << target_des_infos.subVec(1, target_des_infos.length() - 1);
    }
    f_infos.append(VMField(target_val, VMField::UnknownType));
    if (weightsize_ > 0) {
      f_infos.append(source_infos.subVec(
            source_infos.length() - source->weightsize(), source->weightsize()));
    }
    if (reweight_targets) {
      if (weightsize_ >= 0)
        weightsize_++;
      else
        weightsize_ = 1;
      f_infos.append(VMField("target_weight", VMField::UnknownType));
      width_++;
    }
    setFieldInfos(f_infos);
//    setMetaInfoFromSource();
  }
  else if(source_and_target)
  {
    // We need to browse through the data.
    PP<ProgressBar> pb;
    if (verbosity >= 1)
      pb = new ProgressBar("Building new data view", source_and_target->length());
    //    int target_col = source->inputsize();
    Vec targets(source_and_target->targetsize());
    real weight;
    Vec input(source_and_target->inputsize());
    int data_width = 3;
    if (target_descriptor)
      data_width++;
    if (reweight_targets)
      data_width++;
    data.resize(0, data_width);
    Vec data_row(data_width);
    int count = 0;
    TVec<int> count_targets;
    if (reweight_targets) {
      count_targets.resize(source_and_target->targetsize());
      count_targets.clear();
    }
    for (int i = 0; i < source_and_target->length(); i++) {
      data_row[0] = i;
      source_and_target->getExample(i, input,targets,weight);
      if (target_val == "max")
        data_row[2] = max(targets);
      for (int j = 0; j < targets.length(); j++) {
        data_row[1] = j;
        if (!is_missing(targets[j]) && targets[j] != missing_target) {
          count++;
          if (target_val == "specific_target")
            data_row[2] = targets[j];
          data.appendRow(data_row);
          if (reweight_targets)
            count_targets[j]++;
        }
      }
      if (pb)
        pb->update(i+1);
    }
    if (reweight_targets) {
      int max_target = argmax(count_targets);
      if (count_targets[max_target] == 0)
        PLERROR("In MultiTargetOneHotVMatrix::build_ - A target does not appear in the dataset, cannot reweight targets");
      Vec target_weights(count_targets.length());
      for (int i = 0; i < target_weights.length(); i++) {
        target_weights[i] = real(count_targets[max_target]) / real(count_targets[i]);
        if (verbosity >= 5)
          cout << "Weight for target " << i << ": " << target_weights[i]
               << " (count = " << count_targets[i] << ")" << endl;
      }
      for (int i = 0; i < data.length(); i++)
        data(i, data_width - 1) = target_weights[int(data(i, 1))];
    }
    source_inputsize = source_and_target->inputsize();
    weightsize_ = 0;
    if (source_and_target->weightsize() > 0) {
      // There are weights.
      //  source_inputsize -= source_and_target->weightsize();
      weightsize_ = source_and_target->weightsize();
    }

    length_ = count;
    inputsize_ = source_inputsize + source_and_target->targetsize();
    if (target_descriptor) {
      inputsize_ += target_descriptor->width() - 1;
      // Also build mapping from a target index to a row in 'target_descriptor'.
      int count_found = 0;
      TVec<int> map_target(source_and_target->targetsize());
      for (int i = 0; i < target_descriptor->length(); i++) {
        string target_name = target_descriptor->getString(i, 0);
        int field_index = source_and_target->fieldIndex(target_name) - source_and_target->inputsize();
        if (field_index >= 0) {
          map_target[field_index] = i;
          count_found++;
        }
      }
      if (count_found != source_and_target->targetsize())
        PLERROR("In MultiTargetOneHotVMatrix::build_ - Could not find all target descriptors in 'target_descriptor'");
      // Now fill 'data' accordingly.
      for (int i = 0; i < data.length(); i++)
        data(i, 3) = map_target[int(data(i, 1))];
    }
    targetsize_ = 1;
    width_ = inputsize_ + targetsize_ + weightsize_;
    // Add field names.
    TVec<VMField> source_infos = source_and_target->getFieldInfos();
    TVec<VMField> f_infos(source_infos.length());
    f_infos << source_infos;
    f_infos.resize(inputsize_);
    //TVec<VMField> source_target_infos = source_target->getFieldInfos();
    //f_infos.subVec(source_inputsize, source_target_infos.length()) << source_target_infos;
    if (target_descriptor) {
      TVec<VMField> target_des_infos = target_descriptor->getFieldInfos();
      f_infos.subVec(source_inputsize + source_and_target->targetsize(), target_descriptor->width() - 1)
        << target_des_infos.subVec(1, target_des_infos.length() - 1);
    }
    f_infos.append(VMField(target_val, VMField::UnknownType));
    if (weightsize_ > 0) {
      f_infos.append(source_infos.subVec(
            source_infos.length() - source_and_target->weightsize(), source_and_target->weightsize()));
    }
    if (reweight_targets) {
      if (weightsize_ >= 0)
        weightsize_++;
      else
        weightsize_ = 1;
      f_infos.append(VMField("target_weight", VMField::UnknownType));
      width_++;
    }
    setFieldInfos(f_infos);
//    setMetaInfoFromSource();
  }
    
  
}

///////////////
// getNewRow //
///////////////
void MultiTargetOneHotVMatrix::getNewRow(int i, const Vec& v) const
{
  PLASSERT( (source && source_target) || source_and_target);
  int is = source_inputsize;
  int ts;
  if(source_and_target)
  {
    ts = source_and_target->targetsize();
    // Get the input part.
    source_and_target->getSubRow(int(data(i,0)), 0, v.subVec(0, is));
  }
  else
  {
    ts = source_target->width();
    // Get the input part.
    source->getSubRow(int(data(i,0)), 0, v.subVec(0, is));
  }
  // Fill the one-hot vector.
  v.subVec(is, ts).fill(cold_value);
  if(hot_value == cold_value || (is_missing(cold_value) && is_missing(hot_value)))
    v[is + int(data(i, 1))] = data(i,2);
  else
    v[is + int(data(i, 1))] = hot_value;
  if (target_descriptor)
    // Fill the target descriptor.
    target_descriptor->getSubRow(int(data(i, 3)), 1,
                                 v.subVec(is + ts, target_descriptor->width() - 1));
  // Fill the target.
  v[inputsize_] = data(i, 2);
  // Fill the (potential) weight.
  if (weightsize_ > 0)
  {
    if(source_and_target)
      source_and_target->getSubRow(int(data(i, 0)), source_and_target->width()-source_and_target->weightsize(), v.subVec(inputsize_ + targetsize_, source_and_target->weightsize()));
    else
      source->getSubRow(int(data(i, 0)), is, v.subVec(inputsize_ + targetsize_, source->weightsize()));
  }
  if (reweight_targets)
    v[v.length() - 1] = data(i, 3);
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void MultiTargetOneHotVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(data, copies);
  deepCopyField(source_target, copies);
  deepCopyField(target_descriptor, copies);
  deepCopyField(source_and_target, copies);
}

} // end of namespace PLearn

