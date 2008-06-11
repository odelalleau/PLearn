// -*- C++ -*-

// RBMConnection.h
//
// Copyright (C) 2006 Pascal Lamblin
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

/*! \file RBMConnection.h */


#ifndef RBMConnection_INC
#define RBMConnection_INC

#include <plearn/base/Object.h>
#include "OnlineLearningModule.h"

namespace PLearn {
using namespace std;

// forward declaration
class RBMLayer;

/**
 * Virtual class for the parameters between two layers of an RBM.
 *
 */
class RBMConnection: public OnlineLearningModule
{
    typedef OnlineLearningModule inherited;

public:
    //#####  Public Build Options  ############################################

    //! The learning rate, used both in update() and bpropUpdate() methods
    real learning_rate;

    //! Momentum for the gradient descent
    real momentum;

    //! The method used to initialize the weights:
    //!   - "uniform_linear" = a uniform law in [-1/d, 1/d]
    //!   - "uniform_sqrt"   = a uniform law in [-1/sqrt(d), 1/sqrt(d)]
    //!   - "zero"           = all weights are set to 0
    //! Where d = max( up_layer_size, down_layer_size )
    string initialization_method;

    //#####  Not Options  #####################################################

    //! Number of units in down layer.
    int down_size;

    //! Number of units in up layer.
    int up_size;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    RBMConnection(real the_learning_rate = 0, bool call_build_ = false);

    // Your other public member functions go here

    //! Sets the learning rate
    virtual void setLearningRate( real the_learning_rate );

    //! Sets the momentum
    virtual void setMomentum( real the_momentum );

    //! Sets 'input_vec' to 'input', and 'going_up' to false.
    //! Note that no data copy is made, so 'input' should not be modified
    //! afterwards.
    virtual void setAsUpInput( const Vec& input ) const;

    //! Set 'inputs_mat' to 'inputs', and 'going_up' to false.
    //! Note that no data copy is made, so 'inputs' should not be modified
    //! afterwards.
    virtual void setAsUpInputs( const Mat& inputs ) const;

    //! Sets 'input_vec' to 'input', and 'going_up' to true.
    //! Note that no data copy is made, so 'input' should not be modified
    //! afterwards.
    virtual void setAsDownInput( const Vec& input ) const;

    //! Set 'inputs_mat' to 'inputs', and 'going_up' to true.
    //! Note that no data copy is made, so 'inputs' should not be modified
    //! afterwards.
    virtual void setAsDownInputs( const Mat& inputs ) const;

    //! Accumulates positive phase statistics to *_pos_stats.
    virtual void accumulatePosStats( const Vec& down_values,
                                     const Vec& up_values ) = 0;

    virtual void accumulatePosStats( const Mat& down_values,
                                     const Mat& up_values ) = 0;

    //! Accumulates negative phase statistics to *_neg_stats.
    virtual void accumulateNegStats( const Vec& down_values,
                                     const Vec& up_values ) = 0;

    virtual void accumulateNegStats( const Mat& down_values,
                                     const Mat& up_values ) = 0;

    //! Updates parameters according to contrastive divergence gradient
    virtual void update() = 0;

    //! Updates parameters according to contrastive divergence gradient,
    //! not using the statistics but the explicit values passed
    virtual void update( const Vec& pos_down_values,
                         const Vec& pos_up_values,
                         const Vec& neg_down_values,
                         const Vec& neg_up_values);

    //! Updates parameters according to contrastive divergence gradient,
    //! not using the statistics but explicit matrix values.
    virtual void update( const Mat& pos_down_values,
                         const Mat& pos_up_values,
                         const Mat& neg_down_values,
                         const Mat& neg_up_values);

    // neg_stats <-- gibbs_chain_statistics_forgetting_factor * neg_stats
    //              +(1-gibbs_chain_statistics_forgetting_factor)
    //               * gibbs_neg_up_values'*gibbs_neg_down_values
    // delta w = -lrate * ( pos_up_values'*pos_down_values
    //                   - ( background_gibbs_update_ratio*neg_stats
    //                      +(1-background_gibbs_update_ratio)
    //                       * cd_neg_up_values'*cd_neg_down_values))
    virtual void updateCDandGibbs( const Mat& pos_down_values,
                                   const Mat& pos_up_values,
                                   const Mat& cd_neg_down_values,
                                   const Mat& cd_neg_up_values,
                                   const Mat& gibbs_neg_down_values,
                                   const Mat& gibbs_neg_up_values,
                                   real background_gibbs_update_ratio);

