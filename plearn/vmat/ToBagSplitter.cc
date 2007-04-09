// -*- C++ -*-

// ToBagSplitter.cc
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
 * $Id$
 ******************************************************* */

// Authors: Olivier Delalleau

/*! \file ToBagSplitter.cc */


#include "SelectRowsVMatrix.h"
#include <plearn/var/SumOverBagsVariable.h>  //!< For SumOverBagsVariable::TARGET_COLUMN_LAST.
#include "ToBagSplitter.h"

namespace PLearn {
using namespace std;

///////////////////
// ToBagSplitter //
///////////////////
ToBagSplitter::ToBagSplitter()
    : Splitter(),
      expected_size_of_bag(10)
{}

PLEARN_IMPLEMENT_OBJECT(ToBagSplitter,
                        "A Splitter that makes any existing splitter operate on bags only.",
                        "The dataset provided must contain bag information, as described in\n"
                        "SumOverBagsVariable");

////////////////////
// declareOptions //
////////////////////
void ToBagSplitter::declareOptions(OptionList& ol)
{
    declareOption(ol, "expected_size_of_bag", &ToBagSplitter::expected_size_of_bag, OptionBase::buildoption,
                  "The expected size of each bag. It is not compulsory to change this option.");

    declareOption(ol, "sub_splitter", &ToBagSplitter::sub_splitter, OptionBase::buildoption,
                  "The underlying splitter we want to make operate on bags.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void ToBagSplitter::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void ToBagSplitter::build_()
{
    if (dataset) {
        // Prepare the bags index list.
        int max_ninstances = 1;
        // The first column in bags_store gives the number of instances in the bag,
        // and the following columns give the indices of the corresponding rows in
        // the original dataset.
        Mat bags_store(dataset->length() / expected_size_of_bag + 1, expected_size_of_bag + 1);
        int num_bag = 0;
        int num_instance = 0;
        int bag_signal_column = dataset->inputsize() + dataset->targetsize() - 1; // Bag signal in the last target column.
        for (int i = 0; i < dataset->length(); i++) {
            if (num_instance + 1 >= bags_store.width()) {
                if (num_instance > 10*(expected_size_of_bag+1))
                    PLERROR("ToBagSplitter: found bag size (%d) more than 10 times bigger than expected_size_of_bag (%d)!\n",
                            num_instance,expected_size_of_bag);
                // Need to resize bags_store.
                bags_store.resize(bags_store.length(), bags_store.width() * 2, 0, true);
            }
            if (num_instance >= max_ninstances) {
                max_ninstances = num_instance + 1;
            }
            bags_store(num_bag, num_instance + 1) = i;
            num_instance++;
            if (int(dataset->get(i, bag_signal_column)) & SumOverBagsVariable::TARGET_COLUMN_LAST) {
                // Last element of a bag.
                bags_store(num_bag, 0) = num_instance; // Store the number of instances in this bag.
                num_bag++;
                num_instance = 0;
                if (num_bag >= bags_store.length()) {
                    // Need to resize bags_store.
                    bags_store.resize(bags_store.length() * 2, bags_store.width(), 0, true);
                }
            }
        }
        // Resize to the minimum size needed.
        bags_store.resize(num_bag, max_ninstances + 1, 0, true);
        bags_index = VMat(bags_store);
        bags_index->savePMAT("HOME:tmp/bid.pmat");
        // Provide this index to the sub_splitter.
        sub_splitter->setDataSet(bags_index);
    }
}

//////////////
// getSplit //
//////////////
TVec<VMat> ToBagSplitter::getSplit(int k)
{
    // ### Build and return the kth split
    TVec<VMat> sub_splits = sub_splitter->getSplit(k);
    TVec<VMat> result;
    for (int i = 0; i < sub_splits.length(); i++) {
        // Get the list of corresponding indices in the original dataset.
        Mat indices = sub_splits[i].toMat();
        // Turn it into a TVec<int>.
        TVec<int> indices_int;
        for (int j = 0; j < indices.length(); j++) {
            for (int q = 0; q < indices(j, 0); q++) {
                int indice = int(indices(j, q + 1));
                indices_int.append(indice);
            }
        }
        result.append(new SelectRowsVMatrix(dataset, indices_int));
    }
    return result;
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void ToBagSplitter::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    deepCopyField(bags_index, copies);
    deepCopyField(sub_splitter, copies);

}

///////////////////
// nSetsPerSplit //
///////////////////
int ToBagSplitter::nSetsPerSplit() const
{
    // ### Return the number of sets per split
    return sub_splitter->nSetsPerSplit();
}

/////////////
// nsplits //
/////////////
int ToBagSplitter::nsplits() const
{
    return sub_splitter->nsplits();
}

////////////////
// setDataSet //
////////////////
void ToBagSplitter::setDataSet(VMat the_dataset) {
    inherited::setDataSet(the_dataset);
    // Need to recompute the bags index.
    build();
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
