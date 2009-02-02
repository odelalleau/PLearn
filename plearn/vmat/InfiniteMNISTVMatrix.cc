// -*- C++ -*-

// InfiniteMNISTVMatrix.cc
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

/*! \file InfiniteMNISTVMatrix.cc */


#include "InfiniteMNISTVMatrix.h"

namespace PLearn {
using namespace std;


// Initialize static variables
mnistproblem_t* InfiniteMNISTVMatrix::dataset = 0;
int InfiniteMNISTVMatrix::n_pointers_to_dataset = 0;

PLEARN_IMPLEMENT_OBJECT(
    InfiniteMNISTVMatrix,
    "VMatrix containing an \"infinite\" stream of MNIST samples.",
    "VMatrix that uses the code from \"Training Invariant Support Vector Machines\n"
    "using Selective Sampling\" by Loosli, Canu and Bottou (JMLR 2007), to generate\n"
    "\"infinite\" stream (i.e. INT_MAX sized set) of samples from the MNIST dataset. The samples\n"
    "are obtained by applying some class-invariante transformations on the original MNIST\n"
    "dataset.\n"
    );

InfiniteMNISTVMatrix::InfiniteMNISTVMatrix():
    include_test_examples(false),
    include_validation_examples(false),
    random_switch_to_original_training_set(false),
    proportion_of_switches(0.0),
    seed(1834),
    input_divisor(1.),
    test_images(TEST_IMAGES_PATH),
    test_labels(TEST_LABELS_PATH),
    train_images(TRAIN_IMAGES_PATH),
    train_labels(TRAIN_LABELS_PATH),
    fields(FIELDS_PATH),
    tangent_vectors(TANGVEC_PATH)
/* ### Initialize all fields to their default value */
{
    InfiniteMNISTVMatrix::n_pointers_to_dataset++;
    random_gen = new PRandom();
    image = (unsigned char*)malloc(EXSIZE);
}

InfiniteMNISTVMatrix::~InfiniteMNISTVMatrix()
{
    InfiniteMNISTVMatrix::n_pointers_to_dataset--;
    if( InfiniteMNISTVMatrix::dataset && InfiniteMNISTVMatrix::n_pointers_to_dataset == 0 )
    {
            destroy_mnistproblem(dataset);
            InfiniteMNISTVMatrix::dataset = 0;
    }
    free( image );
}

void InfiniteMNISTVMatrix::getNewRow(int i, const Vec& v) const
{
    int i_dataset;
    if( include_test_examples )
        if( include_validation_examples )
            i_dataset = i;
        else
            if( i < 10000)
                i_dataset = i;
            else
                i_dataset = i + ((i-10000)/50000)*10000;
    else
        if( include_validation_examples )
            i_dataset = i+10000;
        else
            i_dataset = i + (i/50000)*10000 + 10000;

    if( random_switch_to_original_training_set && 
        random_gen->uniform_sample() < proportion_of_switches )
        i_dataset = (i_dataset % 50000)+10000;

    image = compute_transformed_vector_in_place(InfiniteMNISTVMatrix::dataset, i_dataset, image);

    unsigned char* xj=image;
    real* vj=v.data();
    for( int j=0; j<inputsize_; j++, xj++, vj++ )
        *vj = *xj/input_divisor;
    
    v.last() = InfiniteMNISTVMatrix::dataset->y[ (i_dataset<10000) ? i_dataset : 10000 + ((i_dataset - 10000) % 60000) ];
}

void InfiniteMNISTVMatrix::declareOptions(OptionList& ol)
{
     declareOption(ol, "include_test_examples", &InfiniteMNISTVMatrix::include_test_examples,
                   OptionBase::buildoption,
                   "Indication that the test examples from the MNIST dataset should be included.\n"
                   "This option is false by default. If true, these examples will be the first"
                   "10000\n"
                   "of this VMatrix.\n");

     declareOption(ol, "include_validation_examples", &InfiniteMNISTVMatrix::include_validation_examples,
                   OptionBase::buildoption,
                   "Indication that the validation set examples (the last 10000 examples from the\n"
                   "training set) should be included in this VMatrix.\n");     

     declareOption(ol, "random_switch_to_original_training_set", 
                   &InfiniteMNISTVMatrix::random_switch_to_original_training_set,
                   OptionBase::buildoption,
                   "Indication that the VMatrix should randomly (from time to time) provide\n"
                   "an example from the original training set instead of an example\n"
                   "from the global dataset.\n");     

     declareOption(ol, "proportion_of_switches", &InfiniteMNISTVMatrix::proportion_of_switches,
                   OptionBase::buildoption,
                   "Proportion of switches to the original training set.\n");     

     declareOption(ol, "seed", &InfiniteMNISTVMatrix::seed,
                   OptionBase::buildoption,
                   "Seed of random number generator.\n");

     declareOption(ol, "input_divisor", &InfiniteMNISTVMatrix::input_divisor,
                   OptionBase::buildoption,
                   "Value that the inputs should be divided by.\n");     

     declareOption(ol, "test_images", &InfiniteMNISTVMatrix::test_images,
                   OptionBase::buildoption,
                   "File path of MNIST test images.\n");     

     declareOption(ol, "test_labels", &InfiniteMNISTVMatrix::test_labels,
                   OptionBase::buildoption,
                   "File path of MNIST test labels.\n");     

     declareOption(ol, "train_images", &InfiniteMNISTVMatrix::train_images,
                   OptionBase::buildoption,
                   "File path of MNIST train images.\n");     

     declareOption(ol, "train_labels", &InfiniteMNISTVMatrix::train_labels,
                   OptionBase::buildoption,
                   "File path of MNIST train labels.\n");     

     declareOption(ol, "fields", &InfiniteMNISTVMatrix::fields,
                   OptionBase::buildoption,
                   "File path of MNIST fields information.\n");     

     declareOption(ol, "tangent_vectors", &InfiniteMNISTVMatrix::tangent_vectors,
                   OptionBase::buildoption,
                   "File paht of MNIST transformation tangent vectors.\n");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void InfiniteMNISTVMatrix::build_()
{
    random_gen->manual_seed(seed);

    if( !InfiniteMNISTVMatrix::dataset )
    {
        char* test_images_char = new char[test_images.size()+1];
        char* test_labels_char = new char[test_labels.size()+1];
        char* train_images_char = new char[train_images.size()+1];
        char* train_labels_char = new char[train_labels.size()+1];
        char* fields_char = new char[fields.size()+1];
        char* tangent_vectors_char = new char[tangent_vectors.size()+1];
        
        strcpy(test_images_char,test_images.c_str());
        strcpy(test_labels_char,test_labels.c_str());
        strcpy(train_images_char,train_images.c_str());
        strcpy(train_labels_char,train_labels.c_str());
        strcpy(fields_char,fields.c_str());
        strcpy(tangent_vectors_char,tangent_vectors.c_str());
        
        InfiniteMNISTVMatrix::dataset = create_mnistproblem(
            test_images_char,
            test_labels_char,
            train_images_char,
            train_labels_char,
            fields_char,
            tangent_vectors_char);
        if( !InfiniteMNISTVMatrix::dataset )
            PLERROR("In InfiniteMNISTVMatrix(): could not load MNIST dataset");
    }


    if( include_test_examples )
        if( include_validation_examples )
            length_ = INT_MAX;
        else
            // Might be removing more samples than need, but we have so many anyways...
            length_ = INT_MAX - ((INT_MAX-10000)/50000)*10000 + 1;
    
    else
        if( include_validation_examples )
            length_ = INT_MAX - 10000+1;
        else
            // Might be removing more samples than need, but we have so many anyways...
            length_ = INT_MAX - ((INT_MAX-10000)/50000)*10000 - 10000 + 1;

    inputsize_ = 784;
    targetsize_ = 1;
    weightsize_ = 0;
    extrasize_ = 0;
    width_ = 785;

    // ### You should keep the line 'updateMtime(0);' if you don't implement the 
    // ### update of the mtime. Otherwise you can have an mtime != 0 that is not valid.
    updateMtime(0);
    //updateMtime(filename);
    //updateMtime(VMat);
}

// ### Nothing to add here, simply calls build_
void InfiniteMNISTVMatrix::build()
{
    inherited::build();
    build_();
}

void InfiniteMNISTVMatrix::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    PLWARNING("InfiniteMNISTVMatrix::makeDeepCopyFromShallowCopy is not totally implemented. Need "
              "to figure out how to deep copy the \"dataset\" variable (mnistproblem_t*).\n");
    InfiniteMNISTVMatrix::n_pointers_to_dataset++;
    image = (unsigned char*)malloc(EXSIZE);
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
