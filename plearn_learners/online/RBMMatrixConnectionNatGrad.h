// -*- C++ -*-

// RBMMatrixConnectionNatGrad.h
//
// Copyright (C) 2006 Yoshua Bengio
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

// Authors: Yoshua Bengio

/*! \file RBMMatrixConnectionNatGrad.h */


#ifndef RBMMatrixConnectionNatGrad_INC
#define RBMMatrixConnectionNatGrad_INC

#include "RBMMatrixConnection.h"
#include <plearn_learners/generic/NatGradEstimator.h>

namespace PLearn {
using namespace std;


/**
 * Stores and learns the parameters between two linear layers of an RBM.
 *
 */
class RBMMatrixConnectionNatGrad: public RBMMatrixConnection
{
    typedef RBMMatrixConnection inherited;

public:
    //#####  Public Build Options  ############################################

    //! natural gradient estimator for neurons
    //! (if 0 then do not correct the gradient on neurons)
    PP<NatGradEstimator> natgrad_template;

    //#####  Learned Options  #################################################
    TVec<PP<NatGradEstimator> > cd_natgrad;
    TVec<PP<NatGradEstimator> > bp_natgrad;

    //#####  Not Options  #####################################################

    Mat weights_gradient;
    Vec natural_gradient;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    RBMMatrixConnectionNatGrad( real the_learning_rate=0 );

    // Your other public member functions go here

    virtual void update( const Mat& pos_down_values,
                         const Mat& pos_up_values,
                         const Mat& neg_down_values,
                         const Mat& neg_up_values);

    virtual void bpropUpdate(const Mat& inputs, const Mat& outputs,
                             Mat& input_gradients,
                             const Mat& output_gradients,
                             bool accumulate = false);

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(RBMMatrixConnectionNatGrad);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //#####  Protected Member Functions  ######################################
    //! Declares the class options.
    static void declareOptions(OptionList& ol);

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    void build_();

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(RBMMatrixConnectionNatGrad);

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
