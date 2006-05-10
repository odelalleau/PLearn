// -*- C++ -*-

// TestInTrainSplitter.h
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

/*! \file TestInTrainSplitter.h */


#ifndef TestInTrainSplitter_INC
#define TestInTrainSplitter_INC

#include "Splitter.h"

namespace PLearn {
using namespace std;

class TestInTrainSplitter: public Splitter
{

private:

    typedef Splitter inherited;

protected:
    // *********************
    // * protected options *
    // *********************

    // ### declare protected option fields (such as learnt parameters) here
    // ...

    // Fields below are not options.

    //! Store the first split provided by the source_splitter.
    //! This is because we need it the number of points in the train and test
    //! parts in the nsplits() method.
    mutable TVec<VMat> first_source_split;

    //! The number of samples in the test set that would be left if we
    //! didn't make sure they were all added once.
    mutable int n_left;

    //! The number of splits to make for each split given by the source_splitter.
    mutable int n_splits_per_source_split;

    //! The number of samples in the train and test parts, as given by
    //! the source_splitter.
    mutable int n_train, n_test;

    //! The number of sample to add each time in the train set.
    mutable int n_to_add;

public:

    // ************************
    // * public build options *
    // ************************

    real percentage_added;
    PP<Splitter> source_splitter;

    // ****************
    // * Constructors *
    // ****************

    // Default constructor, make sure the implementation in the .cc
    // initializes all fields to reasonable default values.
    TestInTrainSplitter();


    // ******************
    // * Object methods *
    // ******************

private:
    //! This does the actual building.
    // (Please implement in .cc)
    void build_();

protected:
    //! Declares this class' options
    // (Please implement in .cc)
    static void declareOptions(OptionList& ol);

public:
    // simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    //! Declares name and deepCopy methods
    PLEARN_DECLARE_OBJECT(TestInTrainSplitter);


    // ********************************
    // *        Splitter methods      *
    // ********************************

    //! Overridden to forward to underlying splitter and reset 'first_source_split'.
    virtual void setDataSet(VMat the_dataset);

    //! Returns the number of available different "splits"
    virtual int nsplits() const;

    //! Returns the number of sets per split
    virtual int nSetsPerSplit() const;

    //! Compute the first split of source_splitter and store it in 'first_source_split'.
    virtual void getFirstSplit() const;

    //! Returns split number i
    virtual TVec<VMat> getSplit(int i=0);

};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(TestInTrainSplitter);

} // end of namespace PLearn

#endif


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
