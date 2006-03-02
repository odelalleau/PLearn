// -*- C++ -*-

// MixUnlabeledNeighbourVMatrix.cc
//
// Copyright (C) 2006 Pierre-Jean L Heureux 
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

// Authors: Pierre-Jean L Heureux

/*! \file MixUnlabeledNeighbourVMatrix.cc */

#include <plearn/math/PRandom.h>
#include <plearn_learners/generic/PLearner.h>
#include "MixUnlabeledNeighbourVMatrix.h"

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(
    MixUnlabeledNeighbourVMatrix,
    "For a given labeled dataset, find related examples in a second dataset.",
    "The last column of the two dataset contains the relational key. The \n"
    " output will be a concatenated VMat with the labeled example on top \n"
    " and a selection of unlabeled related example at the bottom. The \n"
    " relational key column will be discarded. We put MISSING_VALUE for \n"
    " the unlabeled target. \n"
    );

//////////////////
// MixUnlabeledNeighbourVMatrix //
//////////////////
MixUnlabeledNeighbourVMatrix::MixUnlabeledNeighbourVMatrix() :
/* ### Initialize all fields to their default value */
    seed(-1),
    frac(1),
    random_generator(new PRandom())
{
    // ...
    // ### You may (or not) want to call build_() to finish building the object
    // ### (doing so assumes the parent classes' build_() have been called too
    // ### in the parent classes' constructors, something that you must ensure)
}

////////////////////
// declareOptions //
////////////////////
void MixUnlabeledNeighbourVMatrix::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify  
    // ### one of OptionBase::buildoption, OptionBase::learntoption or 
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    declareOption(ol, "seed", &MixUnlabeledNeighbourVMatrix::seed, OptionBase::buildoption, "Random generator seed (>0) (exceptions : -1 = initialized from clock, 0 = no initialization).");
    declareOption(ol, "source_select", &MixUnlabeledNeighbourVMatrix::source_select, OptionBase::buildoption, "The VMat containing the related examples.");
    declareOption(ol, "frac", &MixUnlabeledNeighbourVMatrix::frac, OptionBase::buildoption, "Fraction of the bag of related examples to be randomly chosen");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void MixUnlabeledNeighbourVMatrix::build()
{
    // ### Nothing to add here, simply calls build_
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void MixUnlabeledNeighbourVMatrix::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation. 
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
    // ### You should assume that the parent class' build_() has already been called.

    random_generator->manual_seed(seed);
 
    if (!source)
        return;
    
    if (source_select && source && source_select->inputsize() != source->inputsize())
        PLERROR("In MixUnlabeledNeighbourVMatrix::build_ - VMats 'source_select'"
                "and 'source' should have the same inputsize().");
    
    // Generating 'indices' inside source_select
//    int relational_key_column = source->targetsize() - 1;
    
    indices.resize(0); // This get rid of the user's build option value.
    TVec<int> bag_indices;

    Vec input,target,targetSel;
    real weight;
    neighbor_weights.resize(0);

    for(int row=0; row < source->length(); row++)
    {
        source->getExample(row,input,target,weight);
        bag_indices.resize(0);
        
        if(source_select) { // If it was given, find the related example
            for(int rowSel=0; rowSel < source_select->length(); row++) {
                source_select->getExample(rowSel,input,targetSel,weight);
                if (targetSel.lastElement() == target.lastElement()) {
                    bag_indices.push_back(rowSel);
                }
            }
            random_generator->shuffleElements(bag_indices);
            int n_kept = int(round(frac*bag_indices.length()));
            for(int i=0; i < n_kept; i++) {
                if (source->weightsize() > 0) {
                    assert( source->weightsize() == 1 );
                    neighbor_weights.append(weight);
                }
                indices.push_back(bag_indices[i]);
            }
        }
    }
    // ?? Modify the width, length, (targetsize, inputsize and weight) size attribute.
    inputsize_ = source->inputsize();
    targetsize_ = source->targetsize()-1;
    weightsize_ = source->weightsize();
    width_ = source->width()-1;
    
    if(source_select) {
        length_ = source->length() + indices.length();
    }else{
        length_ = source->length();
    }
        
    // ### In a SourceVMatrix, you will typically end build_() with:
    setMetaInfoFromSource();
}

///////////////
// getNewRow //
///////////////
void MixUnlabeledNeighbourVMatrix::getNewRow(int i, const Vec& v) const
{
    if (i < source->length()) {
        row_buffer.resize(source->width());
        source->getRow(i, row_buffer);
        int rest_size = width() - inputsize() - targetsize();
        int input_target_size = inputsize() + targetsize();
        v.subVec(0, input_target_size)
            << row_buffer.subVec(0, input_target_size);
        v.subVec(input_target_size, rest_size)
            << row_buffer.subVec(input_target_size + 1, rest_size);
    } else {
        source_select->getSubRow(indices[i - source->length()],0,
                                 v.subVec(0,inputsize()));
        for (int j=inputsize(); j < inputsize() + targetsize(); j++)
            v[j]=MISSING_VALUE;
        if (!neighbor_weights.isEmpty())
            v[v.length() - 1] = neighbor_weights[i - source->length()];
    }
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void MixUnlabeledNeighbourVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(indices, copies);
    deepCopyField(source_select, copies);
    deepCopyField(random_generator, copies);
    deepCopyField(row_buffer, copies);
    deepCopyField(neighbor_weights, copies);
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
