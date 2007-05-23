// -*- C++ -*-

// MaxSubsamplingTest.h
//
// Copyright (C) 2007 Pascal Lamblin
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

// Authors: Pascal Lamblin

/*! \file MaxSubsamplingTest.h */


#ifndef MaxSubsamplingTest_INC
#define MaxSubsamplingTest_INC

#include <plearn/misc/PTest.h>
#include <plearn/math/PRandom.h>
#include <plearn_learners/online/MaxSubsampling2DModule.h>

namespace PLearn {

/**
 * Tests MaxSubsampling2DModule.
 */
class MaxSubsamplingTest : public PTest
{
    typedef PTest inherited;

public:
    //#####  Public Build Options  ############################################

    // ### declare public option fields (such as build options) here
    // Start your comments with Doxygen-compatible comments such as //!
    // ### Typically, a PTest options are used to store the test results.

    int batch_size;
    int n_input_images;
    int input_images_length;
    int input_images_width;
    int kernel_length;
    int kernel_width;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    // ### Make sure the implementation in the .cc
    // ### initializes all fields to reasonable default values.
    MaxSubsamplingTest();

    // Your other public member functions go here


    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT
    PLEARN_DECLARE_OBJECT(MaxSubsamplingTest);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    // (PLEASE IMPLEMENT IN .cc)
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    //#####  PLearn::PTest Protocol  ##########################################

    //! The method performing the test. A typical test consists in some output
    //! (to pout and / or perr), and updates of this object's options.
    virtual void perform();

protected:
    //#####  Protected Options  ###############################################

    // ### Declare protected option fields (such as learned parameters) here
    int output_images_length;
    int output_images_width;
    int input_images_size;
    int output_images_size;
    int input_size;
    int output_size;

    PP<MaxSubsampling2DModule> max_module;
    PP<PRandom> random_gen;

protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    // (PLEASE IMPLEMENT IN .cc)
    static void declareOptions(OptionList& ol);

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
DECLARE_OBJECT_PTR(MaxSubsamplingTest);

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
