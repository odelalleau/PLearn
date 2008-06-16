// -*- C++ -*-

// RBMSparse1DMatrixConnection.h
//
// Copyright (C) 2008 Jerome Louradour
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

// Authors: Jerome Louradour

/*! \file RBMSparse1DMatrixConnection.h */


#ifndef RBMSparse1DMatrixConnection_INC
#define RBMSparse1DMatrixConnection_INC

#include "RBMMatrixConnection.h"

namespace PLearn {
using namespace std;


/**
 * Stores and learns the parameters between two linear layers of an RBM.
 *
 */
class RBMSparse1DMatrixConnection: public RBMMatrixConnection
{
    typedef RBMMatrixConnection inherited;

public:
    //#####  Public Build Options  ############################################

    int filter_size;

    bool enforce_positive_weights;

    //#####  Learned Options  #################################################

    int step_size;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    RBMSparse1DMatrixConnection( real the_learning_rate=0 );

    // Your other public member functions go here
    virtual void accumulatePosStats( const Mat& down_values,
                                     const Mat& up_values );

    virtual void accumulateNegStats( const Mat& down_values,
                                     const Mat& up_values );

    //! Computes the vectors of activation of "length" units,
    //! starting from "start", and stores (or add) them into "activations".
    //! "start" indexes an up unit if "going_up", else a down unit.
    virtual void computeProducts(int start, int length,
                                 Mat& activations,
                                 bool accumulate=false ) const;


    //! given the input and the connection weights,
    //! compute the output (possibly resize it  appropriately)
    virtual void fprop(const Vec& input, const Mat& rbm_weights,
                       Vec& output) const;

    virtual void bpropUpdate(const Mat& inputs, const Mat& outputs,
                             Mat& input_gradients,
                             const Mat& output_gradients,
                             bool accumulate = false);

    virtual void bpropAccUpdate(const TVec<Mat*>& ports_value,
                                const TVec<Mat*>& ports_gradient);

    virtual void update( const Mat& pos_down_values,
                         const Mat& pos_up_values,
                         const Mat& neg_down_values,
                         const Mat& neg_up_values);

    //! reset the parameters to the state they would be BEFORE starting
    //! training.  Note that this method is necessarily called from
    //! build().
    virtual void forget();

    //! return the number of parameters
    virtual int nParameters() const;

    virtual Mat getWeights() const;

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(RBMSparse1DMatrixConnection);

    // Simply calls inherited::build() then build_()
    virtual void build();

protected:
    //#####  Protected Member Functions  ######################################
    //! Declares the class options.
    static void declareOptions(OptionList& ol);

    //! Declare the methods that are remote-callable
    static void declareMethods(RemoteMethodMap& rmm);

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    void build_();

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
    int filterStart(int idx) const;
    int filterSize(int idx) const;
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(RBMSparse1DMatrixConnection);

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
