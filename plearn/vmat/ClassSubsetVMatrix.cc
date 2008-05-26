// -*- C++ -*-

// ClassSubsetVMatrix.cc
//
// Copyright (C) 2004 Olivier Delalleau
// Copyright (C) 2008 Jerome Louradour
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
   * $Id: ClassSubsetVMatrix.cc 3761 2005-07-11 18:13:16Z tihocan $
   ******************************************************* */

// Authors: Hugo Larochelle

/*! \file ClassSubsetVMatrix.cc */


#include "ClassSubsetVMatrix.h"

namespace PLearn {
using namespace std;

////////////////////////////
// ClassSubsetVMatrix //
////////////////////////////
ClassSubsetVMatrix::ClassSubsetVMatrix()
  : redistribute_classes(0), one_vs_minus_one_classification(0)
{
  // ...
  // ### You may or may not want to call build_() to finish building the object
  // build_();
}

ClassSubsetVMatrix::ClassSubsetVMatrix(VMat the_source, TVec<int> the_classes)
  : redistribute_classes(0), one_vs_minus_one_classification(0)
{
  source = the_source;
  CopiesMap copies;
  classes = the_classes.deepCopy(copies);
}

ClassSubsetVMatrix::ClassSubsetVMatrix(VMat the_source, int the_class)
  : redistribute_classes(0), one_vs_minus_one_classification(0)
{
  source = the_source;
  classes = TVec<int>(1, the_class);
}


PLEARN_IMPLEMENT_OBJECT(ClassSubsetVMatrix,
    "A VMatrix that keeps examples for a subset of the classes (target).",
    ""
);

////////////////////
// declareOptions //
////////////////////
void ClassSubsetVMatrix::declareOptions(OptionList& ol)
{
  // ### Declare all of this object's options here
  // ### For the "flags" of each option, you should typically specify
  // ### one of OptionBase::buildoption, OptionBase::learntoption or
  // ### OptionBase::tuningoption. Another possible flag to be combined with
  // ### is OptionBase::nosave

  declareOption(ol, "classes", &ClassSubsetVMatrix::classes, OptionBase::buildoption,
                "Classes of examples to keep.\n");

  declareOption(ol, "redistribute_classes", &ClassSubsetVMatrix::redistribute_classes, OptionBase::buildoption,
                "Indication that the class values should be redistributed between 0 and classes.length()-1,\n"
                "based on the order of apperance in the vector classes.\n");

  declareOption(ol, "one_vs_minus_one_classification", &ClassSubsetVMatrix::one_vs_minus_one_classification, OptionBase::buildoption,
                "Indication that, if classes contains 2 class indexes,\n"
                "than they should be mapped to -1 (the first index) and 1.\n");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

///////////
// build //
///////////
void ClassSubsetVMatrix::build()
{
  // ### Nothing to add here, simply calls build_
  inherited::build();
  build_();
}

////////////
// build_ //
////////////
void ClassSubsetVMatrix::build_()
{
  if (source) {
    setMetaInfoFrom(source);
    input.resize(inputsize());
    target.resize(targetsize());
    indices.clear();
    for(int i=0;i<source->length();i++)
    {
      source->getExample(i,input,target,weight);
      if(classes.find((int)target[0]) != -1)
        indices.push_back(i);
    }
    //if(indices.length() == 0)
    //  PLERROR("In ClassSubsetVMatrix::build_(): no examples kept");
    inherited::build();
    if(one_vs_minus_one_classification && classes.length()!=2)
      PLERROR("In ClassSubsetVMatrix::build_(): no examples kept");
  }
}

real ClassSubsetVMatrix::get(int i, int j) const
{
  if(!redistribute_classes || j != inputsize())
    return source->get(indices[i], j);
  else
  {
    if(one_vs_minus_one_classification)
      return 2*classes.find((int)source->get(indices[i], j))-1;
    else
      return classes.find((int)source->get(indices[i], j));
  }
}

void ClassSubsetVMatrix::getSubRow(int i, int j, Vec v) const
{
  source->getSubRow(indices[i], j, v);
  if(redistribute_classes && j+v.length() > inputsize())
  {
    if(one_vs_minus_one_classification)
      v[inputsize()-j] = 2*classes.find((int)v[inputsize()-j])-1;
    else
      v[inputsize()-j] = classes.find((int)v[inputsize()-j]);
  }
}


/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void ClassSubsetVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(classes,copies);
  deepCopyField(input,copies);
  deepCopyField(target,copies);
  //PLERROR("ClassSubsetVMatrix::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

} // end of namespace PLearn

