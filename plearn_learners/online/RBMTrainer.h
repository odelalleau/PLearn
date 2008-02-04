// -*- C++ -*-

// RBMTrainer.h
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

/*! \file RBMTrainer.h */


#ifndef RBMTrainer_INC
#define RBMTrainer_INC

#include <plearn/base/Object.h>
#include <plearn/vmat/VMat.h>
#include <plearn_learners/online/RBMModule.h>

namespace PLearn {

/**
 * The first sentence should be a BRIEF DESCRIPTION of what the class does.
 * Place the rest of the class programmer documentation here.  Doxygen supports
 * Javadoc-style comments.  See http://www.doxygen.org/manual.html
 *
 * @todo Write class to-do's here if there are any.
 *
 * @deprecated Write deprecated stuff here if there is any.  Indicate what else
 * should be used instead.
 */
class RBMTrainer : public Object
{
    typedef Object inherited;

public:
    //#####  Public Build Options  ############################################

    //! ### declare public option fields (such as build options) here
    //! Start your comments with Doxygen-compatible comments such as //!
    int n_visible;
    int n_hidden;
    string visible_type;
    bool update_with_h0_sample;
    bool sample_v1_in_chain;
    bool compute_log_likelihood;
    int n_stages;
    real learning_rate;
    int32_t seed;
    int n_train;
    int n_valid;
    int n_test;
    int batch_size;
    PPath data_filename;
    PPath save_path;
    PPath save_name;
    bool print_debug;
    bool use_fast_approximations;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    RBMTrainer();

    // Your other public member functions go here
    Mat NLL(const Mat& examples);

    Mat recError(const Mat& examples);

    void CD1(const Mat& examples);

    virtual void run();


    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(RBMTrainer);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    // (PLEASE IMPLEMENT IN .cc)
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //#####  Protected Options  ###############################################

    // ### Declare protected option fields (such as learned parameters) here
    VMat data;
    VMat train_input;
    VMat valid_input;
    VMat test_input;

    PP<RBMModule> rbm;
    PP<RBMLayer> visible;
    PP<RBMLayer> hidden;
    PP<RBMConnection> connection;

    TVec<string> ports;
    TVec<string> state_ports;
    int n_ports;
    int n_state_ports;
    TVec<int> state_ports_indices;
    int nll_index;
    int visible_index;
    int rec_err_index;

protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    static void declareOptions(OptionList& ol);

    static void declareMethods(RemoteMethodMap& rmm);

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    void build_();

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
    mutable TVec<Mat*> nll_values;
    mutable TVec<Mat*> rec_err_values;

    mutable Mat h0_a;
    mutable Mat h0_e;
    mutable Mat h0_s;
    mutable Mat h0;

    mutable Mat v1_a;
    mutable Mat v1_e;
    mutable Mat v1_s;
    mutable Mat v1;



};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(RBMTrainer);

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
