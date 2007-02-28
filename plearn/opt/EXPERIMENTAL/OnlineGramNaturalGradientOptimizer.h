// -*- C++ -*-

// OnlineGramNaturalGradientOptimizer.h
//
// Copyright (C) 2007 Pierre-Antoine Manzagol
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

// Authors: Pierre-Antoine Manzagol

/*! \file OnlineGramNaturalGradientOptimizer.h */


#ifndef ONLINEGRAMNATURALGRADIENTOPTIMIZER_INC
#define ONLINEGRAMNATURALGRADIENTOPTIMIZER_INC

#include <plearn/opt/Optimizer.h>

#include <ctime>

namespace PLearn {
using namespace std;

/**
 * Implements an online natural gradient, based on keeping an estimate
 * of the gradients' covariance C through its main eigen vectors and values
 * which are updated through those of the gram matrix. This is n_eigen^2
 * instead of n_parameter^2.
 *
 * @todo 
 * @deprecated 
 */
class OnlineGramNaturalGradientOptimizer : public Optimizer
{
    typedef Optimizer inherited;
      
public:
    //#####  Public Build Options  ############################################

    real learning_rate;
    real gamma;
    real reg;
    int opt_batch_size;
    int n_eigen;


public:
    //#####  Public Member Functions  #########################################    

    OnlineGramNaturalGradientOptimizer();

    void gramEigenNaturalGradient();

    //#####  Optimizer Member Functions  #######################################

    virtual bool optimizeN(VecStatsCollector& stats_coll);

    //#####  PLearn::Object Protocol  #########################################
    PLEARN_DECLARE_OBJECT(OnlineGramNaturalGradientOptimizer);

    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    virtual void build()
    {
        inherited::build();
        build_();
    }

protected:
    //#####  Protected Options  ###############################################

    // ### Declare protected option fields (such as learned parameters) here


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

    int n_optimizeN_calls;

    // The batch index
    int bi;
    int n_eigen_cur;
    int n_eigen_old;

    // Holds the gradients and their mean
    Mat gradients;
    Vec mu;

    real total_variance, variance_percentage;

    // the gram matrix - G = UDU' or in our case U'DU
    Mat gram;
    Mat U;
    Vec D;

    // The covariance matrix is C = VDV' (with the eigen vectors in V's columns)
    // or in our case cov_eigen_vectors' diag(cov_eigen_values) cov_eigen_vectors
    Mat cov_eigen_vec;
    Mat old_cov_eigen_vec;
    Mat swap;
    Vec cov_eigen_val;

    Mat cov_norm_eigen_vec;

    // 
    Vec dot_prod;
    Vec scaled_dot_prod;

    // The natural gradient, ie C^{-1} mu
    Vec naturalg;

    clock_t t0, t1, t2, t3;

};

DECLARE_OBJECT_PTR(OnlineGramNaturalGradientOptimizer);


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
