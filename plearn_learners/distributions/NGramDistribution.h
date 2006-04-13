// -*- C++ -*-

// NGramDistribution.h
//
// Copyright (C) 2004 Hugo Larochelle
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

// Authors: Hugo Larochelle

/*! \file NGramDistribution.h */


#ifndef NGramDistribution_INC
#define NGramDistribution_INC

#define THIS_PRECISION 0.0000001
#define EM_PRECISION 0.0001

#include <plearn_learners/distributions/PDistribution.h>
#include <plearn_learners/distributions/NGramTree.h>
#include <plearn/base/ms_hash_wrapper.h>

namespace PLearn {
using namespace std;

//! This class implements an ngram distribution for symbol sequence modeling
class NGramDistribution: public PDistribution
{

private:

    typedef PDistribution inherited;

protected:

    // *********************
    // * protected options *
    // *********************

    int voc_size;

public:

    // ************************
    // * public build options *
    // ************************

    //! Replace nan values with -1
    bool nan_replace;

    //! N in NGram
    int n;

    //! Additive constant for Add-delta smoothing
    real additive_constant;

    //! Discount constant for absolute discounting smoothing
    real discount_constant;

    //! Proportion of the training set used for validation
    real validation_proportion;

    //! Smoothing parameter
    string smoothing;

    //! Lambda estimation technique
    string lambda_estimation;

    //! Lambdas for Jelinek-Mercer smoothing
    Vec lambdas;

    //! NGram tree
    PP<NGramTree> tree;


    // ****************
    // * Constructors *
    // ****************

    //! Default constructor.
    NGramDistribution();

    // *************************
    // * PDistribution methods *
    // *************************

private:

    //! This does the actual building.
    void build_();

    //! Takes a row of a VMat and gives the ngram associated
    void getNGrams(Vec row, TVec<int>& ngram) const;

protected:

    //! Declare this class' options.
    static void declareOptions(OptionList& ol);

public:

    // ************************
    // **** Object methods ****
    // ************************

    //! Simply call inherited::build() then build_().
    virtual void build();

    //! Transform a shallow copy into a deep copy.
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

    // Declare other standard object methods.
    PLEARN_DECLARE_OBJECT(NGramDistribution);

    // *******************************
    // **** PDistribution methods ****
    // *******************************

    //! Return log of probability density log(p(y | x)).
    virtual real log_density(const Vec& x) const;

    //! Return survival function: P(Y>y | x).
    virtual real survival_fn(const Vec& y) const;

    //! Return cdf: P(Y<y | x).
    virtual real cdf(const Vec& y) const;

    //! Return E[Y | x].
    virtual void expectation(Vec& mu) const;

    //! Return Var[Y | x].
    virtual void variance(Mat& cov) const;

    //! Return a pseudo-random sample generated from the distribution.
    virtual void generate(Vec& y) const;

    //! Return probability density p(y | x)
    virtual real density(const Vec& y) const;

    // **************************
    // **** PLearner methods ****
    // **************************

    //! (Re-)initializes the PDistribution in its fresh state (that state may depend on the 'seed' option).
    //! And sets 'stage' back to 0 (this is the stage of a fresh learner!).
    virtual void forget();

    //! For this distribution, won't do anything. Just implemented to work with PTester
    virtual void train();

};

// Declare a few other classes and functions related to this class.
DECLARE_OBJECT_PTR(NGramDistribution);

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
