// -*- C++ -*-

// TargetEncodingLearner.cc
//
// Copyright (C) 2006 Xavier Sanint-Mleux
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

// Authors: Xavier Saint-Mleux

/*! \file TargetEncodingLearner.cc */


#include "TargetEncodingLearner.h"
#include <plearn/vmat/EncodedVMatrix.h>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(TargetEncodingLearner,"",
    "This learner encodes input data so that field values are replaced by the weighted average target for that value.\n"
    "Fields that have more than max_nvals_for_encoding different values are not encoded.\n");

TargetEncodingLearner::TargetEncodingLearner()
  :max_nvals_for_encoding(1000),
   sublearner(0),
   encodings(),
   encode_col(),
   mean(0.),
   encodings_learnt(false)
{
}

void TargetEncodingLearner::declareOptions(OptionList& ol)
{
    declareOption(ol, "max_nvals_for_encoding", &TargetEncodingLearner::max_nvals_for_encoding,
                  OptionBase::buildoption,
                  "Max number of different values for a field to be encoded.\n");

    declareOption(ol, "sublearner", &TargetEncodingLearner::sublearner,
                  OptionBase::buildoption,
                  "Learner to be trained/tested on the encoded dataset.\n");

    declareOption(ol, "encodings", &TargetEncodingLearner::encodings,
                  OptionBase::learntoption,
                  "Learnt encodings for the trainset.\n");

    declareOption(ol, "encode_col", &TargetEncodingLearner::encodings,
                  OptionBase::learntoption,
                  "Wether each col. should be encoded.\n");

    declareOption(ol, "mean", &TargetEncodingLearner::mean,
                  OptionBase::learntoption,
                  "Weighted mean all targets.\n");

    declareOption(ol, "encodings_learnt", &TargetEncodingLearner::encodings_learnt,
                  OptionBase::learntoption,
                  "Wether encodings have already been learnt.\n");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void TargetEncodingLearner::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation.
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of
    // ###    all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning"
    // ###    options have been modified.
    // ### You should assume that the parent class' build_() has already been
    // ### called.
  encodings_learnt= false;
}

// ### Nothing to add here, simply calls build_
void TargetEncodingLearner::build()
{
    inherited::build();
    build_();
}


void TargetEncodingLearner::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("TargetEncodingLearner::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}


void TargetEncodingLearner::setExperimentDirectory(const PPath& the_expdir)
{
  inherited::setExperimentDirectory(the_expdir);
  sublearner->setExperimentDirectory(the_expdir / "SubLearner");
}


int TargetEncodingLearner::outputsize() const
{
    return sublearner->outputsize();
}

void TargetEncodingLearner::forget()
{
    inherited::forget();
    stage = 0;
    encodings_learnt= false;
}

void TargetEncodingLearner::train()
{
    if (!initTrain())
        return;

    if(stage<1)
    {

      buildEncodingsFromTrainset();
      VMat vm= new EncodedVMatrix(getTrainingSet(), encodings, defaults, encode_col);
      vm->setMetaDataDir(getExperimentDirectory() / "EncodedVMatrix.metadata");
      sublearner->setTrainingSet(vm);
      ++stage;
    }

    sublearner->train();

}

void TargetEncodingLearner::buildEncodingsFromTrainset()
{
  int l = train_set->length();
  int n = train_set->inputsize();
  Vec input;
  Vec target;
  real weight;

  encodings.resize(n);

  TVec<map<real, pair<real, real> > > stats(n);
  
  encode_col.resize(n);
  encode_col.fill(true);
  
  PP<ProgressBar> pb;
  if(report_progress)
    pb = new ProgressBar("TargetEncodingLearner computing statistics ",l);

  real tot_weight= 0.;
  real tot_wt_targ= 0.;

  for(int i= 0; i < l; ++i)
    {
      train_set->getExample(i, input, target, weight);
      if(target.length() != 1)
	PLERROR("TargetEncodingLearner supports only one target");
      tot_weight+= weight;
      tot_wt_targ+= weight*target[0];
      for(int j= 0; j < n; ++j)
	{
	  if(encode_col[j])
	    {
	      real val= input[j];

	      map<real, pair<real, real> >::iterator it= stats[j].find(val);
	      
	      if(it == stats[j].end())
		stats[j].insert(make_pair(val, make_pair(weight, weight*target[0])));
	      else
		{
		  it->second.first+= weight;
		  it->second.second+= weight*target[0];
		}
	      if(static_cast<int>(stats[j].size()) > max_nvals_for_encoding)
		{
		  encode_col[j]= false;
		  stats[j].clear();
		}
	    }
	}

      if(pb)
	pb->update(i);
    }

  mean= tot_wt_targ / tot_weight;
  defaults.resize(n);
  defaults.fill(mean);

  for(int i= 0; i < n; ++i)
    {
      encodings[i].clear();
      for(map<real, pair<real, real> >::iterator it= stats[i].begin(); it != stats[i].end(); ++it)
	{
	  real count= it->second.first;
	  real sum= it->second.second;
	  real avg= sum/count;
	  encodings[i].insert(make_pair(it->first, avg));
	}
    }

  encodings_learnt= true;
}



void TargetEncodingLearner::computeOutput(const Vec& input, Vec& output) const
{
  if(!encodings_learnt)
    PLERROR("TargetEncodingLearner::computeOutput encodings not learnt");
  EncodedVMatrix::encodeRow(encodings, defaults, encode_col, input);
  sublearner->computeOutput(input, output);
}

void TargetEncodingLearner::computeCostsFromOutputs(const Vec& input, const Vec& output,
                                           const Vec& target, Vec& costs) const
{
  if(!encodings_learnt)
    PLERROR("TargetEncodingLearner::computeCostsFromOutputs encodings not learnt");
  //EncodedVMatrix::encodeRow(encodings, defaults, encode_col, input);
  sublearner->computeCostsFromOutputs(input, output, target, costs);
}

TVec<string> TargetEncodingLearner::getTestCostNames() const
{
    return sublearner->getTestCostNames();
}

TVec<string> TargetEncodingLearner::getTrainCostNames() const
{
    return sublearner->getTrainCostNames();
}

TVec<string> TargetEncodingLearner::getOutputNames() const
{
    return sublearner->getOutputNames();
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