    // neg_stats <-- gibbs_chain_statistics_forgetting_factor * neg_stats
    //              +(1-gibbs_chain_statistics_forgetting_factor)
    //               * gibbs_neg_up_values'*gibbs_neg_down_values
    // delta w = -lrate * ( pos_up_values'*pos_down_values - neg_stats )
    virtual void updateGibbs( const Mat& pos_down_values,
                              const Mat& pos_up_values,
                              const Mat& gibbs_neg_down_values,
                              const Mat& gibbs_neg_up_values);

    //! Clear all information accumulated during stats
    virtual void clearStats() = 0;

    //! Computes the vectors of activation of "length" units,
    //! starting from "start", and stores (or add) them into "activations".
    //! "start" indexes an up unit if "going_up", else a down unit.
    virtual void computeProduct( int start, int length,
                                 const Vec& activations,
                                 bool accumulate=false ) const = 0;

    //! Same as 'computeProduct' but for mini-batches.
    virtual void computeProducts(int start, int length,
                                 Mat& activations,
                                 bool accumulate=false ) const = 0;

    //! given the input, compute the output (possibly resize it  appropriately)
    virtual void fprop(const Vec& input, Vec& output) const;
    virtual void fprop(const Mat& inputs, Mat& outputs);

    //! provide the internal weight values (not a copy)
    virtual void getAllWeights(Mat& rbm_weights) const;

    //! set the internal weight values to rbm_weights (not a copy)
    virtual void setAllWeights(const Mat& rbm_weights);

    //! back-propagates the output gradient to the input and the weights
    //! (the weights are not updated)
    virtual void petiteCulotteOlivierUpdate(
        const Vec& input, const Mat& rbm_weights,
        const Vec& output,
        Vec& input_gradient, Mat& rbm_weights_gradient,
        const Vec& output_gradient,
        bool accumulate = false);

    //! Computes the contrastive divergence gradient with respect to the weights
    //! It should be noted that bpropCD does not call clearstats().
    virtual void petiteCulotteOlivierCD(Mat& weights_gradient,
                                        bool accumulate = false);

    //! Computes the contrastive divergence gradient with respect to the weights
    //! given the positive and negative phase values.
    virtual void petiteCulotteOlivierCD(const Vec& pos_down_values,
                                        const Vec& pos_up_values,
                                        const Vec& neg_down_values,
                                        const Vec& neg_up_values,
                                        Mat& weights_gradient,
                                        bool accumulate = false);

    //! Return the list of ports in the module.
    virtual const TVec<string>& getPorts();

    //! Return the size of all ports
    virtual const TMat<int>& getPortSizes();

    //! return the number of parameters
    virtual int nParameters() const = 0;

    //! Make the parameters data be sub-vectors of the given global_parameters.
    //! The argument should have size >= nParameters. The result is a Vec
    //! that starts just after this object's parameters end, i.e.
    //!    result = global_parameters.subVec(nParameters(),global_parameters.size()-nParameters());
    //! This allows to easily chain calls of this method on multiple RBMConnection.
    virtual Vec makeParametersPointHere(const Vec& global_parameters) = 0;

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_ABSTRACT_OBJECT(RBMConnection);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //#####  Not Options  #####################################################

    //! Pointer to current input vector.
    mutable Vec input_vec;

    //! Pointer to current inputs matrix.
    mutable Mat inputs_mat;

    //! Tells if input_vec comes from down (true) or up (false)
    mutable bool going_up;

    //! Number of examples accumulated in *_pos_stats
    int pos_count;

    //! Number of examples accumulated in *_neg_stats
    int neg_count;

    //! Port names
    TVec<string> ports;

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
DECLARE_OBJECT_PTR(RBMConnection);

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
