// -*- C++ -*-

// ClassLeaveOneOutSplitter.cc
//
// Copyright (C) 2006 Hugo Larochelle 
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
   * $Id: .pyskeleton_header 544 2003-09-01 00:05:31Z plearner $ 
   ******************************************************* */

// Authors: Hugo Larochelle

/*! \file ClassLeaveOneOutSplitter.cc */

#include "RemoveRowsVMatrix.h"
#include "SelectRowsVMatrix.h"
#include "ClassLeaveOneOutSplitter.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    ClassLeaveOneOutSplitter,
    "Splitter that separates examples of one class (test) from the examples of other classes (train)",
    "This splitter is intended to measure inductive transfer performance from some tasks to another task"
    );

ClassLeaveOneOutSplitter::ClassLeaveOneOutSplitter() 
    :Splitter(),nclasses(-1)
{
}

void ClassLeaveOneOutSplitter::declareOptions(OptionList& ol)
{
    declareOption(ol, "nclasses", &ClassLeaveOneOutSplitter::nclasses, OptionBase::buildoption,
                  "Number of classes. If classes if defined, then it is ignored. If not, then classes is filled with all possible classes.");
    declareOption(ol, "classes", &ClassLeaveOneOutSplitter::classes, OptionBase::buildoption,
                  "Classes to isolate from the others.");
    declareOption(ol, "append_train", &ClassLeaveOneOutSplitter::append_train, OptionBase::buildoption,
                  "Indication that the training set should be appended to the split sets lists.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void ClassLeaveOneOutSplitter::build_()
{
    if(classes.length() == 0)
    {
        if(nclasses <= 0) PLERROR("In ClassLeaveOneOutSplitter::build_(): nclasses should be > 0");
        for(int i=0; i<nclasses; i++)
            classes.push_back(i);
    }
}

// ### Nothing to add here, simply calls build_
void ClassLeaveOneOutSplitter::build()
{
    inherited::build();
    build_();
}

void ClassLeaveOneOutSplitter::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    Splitter::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(classes, copies);

    //PLERROR("ClassLeaveOneOutSplitter::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

int ClassLeaveOneOutSplitter::nsplits() const
{
    return classes.length();
}

int ClassLeaveOneOutSplitter::nSetsPerSplit() const
{
    return append_train ? 3 : 2;
}

TVec<VMat> ClassLeaveOneOutSplitter::getSplit(int k)
{
    
    if (k >= nsplits())
        PLERROR("ClassLeaveOneOutSplitter::getSplit() - k (%d) cannot be greater than "
                " the number of splits (%d)", k, nsplits());
 
    int class_k = classes[k];
    Vec row(dataset->width());
    TVec<int> indices(0);
    Vec indices_real(0);
    for(int i=0; i<dataset->length(); i++)
    {
        dataset->getRow(i,row);
        if((int)row[dataset->inputsize()] == class_k)
        {
            indices.push_back(i);
            indices_real.push_back(i);
        }
    }
    
    TVec<VMat> split(2);
    split[0] = new RemoveRowsVMatrix(dataset,indices_real);
    // RemoveRowsVMatrix does not set the different sizes
    // Maybe this should be corrected?
    split[0]->defineSizes(dataset->inputsize(),dataset->targetsize(),dataset->weightsize());
    split[1] = new SelectRowsVMatrix(dataset,indices);
    if(append_train) split.append(split[0]);
    return split;
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
