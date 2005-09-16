// -*- C++ -*-

// SelectRowsMultiInstanceVMatrix.cc
//
// Copyright (C) 2005 Benoit Cromp 
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

// Authors: Benoit Cromp

/*! \file SelectRowsMultiInstanceVMatrix.cc */
#include <plearn/math/PRandom.h>
#include <plearn_learners/generic/PLearner.h>
#include "SelectRowsMultiInstanceVMatrix.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    SelectRowsMultiInstanceVMatrix,
    "ONE LINE DESCRIPTION",
    "MULTI-LINE \nHELP"
    );

//////////////////
// SelectRowsMultiInstanceVMatrix //
//////////////////
SelectRowsMultiInstanceVMatrix::SelectRowsMultiInstanceVMatrix() :
    random_generator(new PRandom())
/* ### Initialize all fields to their default value */
{
    // ...
    // ### You may (or not) want to call build_() to finish building the object
    // ### (doing so assumes the parent classes' build_() have been called too
    // ### in the parent classes' constructors, something that you must ensure)
}

////////////////////
// declareOptions //
////////////////////
void SelectRowsMultiInstanceVMatrix::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify  
    // ### one of OptionBase::buildoption, OptionBase::learntoption or 
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    // ### ex:
    // declareOption(ol, "myoption", &SelectRowsMultiInstanceVMatrix::myoption, OptionBase::buildoption,
    //               "Help text describing this option");
    // ...
    declareOption(ol, "seed", &SelectRowsMultiInstanceVMatrix::seed, OptionBase::buildoption, "Random generator seed (>0) (exceptions : -1 = initialized from clock, 0 = no initialization).");
//    declareOption(ol, "indices", &SelectRowsMultiInstanceVMatrix::indices, OptionBase::buildoption, "Indices of kept rows");
      declareOption(ol, "multi_nnet", &SelectRowsMultiInstanceVMatrix::multi_nnet, OptionBase::buildoption, "MultiNNet from which you select instances");
      declareOption(ol, "frac", &SelectRowsMultiInstanceVMatrix::frac, OptionBase::buildoption, "Fraction of the bag to be randomly chosen (Note: this is the fraction of the bag to be randomly chosen in addition of the best instance, which is always added");
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
   
}

///////////
// build //
///////////
void SelectRowsMultiInstanceVMatrix::build()
{
    // ### Nothing to add here, simply calls build_
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void SelectRowsMultiInstanceVMatrix::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation. 
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
    // ### You should assume that the parent class' build_() has already been called.

    random_generator->manual_seed(seed);

// Generating 'indices' and 'mi_info' vector.
    
    int bag_signal_column = source->targetsize() - 1;
    int first_row = 0;
    int x_max;

    indices.resize(0); // This get rid of the user's build option value.
    mi_info.resize(0);
    TVec<int> bag_indices;
    TVec<double> bag_prob;
    bag_indices.resize(0);
    bag_prob.resize(0);
    // Vec prob;
    Vec input,target,output(1);
    real weight;
    int indices_size;
    for(int row=0; row<source->length(); row++)
    {
      source->getExample(row,input,target,weight);
        switch(int(target[bag_signal_column]))
        {
        case 0:
            multi_nnet->computeOutput(input,output);
            bag_prob.push_back(output[0]);
            break;
        case 1:
            first_row = row;
            multi_nnet->computeOutput(input, output);
            bag_prob.push_back(output[0]);
            break;
        case 2:
            multi_nnet->computeOutput(input, output);
            bag_prob.push_back(output[0]);
            x_max=argmax(bag_prob);
            bag_indices.resize(0);
            for(int i=0;i<row-first_row;i++) {
                if(i!=x_max) bag_indices.push_back(i);
            }
            random_generator->shuffleElements(bag_indices);
            // Append retained elements to indices
            indices_size = indices.length();
            indices.push_back(first_row+x_max);
            for(int i=0;i<frac*bag_indices.length();i++) {
                indices.push_back(first_row+bag_indices[i]);
            }
            if(indices.length()-indices_size ==1) mi_info.push_back(3);
            if(indices.length()-indices_size > 1) {
                mi_info.push_back(1);
                for(int i=0;i<indices.length()-indices_size-2;i++) mi_info.push_back(0);
                mi_info.push_back(2);
            }
            bag_prob.resize(0);
            break;
        case 3:
            indices.push_back(row);
            mi_info.push_back(3);
            break;
        };
    }

// ?? Modify the width, length, (tagetsize, inputsize and weight) size attribute.
    length_ = indices.length();
    inputsize_ = source->inputsize();
    targetsize_ = source->targetsize();
    weightsize_ = source->weightsize(); 
 
    // ### In a SourceVMatrix, you will typically end build_() with:
    setMetaInfoFromSource();
}

///////////////
// getNewRow //
///////////////
void SelectRowsMultiInstanceVMatrix::getNewRow(int i, const Vec& v) const
{
    int bag_signal_column = inputsize_ + targetsize_ - 1;
    source->getRow(indices[i], v);
    v[bag_signal_column]=mi_info[i];
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void SelectRowsMultiInstanceVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields 
    // ### that you wish to be deepCopied rather than 
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("SelectRowsMultiInstanceVMatrix::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
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
