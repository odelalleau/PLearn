// -*- C++ -*-

// InfiniteMNISTVMatrix.h
//
// Copyright (C) 2008 Hugo Larochelle
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

// Authors: Hugo Larochelle

/*! \file InfiniteMNISTVMatrix.h */


#ifndef InfiniteMNISTVMatrix_INC
#define InfiniteMNISTVMatrix_INC

#define TEST_IMAGES_PATH "/home/fringant2/lisa/data/mnist/t10k-images-idx3-ubyte"
#define TEST_LABELS_PATH "/home/fringant2/lisa/data/mnist/t10k-labels-idx1-ubyte"
#define TRAIN_IMAGES_PATH "/home/fringant2/lisa/data/mnist/train-images-idx3-ubyte"
#define TRAIN_LABELS_PATH "/home/fringant2/lisa/data/mnist/train-labels-idx1-ubyte"
#define FIELDS_PATH "/home/fringant2/lisa/data/mnist/fields_float_1522x28x28.bin"
#define TANGVEC_PATH "/home/fringant2/lisa/data/mnist/tangVec_float_60000x28x28.bin"

#include <plearn/vmat/RowBufferedVMatrix.h>
#include <plearn/math/PRandom.h>
#include <kernel-invariant.h>
//#include "kernel-invariant2.h"

namespace PLearn {

/**
 * VMatrix that uses the code from "Training Invariant Support Vector Machines 
 * using Selective Sampling" by Loosli, Canu and Bottou (JMLR 2007), to generate 
 * "infinite" stream (i.e. INT_MAX sized set) of samples from the MNIST dataset. The samples
 * are obtained by applying some class-invariante transformations on the original MNIST
 * dataset.
 */
class InfiniteMNISTVMatrix : public RowBufferedVMatrix
{
    typedef RowBufferedVMatrix inherited;

public:
    //#####  Public Build Options  ############################################

    //! Indication that the test examples from the MNIST dataset should be included.
    //! This option is false by default. If true, these examples will be the first 10000
    //! of this VMatrix.
    bool include_test_examples;

    //! Indication that the validation set examples (the last 10000 examples from the
    //! training set) should be included in this VMatrix.
    bool include_validation_examples;

    //! Indication that the VMatrix should randomly (from time to time) provide
    //! an example from the original training set instead of an example
    //! from the global dataset
    bool random_switch_to_original_training_set;
    
    //! Proportion of switches to the original training set
    real proportion_of_switches;
    
    //! Seed of random number generator
    int seed;

    //! Value that the inputs should be divided by.
    real input_divisor;

    //! Random number generator
    PP< PRandom > random_gen;

    // Files required for loading infinite MNIST dataset
    //! File path of MNIST test images.
    string test_images;
    //! File path of MNIST test labels.
    string test_labels;
    //! File path of MNIST train images.
    string train_images;
    //! File path of MNIST train labels.
    string train_labels;
    //! File path of MNIST fields information.
    string fields;
    //! File path of MNIST transformation tangent vectors.
    string tangent_vectors;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    // ### Make sure the implementation in the .cc
    // ### initializes all fields to reasonable default values.
    InfiniteMNISTVMatrix();

    ~InfiniteMNISTVMatrix();

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS
    PLEARN_DECLARE_OBJECT(InfiniteMNISTVMatrix);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    // (PLEASE IMPLEMENT IN .cc)
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //#####  Protected Options  ###############################################

    static mnistproblem_t *dataset;
    static int n_pointers_to_dataset;
    mutable unsigned char* image;

protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    // (PLEASE IMPLEMENT IN .cc)
    static void declareOptions(OptionList& ol);

    //! Fill the vector 'v' with the content of the i-th row.
    //! 'v' is assumed to be the right size.
    // (PLEASE IMPLEMENT IN .cc)
    virtual void getNewRow(int i, const Vec& v) const;

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    // (PLEASE IMPLEMENT IN .cc)
    void build_();

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(InfiniteMNISTVMatrix);

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
