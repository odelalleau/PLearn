// -*- C++ -*-

// Splitter.h
//
// Copyright (C) 2002 Pascal Vincent, Frederic Morin
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

/*! \file Splitter.h */
#ifndef Splitter_INC
#define Splitter_INC

#include <plearn/base/Object.h>
#include <plearn/base/Array.h>
#include "VMat.h"

namespace PLearn {
using namespace std;

/**
 * This class is an abstract base class for mechanisms allowing to "split" a
 * dataset into one or several partitions (or "splits").
 *
 * Thus for instance a subclass can be used to implement k-fold splits (for
 * k-fold cross validation), where each of the k splits returned by
 * getSplit(i=0..k-1) would be an 2-element array containing the corresponding
 * training-set and test-set.
 *
 * A splitter is an essential part of a PTester.
 */
class Splitter : public Object
{
    typedef Object inherited;

protected:
    // *********************
    // * protected options *
    // *********************

    VMat dataset;

public:
    // ****************
    // * Constructors *
    // ****************

    PLEARN_DECLARE_ABSTRACT_OBJECT(Splitter);

    Splitter() {};

    //! Sets the dataset on which the splits are to be based
    virtual void setDataSet(VMat the_dataset);

    //! Returns the dataset given with setDataSet
    VMat getDataSet()
    { return dataset; }

    //! Returns the number of available different "splits"
    virtual int nsplits() const = 0;

    //! Returns the number of sets per split
    virtual int nSetsPerSplit() const = 0;

    //! Returns split number i
    virtual TVec<VMat> getSplit(int i=0) = 0;

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:    
    //! Declares the class options.
    static void declareOptions(OptionList& ol);

    //! Declare the methods that are remote-callable
    static void declareMethods(RemoteMethodMap& rmm);
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(Splitter);


// A few useful function for splitting the datasets... (can be called in Splitter subclasses

/*!   Splits the dataset d into a train and a test subset
  If test_fraction is <1.0 then the size of the test subset is set to be ntest = int(test_fraction*d.length())
  If test_fraction is >=1.0 then ntest = int(test_fraction)
  Last argument i allows to get the ith split of a K-fold cross validation
  i = 0 corresponds to having the last ntest samples used as the test set
  i = 1 means having the ntest samples before those as the test set, etc...
  If the bool 'use_all' is true, then the test set may contain more examples so that
  if we ask all splits, all the examples are contained in one test split.
*/
void split(VMat d, real test_fraction, VMat& train, VMat& test, int i = 0, bool use_all = false);

//!  Splits the dataset d into 3 subsets
void split(VMat d, real validation_fraction, real test_fraction, VMat& train, VMat& valid, VMat& test,bool do_shuffle=false);

/*!   Splits the dataset d into a train and a test subset
  randomly picking which samples should be in each subset.
  (test_fraction has the same meaning a sabove).
  Return the permuted indices (the last ntest of which are the test indices
  and the remainder are the train indices).
*/
Vec randomSplit(VMat d, real test_fraction, VMat& train, VMat& test);

//!  Splits the dataset d into 3 subsets (similar to above)
void randomSplit(VMat d, real validation_fraction, real test_fraction, VMat& train, VMat& valid, VMat& test);


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
