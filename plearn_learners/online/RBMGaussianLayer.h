// -*- C++ -*-

// RBMGaussianLayer.h
//
// Copyright (C) 2006 Dan Popovici & Pascal Lamblin
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

// Authors: Dan Popovici & Pascal Lamblin

/*! \file RBMGaussianLayer.h */


#ifndef RBMGaussianLayer_INC
#define RBMGaussianLayer_INC

#include "RBMLayer.h"

namespace PLearn {
using namespace std;

/**
 * Layer in an RBM formed with Gaussian units
 *
 */
class RBMGaussianLayer: public RBMLayer
{
    typedef RBMLayer inherited;

public:
    //#####  Public Build Options  ############################################

    real min_quad_coeff;

    bool share_quad_coeff;

    //! Number of units when share_quad_coeff is False
    //! or 1 when share_quad_coeff is True
    int size_quad_coeff;

    //! Value for the usually learned standard deviation, if it should not be learned.
    //! This will fix the value of the quad coeffs to the appropriate value.
    //! If < 0, then this option is ignored.
    real fixed_std_deviation;

public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    RBMGaussianLayer( real the_learning_rate=0. );

    //! Constructor from the number of units in the multinomial
    RBMGaussianLayer( int the_size, real the_learning_rate=0. );

    //! Constructor from the number of units in the multinomial,
    //! with an aditional option
    RBMGaussianLayer( int the_size, real the_learning_rate=0.,
                      bool do_share_quad_coeff=false );

    //! compute a sample, and update the sample field
    virtual void generateSample() ;

    virtual void generateSamples() ;

    //! compute the expectation
    virtual void computeExpectation() ;

    //! compute the batch expectation
    virtual void computeExpectations() ;

    //! compute the standard deviation
    virtual void computeStdDeviation() ;

    //! forward propagation
    virtual void fprop( const Vec& input, Vec& output ) const;

    //! back-propagates the output gradient to the input
    virtual void bpropUpdate(const Vec& input, const Vec& output,
                             Vec& input_gradient, const Vec& output_gradient,
                             bool accumulate=false);

    //! Back-propagate the output gradient to the input, and update parameters.
    virtual void bpropUpdate(const Mat& inputs, const Mat& outputs,
                             Mat& input_gradients,
                             const Mat& output_gradients,
                             bool accumulate = false)
    {
        PLASSERT_MSG(false, "Not implemented");
    }

    //! Accumulates positive phase statistics
    virtual void accumulatePosStats( const Vec& pos_values );

    //! Accumulates negative phase statistics
    virtual void accumulateNegStats( const Vec& neg_values );

    //! Update parameters according to accumulated statistics
    virtual void update();

    //! Update parameters according to one pair of vectors
    virtual void update( const Vec& pos_values, const Vec& neg_values );

    //! Batch version
    virtual void update( const Mat& pos_values, const Mat& neg_values );

    //! resets activations, sample, expectation and sigma fields
    virtual void reset();

    //! resets the statistics and counts
    virtual void clearStats();

    //! forgets everything
    virtual void forget();

    //! compute bias' unit_values + min_quad_coeff.^2' unit_values.^2
    virtual real energy(const Vec& unit_values) const;

    //! Computes -log(\sum_{possible values of h} exp(h' unit_activations))
    //! This quantity is used for computing the free energy of a sample x in
    //! the OTHER layer of an RBM, from which unit_activations was computed.
    virtual real freeEnergyContribution(const Vec& unit_activations) const;

    virtual int getConfigurationCount()
    {
        return INFINITE_CONFIGURATIONS;
    }

    //! Computes the negative log-likelihood of target given the
    //! internal activations of the layer
    virtual real fpropNLL(const Vec& target);
    virtual void fpropNLL(const Mat& targets, const Mat& costs_column);

    //! Computes the gradient of the negative log-likelihood of target
    //! with respect to the layer's bias, given the internal activations
    virtual void bpropNLL(const Vec& target, real nll, Vec& bias_gradient);
    virtual void bpropNLL(const Mat& targets, const Mat& costs_column,
                          Mat& bias_gradients);

    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    PLEARN_DECLARE_OBJECT(RBMGaussianLayer);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:
    //#####  Learned Options  #################################################
    //! Quadradic coefficients
    Vec quad_coeff;

    //#####  Not Options  #####################################################
    Vec quad_coeff_pos_stats;
    Vec quad_coeff_neg_stats;
    Vec quad_coeff_inc;

    //! Stores the standard deviations
    Vec sigma;
    bool sigma_is_up_to_date;

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
DECLARE_OBJECT_PTR(RBMGaussianLayer);

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
