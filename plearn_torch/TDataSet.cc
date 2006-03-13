// -*- C++ -*-

// TDataSet.cc
//
// Copyright (C) 2005 Olivier Delalleau 
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

// Authors: Olivier Delalleau

/*! \file TDataSet.cc */


#include "TDataSet.h"

namespace PLearn {
using namespace std;

TDataSet::TDataSet() 
: n_inputs(0),
  n_real_examples(0),
  n_targets(0),
  real_current_example_index(-1),
  select_examples(0),
  dataset(0)
{
}

PLEARN_IMPLEMENT_OBJECT(TDataSet,
    "Interface between PLearn and a Torch DataSet object",
    ""
);

void TDataSet::declareOptions(OptionList& ol)
{
  // ### Declare all of this object's options here
  // ### For the "flags" of each option, you should typically specify  
  // ### one of OptionBase::buildoption, OptionBase::learntoption or 
  // ### OptionBase::tuningoption. Another possible flag to be combined with
  // ### is OptionBase::nosave

  declareOption(ol, "n_inputs", &TDataSet::n_inputs, OptionBase::learntoption,
      "The input size.");

  declareOption(ol, "n_targets", &TDataSet::n_targets, OptionBase::learntoption,
      "The target size.");

  declareOption(ol, "n_real_examples", &TDataSet::n_real_examples, OptionBase::learntoption,
      "Total number of examples in the dataset.");

  declareOption(ol, "real_current_example_index", &TDataSet::real_current_example_index, OptionBase::learntoption,
      "(Real) index of the current example.");

  declareOption(ol, "select_examples", &TDataSet::select_examples, OptionBase::learntoption,
      "True if a subset of the examples is selected.");

  declareOption(ol, "selected_examples", &TDataSet::selected_examples, OptionBase::learntoption,
      "The indices of the selected examples (empty means all examples are selected).");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void TDataSet::build_()
{
  // ### This method should do the real building of the object,
  // ### according to set 'options', in *any* situation. 
  // ### Typical situations include:
  // ###  - Initial building of an object from a few user-specified options
  // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
  // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
  // ### You should assume that the parent class' build_() has already been called.
}

void TDataSet::build()
{
  inherited::build();
  build_();
}

void TDataSet::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("TDataSet::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

//////////////////////
// updateFromPLearn //
//////////////////////
void TDataSet::updateFromPLearn(Torch::Object* ptr) {
  if (ptr)
    dataset = (Torch::DataSet*) ptr;
  else
    PLERROR("In TDataSet::updateFromPLearn - Torch::DataSet is an abstract class "
            "and cannot be instantiated");

  FROM_P_BASIC(n_real_examples,            n_real_examples,            dataset, n_real_examples           );
  FROM_P_BASIC(select_examples,            select_examples,            dataset, select_examples           );
  FROM_P_BASIC(n_inputs,                   n_inputs,                   dataset, n_inputs                  );
  FROM_P_BASIC(n_targets,                  n_targets,                  dataset, n_targets                 );
  FROM_P_BASIC(real_current_example_index, real_current_example_index, dataset, real_current_example_index);

  // We assume an empty 'selected_examples' vector means we select them all
  // (not to waste space when saving this object).
  // TODO Find a cleaner way to do this.
  if (selected_examples.isEmpty())
    selected_examples = TVec<int>(0, n_real_examples - 1, 1);
  FROM_P_TVEC(selected_examples, selected_examples, dataset, selected_examples, n_examples);
  
  inherited::updateFromPLearn(dataset);
  // NB: not updating subsets, n_examples_subsets, n_subsets, pushed_examples,
  //                  inputs, targets
}

/////////////////////
// updateFromTorch //
/////////////////////
void TDataSet::updateFromTorch() {
  FROM_T_BASIC(n_real_examples,            n_real_examples,            dataset, n_real_examples           );
  FROM_T_BASIC(select_examples,            select_examples,            dataset, select_examples           );
  FROM_T_BASIC(n_inputs,                   n_inputs,                   dataset, n_inputs                  );
  FROM_T_BASIC(n_targets,                  n_targets,                  dataset, n_targets                 );
  FROM_T_BASIC(real_current_example_index, real_current_example_index, dataset, real_current_example_index);

  if (dataset->n_examples == 0)
    PLERROR("In TDataSet::updateFromTorch - No selected examples is currently not supported");

  if (dataset->n_examples == n_real_examples && options["selected_examples"])
    selected_examples.resize(0);
  else
    FROM_T_TVEC(selected_examples, selected_examples, dataset, selected_examples, n_examples);

  inherited::updateFromTorch();
}

} // end of namespace PLearn
